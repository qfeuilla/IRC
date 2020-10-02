/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/02 15:04:51 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OtherServ.hpp"
#include <iterator>

bool		custom_send(std::string ms, OtherServ *s) {
	s->recv_ms += 1;
	s->Kb_recv += sizeof(ms);
	send(s->sock, ms.c_str(), ms.length(), 0);
	return (true);
}

OtherServ::OtherServ(int socket, bool share_data, Environment *e): _stream() {
	std::string ms;
	
	creation = time(NULL);
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
					c->share_Client(sock);
				}
			}
		}
		for (Fd *f: ev->client_history) {
			Client *c = reinterpret_cast<Client *>(f);

			if (c->nick_set) {
				c->share_Client(sock);
				ms = ":";
				ms += c->nick;
				ms += " QUIT";
				ms += CRLF;
				custom_send(ms, this);
			}
		}
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Client *c : sv->clients) {
					c->share_Client(sock);
				}
				for (Client *c : sv->clients_history) {
					c->share_Client(sock);
					ms = ":";
					ms += c->nick;
					ms += " QUIT";
					ms += CRLF;
					custom_send(ms, this);
				}
			}	
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
		clients.push_back(new Client(cmd->arguments[0]));
	} else {
		if (change_nick(cmd->prefix, cmd->arguments[0])) {
		} else {
			clients.push_back(new Client(cmd->arguments[0]));
		}
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::QUIT(Command *cmd) {
	std::string ms = cmd->line;
	ms += CRLF;
	std::vector<Client *>::iterator c;

	c = search_nick(cmd->prefix);
	if (c != clients.end()) {
		if (cmd->arguments.size() >= 1) {
			(*c)->last = time(NULL);
		}
		clients_history.push_back(*c);
		clients.erase(c);
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
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
			custom_send(ms, tmpo[0]);
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
			custom_send(ms, tmpo[0]);
		}
		std::cout << ms << std::endl;
	}
}

