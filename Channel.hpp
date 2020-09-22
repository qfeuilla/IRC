#ifndef CHANNEL_H
#define CHANNEL_H

#include <iostream>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <algorithm>
#include <list>
#include <map>

class Client;
class Channel
{
public:
	typedef	int			socket_t;
private:
	typedef	std::map<std::string, socket_t>	_users_map;
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
		int			l; // User limit
		std::string	k; // Channel password
		int			users;
		_Chan_modes(): invitation_list(), o(), v(), p(false), s(false), i(false), t(false), m(false), l(-1), k(), users(0) {}
	};
	
	
	std::string							_name;
	_users_map							_users;
	_Chan_modes							_modes;

	bool		_hasRights(const std::string &userName)
	{
		_Chan_modes::usr_list::iterator	user = std::find(_modes.o.begin(), _modes.o.end(), userName);
		return (user != _modes.o.end());
	}

	bool		_is_in_list(const std::string &userName, _Chan_modes::usr_list &listToCheck)
	{
		_Chan_modes::usr_list::iterator	user = std::find(listToCheck.begin(), listToCheck.end(), userName);
		return (user != listToCheck.end());
	}
	bool		_is_in_chan(const std::string &userName)
	{
		_users_map::iterator	user = _users.find(userName);
		return (user != _users.end());
	}
	void		_print_channel(void) {
		_users_map::iterator	current = _users.begin();
		_users_map::iterator	end = _users.end();

		std::cout << "sockets in " << _name << "\n";
		while (current != end) {
			std::cout << "fd num = " << (*current).second << "(" << (*current).first << "), "; 
			++current;
		}
		std::cout << "\n\n";
	}
public:
	Channel();
	Channel(const std::string &name, std::string nick, socket_t socket);
	~Channel();
	Channel	&operator=(const Channel& other);

	const std::string	&getName() const;

	static void			sendMsgToSocket(socket_t socket, const std::string &msg);

	// *	join(socket) returns true on succes (false if socket was already in the channel before the call)
	bool				join(std::string nick, socket_t socket, const std::string &passwd);
	// *	leave(socket) returns true if the channel is now empty
	bool				leave(std::string nick);
	
	// MODES METHODS
	bool	mode_o(bool append, std::string nick, socket_t socket, const std::string &target);
	bool	mode_v(bool append, std::string nick, socket_t socket, const std::string &target);

	bool	mode_p(bool append, std::string nick, socket_t socket);
	bool	mode_s(bool append, std::string nick, socket_t socket);
	bool	mode_i(bool append, std::string nick, socket_t socket);
	bool	mode_t(bool append, std::string nick, socket_t socket);
	bool	mode_m(bool append, std::string nick, socket_t socket);
	bool	mode_l(bool append, std::string nick, socket_t socket, int limit);
	bool	mode_k(bool append, std::string nick, socket_t socket, const std::string &passwd);

	// exceptions
	class badName: public std::exception
	{
	private:
		std::string		_name;
		std::string		_reason;
		std::string		_errorMsg;
	public:
		badName(const std::string &name, const std::string &reason);
		virtual const char* what() const throw();
	};

};

#endif