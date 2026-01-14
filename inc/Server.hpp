#pragma once

#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <sys/types.h> 
#include <fcntl.h>
#include <sys/epoll.h>
#include <map>
#include <csignal>

#include "Enum.hpp"
#include "utils.hpp"
#include "macro.hpp"

#define MAX_EVENTS 200

#define C_G		"\033[0;32m\001"
#define C_B     "\033[38;5;110m\001"
#define C_R		"\033[0;31m\001"
#define C_RST	"\033[0m\002"

class Client;
class Channel;

class Server
{
private:
	int						_epollFd = -1;
	int						_serverFd = -1;
	std::string				_pass = "";
	std::string				_name = "";
	std::vector<Client*>	_clientInfo;
	std::vector<Channel*>	_channelInfo;
	int						_port = -1;
	struct sockaddr_in		_details;
	struct epoll_event		_events[MAX_EVENTS];

public:
	Server() = default;
	~Server();

//getters
	int 					getServerfd() const;
	int 					getEpollfd() const;
	std::string& 			getServerName();
	struct epoll_event* 	getEpollEvents();
	std::vector<Client*>& 	getClientInfo();
	std::vector<Channel*>& 	getChannelInfo();

	void		setupServerDetails(char *argv[]);
	void		setupSocket();
	void		setupEpoll();
	void		handleEvents();
	void		handleNewClient();
	void		handleCommand(Server &server, Client &client, 
					std::string command, std::vector<std::string> &tokens);
	void		attemptRegister(Client &client);
	void		handleDisconnects();
	void		disconnectClient(Client *client);
	void		removeChannel(Channel* chann);

	void 		receive(Client &c);
	void 		reply(Client &c);
	void 		parseMessage(Client &c, const std::string &line);
	void		logMessages(std::string command, std::vector<std::string> msg, int fd);
	void		logMessages(std::string msg, int fd);

	void		pass(Client &client, std::vector<std::string> tokens);
	void 		nick(Client &client, std::vector<std::string> tokens);
	void 		user(Client &client, std::vector<std::string> tokens);
	void 		ping(Client &client, std::vector<std::string> tokens);
	void 		handleJoin(Client& client, std::vector<std::string> tokens);
	void 		handleTopic(Client& client, std::vector<std::string> tokens);
	void 		handleMode(Client& client, std::vector<std::string> tokens);
	void 		handleInvite(Client& client, std::vector<std::string> tokens);
	void 		handlePrivmsg(Client& client, std::vector<std::string> tokens);
	void 		handleQuit(Client& client, std::vector<std::string>& tokens);
	void		kickClient(Client& client, std::vector<std::string>& params);
	void		partChannel(Client& client, std::vector<std::string>& params);

// other methods
	Channel*	findChannel(std::string newChannel);
	Client*		findClient(std::string nickName);
	Client*		findClientByFd(int fd);
	Channel*	setActiveChannel(std::string buffer);
	Channel*	createChannel(std::string chanName);
	bool 		mappingChannelKey(std::vector<std::string> tokens, 
					Client& client, std::map<std::string, 
					std::string>& channelKeyMap);

// sending messages
	void		sendMsg(Client& client, std::string& msg);
	void		sendTopic(Client& client, Channel& channel);
	void		sendJoinSuccessMsg( Client& client, Channel& channel);
	void		sendSetModeMsg(Client& client, Channel& channel, 
					std::string& executedMode, std::string& executedArgs);
	void		sendNameReply(Client& client, Channel& channel);
	void		broadcastChannelMsg(std::string& msg, Channel& channel);
	void 		broadcastChannelMsg(std::string& msg, Channel& channel,
					Client& client);
	void		broadcastUsersMsg(std::string& msg, Client& client, bool sender);
	void		sendClientErr(int num, Client& client, Channel* channel, 
					std::vector<std::string> otherArgs);
	void		sendKickMsg(std::string oper, std::string client, 
					std::vector<std::string>& params, Channel& channel);
	void		sendPartMsg(Client& client, std::vector<std::string>& params, 
					Channel& channel);
	void		sendWelcomeMsg(Client& client);

};

