/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:50:59 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/22 00:52:03 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Fd.hpp"
# include "Environment.hpp"

class Client : public Fd {
public:
	Client(Environment *, int);
	~Client();

	Client(const Client &);

	virtual void	read_func();
	virtual void	write_func();

	// * Functions Client
	void			PASS(Command *);
	void			NICK(Command *);
	void			USER(Command *);

	int				execute_parsed(Command *);

	time_t				creation;
	time_t				last;

	std::string			pass;
	std::string			nick;
	std::string			username;
	std::string			hostname;
	std::string			servername;
	std::string			realname;

private:
	Environment			*ev;
	bool				pass_set;
	bool				nick_set;
	bool				is_setup;
	
};

std::ostream &			operator<<( std::ostream & o, Client const & i );

#endif