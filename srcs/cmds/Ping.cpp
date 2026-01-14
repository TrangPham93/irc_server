#include "Server.hpp"
#include "Client.hpp"
#include "utils.hpp"

void Server::ping(Client &client, std::vector<std::string> tokens)
{
		std::string message = RPL_PONG(tokens[0]);
		sendMsg(client, message);
}
