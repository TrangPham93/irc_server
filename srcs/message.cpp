#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

void	Server::reply(Client &c)
{
	if (c.getOutput().empty())
		return ;
	if (send(c.getClientFd(), c.getOutput().c_str(), c.getOutput().size(), 0) < 0)
	{
		std::cerr << "Error: failed to send" << std::endl;
		return;
	}
	logMessages(c.getOutput(), getServerfd());
	c.clearOutput();

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = c.getClientFd();
	epoll_ctl(_epollFd, EPOLL_CTL_MOD, c.getClientFd(), &ev);
}

/** @brief send message to the requesting member */
void	Server::sendMsg(Client& client, std::string& msg)
{
	client.appendToOutput(msg);

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = client.getClientFd();
	epoll_ctl(_epollFd, EPOLL_CTL_MOD, client.getClientFd(), &ev);
}

/**
 * @brief send message to all member on channels and the sender itself
 */
void Server::broadcastChannelMsg(std::string& msg, Channel& channel)
{
	for (Client* user : channel.getUserList())
		this->sendMsg(*user, msg);
}

void Server::logMessages(std::string msg, int fd)
{
	if (fd == 2)
		std::cerr << C_R << "SERV >> ";
	else if (fd <= 4)
		std::cout << C_G << "SERV >> ";
	else
		std::cout << C_B << msg;
	std::cout << msg << C_RST;
}

void Server::logMessages(std::string command, std::vector<std::string> msg, int fd)
{
	std::cout << C_B << "SERV << " << "fd " << fd << " | ";
	std::cout << command << " -> ";
	for (auto it : msg)
		std:: cout << it << " ";
	std::cout << C_RST << std::endl;
}

/**
 * @brief send message to all member on channels EXCEPT the sender itself
 */
void Server::broadcastChannelMsg(std::string& msg, Channel& channel, Client& client)
{
	for (Client* user : channel.getUserList())
	{
		if (&client != user)
			this->sendMsg(*user, msg);
	}
}

/**
 * @brief send message to all members on shared channels (but only once) and also the sender itself IF sender is marked as true
 */
void Server::broadcastUsersMsg(std::string& msg, Client& client, bool sender)
{
	std::vector<int> uniqueClients;
	for (Channel* channel : client.getJoinedChannels())
	{
		for (Client* user : channel->getUserList())
		{
			if (find(uniqueClients.begin(), uniqueClients.end(), user->getClientFd()) == uniqueClients.end())
			{
				uniqueClients.push_back(user->getClientFd());
				if (sender == true || (sender == false && client.getClientFd() != user->getClientFd()))
					sendMsg(*user, msg);
			}
		}
	}
	if (sender == true && client.getJoinedChannels().size() == 0)
		sendMsg(client, msg);
}

/** 
 * @brief if no topic set when client joins the channel, do not send back the topic.
 * otherwise, send the topic RPL_TOPIC & optionally RPL_TOPICWHOTIME, list of users 
 * currently joined the channel, including the current client( multiple RPL_NAMREPLY 
 * and 1 RPL_ENDOFNAMES) and the channel creation timestamp. 
 */
void	Server::sendJoinSuccessMsg( Client& client, Channel& channel)
{
	std::string	user = client.makeUser();

	std::string joinMsg = user + " JOIN #" + channel.getChannelName() + " \r\n";
	this->sendMsg(client, joinMsg);
	if (!channel.getTopic().empty())
	{
		this->sendClientErr(RPL_TOPIC, client, &channel, {});
		this->sendClientErr(RPL_TOPICWHOTIME, client, &channel, {});
	}
	this->sendNameReply(client, channel);
	this->broadcastChannelMsg(joinMsg, channel, client);
}

/** @brief send topic or no topic */
void	Server::sendTopic(Client& client, Channel& channel)
{
	std::string	server = this->getServerName(),
				nick = client.getNick(),
				chanName = channel.getChannelName();

	if (!channel.getTopic().empty())
	{
		this->sendClientErr(RPL_TOPIC, client, &channel, {});
		this->sendClientErr(RPL_TOPICWHOTIME, client, &channel, {});
	}
	else
		this->sendClientErr(RPL_NOTOPIC, client, &channel, {});
}

/** @brief send list of users in channel */
void	Server::sendNameReply(Client& client, Channel& channel)
{
	std::string	server = this->getServerName(),
				nick = client.getNick(),
				chanName = channel.getChannelName();

	this->sendClientErr(RPL_NAMREPLY, client, &channel, {});
	this->sendClientErr(RPL_ENDOFNAMES, client, &channel, {});
}

