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
	struct sockaddr_in	serv_addr;
	int sock_serv;
	std::string			ms;

	if ((sock_serv = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socker creation error\n");
		return (EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6667);
	
	// convert address from IPv4 text to binary inside the adress struct
	if (inet_pton(serv_addr.sin_family, "195.154.200.232", &serv_addr.sin_addr) <= 0) {
		perror("invalid address, or not supported\n");
		return (EXIT_FAILURE);
	}

	if (connect(sock_serv, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
		perror("Connection to the server failed\n");
		return(EXIT_FAILURE);
	}

	int			r;
	send(sock_serv, ms.c_str(), ms.size(), 0);
	ms = "SERVER 88.120.198.47 1 :test serv ft irc";
	ms += CRLF;
	send(sock_serv, ms.c_str(), ms.size(), 0);

	std::cerr << "READ OTHER SERVER : " << std::endl;
	
	while (true) {
		memset(&buf_read, 0, BUF_SIZE + 1);
		r = recv(sock_serv, &buf_read, BUF_SIZE, 0);
		std::string tmp;
		tmp = std::string(buf_read);
		tmp = std::string(&tmp[0], &tmp[tmp.size() - 2]);
		std::cerr << tmp << std::endl;
	}

	delete ev->clients_fd[sock_serv];
	ev->clients_fd[sock_serv] = new OtherServ(sock_serv);

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
			std::cerr << "Error on link server info";
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
		perror("socket opt change failed\n");
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
