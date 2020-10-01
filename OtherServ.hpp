/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/02 00:40:01 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OTHERSERV_HPP
# define OTHERSERV_HPP

#include "Client.hpp"
#include "ft_irc.hpp"

class Environment;
class Client;

class OtherServ: public Fd {
public:
	OtherServ(int, bool, Environment *);
	OtherServ(const OtherServ &);

	~OtherServ();

	void			NICK(Command *);
	void			QUIT(Command *);
	void			PRIVMSG(Command *);
	void			NOTICE(Command *);
	void			USER(Command *);
	void			MODE(Command *);
	void			AWAY(Command *);
	void			TIME(Command *);
	void			ADDS(Command *);
	void			NSERV(Command *);
	void			DELS(Command *);
	void			KILL(Command *);

	virtual void	read_func();
	virtual void	write_func();

	std::vector<Client *>::iterator	search_nick(std::string);
	std::vector<Client *>::iterator	search_history_nick(std::string);
	std::vector<Client *>			search_list_with_mode(char);

	int				execute_parsed(Command *);

	bool			change_nick(std::string, std::string);

	std::string				name;
	std::string				port;
	int						hop_count = 1;
	unsigned int			token;
	std::string				info;
	std::vector<Client *>	clients;
	std::vector<Client *>	clients_history;
	int						connected = 1;
	
	int						recv_ms = 0;
	int						send_ms = 0;
	int						sendq = 0;
	size_t					Kb_sent = 0;
	size_t					Kb_recv = 0;

	time_t					creation;

private :
	Environment		*ev;
};

#endif