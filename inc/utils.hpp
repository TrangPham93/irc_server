#pragma once

#include <iostream>
#include <vector>

class Client;
class Channel;

namespace utils {

	std::vector<std::string>	ft_splitString(std::string buffer, char delimiter);
	std::string ft_trimString(std::string msg);
	std::string ft_stringToLower(std::string str);
	std::string extractChannelName(std::string str);
	bool		compareCasemappingStr(std::string s1, std::string s2);
	std::string joinTokenVector(const std::vector<std::string>& tokens);

	// messaging utils
	std::string makePrivMsgToChan(std::string& token, Client& client, Channel& chan);
	std::string makePrivMsgToClient(std::string& token, Client& client,Client& partner);

}

std::string	makeNumericReply(std::string prefix, int code, 
	std::string target, std::vector<std::string> params, std::string trailing);

std::string	getTarget(Client &client);
bool		containsSpaces(std::string &str);

Client* checkClientExistence(std::vector<Client*>& list, std::string nick);