void Server::sendPartMsg(Client& client, std::vector<std::string>& params, Channel& channel)
{
	//check if a reason for kicking exists
	std::string reason;
	if (params.size() > 0) {
		for (size_t i = 1; i < params.size(); ++i) {
			reason += params[i];
			if (i + 1 != params.size())
				reason += " ";
		}
	}
	else
		reason = "";
	
	std::string	user = client.makeUser();

	std::string msg = user + " PART #" + channel.getChannelName() + " " + reason + "\r\n";
	broadcastChannelMsg(msg, channel);
}

void Server::sendKickMsg(std::string oper, std::string client, std::vector<std::string>& params, Channel& channel)
{
	//check if a reason for kicking exists
	std::string reason;
	if (params.size() > 2 && params[2].length() > 1) {
		for (size_t i = 2; i < params.size(); ++i) {
			reason += params[i];
			if (i + 1 != params.size())
				reason += " ";
		}
	}
	else
		reason = oper;
	std::string msg =	":" + oper + "!" + oper + "@localhost"
						+ " KICK " + "#" + channel.getChannelName()
						+ " " + client + " " + reason + "\r\n";
	broadcastChannelMsg(msg, channel);
}

void Server::sendWelcomeMsg(Client& client)
{
	std::string message = RPL_WELCOME(_name, client.getNick());
	sendMsg(client, message);
	message = RPL_YOURHOST(_name, client.getNick(), "1.1");
	sendMsg(client, message);
	message = RPL_CREATED(_name, client.getNick(), "today");
	sendMsg(client, message);
	message = RPL_MYINFO(_name, client.getNick(), "1.1", "o", "itkol");
	sendMsg(client, message);

	std::vector<std::string> info = 
	{
		"LINELEN=" + std::to_string(MSG_SIZE),
		"USERLEN=" + std::to_string(USERLEN),
		"NICKLEN=" + std::to_string(NICKLEN),
		"CHANLIMIT=" + std::to_string(CHANLIMIT),
		"CHANNELLEN=" + std::to_string(CHANNELLEN),
		"CHANMODES=" + std::string(CHANMODES),
		"CASEMAPPING=" + std::string(CASEMAPPING)
	};
	std::string infoPack;
	for (size_t i = 0; i < info.size(); i++)
		infoPack = infoPack + info[i] + " ";
	message = RPL_ISUPPORT(_name, client.getNick(), infoPack);
	sendMsg(client, message);
	message =
	":" + getServerName() + " 375 " + client.getNick() + " :- " + getServerName() + " Message of the day -\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + ".----------------.  .----------------.  .----------------.  .----------------.  .----------------.  .----------------. \r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| |  _________   | || |  _________   | || |              | || |     _____    | || |  _______     | || |     ______   | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| | |_   ___  |  | || | |  _   _  |  | || |              | || |    |_   _|   | || | |_   __ \\    | || |   .' ___  |  | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| |   | |_  \\_|  | || | |_/ | | \\_|  | || |              | || |      | |     | || |   | |__) |   | || |  / .'   \\_|  | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| |   |  _|      | || |     | |      | || |              | || |      | |     | || |   |  __ /    | || |  | |         | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| |  _| |_       | || |    _| |_     | || |              | || |     _| |_    | || |  _| |  \\ \\_  | || |  \\ `.___.'\\  | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| | |_____|      | || |   |_____|    | || |   _______    | || |    |_____|   | || | |____| |___| | || |   `._____.'  | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| |              | || |              | || |  |_______|   | || |              | || |              | || |              | |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + "| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :" + " '----------------'  '----------------'  '----------------'  '----------------'  '----------------'  '----------------'\r\n"
	":" + getServerName() + " 372 " + client.getNick() + " :Created by Karoliina Hiidenheimo, Trang Pham and Anton Kiiski.\r\n"
	":" + getServerName() + " 376 " + client.getNick() + " :End of /MOTD command.\r\n";
	sendMsg(client, message);

	std::cout << "User set: " << client.getUserName() << std::endl;
	std::cout << "Real name set: " << client.getRealName() << std::endl;
	std::cout << "Host set: " << client.getHostName() << std::endl;
	std::cout << "Nick set: " << client.getNick() << std::endl;
	std::cout << "Server set: " << getServerName() << std::endl;
}

