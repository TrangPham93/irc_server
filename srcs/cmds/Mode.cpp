#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "macro.hpp"

static bool modeNeedParams(char mode, bool addMode)
{
	switch (mode)
	{
	case I_MODE:
		return false;
	
	case T_MODE:
		return false;
	
	case L_MODE:
		if (addMode)
			return true;
		else
			return false;

	case O_MODE:
		return true;

	case K_MODE:
		return true;

	default:
		return false;
	}
}

bool Channel::parsingMode(std::vector<std::string> tokens, std::vector<ModeInfo>& parsedModeVec)
{
	std::string modeStr;
	bool		addMode = true;
	while (!tokens.empty())
	{
		modeStr = tokens[0];
		tokens.erase(tokens.begin());

		for (char mode : modeStr)
		{
			if (mode == '+') {addMode = true; continue;}	
			if (mode == '-') {addMode = false; continue;}

			ModeInfo modeIn;
			modeIn.add = addMode;
			modeIn.mode = mode;

			if (modeNeedParams(mode, addMode))
			{
				if (tokens.empty()) // mode needs param but not given
					modeIn.params = "";
				else
				{
					modeIn.params = tokens.front();
					tokens.erase(tokens.begin());
				}
			}
			parsedModeVec.push_back(modeIn);
		}
	}

	return true;
}

/** @brief  If <modestring> is given, the user MUST have appropriate channel privileges */
bool Channel::validateModeInstruction(Client& client, std::vector<ModeInfo> parsedModeVec)
{
	Server& server = client.getServer();
	if (!this->isChanop(client.getNick()))
	{
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, client, this, {});
		return false;
	}

	for (auto m : parsedModeVec)
	{
		if (_modeHandlers.count(m.mode) == 0)
		{
			std::string unknownMode(1, m.mode);
			server.sendClientErr(ERR_UNKNOWNMODE, client, this, 
					{unknownMode});
			return false;
		}
		if (modeNeedParams(m.mode, m.add) && m.params.empty())
		{
			if (m.mode == K_MODE && !m.add)
				continue;
			std::string cmd = "MODE ";
			cmd += m.add ? "+" : "-";
			cmd += m.mode;
			server.sendClientErr(ERR_NEEDMOREPARAMS, client, this, {cmd});
			return false;
		}
	}
	return true;
}

static void combineExecutedMode(std::string& executedMode, char& mode, bool addMode)
{
	bool activeAddMode = true;

	if (executedMode.empty())
	{
		if (addMode)
			executedMode += "+";
		else
			executedMode += "-";
		executedMode += mode;
	}
	else
	{
		for (size_t i  = 0; i < executedMode.length(); i++)
		{
			if (executedMode[i] == '+')
				activeAddMode = true;
			else if (executedMode[i] == '-')
				activeAddMode = false;
		}
		if (addMode == activeAddMode)
			executedMode += mode;
		else
		{
			executedMode += addMode ? "+" : "-";
			executedMode += mode;
		}
	}
}

/** @brief Servers MAY choose to hide sensitive information when sending the mode changes like key MODE args. Use asterisk to hide that args*/
static void restrictRemoveKeyMode(std::string& executedMode, std::string& executedArgs)
{
	bool activeAddMode;
	bool hasKeyMode = false;

	if (executedMode.empty())
		return;
	for (size_t i  = 0; i < executedMode.length(); i++)
	{
		if (executedMode[i] == '+')
			activeAddMode = true;
		else if (executedMode[i] == '-')
			activeAddMode = false;
		else if (executedMode[i] == 'k')
		{
			hasKeyMode = true;
			break;
		}
	}
	if (activeAddMode == false && hasKeyMode)
		executedArgs = "*";
}

void Server::sendSetModeMsg(Client& client, Channel& channel, std::string& executedMode, std::string& executedArgs)
{
	std::string modeStr;

	if (!executedMode.empty())
		modeStr += executedMode;
	if (!executedArgs.empty())
		modeStr += " " + executedArgs;

	std::string	modeMsg = client.makeUser() + " MODE #" + 
			channel.getChannelName() + " " + modeStr + " \r\n";
	this->broadcastChannelMsg(modeMsg, channel);
}

void Channel::executeModeCmd(Client& client, std::vector<ModeInfo>& parsedModeVec)
{
	channelMsg	msgEnum;
	std::string	executedMode, executedArgs;
	Server& server = client.getServer();

	for (auto& m : parsedModeVec)
	{
		if (m.mode == O_MODE)
		{
			Client* user = server.findClient(m.params);
			if (!user)
			{
				server.sendClientErr(ERR_NOSUCHNICK, client, this, {m.params});
				break;
			}
			
			if (!this->isClientOnChannel(*user))
			{
				server.sendClientErr(ERR_USERNOTINCHANNEL, client, this, {m.params});
				break;
			}
		}
		if (m.mode == L_MODE && m.add)
		{
			int limit;
			try
			{
				limit = std::stoi(m.params);
			}
			catch(const std::exception& e)
			{
				break;
			}
			if (limit <= 0)
				break;
		}
		msgEnum = (this->*(_modeHandlers[m.mode]))(m.add, m.params);
		if (msgEnum == SET_MODE_OK)
		{
			combineExecutedMode(executedMode, m.mode, m.add);
			executedArgs += (m.params.empty() ? "" : m.params + " ");
		}
	}
	restrictRemoveKeyMode(executedMode, executedArgs);
	if (!executedMode.empty())
	{
		client.getServer().sendSetModeMsg(client, *this, executedMode, executedArgs);
	}
}

/** @brief mode applied: itkol => only handle mode for channel, not for user.
*/
void Server::handleMode(Client& client, std::vector<std::string> tokens)
{
	Channel*	channelPtr = nullptr;
	if (tokens.empty())
	{
		sendClientErr(ERR_NEEDMOREPARAMS, client, channelPtr, {"MODE"});
		return ;
	}

	std::string	channelName = tokens[0]; 
	//ignore mode for user in channel
	if (channelName.find("#") == std::string::npos && this->findClient(channelName))
		return;
	if (channelName.find("#") != std::string::npos)	
		channelName = utils::extractChannelName(channelName);
	channelPtr = this->findChannel(channelName);
	if (!channelPtr) 
	{
		this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, {channelName});
		return ; 
	}

	tokens.erase(tokens.begin(), tokens.begin() + 1);

	//  If <modestring> is not given, inform currently-set modes of a channel. 
	if (tokens.empty())
	{
		sendClientErr(RPL_CHANNELMODEIS, client, channelPtr, 
			{channelPtr->getMode()[0], channelPtr->getMode()[1]});
		sendClientErr(RPL_CREATIONTIME, client, channelPtr, {});
		return ;
	}

	std::vector<ModeInfo> parsedModeVec;
	if (!channelPtr->parsingMode(tokens, parsedModeVec))
		return ;

	if (!channelPtr->validateModeInstruction(client, parsedModeVec))
		return ;

	channelPtr->executeModeCmd(client, parsedModeVec);
	
}
