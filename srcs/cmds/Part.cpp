#include "Client.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "utils.hpp"
#include "macro.hpp"

void Server::partChannel(Client& client, std::vector<std::string>& params)
{
	std::string error;

	// check that there is correct amount of params
	if (params.size() == 0) {
		sendClientErr(ERR_NEEDMOREPARAMS, client, nullptr, {"KICK"});
		return ;
	}

	std::string channelString = params[0];

	// check for multiple channels and put them into a vector
	std::vector<std::string> channList;
	if (params[0][0] == '#' && params[0].find(",") != std::string::npos) {
		channList = utils::ft_splitString(params[0], ',');
	}
	else
		channList.push_back(channelString);

	for (size_t i = 0; i < channList.size(); ++i) {

		// check that channel exists
		Channel* chann = setActiveChannel(channList[i]);
		if (!chann) {
			sendClientErr(ERR_NOSUCHCHANNEL, client, chann, {channList[i]});
			continue ;
		}

		// check if client parting is on channel
		if (!chann->isClientOnChannel(client)) {
			sendClientErr(ERR_NOTONCHANNEL, client, chann, {client.getNick()});
			continue ;
		}

		sendPartMsg(client, params, *chann);

		// remove user from channels client list and the channel from users channellist
		chann->removeUser(client.getNick());
		client.removeChannel(chann);

		// if channel is empty after this, remove it from servers list of channels
		auto ite = chann->getUserList();
		if (ite.empty()) {
			removeChannel(chann);
		}
	}
}
