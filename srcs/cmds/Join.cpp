#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

/** @note not consider the case of local channel start with '&'. A regular channel start with '#', not including forbidden characters and maximum length is CHANNELLEN */
bool	Client::isValidChanName(std::string name)
{
	if (name.empty() || name.length() > CHANNELLEN)
	{
		this->getServer().sendClientErr(ERR_BADCHANNAME, *this, nullptr, {name});
		return false;
	}

	std::regex chanNameRegex("^#[^ \\x07,]+$");
	if (!std::regex_match(name, chanNameRegex))
	{
		this->getServer().sendClientErr(ERR_BADCHANNAME, *this, nullptr, {name});
		return false;
	}
	return true;
}

/** @brief validate the channel name and create a map of <channel name - key> 
 *  @note The insert() operation adds a new key-value pair to the map only 
			if the key is not already present.
			If the key exists, insert() does not update the value and the map unchanged.
*/
bool Server::mappingChannelKey(std::vector<std::string> tokens, Client& client, std::map<std::string, std::string>& channelKeyMap)
{
	if (tokens.empty())
	{
		
		client.getServer().sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"JOIN"});
		return false;
	}
	
	std::vector<std::string> channelList;
	std::vector<std::string> keyList;
	if (tokens[0].find(",") != std::string::npos)
		channelList = utils::ft_splitString(tokens[0], ',');
	else
		channelList.push_back(tokens[0]);
	if (tokens.size() > 1) //has key
	{
		if (tokens[1].find(",") != std::string::npos)
			keyList = utils::ft_splitString(tokens[1], ',');
		else
			keyList.push_back(tokens[1]);
	}
	else
	{
		for (size_t i = 0; i < channelList.size(); ++i)
			keyList.push_back("");
	}
	// add to map
	for (size_t i = 0; i < channelList.size(); ++i)
	{
		if (client.isValidChanName(channelList[i]))
		{
			channelList[i].erase(0, 1); // remove the hash
			if (i < keyList.size())
				channelKeyMap.insert({channelList[i], keyList[i]});
			else
				channelKeyMap.insert({channelList[i], ""});
		}
		else
			continue;
	}
	return true;
}

/** @brief check whether client can join this channel: channel key comparison, channel limit per client, client limit per channel and invitation requirement */
channelMsg Channel::canClientJoinChannel( Client& client, std::string clientKey)
{
	Server& server = client.getServer();

	if (this->isClientOnChannel(client))
		return ALREADY_ON_CHAN;

	if (client.getJoinedChannels().size() >= CHANLIMIT)
	{
		server.sendClientErr(ERR_TOOMANYCHANNELS, client, this, {} );
		return NO_MSG;
	}

	if (!this->getChanKey().empty() && this->getChanKey() != clientKey)
	{
		server.sendClientErr(ERR_BADCHANNELKEY,client, this, {} );
		return NO_MSG;
	}

	// check again the stored limit for channel, if internal corruption then just let the client joins, ignore +l
	std::string	clientLimit;
	if (this->isModeActive(L_MODE, clientLimit))
	{
		size_t limit = std::stoi(clientLimit);
		if (this->_userList.size() >= limit && !this->hasInvitedClient(&client))
		{
			server.sendClientErr(ERR_CHANNELISFULL, client, this, {});
			return NO_MSG;
		}
	}

	if (this->isModeActive(I_MODE))
	{
		if (!this->hasInvitedClient(&client))
		{
			server.sendClientErr(ERR_INVITEONLYCHAN, client, this, {});
			return NO_MSG;
		}
	}
	
	return JOIN_OK;
}

/** @brief return a pointer to a new channel with non-existing channel name */
Channel* Server::createChannel(std::string chanName)
{
	this->getChannelInfo().push_back(new Channel(chanName));
	Channel* newChannel = this->getChannelInfo().back();
	newChannel->setChannelCreationTimestamp();
	newChannel->addMode(T_MODE, "");
	return newChannel;
}


/** @brief regular channel: This channel is what’s referred to as a normal channel. Clients can join this channel, and the first client who joins a normal channel is made a channel operator, along with the appropriate channel membership prefix. A new channel created has no pre-set mode. */
void Server::handleJoin(Client& client, std::vector<std::string> tokens)
{
	std::map<std::string, std::string>		channelKeyMap;

	if (tokens.empty())
	{
		this->sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"JOIN"});
		return;
	}

	if (!mappingChannelKey(tokens, client, channelKeyMap))
		return;

	for (auto chan : channelKeyMap)
	{
		std::string channelName = chan.first;
		std::string clientKey = chan.second;

		if (channelName.size() > CHANNELLEN)
		{
			this->sendClientErr(ERR_BADCHANNAME, client, nullptr, {channelName});
			continue;
		}
		// client leave all channels they are currently connected to
		if (channelName == "0")
		{
			std::vector<Channel*> joined = client.getJoinedChannels();
			for (auto chan : joined)
			{
				std::vector<std::string> v{chan->getChannelName()};
				partChannel(client, v);
			}
			return;
		}

		Channel* channelPtr = this->findChannel(channelName);
		if (!channelPtr)
			channelPtr = this->createChannel(channelName);

		channelMsg result = channelPtr->canClientJoinChannel(client, clientKey);
		if (result == JOIN_OK)
		{
			client.addJoinedChannel(channelPtr);
			channelPtr->addUser(&client);
			this->sendJoinSuccessMsg(client, *channelPtr);			
		}
		else if (result == ALREADY_ON_CHAN)
		{
			this->sendTopic(client, *channelPtr);
			this->sendNameReply(client, *channelPtr);
		}
	}
	return;
}
