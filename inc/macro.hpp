#pragma once

#define MSG_SIZE 512
#define NICKLEN 15
#define USERLEN 15
#define CHANMODES "itkol"
#define CHANLIMIT 50 //the number of channels a client may join.
#define CHANNELLEN 50
#define CASEMAPPING "ascii"

#define L_MODE 'l'
#define I_MODE 'i'
#define T_MODE 't'
#define K_MODE 'k'
#define O_MODE 'o'

/* REPLIES */
#define RPL_NOTOPIC				331
#define RPL_TOPIC				332
#define RPL_NAMREPLY			353
#define RPL_TOPICWHOTIME		333
#define RPL_CHANNELMODEIS		324
#define RPL_CREATIONTIME		329
#define RPL_INVITING			341
#define RPL_ENDOFNAMES			366

/* ERRORS */
#define ERR_NOSUCHNICK			401
#define ERR_NOSUCHCHANNEL		403
#define ERR_CANNOTSENDTOCHAN	404
#define ERR_TOOMANYCHANNELS		405
#define ERR_NORECIPIENT			411
#define ERR_NOTEXTTOSEND		412
#define ERR_INPUTTOOLONG		417
#define ERR_UNKNOWNCOMMAND		421
#define ERR_USERNOTINCHANNEL	441
#define ERR_NOTONCHANNEL		442
#define ERR_USERONCHANNEL		443
#define ERR_NEEDMOREPARAMS		461
#define ERR_CHANNELISFULL		471
#define ERR_UNKNOWNMODE 		472
#define ERR_INVITEONLYCHAN		473
#define ERR_BADCHANNELKEY		475
#define ERR_BADCHANNAME			479  // not  in official irc document, Libera/InspIRCd-specific error code, means “Illegal channel name
#define ERR_CHANOPRIVSNEEDED	482
#define ERR_INVALIDKEY			525

#define ERR_PASS "Password cannot contain whitespace characters"
#define INPUT_FORMAT "Expected format: <./ircserv> <port number> <password>"
#define ERR_PORT "Port number expected to be of numeric value between numbers 1024 and 65535"
#define ERR_SOCKET "Failed to create a socket"
#define ERR_BIND "Failed to bind server socket"
#define ERR_LISTEN "Failed to listen on server socket"
#define ERR_EPOLL "Failed to create epoll instance"
#define ERR_EPOLLCTL "Failed to add file descriptor to epoll"
#define ERR_ACCEPT "Failed to accept new client"

#define RPL_WELCOME(servername, nickname) ":" + servername + " 001 " + nickname + " :Welcome to the " + servername + " Network, " + nickname + "\r\n"
#define RPL_YOURHOST(servername, nickname, version) ":" + servername + " 002 " + nickname + " :Your host is " + servername + ", running version " + version + "\r\n"
#define RPL_CREATED(servername, nickname, datetime) ":" + servername + " 003 " + nickname + " :This server was created " + datetime + "\r\n"
#define RPL_MYINFO(servername, nickname, version, umodes, cmodes) ":" + servername + " 004 " + nickname + " " + servername + " " + version + " " + umodes + " " + cmodes + "\r\n"
#define RPL_ISUPPORT(servername, nickname, info) ":" + servername + " 005 " + nickname + " " + info + ":are supported by this server\r\n"

#define RPL_PONG(token) "PONG " + token + "\r\n"
#define NEW_NICK(oldnick, user, host, newnick) ":" + oldnick + "!" + user + "@" + host + " NICK " + newnick + "\r\n"

#define ERR_NONICKNAMEGIVEN(servername, nickname) ":" + servername + " 431 " + nickname + " :No nickname given\r\n"
#define ERR_ERRONEUSNICKNAME(servername, nickname, badnick) ":" + servername + " 432 " + nickname + " " + badnick + " :Erroneus nickname\r\n"
#define ERR_PASSWDMISMATCH(servername, nickname) ":" + servername + " 464 " + nickname + " :Password incorrect\r\n"
#define ERR_ALREADYREGISTERED(servername, nickname) ":" + servername + " 462 " + nickname + " :You may not register\r\n"
#define ERR_NICKNAMEINUSE(servername, nickname, badnick) ":" + servername + " 433 " + nickname + " " + badnick + " :Nickname is already in use\r\n"




