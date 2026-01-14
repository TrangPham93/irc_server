#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

/** @brief whether the client has been invited to join the channel */
bool	Channel::hasInvitedClient(Client* client)
{
	if (!_invitedUser.empty() && _invitedUser.find(client) != _invitedUser.end())
		return true;
	return false;
}

static bool isValidInvitation(std::vector<std::string>& tokens, Client& client, 
	Channel*& chann, Client*& invitedClient)
{
	if (tokens.size() < 2)
	{
		client.getServer().sendClientErr(461, client, chann, {"INVITE"});
		return false;
	}
	if (!client.isValidChanName(tokens[1]))
	{
		return false;
	}

	std::string nickName = tokens[0];
	std::string chanName = tokens[1].substr(tokens[1].find("#") + 1, tokens[1].length() - 1);

	chann = client.getServer().findChannel(chanName);
	if (!chann)
	{
		client.getServer().sendClientErr(ERR_NOSUCHCHANNEL, client, chann, {chanName}); 
		return false;
	}

	invitedClient = client.getServer().findClient(nickName);
	if (!invitedClient)
	{
		client.getServer().sendClientErr(ERR_NOSUCHNICK, client, chann, {nickName});
		return false;
	}
	if (!chann->isClientOnChannel(client))
	{
		client.getServer().sendClientErr(ERR_NOTONCHANNEL, client, chann, {});
		return false;
	}
	if (!client.isOps(*chann))
	{
		client.getServer().sendClientErr(ERR_CHANOPRIVSNEEDED, client, chann, {});
		return false;
	}

	if (chann->isClientOnChannel(*invitedClient))
	{
		client.getServer().sendClientErr(ERR_USERONCHANNEL, client, chann, {invitedClient->getNick()});
		return false;
	}

	return true;
}

/** @brief The INVITE command is used to invite a user to a channel. The parameter <nickname> is the nickname of the person to be invited to the target channel <channel>. The target channel SHOULD exist (at least one user is on it). Otherwise, the server SHOULD reject the command with the ERR_NOSUCHCHANNEL numeric.
 * Only members of the channel are allowed to invite other users. Otherwise, the server MUST reject the command with the ERR_NOTONCHANNEL numeric.
 * Servers MAY reject the command with the ERR_CHANOPRIVSNEEDED numeric. In particular, they SHOULD reject it when the channel has invite-only mode set, and the user is not a channel operator.
 */
void Server::handleInvite(Client& client, std::vector<std::string> tokens)
{
	Channel*	chann = nullptr;
	Client*		invitedClient = nullptr;

	if (!isValidInvitation(tokens, client, chann, invitedClient))
		return;

	//if valid invitation then send msg
	if (invitedClient)
	{
		chann->addInvitedUser(invitedClient);
		std::string	inviteMsg = client.makeUser() + " INVITE " + 
			invitedClient->getNick() + " #" + chann->getChannelName() + " \r\n";
		this->sendMsg(*invitedClient, inviteMsg);
		this->sendClientErr(RPL_INVITING, client, chann, 
			{invitedClient->getNick()});
	}
}


