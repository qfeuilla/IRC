/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/10 22:57:38 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OTHERSERV_HPP
# define OTHERSERV_HPP

#include "ft_irc.hpp"
#include "Client.hpp"
#include "ChannelMaster.hpp"
#include "utils.hpp"

class Channel;
class Environment;
class Client;

class OtherServ: public Fd {
public:
	OtherServ(int, Environment *, std::string);
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
	void			TRACE(Command *);
	void			TRACEUP(Command *);
	void			SQUIT(Command *);
	void			READY(Command *);

	void			CHAN_CHG(Command *);
	void			CHAN_RPL(Command *);
	void			JOIN(Command *);
	void			PART(Command *);
	void			KICK(Command *);
	void			TOPIC(Command *);
	void			INVITE(Command *);
	void			NAMES(Command *);


	void			chanModes(Command *);

	virtual void	read_func();
	virtual void	write_func();

	std::vector<Client *>::iterator	search_nick(std::string);
	std::vector<Client *>::iterator	search_history_nick(std::string);
	std::vector<Client *>			search_list_with_mode(char);

	int				execute_parsed(Command *);

	bool			change_nick(std::string, std::string);

	std::string				name;
	std::string				port;
	int						porti;
	int						hop_count = 1;
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
	
	std::vector<Chan>		chans;
	std::vector<Chan>::iterator	getChan(const std::string &name);
	bool	chanWHO(Client *client, const std::vector<std::string> &args);
	void	sendPartMessage(Chan &chan, const std::string &nickName);
private :
	Environment		*ev;

	std::string		_stream;
};

#endif