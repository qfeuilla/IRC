/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:31:51 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 19:04:21 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Environment.hpp"
#include "Client.hpp"

class Server : public Fd {
public:
	Server();
	Server(int, char**);

	~Server();

	virtual void	read_func(int);
	virtual void	write_func(int);

	void	accept_srv(int);

	void	load_options(int, char **);
	void	create();

	void	init_fd();
	void	serv_select();
	void	do_actions();

private:
	int					port;
	int					action;
	Environment			*ev;
	int					s;
};

#endif