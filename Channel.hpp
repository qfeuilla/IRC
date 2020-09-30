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

class Client;

bool		custom_send(std::string ms, Client *c);

class Channel
{
public:
	typedef	int			socket_t;
private:
	typedef	std::map<std::string, Client*>	_users_map;
	class _Chan_modes
	{
	public:
		typedef	std::list<std::string>	usr_list;
		usr_list	invitation_list; // if i is set to true

		usr_list	o; // Operator privileges
		usr_list	v; // Ability to speak on a moderated channel
		bool		p; // Private channel
		bool		s; // Secret channel
		bool		i; // Users can't join without invite
		bool		t; // Topic can only be set by an operator
		bool		m; // Only voiced users and operators can talk
		bool		n; // Users outside channel cannot talk in channel
		int			l; // User limit
		std::string	k; // Channel password
		int			users;
		_Chan_modes(): invitation_list(), o(), v(), p(false), s(false), i(false), t(true), m(false), n(false), l(-1), k(), users(0) {}
	};
	
	
	std::string		_name;
	_users_map		_users;
	_Chan_modes		_modes;
	std::string		_topic;

	std::string		_srv_name;
	std::string		_creator;

	bool		_hasRights(const std::string &userName) const
	{
		_Chan_modes::usr_list::const_iterator	user = std::find(_modes.o.begin(), _modes.o.end(), userName);
		return (user != _modes.o.end());
	}

	bool		_is_in_list(const std::string &userName, const _Chan_modes::usr_list &listToCheck) const
	{
		_Chan_modes::usr_list::const_iterator	user = std::find(listToCheck.begin(), listToCheck.end(), userName);
		return (user != listToCheck.end());
	}
	void		_print_channel(void) const
	{
		_users_map::const_iterator	current = _users.begin();
		_users_map::const_iterator	end = _users.end();

		std::cout << "sockets in " << _name << "\n";
		while (current != end) {
			std::cout << "fd num = " << (*current).second << "(" << (*current).first << "), "; 
			++current;
		}
		std::cout << "\n\n";
	}
public:
	Channel();
	Channel(const std::string &name, Client *client, const std::string &srvName);
	~Channel();
	Channel	&operator=(const Channel& other);

	const std::string	&getName() const;
	std::string			getUsersNum() const;
	const std::string	&getCreator() const;
	const std::string	&getTopic() const;
	bool				setTopic(Client *client, const std::string &newTopic);
	bool				isEmpty() const;

	static std::string	parseArg(size_t fromIndex, const std::vector<std::string> &args);
	
	// * send msg to everyone in the channel but the sender
	bool				broadcastMsg(Client *sender, const std::string &msg) const;

	// *	join returns true on succes (false if socket was already in the channel before the call)
	bool				join(Client *client, const std::string &passwd);
	// *	leave returns true on succes
	bool				leave(Client *client, const std::string &reason, bool muted = false);
	// *	kick returns true on succes
	bool				kick(Client *client, const std::string &guyToKick, const std::string &reason);
	// *	invite returns true on succes 
	bool				invite(Client *client, const std::string &guyToInvite);
	// *	quit returns true on succes, should always return true in practice
	bool				quit(Client *client, const std::vector<std::string> &args);

	// MODES METHODS
	bool	mode_o(bool append, Client *client, const std::string &target);
	bool	mode_v(bool append, Client *client, const std::string &target);

	bool	mode_p(bool append, Client *client);
	bool	mode_s(bool append, Client *client);
	bool	mode_i(bool append, Client *client);
	bool	mode_t(bool append, Client *client);
	bool	mode_m(bool append, Client *client);
	bool	mode_n(bool append, Client *client);
	bool	mode_l(bool append, Client *client, int limit);
	bool	mode_k(bool append, Client *client, const std::string &passwd);

	bool	getModeN() const;

	std::string	getModes() const;

	bool	isInChan(const std::string &userName) const;

	bool	msgErrors(Client *client, bool sendErrors = true) const;

	void	changeNick(const std::string &oldNick, const std::string &newNick);

};

#endif