/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/12 22:02:19 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OtherServ.hpp"
#include <iterator>

OtherServ::OtherServ(int socket, Environment *e, int mode): _stream() {	
	creation = time(NULL);
	sock = socket;
	type = FD_OTHER;
	ev = e;
	if (mode == 1) {
		already_setup_name = true;
	} else {
		already_setup_name = false;
	}
}

OtherServ::OtherServ(const OtherServ &cpy) {
	sock = cpy.sock;
	name = cpy.name;
	hop_count = cpy.hop_count;
	info = cpy.info;
}

OtherServ::~OtherServ() { 
	for (Client * c : clients_history) {
		ev->client_history.push_back(c);
	}
	for (Client * c : clients) {
		ev->client_history.push_back(c);
	}
}

void	OtherServ::READY(Command *cmd) {
	std::string ms;
	(void)cmd;

	ms = ":";
	ms += *ev->serv;
	ms += " SERVER ";
	ms += *ev->serv;
	ms += " 1 ";
	ms += ":irc server for 42 ";
	custom_send(ms, this);

	for (OtherServ *sv : ev->otherServers) {
		if (sv != *(ev->otherServers.end() - 1)) {
			for (std::string tm : sv->connected_sv) {
				ms = ":";
				ms += *ev->serv;
				ms += " SERVER ";
				ms += tm;
				ms += " ";
				ms += std::to_string(sv->connected_hop[tm] + 1);
				ms += " ";
				ms += sv->connected_info[tm];
				custom_send(ms, this);
			}
			ms = ":";
			ms += *ev->serv;
			ms += " SERVER ";
			ms += sv->name;
			ms += " 2 ";
			ms += sv->info;
			custom_send(ms, this);
		}
	}

	// * **NICKS
	for (Fd *f: ev->clients_fd) {
		if (f->type == FD_CLIENT) {
			Client *c = reinterpret_cast<Client *>(f);

			if (c->nick_set) {
				c->share_Client(this);
			}
		}
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			for (Client *c : sv->clients) {
				c->share_Client(this);
			}
		}	
	}

	// * **CHANNELS
	ev->channels->shareAll(this);
}

