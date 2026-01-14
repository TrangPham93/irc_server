#include "utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"

using namespace utils;

std::string utils::ft_stringToLower(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		str[i] = static_cast<char>(std::tolower(static_cast<char>(str[i])));
	}
	return str;
}

std::string utils::ft_trimString(std::string msg)
{
    std::string leadingTrim = msg.substr(msg.find_first_not_of(" \a\b\t\n\\v\f\r"), msg.length() - msg.find_first_not_of(" \a\b\t\n\\v\f\r"));
    std::string trailingTrim = leadingTrim.substr(0, leadingTrim.find_last_not_of(" \a\b\t\n\\v\f\r") + 1);
    return trailingTrim;
}

/** @brief split string into tokens using delimiter */
std::vector<std::string> utils::ft_splitString(std::string buffer, char delimiter)
{
	std::istringstream	tokenStream(buffer); //save buffer string to an istringstream obj
	std::string			aToken;
	std::vector<std::string> tokens;

	while (std::getline(tokenStream, aToken, delimiter))
	{
		tokens.push_back(aToken);
	}
	return tokens;
}

/* The most common form of reply is the numeric reply, used for both errors and normal replies. Distinct from a normal message, a numeric reply MUST contain a <source> and use a three-digit numeric as the command. A numeric reply SHOULD contain the target of the reply as the first parameter of the message. A numeric reply is not allowed to originate from a client */
// : channel names, usernames, modes, numbers -> param
// Everything after : is considered the trailing field
std::string makeNumericReply(std::string prefix, int code, std::string target, std::vector<std::string> params, std::string trailing)
{
	std::string p, s;
	for (auto param : params)
		p += param + " ";
	s = ":" + prefix + " " + std::to_string(code) + " " + target 
		+ (p.empty() ? " " : " " + p)
		+ (trailing.empty() ? "" : ":" + trailing)
		+ "\r\n";
	return s;
}

std::string getTarget(Client &client)
{
	std::string target;
	if (client.getNick().empty())
	{
		target = "*";
	}
	else
		target = client.getNick();
	return target;
}

std::string utils::extractChannelName(std::string buffer)
{
	std::string	channelName;
	
	size_t hashPos = buffer.find("#");
	if (hashPos == std::string::npos)
		return nullptr;
	
	size_t chanEndPos = buffer.find(' ', hashPos);
	if (chanEndPos == std::string::npos)
		chanEndPos = buffer.length();

	channelName = buffer.substr(hashPos + 1, chanEndPos - hashPos -1);
	return channelName;

}

bool	utils::compareCasemappingStr(std::string s1, std::string s2)
{
	if (!s1.empty() && !s2.empty())
	{
		if (utils::ft_stringToLower(s1) == utils::ft_stringToLower(s2))
			return true;	
	}
	return false;
}

Client* checkClientExistence(std::vector<Client*>& list, std::string nick)
{
	Client* c = nullptr;

	for (auto it:list) {
		if ((*it).getNick() == nick) {
			c = it;
			break ;
		}
	}
	return c;
}

std::string utils::joinTokenVector(const std::vector<std::string>& tokens)
{
	std::string str = "";
	if (tokens.size() == 0)
		return str;

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		if (i == tokens.size() - 1)
			str += tokens[i];
		else
			str += tokens[i] + " ";
	}
	return str;
}

bool containsSpaces(std::string &str)
{
	int N = str.length();
	for (int i = 0; i < N; i++)
	{
		if (isspace(str[i]) != 0)
			return true;
	}
	return false;
}
