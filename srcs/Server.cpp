#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include "utils.hpp"

/* @note rememeber to check all on-heap allocated memory, such as chan, client */
Server::~Server()
{
	for (auto client = _clientInfo.begin(); client != _clientInfo.end();) 
	{
		Client *ptr = *client;
		disconnectClient(ptr);
		client = _clientInfo.begin();
	}
	for (auto chan : _channelInfo)
		delete chan;
	if (_epollFd != -1)
		close(_epollFd);
	if (_serverFd != -1)
		close(_serverFd);
}

/* @def check if the channel exists
	@return ptr to channel if exist else return nullptr */
Channel* Server::findChannel(std::string newChannel) 
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end(); ++it)
	{
		if (utils::compareCasemappingStr((*it)->getChannelName(), newChannel))
			return *it;
	}
	return nullptr;
}

std::string Client::getQuitMsg()
{
	return _quitMsg;
}


void Server::disconnectClient(Client *client)
{
	std::cout << "Client fd " << client->getClientFd() << " disconnected" << std::endl;
	auto it = std::find(_clientInfo.begin(), _clientInfo.end(), client);
	if (it == _clientInfo.end())
		return ;
	
	if (client->getQuitMsg().empty())
		client->setQuitMsg("Client is disconnected other than QUIT");
	std::string serverMsg = client->makeUser() + " QUIT :" + client->getQuitMsg() + "\r\n";
	this->broadcastUsersMsg(serverMsg, *client, false);

	for (auto chan : client->getJoinedChannels())
		chan->removeUser(client->getNick());
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, client->getClientFd(), NULL);
	close(client->getClientFd());
	getClientInfo().erase(it);
	delete client;
}

/*Port is a 16-bit unsigned int, meaning valid range is 0-65535.
However, ports under 1024 are privileged and require root privileges. In our program,
we would get an error of failing to bind a server socket.*/
void Server::setupServerDetails(char *argv[])
{
	size_t pos;

	_name = argv[0];
	_name.erase(0, _name.find_last_of("/") + 1);
	try
	{	_port = std::stoi(argv[1], &pos); }
	catch (const std::exception&)
	{
		throw std::runtime_error(ERR_PORT);
	}
	std::string s = argv[1];
	if (pos != s.length() || _port < 1024 || _port > 65535)
		throw std::runtime_error(ERR_PORT);
	_pass = argv[2];
	if ((containsSpaces(_pass) == true))
		throw std::runtime_error(ERR_PASS);
	std::cout << "Server's port is: " << _port << " and password is : " << _pass << std::endl;
}

/* Creating the listening server socket (object representing one endpoint of a network connection)
- AF_INET uses IPv4 addresses
- htons converts the port number from host byte order into network byte order (ports could appear swapped or broken without)
- SOCK_STREAM means TCP, connection-based, reliable communication, guarantees data arrives in order without loss or duplication
(alternative would be UDP, Datagram Sockets which is connectionless, unreliable communication, faster but does not guarantee delivery or order)
- INADDR_ANY accepts any IP that is attempting to connect
- socket creates the socket object and returns a fd
- setsockopt, allows a socket to bind to an address/port that is still in use. So this allows to close the server and immediately restart.
- Bind binds these settings for the main listening socket
- Listen changes the socket into passive mode*/
void Server::setupSocket()
{
	_details.sin_family = AF_INET;
	_details.sin_port = htons(_port);
	_details.sin_addr.s_addr = INADDR_ANY;
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw std::runtime_error(ERR_SOCKET);
	fcntl(_serverFd, F_SETFL, O_NONBLOCK);
	int opt = 1;
	setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	if (bind(_serverFd, (struct sockaddr *)&_details, sizeof(_details)) == -1)
		throw std::runtime_error(ERR_BIND);
	if (listen(_serverFd, 1) == -1)
		throw std::runtime_error(ERR_LISTEN);
}

/* Setting an event tracker
- epoll_create creates an epoll instance
- EPOLLIN tells to notify when this FD is readable (new client in this scenario)
- We do not need to track EPOLLOUT because server socket never sends data*/
void Server::setupEpoll()
{
	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
		throw std::runtime_error(ERR_EPOLL);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _serverFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serverFd, &ev) == -1)
		throw std::runtime_error(ERR_EPOLLCTL);
}

