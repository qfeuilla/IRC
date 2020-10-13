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
#include <openssl/ssl.h>
#include <openssl/err.h>

class Server : public Fd {
public:
	Server();
	Server(int, char**);
	Server(Environment *, int);

	~Server();

	virtual void	read_func();
	virtual void	write_func();
	void			accept_srv();

	bool			load_other_servs(std::string);

	void			load_options(int, char **);
	void			create();

	void			init_fd();
	void			serv_select();
	void			do_actions();

	bool			active();

	std::string		getIP() const;

	Environment			*ev;
private:
	SSL_CTX				*ctx = nullptr;
	int					port;
	int					action;
	Server				*tmp;
};

#endif