#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"
#include "macro.hpp"

void Server::kickClient(Client& client, std::vector<std::string>& params)
{
	std::string error;

	// check that there is correct amount of params
	if (params.size() < 2) {
		sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"KICK"});
		return ;
	}

	std::string channelString = params[0];
	std::string clientString = params[1];

	// check that channel exists
	Channel* chann = setActiveChannel(channelString);
	if (!chann) {
		sendClientErr(ERR_NOSUCHCHANNEL, client, chann, {channelString});
		return ;
	}

	// check if client kicking is on channel
	if (!chann->isClientOnChannel(client)) {
		sendClientErr(ERR_NOTONCHANNEL, client, chann, {client.getNick()});
		return ;
	}

	// check that client kicking is a channel operator
	if (!client.isOps(*chann)) {
		sendClientErr(ERR_CHANOPRIVSNEEDED, client, chann, {client.getNick()});
		return ;
	}

	// check for multiple clients and put them into a vector
	std::vector<std::string> clientList;
	if (clientString.find(",") != std::string::npos)
		clientList = utils::ft_splitString(clientString, ',');
	else
		clientList.push_back(clientString);

	for (size_t i = 0; i < clientList.size(); ++i) {

		// check that client getting kicked is on the server
		Client* cliServer = checkClientExistence(getClientInfo(), clientList[i]);
		if (!cliServer) {
			sendClientErr(ERR_NOSUCHNICK, client, chann, {clientList[i]});
			continue ;
		}

		// check that client getting kicked is on the channel
		Client* cliChannel = checkClientExistence(chann->getUserList(), clientList[i]);
		if (!cliChannel) {
			sendClientErr(ERR_USERNOTINCHANNEL, client, chann, {clientList[i]});
			continue ;
		}

		sendKickMsg(client.getNick(), clientList[i], params, *chann);

		// remove user from channelList and channel from client channelList
		chann->removeUser(clientList[i]);
		cliChannel->removeChannel(chann);
	}

	// if channel is empty after this, remove it from servers list of channels
	auto ite = chann->getUserList();
	if (ite.empty()) {
		removeChannel(chann);
	}
}
