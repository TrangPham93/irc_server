#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "utils.hpp"

using namespace utils;

static bool isValidPrivmsg(Client& client, std::vector<std::string>& tokens) 
{
	if (tokens.empty())
	{
		client.getServer().sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"PRIVMSG"});
		return false;
	}
	if (tokens.size() == 1)
	{
		if (tokens[0].find(":") != std::string::npos)
		{
			client.getServer().sendClientErr(ERR_NORECIPIENT, client, nullptr, {});
			return false;
		}
		else
		{
			client.getServer().sendClientErr(ERR_NOTEXTTOSEND, client, nullptr, {});
			return false;
		}
	}

	if (!tokens[1].empty() && ft_trimString(tokens[1]) == ":" )
	{
		client.getServer().sendClientErr(ERR_NOTEXTTOSEND, client, nullptr, {});
		return false;
	}
	return true;
}

std::string utils::makePrivMsgToChan(std::string& token, Client& client, Channel& chan)
{
	return client.makeUser() + " PRIVMSG #" +  chan.getChannelName() + " " + token + "\r\n";
}

std::string utils::makePrivMsgToClient(std::string& token, Client& client,Client& partner)
{
	return client.makeUser() + " PRIVMSG " + partner.getNick() + " " + token + "\r\n";;
}

void Server::handlePrivmsg(Client& client, std::vector<std::string> tokens)
{
	if (!isValidPrivmsg(client, tokens))
		return;

	std::string	msg;
	std::vector<std::string> targets;

	if (tokens[0].find(",") != std::string::npos)
		targets = ft_splitString(tokens[0], ',');
	else
		targets.push_back(tokens[0]);

	for (auto target : targets)
	{
		if (target.find("#") != std::string::npos) // send privmsg to channel
		{
			Channel* chan = this->setActiveChannel(target);
			if (!chan)
			{
				this->sendClientErr(ERR_NOSUCHNICK, client, nullptr, {target});
				continue;
			}
			
			if (!chan->isClientOnChannel(client))
			{
				this->sendClientErr(ERR_CANNOTSENDTOCHAN, client, chan, {});
				continue;
			}
	
			msg = utils::makePrivMsgToChan(tokens[1], client, *chan);
			this->broadcastChannelMsg(msg, *chan, client);
	
		}
		else // send privmsg to client
		{
			Client* partner = this->findClient(target);
			if (!partner)
			{
				this->sendClientErr(ERR_NOSUCHNICK, client, nullptr, {target});
				continue;
			}
			msg = utils::makePrivMsgToClient(tokens[1], client, *partner);
			this->sendMsg(*partner, msg);
		}
	}
}
