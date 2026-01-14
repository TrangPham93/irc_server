#include "Server.hpp"
#include "Client.hpp"
#include "utils.hpp"

void Server::user(Client &client, std::vector<std::string> tokens)
{
	if (client.getClientState() == NONE)
	{
		std::string message = "PASS needs to come before NICK/USER\r\n";
		sendMsg(client, message);
		client.setClientState(TOBEDISCONNECTED);
		return;
	}
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), client.getNick());
		sendMsg(client, message);
		return ;
	}
	if (tokens.size() == 4 && tokens[3].size() != 0 && tokens[3].find(":") != std::string::npos)
	{
		tokens[3].erase(0, tokens[3].find(":") + 1);		
	}
	if (tokens.size() < 4 || tokens[0].empty() || tokens[3].empty())
	{
		sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"USER"});
		return ;			
	}
	if (tokens[0].size() > USERLEN)
		client.setUserName(tokens[0].substr(0, USERLEN));
	else
		client.setUserName(tokens[0]);
	client.setRealName(tokens[3]);
	attemptRegister(client);
}
