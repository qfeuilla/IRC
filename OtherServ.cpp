/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/07 19:57:18 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OtherServ.hpp"
#include <iterator>

OtherServ::OtherServ(int socket, Environment *e, std::string prt): _stream() {	
	creation = time(NULL);
	sock = socket;
	type = FD_OTHER;
	ev = e;
	port = prt;
	std::cout << "other serv port : " << port << std::endl;
	porti = std::stoi(port);
}

OtherServ::OtherServ(const OtherServ &cpy) {
	sock = cpy.sock;
	name = cpy.name;
	hop_count = cpy.hop_count;
	token = cpy.token;
	info = cpy.info;
}

OtherServ::~OtherServ() { 
	for (Client *c : clients) {
		delete c;
	} for (Client *c : clients_history) {
		delete c;
	}
}

void	OtherServ::READY(Command *cmd) {
	std::string ms;
	(void)cmd;

	int tmp = 0;
	for (OtherServ *sv : ev->otherServers) {
		tmp += sv->connected;
	}

	// Notify incoming server of number of servers
	ms = "NSERV ";
	ms += std::to_string(tmp);
	custom_send(ms, this);

	// * **CHANNELS
	std::vector<Chan>	thisServChans = ev->channels->getChans();
	for (Chan &chan : thisServChans) {
		std::string	usersStr = utils::strJoin(chan.nicknames, ',');
		if (usersStr.size() == 0)
			usersStr = "nobody";
		ms = "CHAN_CHG ";
		ms += chan.name + "," + chan.usersNum + "," + chan.modes + " ";
		ms += usersStr + " ";
		ms += (chan.topic != "") ? ":" + chan.topic : ":!";
		custom_send(ms, this);
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			for (Chan &chan : sv->chans) {
				std::string	usersStr = utils::strJoin(chan.nicknames, ',');
				if (usersStr.size() == 0)
					usersStr = "nobody";
				ms = "CHAN_CHG ";
				ms += chan.name + "," + chan.usersNum + "," + chan.modes + " ";
				ms += usersStr + " ";
				ms += (chan.topic != "") ? ":" + chan.topic : ":!";
				custom_send(ms, this);
			}
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
	for (Fd *f: ev->client_history) {
		Client *c = reinterpret_cast<Client *>(f);

		if (c->nick_set) {
			c->share_Client(this);
			ms = ":";
			ms += c->nick;
			ms += " QUIT";
			custom_send(ms, this);
		}
	}
	for (OtherServ *sv : ev->otherServers) {
		if (sv != this) {
			for (Client *c : sv->clients) {
				c->share_Client(this);
			}
			for (Client *c : sv->clients_history) {
				c->share_Client(this);
				ms = ":";
				ms += c->nick;
				ms += " QUIT";
				custom_send(ms, this);
			}
		}	
	}
}

void	OtherServ::NICK(Command *cmd) {
	std::string ms = cmd->line;

	if (cmd->prefix.empty()) {
		clients.push_back(new Client(cmd->arguments[0], this));
	} else {
		if (change_nick(cmd->prefix, cmd->arguments[0])) {
		} else {
			clients.push_back(new Client(cmd->arguments[0], this));
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

	if (cmd->arguments.size() >= 1)
		ch = cmd->arguments[0].at(0);

	if (ch == '#' || ch == '!' || ch == '+') {
		chanModes(cmd);
		return ;
	}
	c = *search_nick(cmd->prefix);
	c->i_mode = false;
	c->o_mode = false;
	c->w_mode = false;
	c->s_mode = false;
	if (cmd->arguments.size() >= 2) {
		for (char m : cmd->arguments[1]) {
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

void	OtherServ::TIME(Command *cmd) {
	std::string ms;
	Client		*c;
	
	c = *search_nick(cmd->prefix);
	std::cout << "before crash\n";
	c->creation = std::strtoll(cmd->arguments[0].c_str(), NULL, 10);
	std::cout << "after crash\n";
	c->last = std::strtoll(cmd->arguments[1].c_str(), NULL, 10);

	ms = cmd->line;
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
		custom_send(ans, c);
		// ? leak ?
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
				custom_send(ms, this);
			}
		}
	} else {
		ms = cmd->line;
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
			custom_send(ms, tmpo[0]);
		}
	}
}

void	OtherServ::SQUIT(Command *cmd) {
	std::string ms;

	if (*ev->serv == cmd->arguments[0] && std::to_string(htons(ev->sin.sin_port)) == cmd->arguments[1]) {
		ev->active = false;
	} else {
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				ms = cmd->line;
				custom_send(ms, sv);
			}
		}
	}
} 

