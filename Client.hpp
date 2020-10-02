/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:50:59 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/02 13:20:50 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Fd.hpp"
# include "Environment.hpp"
# include "Channel.hpp"
# include <list>

class Environment;
class Channel;

class Client : public Fd {
public:
	Client(Environment *, int, struct sockaddr_in);
	// Constructor for multi serv
	Client(std::string);
	~Client();

	Client(const Client &);

	virtual void		read_func();
	virtual void		write_func();

	// * Functions Client
	void			PASS(Command *);
	void			NICK(Command *);
	void			USER(Command *);
	void			OPER(Command *);
	void			MODE(Command *);
	void			QUIT(Command *);
	void			PRIVMSG(Command *);
	void			NOTICE(Command *);
	void			MOTD(Command *);
	void			LUSERS(Command *);
	void			VERSION(Command *);
	void			STATS(Command *);
	void			LINKS(Command *);
	void			TIME(Command *);
	void			ADMIN(Command *);
	void			INFO(Command *);
	void			WHO(Command *);
	void			WHOIS(Command *);
	void			WHOWAS(Command *);
	void			KILL(Command *);
	void			PING(Command *);
	void			AWAY(Command *);
	void			DIE(Command *);
	void			SUMMON(Command *);
	void			USERS(Command *);
	void			WALLOPS(Command *);
	void			USERHOST(Command *);
	void			ISON(Command *);
	void			JOIN(Command *);
	void			PART(Command *);
	void			KICK(Command *);
	void			TOPIC(Command *);
	void			INVITE(Command *);
	void			SERVER(Command *);
	void			TRACE(Command *);

	int				execute_parsed(Command *);

	void			exec_registerMS();
	std::string		get_userMODEs_ms(bool);
	bool			set_uMODE(char c, bool add);

	Client			*getOtherClient(const std::string &name);

	void			share_Client(int socket);

	time_t				creation;
	time_t				last;

	std::string			pass;
	std::string			nick;
	std::string			username;
	std::string			hostname;
	std::string			servername;
	std::string			realname;

	bool				i_mode;
	bool				o_mode;
	bool				w_mode;
	bool				s_mode;
	
	struct sockaddr_in	csin;

	bool				is_away = false;
	std::string			away_ms;

	int					recv_ms = 0;
	int					send_ms = 0;
	int					sendq = 0;
	size_t				Kb_sent = 0;
	size_t				Kb_recv = 0;
	
	std::list<Channel*>	channels;

	bool				nick_set;

private:
	Environment			*ev;
	bool				pass_set;
	bool				is_setup;
};

std::ostream &			operator<<( std::ostream & o, Client const & i );

#endif