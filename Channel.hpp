#ifndef CHANNEL_H
#define CHANNEL_H

#include <iostream>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <algorithm>
#include <list>
#include <map>
#include "Client.hpp"
#include "ft_irc.hpp"
#include "utils.hpp"
#include "OtherServ.hpp"

class OtherServ;
class Client;

bool		custom_send(std::string ms, Client *c);
bool		custom_send(std::string ms, OtherServ *s);

class Channel
{
public:
	typedef	int			socket_t;
private:
	typedef	std::map<std::string, Client*>	_users_map;
	typedef	std::map<std::string, std::pair<std::string, time_t> >	_ban_map;
	class _Chan_modes
	{
	public:
		typedef	std::list<std::string>	usr_list;

		usr_list	invitation_list; // if i is set to true

		usr_list	o; // Operator privileges
		usr_list	v; // Ability to speak on a moderated channel
		_ban_map	b; // ban map (map [mask] = pair<operatorName, date>)
		_ban_map	e; // exception map (map [mask] = pair<operatorName, date>)
		_ban_map	I; // invite map (map [mask] = pair<operatorName, date>)
		bool		p; // Private channel
		bool		s; // Secret channel
		bool		i; // Users can't join without invite
		bool		t; // Topic can only be set by an operator
		bool		m; // Only voiced users and operators can talk
		bool		n; // Users outside channel cannot talk in channel
		bool		q; // Quiet channel
		int			l; // User limit
		std::string	k; // Channel password
		int			users;
		_Chan_modes(): invitation_list(), o(), v(), b(), e(), I(), p(false), s(false), i(false), t(false), m(false), n(false), q(false), l(-1), k(), users(0) {}
	};
	
	
	std::string		_name;
	_users_map		_users;
	_Chan_modes		_modes;
	std::string		_topic;

	std::string		_creator;

	bool		_hasRights(const std::string &userName) const
	{
		_Chan_modes::usr_list::const_iterator	user = std::find(_modes.o.begin(), _modes.o.end(), utils::ircLowerCase(userName));
		return (user != _modes.o.end());
	}

	bool		_isInList(const std::string &userName, const _Chan_modes::usr_list &listToCheck) const
	{
		_Chan_modes::usr_list::const_iterator	user = std::find(listToCheck.begin(), listToCheck.end(), utils::ircLowerCase(userName));
		return (user != listToCheck.end());
	}
	bool		_isBanned(Client *client) const;
	bool		_isInvited(Client *client) const;
	bool		_isInExceptionList(Client *client) const;
public:
	Channel();
	Channel(const std::string &name, Client *client, OtherServ *srv);
	~Channel();
	Channel	&operator=(const Channel& other);

	const std::string	&getName() const;
	int					getUsersNum() const;
	std::string			getUsersStr() const;
	std::vector<std::string>	getUsersVec() const;
	const std::string	&getCreator() const;
	const std::string	&getTopic() const;
	bool				setTopic(Client *client, const std::string &newTopic);
	bool				isEmpty() const;

	static std::string	parseArg(size_t fromIndex, const std::vector<std::string> &args);
	
	// * send msg to everyone in the channel but the sender
	bool				broadcastMsg(Client *sender, const std::string &msg) const;

	// *	join returns true on succes (false if socket was already in the channel before the call)
	bool				join(Client *client, const std::string &passwd, OtherServ *svFrom);
	// *	leave returns true on succes
	bool				leave(Client *client, const std::string &reason, OtherServ *svFrom, bool muted = false);
	// *	kick returns true on succes
	bool				kick(Client *client, const std::string &guyToKick, const std::string &reason);
	// *	invite returns true on succes 
	bool				invite(Client *client, const std::string &guyToInvite);
	// *	quit returns true on succes, should always return true in practice
	bool				quit(Client *client, const std::vector<std::string> &args);

	// MODES METHODS
	bool	mode_O(OtherServ *svFrom, bool append, Client *client, const std::string &target);

	bool	mode_o(OtherServ *svFrom, bool append, Client *client, const std::string &target);
	bool	mode_v(OtherServ *svFrom, bool append, Client *client, const std::string &target);
	bool	mode_b(OtherServ *svFrom, bool append, Client *client, const std::string &mask);
	bool	mode_e(OtherServ *svFrom, bool append, Client *client, const std::string &mask);
	bool	mode_I(OtherServ *svFrom, bool append, Client *client, const std::string &mask);

	bool	mode_p(OtherServ *svFrom, bool append, Client *client);
	bool	mode_s(OtherServ *svFrom, bool append, Client *client);
	bool	mode_i(OtherServ *svFrom, bool append, Client *client);
	bool	mode_t(OtherServ *svFrom, bool append, Client *client);
	bool	mode_m(OtherServ *svFrom, bool append, Client *client);
	bool	mode_n(OtherServ *svFrom, bool append, Client *client);
	bool	mode_q(OtherServ *svFrom, bool append, Client *client);
	bool	mode_l(OtherServ *svFrom, bool append, Client *client, int limit);
	bool	mode_k(OtherServ *svFrom, bool append, Client *client, const std::string &passwd);

	bool	getModeN() const;

	std::string	getModes() const;

	bool	isInChan(const std::string &userName) const;

	bool	msgErrors(Client *client, bool sendErrors = true) const;

	void	changeNick(const std::string &oldNick, const std::string &newNick);

	static bool		rplMsg(std::string ms, Client *c);

	void			showBanlist(Client *client) const;
	void			showInvitelist(Client *client) const;
	void			showExceptionlist(Client *client) const;
	void			showChanCreator(Client *client) const;
	bool			who(Client *client) const;
	bool			usrList(Client *client) const;

};

#endif