#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"

void Server::pass(Client &client, std::vector<std::string> tokens)
{
	if (_pass.empty())
		return ;
	if (client.getClientState() == REGISTERED)
	{
		std::string message = ERR_ALREADYREGISTERED(getServerName(), getTarget(client));
		sendMsg(client, message);
		return ;	
	}
	else if (tokens.size() == 0)
	{
		sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"PASS"});
		client.setClientState(TOBEDISCONNECTED);
	}
	else if (tokens[0].compare(_pass) != 0)
	{
		std::string message = ERR_PASSWDMISMATCH(getServerName(), getTarget(client));
		sendMsg(client, message);
		client.setClientState(TOBEDISCONNECTED);
	}
	else
	{
		std::cout << "Password matched!" << std::endl;
		client.setClientState(REGISTERING);
		attemptRegister(client);
	}
}

