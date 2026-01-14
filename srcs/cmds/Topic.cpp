#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

/** @brief remove the ':' from topic if exist.
 * If new topic is longer than MSG_SIZE allows (including channel name, etc.),
 * truncate the new topic, otherwise return as it is */
std::string Channel::truncateTopic(std::string tokens)
{
	std::string newTopic;
	if (tokens[0] == ':')
		newTopic = tokens.substr(1, tokens.length() - 1);
	else
		newTopic = tokens;

	int	topicLen = tokens.size();
	int maxTopic = MSG_SIZE - this->getChannelName().size() - 10;
	if (topicLen > maxTopic)
	{
		std::string truncateTopic = newTopic.substr(0, maxTopic);
		return truncateTopic;
	}
	return newTopic;
}

/** @brief if the t_mode is on, only chanop can set/remove topic.
 * If <topic> is an empty string, the topic for the channel will be cleared. 
 * If the <topic> param is provided but the same as the previous topic (ie. it is unchanged), servers MAY notify the author and/or other users anyway. */
bool Channel::setTopic(std::string tokens, Client& client)
{
	Server& server = client.getServer();
	
	if (std::to_string(tokens[0]) == ":") //there is only ":"
	{
		_topic = "";
		return true;
	}

	std::string newTopic = this->truncateTopic(tokens);
	if (this->isModeActive(T_MODE) && !client.isOps(*this))
	{
		server.sendClientErr(ERR_CHANOPRIVSNEEDED, client, this, {});
		return false; 
	}

	_topic = newTopic;	
	this->setTopicSetter(client);
	this->setTopicTimestamp();
	return true;
}

/** @brief send/set topic for channel, topic has TOPICLEN limit */
void Server::handleTopic(Client& client, std::vector<std::string> tokens)
{
	Channel*	channelPtr;
	std::string	channelName;

	if (tokens.empty())
	{
		this->sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"TOPIC"});
		return;
	}

	channelName = tokens[0];
	if (!client.isValidChanName(channelName))
		return;

	channelPtr = this->setActiveChannel(channelName);
	if (channelPtr == nullptr)
	{
		this->sendClientErr(ERR_NOSUCHCHANNEL, client, nullptr, {channelName});
		return;
	}
	if (!channelPtr->isClientOnChannel(client))
	{
		this->sendClientErr(ERR_NOTONCHANNEL, client, channelPtr, {});
		return;
	}

	tokens.erase(tokens.begin(), tokens.begin() + 1);
	if (tokens.empty() && channelPtr) // ask TOPIC
		this->sendTopic(client, *channelPtr);
	else // set TOPIC
	{
		std::string topicStr = utils::joinTokenVector(tokens);
		if (!channelPtr->setTopic(topicStr, client))
			return ;

		std::string	returnMsg = client.makeUser() + " TOPIC #" + 
			channelPtr->getChannelName() +" :" + channelPtr->getTopic() + "\r\n";
		this->broadcastChannelMsg(returnMsg, *channelPtr);
	}
}
