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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

Server::~Server() {
	std::cout << "server destructed" << std::endl;
}

Server::Server() {
	type = FD_SERV;
	ev = new Environment();
}

Server::Server(int ac, char **av) {
	type = FD_SERV;
	ev = new Environment();
	load_options(ac, av);
}

void		Server::load_options(int ac, char **av) {
	if (ac != 2) {
		std::cerr << "Usage: " << av[0] <<" port" << std::endl;
		exit(EXIT_FAILURE);
	}
	port = std::atoi(av[1]);
}

void		Server::create() {
	struct sockaddr_in	sin;
	struct protoent	*pe;
	int			opt = 1;
	
	pe = reinterpret_cast<struct protoent*>(Xv(NULL, getprotobyname("tcp"), "getprotobyname"));
	s = X(-1, socket(PF_INET, SOCK_STREAM, pe->p_proto), "socket");
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("socket opt change failed\n");
		exit (EXIT_FAILURE);
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	X(-1, bind(s, (struct sockaddr*)&sin, sizeof(sin)), "bind");
	X(-1, listen(s, 42), "listen");
	ev->clients_fd[s] = this;
}

void		Server::accept_srv(int socket) {
	int					cs;
	struct sockaddr_in	csin;
	socklen_t			csin_len;

	csin_len = sizeof(csin);
	cs = X(-1, accept(socket, (struct sockaddr*)&csin, &csin_len), "accept");
	std::cout << "New client fd:" << cs << " from " 
		<< inet_ntoa(csin.sin_addr) << ":" 
		<< ntohs(csin.sin_port) << std::endl;
	
	delete ev->clients_fd[cs];
	ev->clients_fd[cs] = new Client(ev);
}

void		Server::read_func(int socket) {
	accept_srv(socket);
}

void		Server::write_func(int socker) { }

void	
	Server::init_fd() {
	ev->clients_fd[s] = this;
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
			ev->clients_fd[i]->read_func(i);
		if (FD_ISSET(i, &ev->fd_write))
			ev->clients_fd[i]->write_func(i);
		if (FD_ISSET(i, &ev->fd_read) ||
			FD_ISSET(i, &ev->fd_write))
			action--;
		i++;
	}
}