#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

void Server::nick(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == NONE)
	{
		std::string message = "PASS needs to come before NICK/USER\r\n";
		sendMsg(client, message);
		client.setClientState(TOBEDISCONNECTED);
		return;
	}
	std::regex pattern(R"(^[A-Za-z\[\]{}\\|_][A-Za-z0-9\[\]{}\\|_]*$)");
	std::string oldnick = client.getNick();
	if (tokens.size() == 0)
	{
		std::string message = ERR_NONICKNAMEGIVEN(getServerName(), getTarget(client));
		sendMsg(client, message);
		return ;			
	}
	if (std::regex_match(tokens[0], pattern) == false || tokens[0].length() > NICKLEN)
	{
		std::string message = ERR_ERRONEUSNICKNAME(getServerName(), getTarget(client), tokens[0]);
		sendMsg(client, message);
		return ;
	}
	if (tokens[0] == client.getNick())
		return ;
	for (size_t i = 0; i < getClientInfo().size(); i++)
	{
		if (utils::compareCasemappingStr(getClientInfo()[i]->getNick(), tokens[0]) == true)
		{
			std::string message = ERR_NICKNAMEINUSE(getServerName(), getTarget(client), tokens[0]);
			sendMsg(client, message);
			return ;
		}
	}
	client.setNick(tokens[0]);
	if (client.getClientState() == REGISTERED)
	{
		std::string message = NEW_NICK(oldnick, client.getUserName(), client.getHostName(), client.getNick());	
		broadcastUsersMsg(message, client, true);
	}
	if (client.getClientState() != REGISTERED)
	{
		attemptRegister(client);
	}
}
