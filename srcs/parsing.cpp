#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

void Server::parseMessage(Client &c, const std::string &line)
{
	std::vector<std::string> msg;
	std::string command;

	size_t i = 0;
	const size_t n = line.size();
	if (n > MSG_SIZE)
	{
		sendClientErr(ERR_INPUTTOOLONG, c, nullptr, {});
		return ;
	}

	// skip leading spaces
	i = line.find_first_not_of(' ', i);

	// command
	size_t cmdStart = i;

	i = line.find(' ', i);

	// no command
	if (cmdStart == i)
		return ;

	command = line.substr(cmdStart, i - cmdStart);

	for (char &c : command)
		c = std::toupper(static_cast<unsigned char>(c));

	// parameters
	int j = 1;
	while (i < n) {
		// skip spaces before next parameter
		i = line.find_first_not_of(' ', i);
		if (i >= n)
			break ;

		if (line[i] == ':') {
			std::string trailing = line.substr(i);
			msg.push_back(trailing);
			break ;
		}
		else {
			// read middle param until space
			size_t paramStart = i;

			i = line.find(' ', i);
			msg.push_back(line.substr(paramStart, i - paramStart));
		}
		++j;
	}
	logMessages(command, msg, c.getClientFd());
	handleCommand(*this, c, command, msg);
}

void Server::receive(Client &c)
{
	// Recieve data from the client
	char buffer[512];
	ssize_t bytes = 1;
	
	bytes = recv(c.getClientFd(), buffer, sizeof(buffer), MSG_DONTWAIT);
	// Errorhandling
	if (bytes < 0) {
		std::cerr << "Failed to receive from client: " << c.getClientFd() << std::endl;
		c.setClientState(DISCONNECTING);
	}
	else if (bytes == 0)
		c.setClientState(DISCONNECTING);

	// Buffer recieved data
	else {
		c.appendToInput(buffer, bytes);
		// Check if message if complete
		while (true) {
			size_t newline = c.getInput().find("\r\n");
			if (newline == c.getInput().npos)
				break ;
			auto begin = c.getInput().begin();
			auto end = c.getInput().begin() + newline;
			parseMessage(c, std::string(begin, end));
			c.eraseFromInput(newline);
		}
	}
}

void Server::handleCommand(Server &server, Client &client, std::string command, std::vector<std::string> &tokens)
{
	if (client.getClientState() == DISCONNECTING)
		return ;
	if (command == "PASS")
		pass(client, tokens);
	else if (command == "NICK")
		nick(client, tokens);
	else if (command == "USER")
		user(client, tokens);
	else if (client.getClientState() != REGISTERED)
		return;
	else if (command == "PING")
		ping(client, tokens);
	else if (command == "JOIN")
		server.handleJoin(client, tokens);
	else if (command == "TOPIC")
		server.handleTopic(client, tokens);
	else if (command == "MODE")
		server.handleMode(client, tokens);
	else if (command == "INVITE")
		server.handleInvite(client, tokens);
	else if (command == "PRIVMSG")
		server.handlePrivmsg(client, tokens);
	else if (command == "KICK")
		server.kickClient(client, tokens);
	else if (command == "PART")
		server.partChannel(client, tokens);
	else if (command == "QUIT")
		server.handleQuit(client, tokens);
	else
		sendClientErr(ERR_UNKNOWNCOMMAND, client, nullptr, {command});
}