/*Handling a new client
- struct sockaddr_in clientAddrress holds the client's IP address and port.
- Calling accept fills the struct with the info (previously this was marked as NULL but then we wouldn't have stored IP anywhere)
- More importantly, accept accepts a new connection and returns a new socket fd.
- Now the struct contains the IPv4 address and inet_ntoa converts it into a string. (previously it was in binary)
- Then we make the client socket non blocking, which means the program wont pause waiting for data from this socket.
- Epoll is like event manager, it contains a list of sockets that we want to "track", so now here we are adding
this socket to the list with the settings EPOLLIN because we want to know when a socket is is readable (when we have received something)
and EPOLLOUT because we want to know when a socket is writeable (when we can reply back to the clients)*/
void Server::handleNewClient()
{
	Client *newClient = new Client(*this);
	struct sockaddr_in clientAddress;
	socklen_t addressLength = sizeof(clientAddress);
	newClient->setClientFd(accept(_serverFd, (struct sockaddr *)&clientAddress, &addressLength));
	if (newClient->getClientFd() == -1)
	{
		delete newClient;
		throw std::runtime_error(ERR_ACCEPT);
	}
	char *clientIP = inet_ntoa(clientAddress.sin_addr);
	newClient->setHostName(clientIP);
	_clientInfo.push_back(newClient);
	fcntl(newClient->getClientFd(), F_SETFL, O_NONBLOCK);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = newClient->getClientFd();
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newClient->getClientFd(), &ev) == -1)
		throw std::runtime_error(ERR_EPOLLCTL);
	if (_pass.empty())
		newClient->setClientState(REGISTERING);
	else
		newClient->setClientState(NONE);
	std::cout << "New connection, fd: " << newClient->getClientFd() << std::endl;
}

void Server::handleDisconnects()
{
	for (auto it = _clientInfo.begin(); it != _clientInfo.end();)
	{
		if ((*it)->getClientState() == DISCONNECTING)
		{
			disconnectClient(*it);
			it = _clientInfo.begin();
		}
		else
		{
			if ((*it)->getClientState() == TOBEDISCONNECTED)
				(*it)->setClientState(DISCONNECTING);
			++it;
		}
	}
}

/*Event handling loop
- epoll_wait waits for notifications from the sockets, then it fills the epollEvents with fds that have communicated something
- if fd is the server fd, we know a new client connection is waiting, otherwise it is an existing client*/
void Server::handleEvents()
{
	int eventCount = epoll_wait(getEpollfd(), getEpollEvents(), MAX_EVENTS, -1);
	for (int i = 0; i < eventCount; ++i)
	{
		int fd = getEpollEvents()[i].data.fd;
		if (fd == getServerfd())
		{
			handleNewClient();
			continue;
		}
		Client *c = findClientByFd(fd);
		if (getEpollEvents()[i].events & (EPOLLERR | EPOLLHUP))
		{
			c->setClientState(DISCONNECTING);
			continue ;
		}
		if (getEpollEvents()[i].events & EPOLLIN)
			receive(*c);
		if (getEpollEvents()[i].events & EPOLLOUT)
			reply(*c);
	}
	handleDisconnects();
}

void Server::attemptRegister(Client &client)
{
	if (client.getClientState() != REGISTERING)
		return;
	if (client.getNick().empty() || client.getUserName().empty())
		return;
	client.setClientState(REGISTERED);
	sendWelcomeMsg(client);
}

int Server::getEpollfd() const
{
	return _epollFd;
}

std::string& Server::getServerName()
{
	return _name;
}

struct epoll_event* Server::getEpollEvents()
{
	return _events;
}

int Server::getServerfd() const
{
	return _serverFd;
}

std::vector<Client*>& Server::getClientInfo()
{
	return _clientInfo;
}

std::vector<Channel*>& Server::getChannelInfo()
{
	return _channelInfo;
}

void Server::removeChannel(Channel* chann)
{
	for (auto it = _channelInfo.begin(); it != _channelInfo.end();) {
		if ((*it) == chann)
		{
			delete *it;
			it = _channelInfo.erase(it);
		}
		else
			++it;
	}
}

/** @return remove the # from the token and return pointer to existing channel, 
 * otherwhile nullpointer */
Channel* Server::setActiveChannel(std::string buffer)
{
	std::string	channelName;

	if (buffer.find("#") != std::string::npos)
	{
		size_t hashPos = buffer.find("#");
		if (hashPos == std::string::npos)
			return nullptr;
		
		size_t chanEndPos = buffer.find(' ', hashPos);
		if (chanEndPos == std::string::npos)
			chanEndPos = buffer.length();
	
		channelName = buffer.substr(hashPos + 1, chanEndPos - hashPos -1);
	}
	else
	{
		channelName = buffer;
	}
	return this->findChannel(channelName);
}

Client*	Server::findClient(std::string nickName)
{
	for (auto it = _clientInfo.begin(); it != _clientInfo.end(); ++it)
	{
		if (utils::compareCasemappingStr((*it)->getNick(), nickName))
			return *it;
	}
	return nullptr;
}

Client*	Server::findClientByFd(int fd)
{
	for (Client *c : _clientInfo)
	{
		if (fd == c->getClientFd())
			return c;
	}
	return nullptr;
}
