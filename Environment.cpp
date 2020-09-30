/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:52:01 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/29 23:56:44 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Environment.hpp"
#include <string>
#include <sys/select.h>
#include <sys/resource.h>
#include <iostream>
#include "Client.hpp"

Environment::Environment() {
	struct rlimit	rlp;

	X(-1, getrlimit(RLIMIT_NOFILE, &rlp), "getrlimit");
	clients_num = rlp.rlim_cur;
	clients_fd = std::vector<Fd *>(clients_num);
	for (int i = 0; i < clients_num ; i++) {
		clients_fd[i] = new Fd();
	}
	cmd_count.emplace(std::pair<std::string, int>("PASS", 0));
	cmd_count.emplace(std::pair<std::string, int>("NICK", 0));
	cmd_count.emplace(std::pair<std::string, int>("USER", 0));
	cmd_count.emplace(std::pair<std::string, int>("OPER", 0));
	cmd_count.emplace(std::pair<std::string, int>("MODE", 0));
	cmd_count.emplace(std::pair<std::string, int>("SERVICE", 0));
	cmd_count.emplace(std::pair<std::string, int>("QUIT", 0));
	cmd_count.emplace(std::pair<std::string, int>("SQUIT", 0));
	cmd_count.emplace(std::pair<std::string, int>("JOIN", 0));
	cmd_count.emplace(std::pair<std::string, int>("PART", 0));
	cmd_count.emplace(std::pair<std::string, int>("TOPIC", 0));
	cmd_count.emplace(std::pair<std::string, int>("NAMES", 0));
	cmd_count.emplace(std::pair<std::string, int>("LIST", 0));
	cmd_count.emplace(std::pair<std::string, int>("INVITE", 0));
	cmd_count.emplace(std::pair<std::string, int>("KICK", 0));
	cmd_count.emplace(std::pair<std::string, int>("PRIVMSG", 0));
	cmd_count.emplace(std::pair<std::string, int>("NOTICE", 0));
	cmd_count.emplace(std::pair<std::string, int>("MOTD", 0));
	cmd_count.emplace(std::pair<std::string, int>("LUSERS", 0));
	cmd_count.emplace(std::pair<std::string, int>("VERSION", 0));
	cmd_count.emplace(std::pair<std::string, int>("STATS", 0));
	cmd_count.emplace(std::pair<std::string, int>("LINKS", 0));
	cmd_count.emplace(std::pair<std::string, int>("TIME", 0));
	cmd_count.emplace(std::pair<std::string, int>("CONNECT", 0));
	cmd_count.emplace(std::pair<std::string, int>("TRACE", 0));
	cmd_count.emplace(std::pair<std::string, int>("ADMIN", 0));
	cmd_count.emplace(std::pair<std::string, int>("INFO", 0));
	cmd_count.emplace(std::pair<std::string, int>("SERVLIST", 0));
	cmd_count.emplace(std::pair<std::string, int>("SQUERY", 0));
	cmd_count.emplace(std::pair<std::string, int>("WHO", 0));
	cmd_count.emplace(std::pair<std::string, int>("WHOIS", 0));
	cmd_count.emplace(std::pair<std::string, int>("WHOWAS", 0));
	cmd_count.emplace(std::pair<std::string, int>("KILL", 0));
	cmd_count.emplace(std::pair<std::string, int>("PING", 0));
	cmd_count.emplace(std::pair<std::string, int>("PONG", 0));
	cmd_count.emplace(std::pair<std::string, int>("ERROR", 0));
	cmd_count.emplace(std::pair<std::string, int>("AWAY", 0));
	cmd_count.emplace(std::pair<std::string, int>("REHASH", 0));
	cmd_count.emplace(std::pair<std::string, int>("DIE", 0));
	cmd_count.emplace(std::pair<std::string, int>("RESTART", 0));
	cmd_count.emplace(std::pair<std::string, int>("SUMMON", 0));
	cmd_count.emplace(std::pair<std::string, int>("USERS", 0));
	cmd_count.emplace(std::pair<std::string, int>("WALLOPS", 0));
	cmd_count.emplace(std::pair<std::string, int>("USERHOST", 0));
	cmd_count.emplace(std::pair<std::string, int>("ISON", 0));
	cmd_count.emplace(std::pair<std::string, int>("SERVER", 0));
	cmd_count.emplace(std::pair<std::string, int>("NJOIN", 0));

	channels = new ChannelMaster();
}

Environment::~Environment() {
	delete channels;
}

void Environment::init_fds() {
	int		i;
	
	i = 0;
	maxfd = 0;
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	while (i < clients_num) {
		if (clients_fd[i]->type != FD_FREE) {
			FD_SET(i, &fd_read);
			if (std::string(clients_fd[i]->buf_write).length() > 0)
				FD_SET(i, &fd_write);
			maxfd = MAX(maxfd, i);
		}
		i++;
	}
}

std::vector<Fd *>	Environment::search_history_nick(std::string nk) {
	std::vector<Fd *> buff;
	
	for (Fd *f : client_history) {
		Client *c = reinterpret_cast<Client *>(f);
		if (c->nick == nk || nk == "*") 
			buff.push_back(c);
	}
	return buff;
}

std::vector<Fd *>	Environment::search_list_nick(std::string nk) {
	std::vector<Fd *> buff;
	
	for (Fd *f : clients_fd) {
		if (f->type == FD_CLIENT) {
			Client *c = reinterpret_cast<Client *>(f);
			if (c->nick == nk || nk == "*") 
				buff.push_back(c);
		}
	}
	return buff;
}

std::vector<OtherServ *>	Environment::search_othersrv_nick(std::string nk) {
	std::vector<OtherServ *> buff;

	for (OtherServ *srv : otherServers) {
		for (std::string nick : srv->nicks) {
			if (nick == nk) {
				buff.push_back(srv);
			}
		}
	}
	return (buff);
}


std::vector<Fd *>	Environment::search_list_with_mode(std::string mask, std::string targ, char c) {
	(void)mask;
	(void)targ;
	std::vector<Fd *> buff;

	for (Fd *f : clients_fd) {
		if (f->type == FD_CLIENT) {
			Client *cl = reinterpret_cast<Client *>(f);
			if (c == 'o' && cl->o_mode) 
				buff.push_back(cl);
			if (c == 'i' && cl->i_mode) 
				buff.push_back(cl);
			if (c == 'w' && cl->w_mode) 
				buff.push_back(cl);
			if (c == 's' && cl->s_mode) 
				buff.push_back(cl);
		}
	}
	return buff;
}