void	OtherServ::USER(Command *cmd) {
	std::string ms;
	Client		*c;
	
	c = *search_nick(cmd->prefix);
	c->username = cmd->arguments[0];
	c->hostname = cmd->arguments[1];
	c->servername = cmd->arguments[2];
	for (size_t i = 3; i < cmd->arguments.size() - 1; i++) {
		c->realname += cmd->arguments[i];
		c->realname += " ";
	}
	c->realname += cmd->arguments[cmd->arguments.size() - 1];
	c->realname = std::string(&c->realname[1], &c->realname[c->realname.length()]);
	ms = cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::MODE(Command *cmd) {
	std::string ms;
	Client		*c;
	
	c = *search_nick(cmd->prefix);
	c->i_mode = false;
	c->o_mode = false;
	c->w_mode = false;
	c->s_mode = false;
	if (cmd->arguments.size() >= 1) {
		for (char m : cmd->arguments[0]) {
			if (m == 'o')
				c->o_mode = true;
			else if (m == 'i')
				c->i_mode = true;
			else if (m == 's')
				c->s_mode = true;
			else if (m == 'w')
				c->w_mode = true;
		}
	}

	std::cout << c->get_userMODEs_ms(false) << std::endl;
	ms = cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::AWAY(Command *cmd) {
	std::string ms;
	Client		*c;
	
	c = *search_nick(cmd->prefix);
	if (cmd->arguments.size() >= 1) {
		for (std::string tmp : cmd->arguments) {
			ms += tmp;
			ms += " ";
		}
		c->away_ms = ms;
		c->is_away = true;
		std::cout << "Away with message : " << c->away_ms << std::endl;
	} else {
		c->is_away = false;
		std::cout << "Not Away" << std::endl;
	}

	ms = cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::TIME(Command *cmd) {
	std::string ms;
	Client		*c;
	
	c = *search_nick(cmd->prefix);
	c->creation = std::strtoll(cmd->arguments[0].c_str(), NULL, 10);
	c->last = std::strtoll(cmd->arguments[1].c_str(), NULL, 10);

	ms = cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::ADDS(Command *cmd) {
	std::string ms;
	
	connected += 1;
	ms += cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::DELS(Command *cmd) {
	std::string ms;
	
	// TODO : adding deletion of client with server and host matching arg 1 and 2
	connected -= std::atoi(cmd->arguments[0].c_str());
	ms = cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::NSERV(Command *cmd) {
	std::string ms;
	int			tmp = 0;

	for (OtherServ *sv : ev->otherServers) {
		tmp += sv->connected;
	}
	connected = std::atoi(cmd->arguments[0].c_str());
}

void	OtherServ::KILL(Command *cmd) {
	std::string ms;
	std::vector<Client *>::iterator c;
	std::vector<Fd *> tmp;

	c = search_nick(cmd->prefix);
	if (c != clients.end()) {
		(*c)->last = time(NULL);
		clients_history.push_back(*c);
		clients.erase(c);
	}
	if (!(tmp = ev->search_list_nick(cmd->prefix)).empty()) {
		Client *c = reinterpret_cast<Client *>(tmp[0]);

		std::string ans;
		ev->cmd_count["QUIT"] += 1;

		if (cmd->arguments.size()) {
			for (size_t i = 0; i < cmd->arguments.size(); i++) {
				ms += cmd->arguments[i];
				ms += " ";
			}
		} else {
			ms = c->nick;
		}
		ev->client_history.push_back(this);
		ans = ":";
		ans += c->nick;
		ans += " You have been kick of the server with the message : ";
		ans += ms;
		ans += CRLF;
		custom_send(ans, c);
		ev->clients_fd[c->sock] = new Fd();
		close(c->sock);
	}

	ms = cmd->line;
	ms += CRLF;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::TRACE(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tmp;

	if (!(tmp = ev->search_list_nick(cmd->arguments[0])).empty() || *ev->serv == cmd->arguments[0]) {
		if (!tmp.empty()) {
			Client *c = reinterpret_cast<Client *>(tmp[0]);
			std::string cl = c->o_mode ? "operator" : "user";
			ms = ":";
			ms += cmd->prefix;
			ms += " TRACEUP ";
			ms += cl;
			ms += " ";
			ms += c->nick;
			ms += " ";
			ms += c->username;
			ms += " ";
			ms += c->servername;
			ms += " ";
			ms += c->hostname;
			ms += " 1";
			time_t now;
			time(&now);
			int diff = difftime(now, c->creation);
			ms += " ";
			ms += std::to_string(diff);
			ms += CRLF;
			custom_send(ms, this);
		} else {
			for (Fd * f : ev->search_list_with_mode("", "", 'o')) {
				Client *c = reinterpret_cast<Client *>(f);
				std::string cl = c->o_mode ? "operator" : "user";
				ms = ":";
				ms += cmd->prefix;
				ms += " TRACEUP ";
				ms += cl;
				ms += " ";
				ms += c->nick;
				ms += " ";
				ms += c->username;
				ms += " ";
				ms += c->servername;
				ms += " ";
				ms += c->hostname;
				ms += " 1";
				time_t now;
				time(&now);
				int diff = difftime(now, c->creation);
				ms += " ";
				ms += std::to_string(diff);
				ms += CRLF;
				custom_send(ms, this);
			}
		}
	} else {
		ms = cmd->line;
		ms += CRLF;
		for (OtherServ * sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv); 
			}
		}
	}
}

void	OtherServ::TRACEUP(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tmp;
	std::vector<OtherServ *> tmpo;

	if (!(tmp = ev->search_list_nick(cmd->prefix)).empty()) {
		Client *c = reinterpret_cast<Client *>(tmp[0]);
		
		ms = cmd->arguments[1];
		ms += "[";
		ms += cmd->arguments[2];
		ms += "@";
		ms += cmd->arguments[3];
		ms += "] (";
		ms += cmd->arguments[4];
		ms += ") ";
		ms += cmd->arguments[5];
		ms += " :";
		ms += cmd->arguments[6];
		ms += CRLF;
		ms = reply_formating(c->servername.c_str(), RPL_TRACEUSER, std::vector<std::string>({cmd->arguments[0], ms}), c->nick.c_str());
		custom_send(ms, c);
	} else {
		if (!(tmpo = ev->search_othersrv_nick(cmd->prefix)).empty()) {
			ms = ":";
			ms += cmd->prefix;
			ms += " TRACEUP ";
			ms += cmd->arguments[0];
			ms += " ";
			ms += cmd->arguments[1];
			ms += " ";
			ms += cmd->arguments[2];
			ms += " ";
			ms += cmd->arguments[3];
			ms += " ";
			ms += cmd->arguments[4];
			ms += " ";
			ms += std::to_string(std::atoi(cmd->arguments[5].c_str()) + 1);
			ms += " ";
			ms += cmd->arguments[6];
			ms += CRLF;
			custom_send(ms, tmpo[0]);
		}
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
	case USER_CC:
		USER(parsed);
		break;
	case MODE_CC:
		MODE(parsed);
		break;
	case AWAY_CC:
		AWAY(parsed);
		break;
	case TIME_CC:
		TIME(parsed);
		break;
	case ADDS_CC:
		ADDS(parsed);
		break;
	case NSERV_CC:
		NSERV(parsed);
		break;
	case DELS_CC:
		DELS(parsed);
		break;
	case KILL_CC:
		KILL(parsed);
		break;
	case TRACE_CC:
		TRACE(parsed);
		break;
	case TRACEUP_CC:
		TRACEUP(parsed);
		break;
	default:
		break;
	}
	return (0);
}

void	OtherServ::read_func() {
	std::string	ms;
	Command		*parsed;
	std::string line;
	size_t		pos;
	size_t		charsToJump;

	fcntl(sock, F_SETFL, O_NONBLOCK);
	memset(&buf_read, 0, BUF_SIZE + 1);
	recv(sock, &buf_read, BUF_SIZE, 0);
	std::string rd = std::string(buf_read);

	_stream += std::string(buf_read);

	if (rd.empty()) {
		ev->clients_fd[sock] = new Fd();
		std::vector<OtherServ *>::iterator tmp;
		tmp = std::find(ev->otherServers.begin(), ev->otherServers.end(), this);
		ev->otherServers.erase(tmp);
		close(sock);
		// * loop through all clients and call QUIT on the others servers
		for (Client * c: clients) {
			ms = ":";
			ms += c->nick;
			ms += " QUIT 1";
			ms += CRLF;
			for (OtherServ *sv : ev->otherServers) {
				if (sv != this) {
					custom_send(ms, sv);
				}
			}
		}

		// * tell other serve that the server is disconnect
		ms = "DELS ";
		ms += std::to_string(connected);
		ms += CRLF;
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
		std::cerr << "Other serv quit" << std::endl;
	} else {

		while (Client::thereIsAFullCmd(pos, charsToJump, _stream)) {
			line = _stream.substr(0, pos);
			_stream = _stream.substr(pos + charsToJump);

			Kb_sent += sizeof(line);
			parsed = parse(line);
			send_ms += 1;
			std::cout << "Other :\n" << *parsed << std::endl;
			execute_parsed(parsed);
			// ! TEST :
			std::cout << "Nick list : " << std::endl;
			for (Client *tmp : clients) {
				std::cout << "- " << tmp->nick << "-" << tmp->realname << std::endl;
			}
			int tmp2 = 0;
			for (OtherServ *sv : ev->otherServers) {
				tmp2 += sv->connected;
			}
			std::cout << "Servers Connected : " << tmp2 << std::endl;
			delete parsed;
		}
	}
}

void	OtherServ::write_func() { }

bool	OtherServ::change_nick(std::string old, std::string nw) {
	for (Client *c : clients) {
		if (c->nick == old) {
			c->nick = nw;
			return (true);
		}
	}
	return (false); 
}

std::vector<Client *>::iterator	OtherServ::search_nick(std::string nk) {
	std::vector<Client *>::iterator buff = clients.begin();
	
	while (buff != clients.end()) {
		Client *c = *buff;
		if (c->nick == nk || nk == "*") 
			return buff;
		buff++;
	}
	return buff;
}

std::vector<Client *>::iterator	OtherServ::search_history_nick(std::string nk) {
	std::vector<Client *>::iterator buff = clients_history.begin();
	
	while (buff != clients_history.end()) {
		Client *c = *buff;
		if (c->nick == nk || nk == "*") 
			return buff;
		buff++;
	}
	return buff;
}

std::vector<Client *>	OtherServ::search_list_with_mode(char c) {
	std::vector<Client *> buff;

	for (Client *cl : clients) {			
		if (c == 'o' && cl->o_mode) 
			buff.push_back(cl);
		if (c == 'i' && cl->i_mode) 
			buff.push_back(cl);
		if (c == 'w' && cl->w_mode) 
			buff.push_back(cl);
		if (c == 's' && cl->s_mode) 
			buff.push_back(cl);
	}
	return buff;
}