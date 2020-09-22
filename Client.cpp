/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:51:25 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/22 15:21:48 by qfeuilla         ###   ########.fr       */
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
	if (nk.length() > 9)
		return (false);
	if (!(is_special(nk[0]) || std::isalpha(nk[0])))
		return (false);
	for (char c : nk) {
		if (!(is_special(c) || std::isalnum(c)
				|| c == '-'))
			return (false);
	}
	return (true);
}

Client::Client(Environment *e, int s) : ev(e) {
	type = FD_CLIENT;
	is_setup = false;
	sock = s;
	creation = time(0);
	nick = "NA";
	i_mode = false;
	o_mode = false;
	s_mode = false;
	w_mode = false;
	servername = SERV_NAME;
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
				if (is_setup) {
					ms += ":";
					ms += nick;
					ms += " NICK :";
					ms += cmd->arguments[0];
					ms += CRLF;
					
					// * save old client for histoy purpose
					ev->client_history.push_back(new Client(*this));
					nick = cmd->arguments[0];

					std::cout << ms;
					send(sock, ms.c_str(), ms.length(), 0);
				} else {
					nick = cmd->arguments[0];
					nick_set = true;
				}
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

void	Client::exec_registerMS() {
	std::string tmp;

	tmp += ":";
	tmp += nick;
	tmp += " MODE ";
	tmp += nick;
	tmp += " +i";
	tmp += CR;
	execute_parsed(parse(tmp));
	// TODO : need more functions (i will do)
}

void	Client::USER(Command *cmd) {
	std::string	ms;

	if (!is_setup) {
		if (cmd->prefix.empty() && cmd->arguments.size() >= 4) {
			username = cmd->arguments[0];
			hostname = cmd->arguments[1];
			if (cmd->arguments[2] != "*")
				servername = cmd->arguments[2];
			for (size_t i = 3; i < cmd->arguments.size() - 1; i++) {
				realname += cmd->arguments[i];
				realname += " ";
			}
			realname += cmd->arguments[cmd->arguments.size() - 1];
			realname = std::string(&realname[1], &realname[realname.length()]);
			is_setup = true;
			exec_registerMS();
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

void	Client::OPER(Command *cmd) {
	std::string ms;

	if (cmd->arguments.size() >= 2) {
		if (ev->accept_operators) {
			if (*ev->password == cmd->arguments[1] 
				&& *ev->username_oper == cmd->arguments[0]) {
					o_mode = true;
					ms = reply_formating(servername.c_str(), RPL_YOUREOPER, {}, nick.c_str());
					std::cout << ms;
					send(sock, ms.c_str(), ms.size(), 0);
			} else {
				ms = reply_formating(servername.c_str(), ERR_PASSWDMISMATCH, {}, nick.c_str());
				std::cout << ms;
				send(sock, ms.c_str(), ms.size(), 0);
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_NOOPERHOST, {}, nick.c_str());
			std::cout << ms;
			send(sock, ms.c_str(), ms.size(), 0);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
		std::cout << ms;
		send(sock, ms.c_str(), ms.size(), 0);
	}
	
}

std::string		Client::get_userMODEs_ms() {
	std::string	ms;

	if (w_mode || o_mode || i_mode || s_mode)
		ms += "+";
	if (w_mode)
		ms += "w";
	if (o_mode)
		ms += "o";
	if (i_mode)
		ms += "i";
	if (s_mode)
		ms += "s";
	ms = reply_formating(servername.c_str(), RPL_UMODEIS, {ms}, nick.c_str());
	return (ms);
}

bool	Client::set_uMODE(char m, bool add) {
	if (m == 'i') {
		i_mode = (add ? true : false);
	} else if (m == 's') {
		s_mode = (add ? true : false);
	} else if (m == 'o') {
		o_mode = (add ? o_mode : false);
	} else if (m == 'w') {
		w_mode = (add ? true : false);
	} else
		return false;
	return true;
}

void	Client::MODE(Command *cmd) {
	std::string		ms;

	if (cmd->arguments.size() >= 2) {
		if (cmd->arguments[0][0] != '#') {
			if (cmd->arguments[0] == nick) {
				size_t	i = 0;
				bool	add = false;

				// * set flags
				for (; i < cmd->arguments[1].length() && i < 4; i++) {
					if (i == 0) {
						if (cmd->arguments[1][i] == '-')
							;
						else if (cmd->arguments[1][i] == '+')
							add = true;
						else
							break;
					} else if (!set_uMODE(cmd->arguments[1][i], add))
						break;
				}

				// * if mode propetly format
				if ((i == cmd->arguments[1].length() || i == 4) && cmd->arguments[1].length() > 1) {
					ms += ":";
					ms += nick;
					ms += " MODE ";
					ms += cmd->arguments[0];
					ms += " ";
					ms += cmd->arguments[1];
					ms += CRLF;
					std::cout << ms;
					send(sock, ms.c_str(), ms.size(), 0);
				} else {
					ms = reply_formating(servername.c_str(), ERR_UMODEUNKNOWNFLAG, {}, nick.c_str());
					std::cout << ms;
					send(sock, ms.c_str(), ms.size(), 0);
				}
			} else {
				ms = reply_formating(servername.c_str(), ERR_USERSDONTMATCH, {}, nick.c_str());
				std::cout << ms;
				send(sock, ms.c_str(), ms.size(), 0);
			}
		} else {
			// TODO : call the channell MODE with Client and Command as params if parmas[0] is a channel;
		}
	} else {
		if (cmd->arguments.size() == 1) {
			ms = get_userMODEs_ms();
			std::cout << ms;
			send(sock, ms.c_str(), ms.size(), 0);
		} else {
			ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
			std::cout << ms;
			send(sock, ms.c_str(), ms.size(), 0);
		}
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
	case OPER_CC:
		OPER(parsed);
		break;
	case MODE_CC:
		MODE(parsed);
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
	o << "Pass : " << cl.pass << "\n";
	o << "Nickname : " << cl.nick << "\n";
	o << "UserName : " << cl.username << "\n";
	o << "HostName : " << cl.hostname << "\n";
	o << "ServerName : " << cl.servername << "\n";
	o << "RealName : " << cl.realname << "\n";
	return (o);
}