void	OtherServ::CHAN_CHG(Command *cmd)
{
	std::string	ms;
	std::string	topic;
	if (cmd->arguments.size() >= 3) {
		std::vector<std::string>	split = parse_comma(cmd->arguments[0]);
		std::vector<std::string>	users = parse_comma(cmd->arguments[1]);
		if (split.size() != 3)
			return ;
		topic = Channel::parseArg(2, cmd->arguments);

		std::vector<Chan>::iterator	ite = getChan(split[0]);
		if (ite == chans.end()) { // this chan does not exist yet: insert new channel data
			chans.push_back(Chan(
				std::string(split[0]),
				std::string(split[1]),
				std::string(split[2]),
				std::string(topic),
				std::vector<std::string>(users)
			));
		} else { // channel exists, just update it
			(*ite).name = std::string(split[0]);
			(*ite).usersNum = std::string(split[1]);
			(*ite).modes = std::string(split[2]);
			(*ite).topic = std::string(topic);
			(*ite).nicknames = std::vector<std::string>(users);
			if ((*ite).usersNum == "0") {
				chans.erase(ite);
			}
		}
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				ms = cmd->line;
				custom_send(ms, sv);
			}
		}
	}
}

void	OtherServ::CHAN_RPL(Command *cmd) {
	std::vector<OtherServ *>	tmpo;
	std::vector<Fd *>	tmpc;
	Client				*c = nullptr;
	std::string			ms;
	std::string			nickName = cmd->prefix;

	if (nickName.empty())
		return ;
	if (!(tmpc = ev->search_list_nick(nickName)).empty()) {
		c = reinterpret_cast<Client *>(tmpc[0]);
		// delete the part ":prefix CHAN_RPL " of the message
		ms = utils::delFirstWord(utils::delFirstWord(cmd->line)) + CRLF;
		custom_send(ms, c);
	} else if (!(tmpo = ev->search_othersrv_nick(nickName)).empty()) {
		ms = cmd->line;
		custom_send(ms, tmpo[0]);
	}
}

void	OtherServ::JOIN(Command *cmd) {
	std::string	ms;
	std::string	chanName;
	std::string	passwd = "";
	std::vector<Client *>::iterator	client;

	if (cmd->prefix.empty())
		return ;
	if (cmd->arguments.size() >= 1) {
		chanName = cmd->arguments[0];
		if (cmd->arguments.size() >= 2)
			passwd = cmd->arguments[1];
		// check if we have the channel
		if (ev->channels->getChannel(chanName)) {
			// if we have channel, we use ChannelMaster.join() method
			client = search_nick(cmd->prefix);
			if (client == clients.end())
				return ; // message forgery won't error the server
			ev->channels->joinChannel(*client, chanName, passwd);
			return ;
		}
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Chan &chan : sv->chans) {
					if (utils::strCmp(chan.name, chanName)) {
						// forward the request to this serv
						ms = ":";
						ms += cmd->prefix; // the user who wants to join the channel
						ms += " JOIN " + chanName + " " + passwd;
						custom_send(ms, sv);
						return ;
					}
				}
			}
		}
	}
}

void	OtherServ::PART(Command *cmd)
{
	std::string	ms;
	std::string	chanName;
	std::string	reason = "";
	std::vector<Client *>::iterator	client;

	if (cmd->prefix.empty())
		return ;
	if (cmd->arguments.size() >= 1) {
		chanName = cmd->arguments[0];
		if (cmd->arguments.size() >= 2)
			reason = Channel::parseArg(1, cmd->arguments);
		// check if we have the channel
		if (ev->channels->getChannel(chanName)) {
			// if we have channel, we use ChannelMaster.leave() method
			client = search_nick(cmd->prefix);
			if (client == clients.end())
				return ; // message forgery won't error the server
			ev->channels->leaveChannel(*client, chanName, reason);
			return ;
		}
		// if we do not have the channel, we forward the msg to the right serv
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Chan &chan : sv->chans) {
					if (utils::strCmp(chan.name, chanName)) {
						// forward the request to this serv
						ms = ":";
						ms += cmd->prefix; // the user who wants to join the channel
						ms += " PART " + chanName + " :" + reason;
						custom_send(ms, sv);
						return ;
					}
				}
			}
		}
	}
}

