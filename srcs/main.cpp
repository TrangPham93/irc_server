
#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

bool activeserver = true;

void signalHandler(int sig)
{
	(void)sig;
	activeserver = false;
}

int main(int argc, char *argv[])
{
	Server server;
	try 
	{
		if (argc != 3)
			throw std::runtime_error(INPUT_FORMAT);
		server.setupServerDetails(argv);
		server.setupSocket();
		server.setupEpoll();
		signal(SIGINT, signalHandler);
		while (activeserver == true)
		{
			server.handleEvents();
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
