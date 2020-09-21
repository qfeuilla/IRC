/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:51:25 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/22 00:57:58 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <fcntl.h>
#include <sstream>

bool		check_nick(std::string nk) {
	// TODO
	(void)nk;
	return true;
}

Client::Client(Environment *e, int s) : ev(e) {
	type = FD_CLIENT;
	is_setup = false;
	sock = s;
	creation = time(0);
	nick = "*";
	servername = "*";
}

Client::~Client() {
	std::cout << "client destructed" << std::endl;
}

// Only match the usage for hitory, Caution if you want to use for something else
Client::Client(const Client &old) {
	type = FD_CLIENT;
	creation = old.creation;
	last = time(0);
	pass = old.pass;
	nick = old.nick;
	username = old.username;
	hostname = old.hostname;
	servername = old.servername;
	realname = old.realname;
	ev = old.ev;
}

void	Client::PASS(Command *cmd) {
	std::string ms;

	if (!is_setup) {
		if (cmd->prefix.empty() && cmd->arguments.size() == 1) {
			pass = cmd->arguments[0];
			pass_set = true;
		} else {
			ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
			std::cout << ms;
			send(sock, ms.c_str(), ms.size(), 0);
		}
	}
	else {
		ms = reply_formating(servername.c_str(), ERR_ALREADYREGISTRED, {}, nick.c_str());
		std::cout << ms;
		send(sock, ms.c_str(), ms.size(), 0);
	}
}

void	Client::NICK(Command *cmd) {
	std::string	ms;

	if (cmd->arguments.size() >= 1) {
		if (check_nick(cmd->arguments[0])) {
			if (ev->search_list_nick(cmd->arguments[0]).empty()) {
				// * if client is setup so it is a nick change
				ms += ":";
				ms += nick;
				ms += " NICK :";
				ms += cmd->arguments[0];
				ms += CRLF;
				if (is_setup) {
					// * save old client for histoy purpose
					ev->client_history.push_back(new Client(*this));
					nick = cmd->arguments[0];
				} else {
					nick = cmd->arguments[0];
					nick_set = true;
				}
				std::cout << ms;
				send(sock, ms.c_str(), ms.length(), 0);
			} else {
				ms = reply_formating(servername.c_str(), ERR_NICKNAMEINUSE, {cmd->arguments[0]}, nick.c_str());
				std::cout << ms;
				send(sock, ms.c_str(), ms.size(), 0);
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_ERRONEUSNICKNAME, {cmd->arguments[0]}, nick.c_str());
			std::cout << ms;
			send(sock, ms.c_str(), ms.size(), 0);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NONICKNAMEGIVEN, {}, nick.c_str());
		std::cout << ms;
		send(sock, ms.c_str(), ms.size(), 0);
	}
}

void	Client::USER(Command *cmd) {
	std::string	ms;

	if (!is_setup) {
		if (cmd->prefix.empty() && cmd->arguments.size() >= 4) {
			username = cmd->arguments[0];
			hostname = cmd->arguments[1];
			servername = cmd->arguments[2];
			for (size_t i = 3; i < cmd->arguments.size() - 1; i++) {
				realname += cmd->arguments[i];
				realname += " ";
			}
			realname += cmd->arguments[cmd->arguments.size() - 1];
			realname = std::string(&realname[1], &realname[realname.length()]);
			is_setup = true;
		} else {
			ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
			std::cout << ms;
			send(sock, ms.c_str(), ms.size(), 0);
		}
	}
	else {
		ms = reply_formating(servername.c_str(), ERR_ALREADYREGISTRED, {}, nick.c_str());
		std::cout << ms;
		send(sock, ms.c_str(), ms.size(), 0);
	}
}

int		Client::execute_parsed(Command *parsed) {
	switch (parsed->cmd_code())
	{
	case PASS_CC:
		PASS(parsed);
		std::cout << *this << std::endl;
		break;
	case NICK_CC:
		NICK(parsed);
		std::cout << *this << std::endl;
		break;
	case USER_CC:
		USER(parsed);
		std::cout << *this << std::endl;
		break;
	default:
		break;
	}
	return (0);
}

void	Client::read_func() {
	int			r;
	std::string resp;
	std::string resps;
	std::string dt;
	Command		*parsed;

	fcntl(sock, F_SETFL, O_NONBLOCK);
	memset(&buf_read, 0, BUF_SIZE + 1);
	r = recv(sock, &buf_read, BUF_SIZE, 0);
	std::string line;
	std::string rd = std::string(buf_read);
	std::istringstream iss(rd);
	last = time(0);
	while (std::getline(iss, line) && !line.empty()) {
		parsed = parse(line);
		std::cout << *parsed << std::endl;
		execute_parsed(parsed);
    }
}

void	Client::write_func() { }

std::ostream &			operator<<( std::ostream & o, Client const & cl ) {
	o << "Pass :" << cl.pass << "\n";
	o << "Nickname :" << cl.nick << "\n";
	o << "UserName :" << cl.username << "\n";
	o << "HostName :" << cl.hostname << "\n";
	o << "ServerName :" << cl.servername << "\n";
	o << "RealName :" << cl.realname << "\n";
	return (o);
}