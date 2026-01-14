#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include <utility>
#include <unistd.h>
#include <sys/socket.h> 
#include <ctime>

#include "Enum.hpp"

class Client;
class Server;

struct ModeInfo {
	bool		add;
	char		mode;
	std::string params;
};

class Channel
{
private:
	std::string					_channelName;
	std::string					_topic;
	std::unordered_set<Client*>	_ops;
	std::unordered_set<Client*>	_halfOps; 
	std::unordered_set<Client*>	_voices;
	std::unordered_set<Client*>	_invitedUser;
	std::unordered_set<Client*>	_normals;
	std::vector<Client*> 		_userList;
	std::map<char, std::string>	_mode;
	std::map<char, channelMsg (Channel::*)(bool, std::string&)> _modeHandlers;
	time_t						_topicSetTimestamp;
	Client*						_topicSetter;
	time_t						_channelCreationTimestamp;
	
public:

	Channel() = delete;
	~Channel() = default;
	Channel(std::string newChannel);

	// getters
	std::string 				getChannelName() const;
	std::string 				getTopic() const;
	std::vector<Client*>&		getUserList() ;
	std::string					getChanKey() const;
	std::vector<std::string> 	getMode() const;
	std::string					printUser() const;
	std::string					getTopicTimestamp();
	std::string					getChannelCreationTimestamp();
	Client*						getTopicSetter();
	std::unordered_set<Client*>&	getOps();
	Client*						findClient(std::string nickName);

	// setters
	bool			setTopic(std::string newTopic, Client& clientset);
	void 			addMode(char key, std::string param);
	void			removeMode(char key);
	void			setTopicTimestamp();
	void			setTopicSetter(Client& setter);
	void			setChannelCreationTimestamp();

	// users container
	void			addUser(Client* newClient);
	void			removeUser(std::string userNick);
	void			addInvitedUser(Client* newClient);
	void			addNormal(Client* client);
	void			removeNormal(std::string userNick);
	void			addChanop(Client* chanop);
	void			removeChanop(std::string opNick);

	bool			isChanop(std::string nick);
	bool			isClientOnChannel( Client& client);
	channelMsg		canClientJoinChannel( Client& client,
					 std::string clientKey);
	bool			hasInvitedClient(Client* client);

	// mode
	bool			isModeActive(char mode);
	bool			isModeActive(char mode, std::string& key);
	channelMsg		handleInviteOnly(bool add, std::string& args);
	channelMsg		handleTopicRestriction(bool add, std::string& args);
	channelMsg		handleChannelKey(bool add, std::string& args);
	channelMsg		handleChannelOperator(bool add, std::string& args);
	channelMsg		handleChannelLimit(bool add, std::string& args);
	std::string		truncateTopic(std::string name);
	bool			parsingMode(std::vector<std::string> tokens,
						std::vector<ModeInfo>& parsedModeVec);
	bool			validateModeInstruction(Client& client, 
						std::vector<ModeInfo> parsedModeVec);
	void			executeModeCmd(Client& client, 
						std::vector<ModeInfo>& parsedModeVec);

};


