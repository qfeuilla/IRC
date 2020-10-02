/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:37:26 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 19:05:16 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <stdlib.h>
#include <iostream>
#include <netdb.h>
#include <sys/select.h>
#include <fcntl.h>
#include <string.h>

Server::~Server() {
	std::cout << "server destructed" << std::endl;
}

Server::Server() {
	type = FD_SERVER;
	ev = new Environment();
	ev->password = new std::string("");
	time(&ev->start);
	ev->accept_operators = true;
	ev->version = new std::string("ft_irc_0.4.2b");
	ev->loc1 = std::string("Paris, France");
	ev->loc2 = std::string("42born2code");
	ev->emails.push_back("quentin.feuillade33@gmail.com");
	ev->emails.push_back("m.lemoniesdesagazan@gmail.com");
}

bool			Server::load_other_servs(std::string servinfo) {
	(void)servinfo;
	// 1 : connect
	// 2 : send SERVER message
	// 3 : stock the socket as a OtherServ
	unsigned int		porti;
	std::string			addr;
	std::string			pass;
	unsigned int		last = 0;
	int					tp = 0;
	std::string			ms;
	int					_sock = 0;
	struct sockaddr_in	serv_addr;
	
	for (size_t i = 0; i < servinfo.length(); i++) {
		if (servinfo[i] == ':') {
			switch (tp)
			{
			case 0:
				addr = std::string(&servinfo[0], &servinfo[i]);
				break;
			case 1:
				porti = std::atoi(std::string(&servinfo[last], &servinfo[i]).c_str());
				break;
			default:
				break;
			}
			last = i + 1;
			tp += 1;
		}
	}
	pass = std::string(&servinfo[last], &servinfo[servinfo.length()]);
	if (tp != 2) {
		return (false);
	}

	if ((_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return (false);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(porti);

	if (inet_pton(serv_addr.sin_family, addr.c_str(), &serv_addr.sin_addr) <= 0) {
		return (false);
	}

	if (connect(_sock, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
		return (false);
	}

	ms += "SERVER ";
	ms += inet_ntoa(ev->sin.sin_addr);
	ms += " ";
	ms += "1";
	ms += " ";
	ms += "42";
	ms += " ";
	ms += std::to_string(port);
	ms += " ";
	ms += pass;
	ms += CRLF;
	send(_sock, ms.c_str(), ms.length(), 0);

	OtherServ *other = new OtherServ(_sock, false, ev);
	other->name = addr;
	other->hop_count = 1;
	other->token = 42;
	other->info = "";
	other->port = std::to_string(porti);
	delete ev->clients_fd[_sock];
	ev->clients_fd[_sock] = other;
	std::cerr << "Fd adding Ok" << std::endl;
	ev->otherServers.push_back(other);
	std::cerr << "OtherServ adding Ok" << std::endl;
	return (true);
}

void		Server::load_options(int ac, char **av) {
	if (ac == 3) {
		port = std::atoi(av[1]);
		delete ev->password;
		ev->password = new std::string(av[2]);
		create();
	} else if (ac == 4) {
		port = std::atoi(av[2]);
		delete ev->password;
		ev->password = new std::string(av[3]);
		create();
		if (!load_other_servs(av[1])) {
			std::cerr << "Error on link server info" << std::endl;
			std::cerr << "Usage: " << av[0] <<"[host:port:password] <port> <password>" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else {
		std::cerr << "Usage: " << av[0] <<"[host:port:password] <port> <password>" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void		Server::create() {
	struct sockaddr_in	sin;
	struct protoent		*pe;
	int					opt = 1;
	
	pe = reinterpret_cast<struct protoent*>(Xv(NULL, getprotobyname("tcp"), "getprotobyname"));
	sock = X(-1, socket(PF_INET, SOCK_STREAM, pe->p_proto), "socket");
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		std::cout << "socket opt change failed\n";
		exit (EXIT_FAILURE);
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	X(-1, bind(sock, (struct sockaddr*)&sin, sizeof(sin)), "bind");
	X(-1, listen(sock, 42), "listen");
	ev->clients_fd[sock] = this;
	ev->sin = sin;
	ev->serv = new std::string(inet_ntoa(sin.sin_addr));
	ev->channels->setSrvName(*(ev->serv));
}

void		Server::accept_srv() {
	int					cs;
	struct sockaddr_in	csin;
	socklen_t			csin_len;

	csin_len = sizeof(csin);
	cs = X(-1, accept(sock, (struct sockaddr*)&csin, &csin_len), "accept");
	std::cout << "New client fd:" << cs << " from " 
		<< inet_ntoa(csin.sin_addr) << ":"
		<< ntohs(csin.sin_port) << std::endl;
	delete ev->clients_fd[cs];
	ev->clients_fd[cs] = new Client(ev, cs, csin);
}

void		Server::read_func() {
	accept_srv();
}

void		Server::write_func() { 
}

void	Server::init_fd() {
	ev->init_fds();
}

void		Server::serv_select() {
	action = select(ev->maxfd + 1, &(ev->fd_read), &(ev->fd_write), NULL, NULL);
}

void		Server::do_actions() {
	int	i;

	i = 0;
	while ((i < ev->clients_num) && (action > 0))
	{
		if (FD_ISSET(i, &ev->fd_read))
			ev->clients_fd[i]->read_func();
		if (FD_ISSET(i, &ev->fd_write))
			ev->clients_fd[i]->write_func();
		if (FD_ISSET(i, &ev->fd_read) ||
			FD_ISSET(i, &ev->fd_write))
			action--;
		i++;
	}
}

bool		Server::active() {
	return (ev->active);
}