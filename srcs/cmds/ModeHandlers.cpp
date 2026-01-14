#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "utils.hpp"
#include "macro.hpp"

/** @brief check whether a mode is active and save the params if mode requires params */
bool	Channel::isModeActive(char mode, std::string& key)
{
	for (auto m : _mode)
	{
		if (m.first == mode)
		{
			key = m.second;
			return true;
		}
	}
	return false;
}

/** @brief check whether a mode is active, no params required */
bool	Channel::isModeActive(char mode)
{
	for (auto m : _mode)
	{
		if (m.first == mode)
			return true;
	}
	return false;
}

/** @brief  
 * The commands which may only be used by channel moderators include:
 * KICK: Eject a client from the channel
 * MODE: Change the channel’s modes
 * INVITE: Invite a client to an invite-only channel (mode +i)
 * TOPIC: Change the channel topic in a mode +t channel */
channelMsg	Channel::handleChannelOperator(bool add, std::string& args)
{
	Client* client = this->findClient(args);

	if (!client)
		return NO_ACTION;
	if (add)
	{		
		this->addChanop(client);
		this->removeNormal(client->getNick());
		return SET_MODE_OK;
	}
	else if (!add && this->isChanop(args))
	{
		this->removeChanop(args);
		this->addNormal(client);
		return SET_MODE_OK;
	}
	return NO_ACTION;
}

/** @brief If this mode is set, its value is an integer and defines the limit of how many clients may be joined to the channel. 
 * If non-interger is passed as params, server ignores and do nothing.*/
channelMsg	Channel::handleChannelLimit(bool add, std::string& args)
{
	std::string	key;
	bool		active = this->isModeActive(L_MODE, key);

	if (add)
	{
		this->removeMode(L_MODE);
		this->addMode(L_MODE, args);
		return SET_MODE_OK;
	}
	else if (!add && active)
	{
		this->removeMode(L_MODE);
		return SET_MODE_OK;
	}
	return NO_ACTION;
}

/**	@brief if this mode is set on a channel, a user must have received an INVITE for this channel before being allowed to join it. 
 * If they have not received an invite, they will receive an ERR_INVITEONLYCHAN (473) reply and the command will fail. */
channelMsg Channel::handleInviteOnly(bool add, std::string& args)
{
	bool active = this->isModeActive(I_MODE);

	if (add)
	{
		if (active)
			return NO_ACTION;
		this->addMode(I_MODE, args);
		return SET_MODE_OK;
	}
	else
	{
		if (active)
		{
			this->removeMode(I_MODE);
			return SET_MODE_OK;
		}
	}
	return NO_ACTION;
}


/** @brief T_MODE controls whether channel privileges are required to set the topic, and does not have any value. 
 * If this mode is enabled, users must have channel privileges such as operator status in order to change the topic of a channel. In a channel that does not have this mode enabled, anyone may set the topic of the channel using the TOPIC command.
*/
channelMsg	Channel::handleTopicRestriction(bool add, std::string& args)
{
	bool active = this->isModeActive(T_MODE);

	if (add)
	{
		if (active)
			return NO_ACTION;
		this->addMode(T_MODE, args);
		return SET_MODE_OK;
	}
	else
	{
		if (active)
		{
			this->removeMode(T_MODE);
			return SET_MODE_OK;
		}
	}
	return NO_ACTION;
}

/** @brief This mode letter sets a ‘key’ that must be supplied in order to join this channel. If this mode is set, its’ value is the key that is required.  */
channelMsg	Channel::handleChannelKey(bool add, std::string& args)
{
	std::string key;
	bool active = this->isModeActive(K_MODE, key);
	
	if (add)
	{
		this->removeMode(K_MODE);
		this->addMode(K_MODE, args);
		return SET_MODE_OK;
	}
	else if (!add && active)
	{
		this->removeMode(K_MODE);
		return SET_MODE_OK;
	}
	return NO_ACTION;
}