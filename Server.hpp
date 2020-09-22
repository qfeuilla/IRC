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
#include <string>

class Server : public Fd {
public:
	Server();
	Server(int, char**);

	~Server();

	virtual void	read_func();
	virtual void	write_func();
	void			accept_srv();

	std::string		generate_servname(std::string);

	void			load_options(int, char **);
	void			create();

	void			init_fd();
	void			serv_select();
	void			do_actions();

private:
	int					port;
	int					action;
	Environment			*ev;
};

#endif