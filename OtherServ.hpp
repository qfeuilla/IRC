/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/30 00:36:17 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OTHERSERV_HPP
# define OTHERSERV_HPP

#include "Client.hpp"
#include "ft_irc.hpp"

class Environment;

class OtherServ: public Fd {
public:
	OtherServ(int, bool, Environment *);
	OtherServ(const OtherServ &);

	~OtherServ();

	void			NICK(Command *);
	void			QUIT(Command *);
	void			PRIVMSG(Command *);
	void			NOTICE(Command *);

	virtual void	read_func();
	virtual void	write_func();

	int				execute_parsed(Command *);

	bool			change_nick(std::string, std::string);

	std::string		name;
	int				hop_count = 1;
	unsigned int	token;
	std::string		info;
	std::vector<std::string>	nicks;
private :
	Environment		*ev;
};

#endif