/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/30 12:18:43 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OtherServ.hpp"
#include <iterator>

OtherServ::OtherServ(int socket, bool share_data, Environment *e) {
	std::string ms;
	
	sock = socket;
	type = FD_OTHER;
	ev = e;
	if (share_data) {
		// TODO : share all the Server data to the new server
		
		// * **NICKS
		for (Fd *f: ev->clients_fd) {
			if (f->type == FD_CLIENT) {
				Client *c = reinterpret_cast<Client *>(f);

				if (c->nick_set) {
					ms = "";
					ms += "NICK ";
					ms += c->nick;
					ms += CRLF;
					send(sock, ms.c_str(), ms.length(), 0);
				}
			}
		}
		for (std::string nk : nicks) {
			ms = "";
			ms += "NICK ";
			ms += nk;
			ms += CRLF;
			send(sock, ms.c_str(), ms.length(), 0);
		}
	}
}

OtherServ::OtherServ(const OtherServ &cpy) {
	sock = cpy.sock;
	name = cpy.name;
	hop_count = cpy.hop_count;
	token = cpy.token;
	info = cpy.info;
}

OtherServ::~OtherServ() { }

void	OtherServ::NICK(Command *cmd) {
	std::string ms = cmd->line;

	ms += CRLF;
	if (cmd->prefix.empty()) {
		nicks.push_back(cmd->arguments[0]);
	} else {
		if (change_nick(cmd->prefix, cmd->arguments[0])) {
		} else {
			nicks.push_back(cmd->arguments[0]);
		}
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			send(sock, ms.c_str(), ms.length(), 0);
		}
	}
}

void	OtherServ::QUIT(Command *cmd) {
	std::string ms = cmd->line;

	ms += CRLF;

	std::vector<std::string>::iterator it = std::find(nicks.begin(), nicks.end(), cmd->prefix);
	nicks.erase(it);
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			send(sock, ms.c_str(), ms.length(), 0);
		}
	}
}

void	OtherServ::PRIVMSG(Command *cmd) {
	std::string ms;
	Client						*c;
	std::vector<Fd *>			tmpc;
	std::vector<OtherServ *>	tmpo;
	std::string					targ;

	targ = cmd->arguments[0];
	if (targ[0] == '#') {
		// TODO : channel
	} else {
		ms = ":";
		ms += cmd->prefix;
		ms += " PRIVMSG ";
		for (std::string tmp : cmd->arguments) {
			ms += tmp;
			ms += " ";
		}
		ms += CRLF;
		if (!(tmpc = ev->search_list_nick(cmd->arguments[0])).empty()) {
			c = reinterpret_cast<Client *>(tmpc[0]);
			custom_send(ms, c);
		} else if (!(tmpo = ev->search_othersrv_nick(cmd->arguments[0])).empty()) {
			send(tmpo[0]->sock, ms.c_str(), ms.length(), 0);
		}
		std::cout << ms << std::endl;
	}
}

void	OtherServ::NOTICE(Command *cmd) {
	std::string ms;
	Client						*c;
	std::vector<Fd *>			tmpc;
	std::vector<OtherServ *>	tmpo;
	std::string					targ;

	targ = cmd->arguments[0];
	if (targ[0] == '#') {
		// TODO : channel
	} else {
		ms = ":";
		ms += cmd->prefix;
		ms += " NOTICE ";
		for (std::string tmp : cmd->arguments) {
			ms += tmp;
			ms += " ";
		}
		ms += CRLF;
		if (!(tmpc = ev->search_list_nick(cmd->arguments[0])).empty()) {
			c = reinterpret_cast<Client *>(tmpc[0]);
			custom_send(ms, c);
		} else if (!(tmpo = ev->search_othersrv_nick(cmd->arguments[0])).empty()) {
			send(tmpo[0]->sock, ms.c_str(), ms.length(), 0);
		}
		std::cout << ms << std::endl;
	}
}
int		OtherServ::execute_parsed(Command *parsed) {
	switch (parsed->cmd_code()) {
	case NICK_CC:
		NICK(parsed);
		break;
	case QUIT_CC:
		QUIT(parsed);
		break;
	case PRIVMSG_CC:
		PRIVMSG(parsed);
		break;
	case NOTICE_CC:
		NOTICE(parsed);
		break;
	default:
		break;
	}
	return (0);
}

void	OtherServ::read_func() { 
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
	if (rd.empty()) {
		delete ev->clients_fd[sock];
		ev->clients_fd[sock] = new Fd();
		close(sock);
		std::cerr << "Other serv quit" << std::endl;
	} else {
		std::istringstream iss(rd);
		while (std::getline(iss, line) && !line.empty()) {
			parsed = parse(line);
			std::cout << *parsed << std::endl;
			execute_parsed(parsed);
		}
		std::cout << "Nick list : " << std::endl;
		for (std::string tmp : nicks) {
			std::cout << "- " << tmp << std::endl;
		}
	}
}

void	OtherServ::write_func() { }

bool	OtherServ::change_nick(std::string old, std::string nw) {
	for (size_t i = 0; i < nicks.size(); i++) {
		if (nicks[i] == old) {
			nicks[i] = nw;
			return (true);
		}
	}
	return (false); 
}