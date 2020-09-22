/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:50:59 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/22 22:53:17 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Fd.hpp"
# include "Environment.hpp"

class Client : public Fd {
public:
	Client(Environment *, int, struct sockaddr_in);
	~Client();

	Client(const Client &);

	virtual void		read_func();
	virtual void		write_func();

	// * Functions Client
	void				PASS(Command *);
	void				NICK(Command *);
	void				USER(Command *);
	void				OPER(Command *);
	void				MODE(Command *);
	void				QUIT(Command *);
	void				PRIVMSG(Command *);

	int					execute_parsed(Command *);

	void				exec_registerMS();
	std::string			get_userMODEs_ms();
	bool				set_uMODE(char c, bool add);

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

private:
	Environment			*ev;
	bool				pass_set;
	bool				nick_set;
	bool				is_setup;
	
};

std::ostream &			operator<<( std::ostream & o, Client const & i );

#endif