void Server::sendClientErr(int num, Client& client, Channel* channel, std::vector<std::string> otherArgs)
{
	std::string server = this->getServerName(),
				nick = client.getNick(),
				chanName, msg, arg;
	if (channel)
		chanName = "#" + channel->getChannelName();
	
	switch (num)
	{
	case ERR_BADCHANNELKEY:
		msg = makeNumericReply(server, num, nick, {chanName}, "Cannot join channel (+k)");
		break;

	case ERR_TOOMANYCHANNELS:
		msg = makeNumericReply(server, num, nick, {chanName}, "You have joined too many channels");
		break;

	case ERR_UNKNOWNMODE:
	{
		if (otherArgs.size() == 1) 
		{
			arg = otherArgs[0]; 
			msg = makeNumericReply(server, num, nick, {arg}, "is unknown mode char to me");
		};
		break;
	}

	case ERR_CHANNELISFULL:
		msg = makeNumericReply(server, num, nick, {chanName}, "Cannot join channel (+l)");
		break;

	case ERR_INVITEONLYCHAN:
		msg = makeNumericReply(server, num, nick, {chanName}, "Cannot join channel (+i)");
		break;
	
	case ERR_NOTONCHANNEL:
		msg = makeNumericReply(server, num, nick, {chanName}, "You're not on that channel");
		break;
	
	case ERR_CHANOPRIVSNEEDED:
		msg = makeNumericReply(server, num,	nick, {chanName}, "You're not channel operator");
		break;

	case ERR_USERNOTINCHANNEL:
	{
		if (otherArgs.size() == 1)
		{
			arg = otherArgs[0]; 
			msg = makeNumericReply(server, num, nick, {arg, chanName}, "They aren't on that channel");
		};
		break ;
	}

	case ERR_NOSUCHNICK:
	{
		if (otherArgs.size() == 1) 
		{
			arg = otherArgs[0]; 
			msg = makeNumericReply(server, num, nick, {arg}, "No such nick/channel");
		};
		break ;
	}

	case ERR_NOSUCHCHANNEL:
	{
		if (otherArgs.size() == 1) 
		{
			arg = otherArgs[0]; 
			msg = makeNumericReply(server, num,	nick, {arg}, "No such channel");
		};
		break;
	}

	case ERR_USERONCHANNEL:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = makeNumericReply(server, num,	nick, {arg, chanName}, "is already on channel");
		};
		break;
	}

	case ERR_CANNOTSENDTOCHAN:
		msg = makeNumericReply(server, num,	nick, {chanName}, "Cannot send to channel");
		break;
	
	case ERR_NORECIPIENT:
		msg = makeNumericReply(server, num,	nick, {}, "No recipient given");
		break;
	
	case ERR_NOTEXTTOSEND:
		msg = makeNumericReply(server, num,	nick, {}, "No text to send");
		break;

	case ERR_BADCHANNAME:
	{
		if (otherArgs.size() == 1)
		{
			arg = otherArgs[0];
			msg = makeNumericReply(server, num,	nick, {arg}, "Illegal channel name");
		}
		break;
	}

	case ERR_INVALIDKEY:
	{
		msg = makeNumericReply(server, num,	nick, {chanName}, "Key is not well-formed");
		break;
	}

	case ERR_NEEDMOREPARAMS:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = makeNumericReply(server, num, nick, {arg}, "Not enough parameters");
		};
		break;	
	}
	
	case ERR_INPUTTOOLONG:
		msg = makeNumericReply(server, num, nick, {}, "Input line was too long");
		break;	

	case ERR_UNKNOWNCOMMAND:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = makeNumericReply(server, num, nick, {arg}, "Unknown command");
		};
		break;	
	}

	//RPL	
	case RPL_NOTOPIC:
		msg = makeNumericReply(server, num, nick, {chanName}, "No topic is set");
		break;
	
	case RPL_TOPIC:
		msg = makeNumericReply(server, num, nick, {chanName}, channel->getTopic());
		break;

	case RPL_TOPICWHOTIME:
		msg = makeNumericReply(server, num, nick, {chanName, 
			channel->getTopicSetter()->getNick(), 
			channel->getTopicTimestamp()}, "");
		break;

	case RPL_NAMREPLY:
		msg = makeNumericReply(server, num, nick,  {"=", chanName}, 
			channel->printUser());
		break;

	case RPL_ENDOFNAMES:
		msg = makeNumericReply(server, num, nick, {chanName}, "End of /NAMES list.");
		break;

	case RPL_INVITING:
	{
		if (otherArgs.size() == 1) 
		{ 
			arg = otherArgs[0];
			msg = makeNumericReply(server, num,	nick, {arg, chanName}, "");
		};
		break;
	}

	case RPL_CREATIONTIME:
	{
		msg = makeNumericReply(server, num, nick, {chanName, 
			channel->getChannelCreationTimestamp()}, "");
		break;
	}

	case RPL_CHANNELMODEIS:
	{
		if (otherArgs.size() == 2)
		{
			std::string modeStr = otherArgs[0];
			std::string modeArgs = otherArgs[1];
			msg = makeNumericReply(server, num, nick, {chanName, modeStr + " " + modeArgs}, "");
		}
		break;
	}
	
	default:
		break;
	}
	this->sendMsg(client, msg);
}
