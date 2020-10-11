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
	SSL_CTX_free(ctx);
	close(sock);
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

Server::Server(Environment *e, int prt) {
	type = FD_SERVER;
	ev = e;
	port = prt;
}

SSL_CTX* InitCTX(void)
{   
	const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = SSLv23_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}


void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile, bool serv)
{
	if (serv) {
		//New lines 
		if (SSL_CTX_load_verify_locations(ctx, CertFile, KeyFile) != 1)
			ERR_print_errors_fp(stderr);

		if (SSL_CTX_set_default_verify_paths(ctx) != 1)
			ERR_print_errors_fp(stderr);
		//End new lines
	}

    /* set the local certificate from CertFile */
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
	/* set the private key from KeyFile (may be the same as CertFile) */
	if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* verify private key */
	if (!SSL_CTX_check_private_key(ctx))
	{
		fprintf(stderr, "Private key does not match the public certificate\n");
		abort();
	}

    /*
		//New lines Force the client-side have a certificate
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
		SSL_CTX_set_verify_depth(ctx, 4);
		//End new lines
	*/
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

	std::cout << "Setup ...." << std::endl;
	OtherServ *other = new OtherServ(_sock, ev, 0);
	other->name = addr;
	other->hop_count = 1;
	other->info = "";
	delete ev->clients_fd[_sock];
	ev->clients_fd[_sock] = other;
	std::cerr << "Fd adding Ok" << std::endl;
	ev->otherServers.push_back(other);
	std::cerr << "OtherServ adding Ok" << std::endl;
	ms = "PASS ";
	ms += pass;
	ms += CRLF;
	std::cout << ms << std::endl;
	send(_sock, ms.c_str(), ms.length(), 0);
	ms = "SERVER ";
	ms += *ev->serv;
	ms += " ";
	ms += "1";
	/*ms += " : ";
	ms += "42";
	ms += " ";
	ms += std::to_string(port);*/
	ms += CRLF;
	std::cout << ms << std::endl;
	send(_sock, ms.c_str(), ms.length(), 0);
	int i = 0;
	while (i < 10000000) i++;
	ms = "READY";
	ms += CRLF;
	send(_sock, ms.c_str(), ms.length(), 0);
	return (true);
}

void		Server::load_options(int ac, char **av) {
	if (ac == 3) {
		port = std::atoi(av[1]);
		ev->tls_port = port + 1;
		delete ev->password;
		ev->password = new std::string(av[2]);
		create();
	} else if (ac == 4) {
		port = std::atoi(av[2]);
		ev->tls_port = port + 1;
		delete ev->password;
		ev->password = new std::string(av[3]);
		create();
		if (!load_other_servs(av[1])) {
			std::cerr << "Error on link server info" << std::endl;
			std::cerr << "Usage: " << av[0] <<" [host:port:password] <port> <password>" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else {
		std::cerr << "Usage: " << av[0] <<" [host:port:password] <port> <password>" << std::endl;
		exit(EXIT_FAILURE);
	}
}


SSL_CTX* InitServerCTX(void) {   
	const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv23_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL ) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void		Server::create() {
	if (port == ev->tls_port) {
		std::cout << "asking passw for TLS certificate" << std::endl;
		char CertFile[] = "ft_irc.pem";
		char KeyFile[] = "ft_irc.key";

		SSL_library_init();

		ctx = InitServerCTX();
		LoadCertificates(ctx, CertFile, KeyFile, true);
	} else {
		tmp = new Server(ev, ev->tls_port);
	}

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
	std::cout << "sock : " << sock << std::endl;
	std::cout << "port : " << port << std::endl;
	delete ev->clients_fd[sock];
	ev->clients_fd[sock] = this;
	delete ev->serv;
	std::string tmp2("localhost.");
	tmp2 += std::to_string(port - 1);
	ev->serv = new std::string(tmp2);
	std::cout << "IP = " << *ev->serv << "\n";
	ev->channels->setSrvName(*(ev->serv));
	if (port != ev->tls_port)
		tmp->create();
}


void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}

void		Server::accept_srv() {
	int					cs;
	struct sockaddr_in	csin;
	socklen_t			csin_len;

	if (port == ev->tls_port)
		std::cout << "TLS connection attempt" << std::endl;
	csin_len = sizeof(csin);
	cs = X(-1, accept(sock, (struct sockaddr*)&csin, &csin_len), "accept");
	std::cout << "New client fd:" << cs << " from " 
		<< inet_ntoa(csin.sin_addr) << ":"
		<< ntohs(csin.sin_port) << std::endl;
	delete ev->clients_fd[cs];
	Client		*nw = new Client(ev, cs, csin);
	ev->clients_fd[cs] = nw;
	if (port == ev->tls_port) {
		SSL		*ssl;
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, cs);
		if ( SSL_accept(ssl) == -1 )
       		ERR_print_errors_fp(stderr);
		else {
			std::cout << "client connected" << std::endl;
			ShowCerts(ssl);
		}
		nw->ssl = ssl;
		nw->is_ssl = true;
	}
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

std::string	Server::getIP() const {
	char hostname[1024];
	std::string	ip;

	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	// std::cout << "Hostname: " << hostname << "\n";
	struct hostent* h;
	h = gethostbyname(hostname);
	if (!h)
		return ("");
	if (h->h_addrtype == AF_INET)
	{
		struct in_addr **address_list = (struct in_addr **)h->h_addr_list;
		if (!address_list[0])
			return ("");
		unsigned int numAddress = htonl((*(address_list[0])).s_addr);

		ip = std::to_string(((numAddress >> 24) & 0xFF));
		ip += "." + std::to_string((numAddress >> 16) & 0xFF);
		ip += "." + std::to_string((numAddress >> 8) & 0xFF);
		ip += "." + std::to_string((numAddress) & 0xFF);
		return (ip);
	}
	return ("");
}