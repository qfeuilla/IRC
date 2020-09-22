#ifndef CHANNEL_H
#define CHANNEL_H

#include <iostream>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>

class Channel
{
public:
	typedef	int			socket_t;
private:
	class _Chan_modes
	{
	public:
		std::map<Channel::socket_t, std::string>	o; // Operator privileges
		std::map<Channel::socket_t, std::string>	v; // Ability to speak on a moderated channel
		bool		p; // Private channel
		bool		s; // Secret channel
		bool		i; // Users can't join without invite
		bool		t; // Topic can only be set by an operator
		bool		m; // Only voiced users and operators can talk
		int			l; // User limit
		std::string	k; // Channel password
		_Chan_modes(): o(), v(), p(false), s(false), i(false), t(false), m(false), l(-1), k() {}
		_Chan_modes &operator=(const _Chan_modes &other)
		{
			o = other.o;
			v = other.v;
			p = other.p;
			s = other.s;
			i = other.i;
			t = other.t;
			m = other.m;
			l = other.l;
			k = other.k;
			return (*this);
		}
	};
	
	typedef	std::map<socket_t, _Chan_modes*>	_users_map;
	
	std::string							_name;
	std::map<socket_t, _Chan_modes*>	_users;

	void	_print_channel(void) {
		_users_map::iterator	current = _users.begin();
		_users_map::iterator	end = _users.end();

		std::cout << "sockets in " << _name << "\n";
		while (current != end) {
			std::cout << "fd num = " << (*current).first << ", "; 
			++current;
		}
		std::cout << "\n\n";
	}
public:
	Channel(const std::string &name = std::string());
	~Channel();
	Channel	&operator=(const Channel& other);

	const std::string	&getName() const;

	static void			sendMsgToSocket(socket_t socket, const std::string &msg);

	// *	join(socket) returns true on succes (false if socket was already in the channel before the call)
	bool				join(socket_t socket);
	// *	leave(socket) returns true if the channel is now empty
	bool				leave(socket_t socket);

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