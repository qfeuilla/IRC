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
	/*
		Les modes disponibles sont :
		i - marque un utilisateur comme invisible ;
		s - marque un utilisateur comme recevant les notifications du serveur ;
		w - l'utilisateur reçoit les WALLOPs ;
		o - drapeau d'opérateur.
	*/
	class _User_modes
	{
	public:
		bool	i;
		bool	s;
		bool	w;
		bool	o;
		_User_modes(): i(false), s(false), w(false), o(false) {}
		_User_modes &operator=(const _User_modes &other)
		{
			i = other.i;
			s = other.s;
			w = other.w;
			o = other.o;
			return (*this);
		}
	};
	
	typedef	std::map<socket_t, _User_modes*>	_users_map;
	
	std::string							_name;
	std::map<socket_t, _User_modes*>	_users;

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