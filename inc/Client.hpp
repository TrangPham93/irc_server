#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex>
#include <functional>

#include "utils.hpp"
#include "macro.hpp"

class Channel;
class Server;

enum ClientState
{
 NONE,
 REGISTERING,
 REGISTERED,
 DISCONNECTING,
 TOBEDISCONNECTED
};

class Client
{
	private:
		int						_clientfd = -1;
		std::string				_clientNick = "";
		std::string				_userName = "";
		std::string				_realName = "";
		std::string				_hostName;

		std::vector<Channel*>	_joinedChannels;
		enum ClientState		_clientState = NONE;
		class Server			&_myServer;	
		std::string				_quitMsg;
		std::string				_input;
		std::string				_output;

	public:
		Client() = delete;
		Client(Server &server);
		~Client() = default;

		// getters
		int						getClientFd();
		std::string 			getNick();
		std::string 			getUserName();
		std::string				getRealName();
		std::string 			getHostName();
		std::vector<Channel*> 	getJoinedChannels();
		enum ClientState		getClientState();
		Server&					getServer();
		std::string				getQuitMsg();
		std::string&			getInput();
		std::string&			getOutput();

		// setters
		void		setClientFd(int num);
		void		setNick(std::string nick);
		void		setUserName(std::string user);
		void		setRealName(std::string user);
		void		setHostName(std::string host);
		void		setClientState(enum ClientState state);
		void		setQuitMsg(std::string msg);

		// other methods
		void		addJoinedChannel(Channel* chan);
		bool		isOps(Channel& channel);
		std::string makeUser();
		bool		isValidChanName(std::string name);
		void		removeChannel(Channel* chann);
		void		appendToInput(char* buffer, size_t size);
		void		eraseFromInput(size_t pos);
		void		appendToOutput(std::string &message);
		void		clearOutput();
};