void	OtherServ::NICK(Command *cmd) {
	std::string ms = cmd->line;
	std::string	newNick = cmd->arguments[0];
	if (!newNick.empty() && newNick.at(0) == ':')
		newNick = newNick.substr(1);

	if (cmd->prefix.empty()) {
		clients.push_back(new Client(newNick, this));
	} else {
		if (change_nick(cmd->prefix, newNick)) {
		} else {
			clients.push_back(new Client(newNick, this));
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
	std::vector<Client *>::iterator c;

	c = search_nick(cmd->prefix);
	if (c != clients.end()) {
		// quit all of the channels the user were on that are on this serv
		ev->channels->doQuit(*c, cmd->arguments);

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

	std::vector<Client *>::iterator cit = search_nick(cmd->prefix);

	targ = cmd->arguments[0];
	if (targ[0] == '#' || targ[0] == '+' || targ[0] == '!') {
		if (cit == clients.end())
			return ;
		ev->channels->broadcastMsg(*cit, targ, cmd->arguments);
	} else {
		ms = ":";
		ms += cmd->prefix;
		ms += " PRIVMSG ";
		for (std::string tmp : cmd->arguments) {
			ms += tmp;
			ms += " ";
		}
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

	std::vector<Client *>::iterator cit = search_nick(cmd->prefix);

	targ = cmd->arguments[0];
	if (targ[0] == '#' || targ[0] == '+' || targ[0] == '!') {
		if (cit == clients.end())
			return ;
		ev->channels->broadcastMsg(*cit, targ, cmd->arguments, false);
	} else {
		ms = ":";
		ms += cmd->prefix;
		ms += " NOTICE ";
		for (std::string tmp : cmd->arguments) {
			ms += tmp;
			ms += " ";
		}
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
	c->setEnv(ev);
	ms = cmd->line;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::MODE(Command *cmd) {
	std::string ms;
	Client		*c;
	char		ch = ' ';
	c = ev->searchClientEverywhere(cmd->prefix);

	if (cmd->arguments.size() >= 1)
		ch = cmd->arguments[0].at(0);

	if (ch == '#' || ch == '!' || ch == '+') {
		if (c)
			ev->channels->mode(c, cmd->arguments, this);
		return ;
	}
	bool add = false;
	c = *search_nick(cmd->prefix);
	if (cmd->arguments.size() > 1) {
		for (size_t i = 0; i < cmd->arguments[1].length() && i < 4; i++) {
			if (i == 0) {
				if (cmd->arguments[1][i] == '-')
					;
				else if (cmd->arguments[1][i] == '+')
					add = true;
				else
					break;
			} else if (!c->set_uMODE(cmd->arguments[1][i], add, 1))
				break;
		}
	}

	std::cout << c->get_userMODEs_ms(false) << std::endl;
	ms = cmd->line;
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
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::VERSION(Command *cmd) {
	std::string	ms;

	if (cmd->arguments[0] == *ev->serv) {
		ms = reply_formating((*ev->serv).c_str(), RPL_VERSION, std::vector<std::string>({*ev->version, "1", *ev->serv, "second server-to-server iterations of irc for 42 project"}), cmd->prefix.c_str());
		custom_send(ms, this);
	} else {
		ms = cmd->line;
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::SERVER(Command *cmd) {
	std::string ms;
	std::string tminfo;

	ms = ":";
	ms += *ev->serv;
	ms += " SERVER ";
	ms += cmd->arguments[0];
	ms += " ";
	ms += std::to_string(std::atoi(cmd->arguments[1].c_str()) + 1);
	ms += " ";
	for (size_t i = 2; i < cmd->arguments.size(); i++) {
		ms += cmd->arguments[i];
		ms += " ";
		tminfo += cmd->arguments[i];
		tminfo += " ";
	}
	if (!already_setup_name) {
		name = cmd->arguments[0];
		info = tminfo;
		already_setup_name = true;
	} else {
		connected_sv.push_back(cmd->arguments[0]);
		connected_hop.emplace(std::pair<std::string, int>(cmd->arguments[0], std::atoi(cmd->arguments[1].c_str())));
		connected_info.emplace(std::pair<std::string, std::string>(cmd->arguments[0], tminfo));
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::DELS(Command *cmd) {
	std::string ms;
	
	ms = cmd->line;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::KILL(Command *cmd) {
	std::string ms;
	std::vector<Client *>::iterator c;
	std::vector<Fd *> tmp;

	c = search_nick(cmd->arguments[0]);
	if (c != clients.end()) {
		(*c)->last = time(NULL);
		clients_history.push_back(*c);
		clients.erase(c);
	}
	if (!(tmp = ev->search_list_nick(cmd->arguments[0])).empty()) {
		Client *c = reinterpret_cast<Client *>(tmp[0]);

		std::string ans;
		ev->cmd_count["QUIT"] += 1;

		if (cmd->arguments.size()) {
			for (size_t i = 1; i < cmd->arguments.size(); i++) {
				ms += cmd->arguments[i];
				ms += " ";
			}
		} else {
			ms = c->nick;
		}
		ans = ":";
		ans += c->nick;
		ans += " You have been kick of the server with the message : ";
		ans += ms;
		custom_send(ans, c);
		ev->client_history.push_back(c);
		ev->clients_fd[c->sock] = new Fd();
		close(c->sock);
	}

	ms = cmd->line;
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			custom_send(ms, sv);
		}
	}
}

void	OtherServ::TRACE(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tmp;
	time_t now = time(NULL);
	OtherServ *tmp2;

	if (cmd->arguments[0] == *ev->serv) {
		ms = reply_formating((*ev->serv).c_str(), RPL_TRACESERVER, std::vector<std::string>({"1", "0" /* // !! attention ajoute le nombre de channel ici ! //*/, *ev->serv}), cmd->prefix.c_str());
		custom_send(ms, this);
		ms = reply_formating((*ev->serv).c_str(), RPL_TRACEEND, std::vector<std::string>({*ev->serv, "0.4.2"}), cmd->prefix.c_str());
		custom_send(ms, this);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (std::string tm : sv->connected_sv) {
					if (tm == cmd->arguments[0]) {
						tmp2 = sv;
						break;
					}
				}
				if (sv->name == cmd->arguments[0]) {
					tmp2 = sv;
					break;
				}
				if (sv->search_nick(cmd->arguments[0]) != sv->clients.end()) {
					tmp2 = sv;
					break;
				}
			}
		}
		ms = reply_formating((*ev->serv).c_str(), RPL_TRACELINK, std::vector<std::string>({"0.4.2", cmd->arguments[0], tmp2->name, "F", std::to_string(now - creation)}), cmd->prefix.c_str());
		custom_send(ms, this);
		ms = cmd->line;
		custom_send(ms, tmp2);
	}
}

void	OtherServ::SQUIT(Command *cmd) {
	std::string ms;
	std::vector<std::string>::iterator tm;

	if (*ev->serv == cmd->arguments[0]) {
		ev->active = false;
	} else {
		tm = search_namecon(cmd->arguments[0]);
		if (tm != connected_sv.end())
			connected_sv.erase(tm);
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				ms = cmd->line;
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::propagateChanMsg(const std::string &ms) {
	for (OtherServ *sv : ev->otherServers) { // propagate the join message
		if (sv != this)
			custom_send(ms, sv);
	}
}

void	OtherServ::JOIN(Command *cmd) {
	
	Client	*client;
	client = ev->searchClientEverywhere(cmd->prefix);
	if (!client)
		return ;
	std::string ms;
	ev->cmd_count["JOIN"] += 1;

	if (cmd->arguments.size() >= 1) {
		if (cmd->arguments[0] == "0") {
			std::list<Channel *>::iterator	current = client->channels.begin();
			while (current != client->channels.end()) {
				Channel	*ch = *current;
				if (ev->channels->leaveChannel(client, ch->getName(), "Leaving all channels", this))
					current = client->channels.begin();
				else
					++current;
			}
		} else {
			ev->channels->join(client, cmd->arguments, &(client->channels), this);
		}
	}
}

void	OtherServ::PART(Command *cmd)
{
	Client	*client;
	client = ev->searchClientEverywhere(cmd->prefix);
	if (!client)
		return ;
	std::string ms;
	ev->cmd_count["PART"] += 1;

	if (cmd->arguments.size() >= 1) {
		ev->channels->leave(client, cmd->arguments, this);
	}
}

void	OtherServ::KICK(Command *cmd)
{
	Client	*client;
	client = ev->searchClientEverywhere(cmd->prefix);
	if (!client)
		return ;
	std::string ms;
	ev->cmd_count["KICK"] += 1;

	if (cmd->arguments.size() >= 2) {
			ev->channels->kick(client, cmd->arguments, this);
	}
}

void	OtherServ::TOPIC(Command *cmd)
{
	Client	*client;
	client = ev->searchClientEverywhere(cmd->prefix);
	if (!client)
		return ;
	std::string ms;
	ev->cmd_count["TOPIC"] += 1;

	if (cmd->arguments.size() >= 1) {
		ev->channels->topic(client, cmd->arguments, this);
	}
}

void	OtherServ::INVITE(Command *cmd)
{
	Client	*client;
	client = ev->searchClientEverywhere(cmd->prefix);
	if (!client)
		return ;
	std::string ms;
	ev->cmd_count["INVITE"] += 1;

	if (cmd->arguments.size() >= 2) {
		ev->channels->invite(client, cmd->arguments, this);
	}
}

void	OtherServ::NAMES(Command *cmd)
{
	std::string	ms;
	std::string	chanName;
	std::vector<Client *>::iterator	client;

	if (cmd->prefix.empty())
		return ;
	if (cmd->arguments.size() >= 1) {
		chanName = cmd->arguments[0];
		// check if we have the channel
		if (ev->channels->getChannel(chanName)) {
			// if we have channel, we use ChannelMaster.invite() method
			client = search_nick(cmd->prefix);
			if (client == clients.end())
				return ; // message forgery won't error the server
			ev->channels->chanNames(*client, chanName);
			return ;
		}
		// if we do not have the channel, we forward the msg to the right serv
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Chan &chan : sv->chans) {
					if (utils::strCmp(chan.name, chanName)) {
						// forward the request to this serv
						ms = ":" + cmd->prefix + " INVITE " + chanName;
						custom_send(ms, sv);
						return ;
					}
				}
			}
		}
	}
}

void	OtherServ::LINKS(Command *cmd) {
	std::string ms = " ";

	if (*ev->serv == cmd->arguments[0]) {
		for (OtherServ *sv : ev->otherServers) {
			for (std::string tm : sv->connected_sv) {
				if (utils::strMatchToLower(cmd->arguments[1], tm)) {
					ms = reply_formating((*ev->serv).c_str(), RPL_LINKS, std::vector<std::string>({tm, sv->name, std::to_string(sv->connected_hop[tm]), sv->connected_info[tm]}), cmd->prefix.c_str());
					custom_send(ms, this);
				}
			}
			if (utils::strMatchToLower(cmd->arguments[1], sv->name)) {
				ms = reply_formating((*ev->serv).c_str(), RPL_LINKS, std::vector<std::string>({sv->name, *(ev->serv), std::to_string(sv->hop_count), sv->info}), cmd->prefix.c_str());
				custom_send(ms, this);
			}
		}
		if (utils::strMatchToLower(cmd->arguments[1], *ev->serv)) {
			ms = reply_formating((*ev->serv).c_str(), RPL_LINKS, std::vector<std::string>({*ev->serv, *ev->serv, "0", "irc server for 42"}), cmd->prefix.c_str());
			custom_send(ms, this);
		}
		ms = reply_formating((*ev->serv).c_str(), RPL_ENDOFLINKS, {cmd->arguments[1]}, cmd->prefix.c_str());
		custom_send(ms, this);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				ms = cmd->line;
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::TIME(Command *cmd) {
	std::string	ms;
	time_t		now = time(NULL);

	if (cmd->arguments[0] == *ev->serv) {
		ms = asctime(localtime(&now));
		ms = std::string(&ms[0], &ms[ms.size() - 1]);
		ms = reply_formating((*ev->serv).c_str(), RPL_TIME, std::vector<std::string>({*ev->serv, ms}), cmd->prefix.c_str());
		custom_send(ms, this);
	} else {
		ms = cmd->line;
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::ADMIN(Command *cmd) {
	std::string ms;

	if (cmd->arguments[0] == *ev->serv) {
		ms = reply_formating((*ev->serv).c_str(), RPL_ADMINME, {*ev->serv}, cmd->prefix.c_str());
		custom_send(ms, this);
		ms = reply_formating((*ev->serv).c_str(), RPL_ADMINLOC1, {ev->loc1}, cmd->prefix.c_str());
		custom_send(ms, this);
		ms = reply_formating((*ev->serv).c_str(), RPL_ADMINLOC2, {ev->loc2}, cmd->prefix.c_str());
		custom_send(ms, this);
		for (std::string s : ev->emails) {
			ms = reply_formating((*ev->serv).c_str(), RPL_ADMINEMAIL, {s}, cmd->prefix.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = cmd->line;
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::INFO(Command *cmd) {
	std::list<std::string>	inf;
	std::list<std::string>::iterator	current;
	std::list<std::string>::iterator	end;
	std::string buf;
	std::string ms;

	if (cmd->arguments[0] == *ev->serv) {
		inf.push_back(" IRC --");
		inf.push_back(" Based on RFCs given on 42 subject page : ");
		inf.push_back(" https://github.com/qfeuilla/ft_irc/blob/master/en.subject.pdf ");
		inf.push_back(" ");
		inf.push_back(" This program is free software; you can redistribute it and/or");
		inf.push_back(" modify it under the terms of the GNU General Public License as");
		inf.push_back(" published by the Free Software Foundation; either version 2, or");
		inf.push_back(" (at your option) any later version.");
		inf.push_back(" ");
		inf.push_back(" ft_irc has been developed to meet the policy needs of the");
		inf.push_back(" RFCs IRC network. ");
		inf.push_back("  ");
		inf.push_back(" ft_irc development is currently ongoin with developpers :");
		inf.push_back(" Quentin FEUILLADE--MONTIXI and Mayeul Le Monies De Sagazan");
		inf.push_back(" see our Github : ");
		inf.push_back(" 	- https://github.com/qfeuilla ");
		inf.push_back(" 	- https://github.com/mle-moni ");
		inf.push_back(" ");
		inf.push_back(" ");
		inf.push_back(" On-line since:");
		buf = std::asctime(gmtime(&(ev->start)));
		inf.push_back(std::string(&buf[0], &buf[buf.size() - 1]));
		inf.push_back(" UTC \n");
		
		std::string line;
		current = inf.begin();
		end = inf.end();

		while (current != end) {
			line = *current;
			line = reply_formating((*ev->serv).c_str(), RPL_INFO, {line}, cmd->prefix.c_str());
			custom_send(line, this);
			++current;
		}

		line = reply_formating((*ev->serv).c_str(), RPL_ENDOFINFO, {}, cmd->prefix.c_str());
		custom_send(line, this);
	} else {
		ms = cmd->line;
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
} 

void	OtherServ::RPL_391(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::RPL_351(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}


void	OtherServ::RPL_364(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::RPL_365(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::RPL_NTRACE(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::RPL_ADMIN(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::RPL_NINFO(Command *cmd) {
	std::string ms;
	std::vector<Fd *> tm;

	ms = cmd->line;
	if (!((tm = ev->search_list_nick(cmd->arguments[0])).empty())) {
		Client *c = reinterpret_cast<Client *>(tm[0]);
		custom_send(ms, c);
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
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
	case SERVER_CC:
		SERVER(parsed);
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
	case SQUIT_CC:
		SQUIT(parsed);
		break;
	case JOIN_CC:
		JOIN(parsed);
		break;
	case PART_CC:
		PART(parsed);
		break;
	case KICK_CC:
		KICK(parsed);
		break;
	case TOPIC_CC:
		TOPIC(parsed);
		break;
	case INVITE_CC:
		INVITE(parsed);
		break;
	case NAMES_CC:
		NAMES(parsed);
		break;
	case READY_CC:
		READY(parsed);
		break;
	case VERSION_CC:
		VERSION(parsed);
		break;
	case LINKS_CC:
		LINKS(parsed);
		break;
	case RPL_351_CC:
		RPL_351(parsed);
		break;
	case RPL_364_CC:
		RPL_364(parsed);
		break;
	case RPL_365_CC:
		RPL_365(parsed);
		break;
	case RPL_391_CC:
		RPL_391(parsed);
		break;
	case RPL_NTRACE_CC:
		RPL_NTRACE(parsed);
		break;
	case ADMIN_CC:
		ADMIN(parsed);
		break;
	case RPL_ADMIN_CC:
		RPL_ADMIN(parsed);
		break;
	case INFO_CC:
		INFO(parsed);
		break;
	case RPL_NINFO_CC:
		RPL_NINFO(parsed);
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
	utils::memset(&buf_read, 0, BUF_SIZE + 1);
	recv(sock, &buf_read, BUF_SIZE, 0);
	std::string rd = std::string(buf_read);

	_stream += std::string(buf_read);

	if (rd.empty()) {
		std::vector<OtherServ *>::iterator tmp;
		tmp = std::find(ev->otherServers.begin(), ev->otherServers.end(), this);
		ev->otherServers.erase(tmp);
		ev->lostServers.push_back(this);
		close(sock);
		// * loop through all clients and call QUIT on the others servers
		for (Client * c: clients) {
			ms = ":";
			ms += c->nick;
			ms += " QUIT 1";
			for (OtherServ *sv : ev->otherServers) {
				if (sv != this) {
					custom_send(ms, sv);
				}
			}
			ev->channels->doQuit(c, std::vector<std::string>({"Server quitted"}));
		}

		// * oups
		for (Chan chan : chans) {
			for (std::string nickName : chan.nicknames) {
				sendPartMessage(chan, nickName);
			}

			// ! CHAN_CHG IS OBSOLETE, send all history of the channel's commands instead

		}

		// * tell other serve that the server is disconnect and all the depending serv
		for (std::string tm : connected_sv) {
			ms = "SQUIT ";
			ms += tm;
			ms += " :disconnection";
			for (OtherServ *sv : ev->otherServers) {
				if (sv != this) {
					custom_send(ms, sv);
				}
			}
		}
		ms = "SQUIT ";
		ms += name;
		ms += " :disconnection";
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
		ev->trash.push_back(this); // !! split trashes
		ev->clients_fd[sock] = new Fd();
		std::cerr << "Other serv quit" << std::endl;
	} else {

		while (Client::thereIsAFullCmd(pos, charsToJump, _stream)) {
			line = _stream.substr(0, pos);
			std::cout << "old :" << line << std::endl;
			std::cout << "OK" << std::endl;
			_stream = _stream.substr(pos + charsToJump);

			Kb_sent += sizeof(line);
			parsed = parse(line);
			send_ms += 1;
			std::cout << "Other :\n" << *parsed << std::endl;
			execute_parsed(parsed);
			std::cout << "Nick list : " << std::endl;
			for (Client *tmp : clients) {
				std::cout << "- " << tmp->nick << "-" << tmp->realname << std::endl;
			}
			std::cout << "Servers Connected to the other serv: " << connected_sv.size() << std::endl;
			for (std::string tmp3 : connected_sv) {
				std::cout << "- " << tmp3 << " at " << connected_hop[tmp3] << " hops with info " << connected_info[tmp3] << std::endl;
			}
			std::cout << "This connection is with : " << name << std::endl;
			delete parsed;
		}
	}
}

void	OtherServ::write_func() { }

bool	OtherServ::change_nick(std::string old, std::string nw) {
	for (Client *c : clients) {
		if (c->nick == old) {
			c->last = time(NULL);
			clients_history.push_back(new Client(*c));
			std::string	oldNick = c->nick;
			c->nick = nw;
			ev->channels->changeNick(oldNick, c->nick);
			return (true);
		}
	}
	return (false); 
}

std::vector<Client *>::iterator	OtherServ::search_nick(std::string nk) {
	std::vector<Client *>::iterator buff = clients.begin();
	
	while (buff != clients.end()) {
		Client *c = *buff;
		if (utils::strMatchToLower(c->nick, nk)) 
			return buff;
		buff++;
	}
	return buff;
}

std::vector<Client *>::iterator	OtherServ::search_history_nick(std::string nk) {
	std::vector<Client *>::iterator buff = clients_history.begin();
	
	while (buff != clients_history.end()) {
		Client *c = *buff;
		if (utils::strMatchToLower(c->nick, nk)) 
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

std::vector<std::string>::iterator OtherServ::search_namecon(std::string sv) {
	std::vector<std::string>::iterator it = connected_sv.begin();

	while (it != connected_sv.end()) {
		if (*it == sv)
			break;
		std::next(it);
	}
	return it;
}

std::vector<Chan>::iterator	OtherServ::getChan(const std::string &name)
{
	std::vector<Chan>::iterator	current = chans.begin();
	std::vector<Chan>::iterator	end = chans.end();

	while (current != end) {
		if (utils::strCmp((*current).name, name))
			return (current);
		++current;
	}
	return (chans.end());
}

bool		OtherServ::chanWHO(Client *client, const std::vector<std::string> &args)
{
	std::string	ms;
	Client		*c;
	std::vector<Chan>::iterator	chan = getChan(args[0]);
	std::vector<std::string>::iterator	end;

	if (chan == chans.end())
		return (false);
	end = chan->nicknames.end();
	if (std::find(chan->nicknames.begin(), end, utils::ircLowerCase(client->nick)) == end) {
		ms = ":" + client->servername + " 315 " + client->nick + " " + args[0] + " :End of /WHO list";
		custom_send(ms, client);
		return (true);
	}
	for (std::string clientNick : chan->nicknames) {
		if (utils::strCmp(clientNick, client->nick)) {
			ms = ":" + client->servername + " 352 " + client->nick + " " + args[0] + " ";
			ms += client->username + " " + client->hostname + " " + client->servername + " " + client->nick;
			ms += " H :" + std::to_string(client->hop_count) + " " + client->realname;
			custom_send(ms, client);
		}
		c = ev->getOtherServClientByNick(clientNick);
		if (!c)
			continue ;

		ms = ":" + client->servername + " 352 " + client->nick + " " + args[0] + " ";
		ms += c->username + " " + c->hostname + " " + c->servername + " " + c->nick;
		ms += " H :" + std::to_string(c->hop_count) + " " + c->realname;
		custom_send(ms, client);
	}

	ms = ":" + client->servername + " 315 " + client->nick + " " + args[0] + " :End of /WHO list";
	custom_send(ms, client);
	return (true);
}

void		OtherServ::sendPartMessage(Chan &chan, const std::string &nickName)
{
	std::vector<Fd *>			tmpc;
	std::vector<Client *>::iterator	client;
	std::vector<OtherServ *>	tmpo;
	Client	*c;
	std::string	ms;

	if (!(tmpc = ev->search_list_nick(nickName)).empty()) {
		c = reinterpret_cast<Client *>(tmpc[0]);
		ms = ":" + c->nick + "!" + c->username + "@";
		ms += c->servername + " PART " + chan.name;
		ms += " :this channel's host server quitted";
		custom_send(ms, c);
	} else if (!(tmpo = ev->search_othersrv_nick(nickName)).empty()) {
		if (tmpo[0] != this && ((client = tmpo[0]->search_nick(nickName)) != tmpo[0]->clients.end())) {
			c = *client;
			ms = ":" + c->nick + "!" + c->username + "@";
			ms += c->servername + " PART " + chan.name;
			ms += " :this channel's host server quitted";
			custom_send(ms, tmpo[0]);
		}
	}
}