void	OtherServ::KICK(Command *cmd)
{
	std::string	ms;
	std::string	chanName;
	std::string	guyToKick;
	std::string	reason = "";
	std::vector<Client *>::iterator	client;

	if (cmd->prefix.empty())
		return ;
	if (cmd->arguments.size() >= 2) {
		chanName = cmd->arguments[0];
		guyToKick = cmd->arguments[1];
		if (cmd->arguments.size() >= 3)
			reason = Channel::parseArg(2, cmd->arguments);
		// check if we have the channel
		if (ev->channels->getChannel(chanName)) {
			// if we have channel, we use ChannelMaster.kick() method
			client = search_nick(cmd->prefix);
			if (client == clients.end())
				return ; // message forgery won't error the server
			ev->channels->kickFromChan(*client, chanName, guyToKick, reason);
			return ;
		}
		// if we do not have the channel, we forward the msg to the right serv
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Chan &chan : sv->chans) {
					if (utils::strCmp(chan.name, chanName)) {
						// forward the request to this serv
						ms = ":";
						ms += cmd->prefix; // the user who wants to kick someone
						ms += " KICK " + chanName + " " + guyToKick + " :" + reason;
						custom_send(ms, sv);
						return ;
					}
				}
			}
		}
	}
}


void	OtherServ::chanModes(Command *cmd) {
	std::vector<Client *>::iterator	client;

	client = search_nick(cmd->prefix);
	if (client == clients.end())
		return ; // message forgery won't error the server
	if (cmd->arguments.size() == 1) {
		ev->channels->getChanModes(*client, cmd->arguments);
		return ;
	}
	ev->channels->mode(*client, cmd->arguments);
}

void	OtherServ::TOPIC(Command *cmd)
{
	std::string	ms;
	std::string	chanName;
	std::string	newTopic = "";
	std::vector<Client *>::iterator	client;

	if (cmd->prefix.empty())
		return ;
	if (cmd->arguments.size() >= 1) {
		chanName = cmd->arguments[0];
		if (cmd->arguments.size() >= 2)
			newTopic = Channel::parseArg(1, cmd->arguments);
		// check if we have the channel
		if (ev->channels->getChannel(chanName)) {
			// if we have channel, we use ChannelMaster.topic() method
			client = search_nick(cmd->prefix);
			if (client == clients.end())
				return ; // message forgery won't error the server
			ev->channels->topic(*client, cmd->arguments);
			return ;
		}
		// if we do not have the channel, we forward the msg to the right serv
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Chan &chan : sv->chans) {
					if (utils::strCmp(chan.name, chanName)) {
						// forward the request to this serv
						ms = ":" + cmd->prefix + " TOPIC " + chanName;
						if (newTopic != "")
							ms += " :" + newTopic;
						custom_send(ms, sv);
						return ;
					}
				}
			}
		}
	}
}

void	OtherServ::INVITE(Command *cmd)
{
	std::string	ms;
	std::string	chanName;
	std::string	guyToInvite;
	std::vector<Client *>::iterator	client;

	if (cmd->prefix.empty())
		return ;
	if (cmd->arguments.size() >= 2) {
		guyToInvite = cmd->arguments[0];
		chanName = cmd->arguments[1];
		// check if we have the channel
		if (ev->channels->getChannel(chanName)) {
			// if we have channel, we use ChannelMaster.invite() method
			client = search_nick(cmd->prefix);
			if (client == clients.end())
				return ; // message forgery won't error the server
			ev->channels->invite(*client, cmd->arguments);
			return ;
		}
		// if we do not have the channel, we forward the msg to the right serv
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				for (Chan &chan : sv->chans) {
					if (utils::strCmp(chan.name, chanName)) {
						// forward the request to this serv
						ms = ":" + cmd->prefix + " INVITE " + guyToInvite + " " + chanName;
						custom_send(ms, sv);
						return ;
					}
				}
			}
		}
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
	case SQUIT_CC:
		SQUIT(parsed);
		break;
	case CHAN_CHG_CC:
		CHAN_CHG(parsed);
		break;
	case CHAN_RPL_CC:
		CHAN_RPL(parsed);
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
		}

		// * tell other serve that the server is disconnect
		ms = "DELS ";
		ms += std::to_string(connected);
		for (OtherServ *sv : ev->otherServers) {
			if (sv != this) {
				custom_send(ms, sv);
			}
		}
		delete ev->clients_fd[sock];
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
			c->last = time(NULL);
			clients_history.push_back(new Client(*c));
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
