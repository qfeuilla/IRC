/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mayeul <mayeul@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:51:25 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/24 15:03:06 by qfeuilla         ###   ########.fr       */
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

bool		custom_send(std::string ms, Client *c) {
	c->recv_ms += 1;
	c->Kb_recv += sizeof(ms);
	send(c->sock, ms.c_str(), ms.length(), 0);
	return (true);
}

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

Client::Client(Environment *e, int s, struct sockaddr_in addr) : channels(), ev(e) {
	type = FD_WAITC;
	is_setup = false;
	sock = s;
	time(&creation);
	nick = "*";
	i_mode = false;
	o_mode = false;
	s_mode = false;
	w_mode = false;
	servername = *e->serv;
	csin = addr;
	hostname = std::string(inet_ntoa(csin.sin_addr)); // * ip of the user if not specified
}

Client::~Client() {
	std::cout << "client destructed" << std::endl;
}

// Only match the usage for hitory, Caution if you want to use for something else
Client::Client(const Client &old) {
	type = FD_CLIENT;
	creation = old.creation;
	time(&last);
	pass = old.pass;
	nick = old.nick;
	username = old.username;
	hostname = old.hostname;
	servername = old.servername;
	realname = old.realname;
	ev = old.ev;
	away_ms = "sleeping";
}

void	Client::PASS(Command *cmd) {
	std::string ms;
	ev->cmd_count["PASS"] += 1;

	if (!is_setup) {
		if (cmd->prefix.empty() && cmd->arguments.size() == 1) {
			pass = cmd->arguments[0];
			pass_set = true;
		} else {
			ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
			custom_send(ms, this);
		}
	}
	else {
		ms = reply_formating(servername.c_str(), ERR_ALREADYREGISTRED, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::NICK(Command *cmd) {
	std::string	ms;
	ev->cmd_count["NICK"] += 1;

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
					
					// * save old client for history purpose
					if (type == FD_CLIENT)
						ev->client_history.push_back(new Client(*this));
					nick = cmd->arguments[0];

					custom_send(ms, this);
					if (!nick_set) {
						nick_set = true;
						exec_registerMS();
					}
				} else {
					nick = cmd->arguments[0];
					nick_set = true;
				}
			} else {
				ms = reply_formating(servername.c_str(), ERR_NICKNAMEINUSE, {cmd->arguments[0]}, nick.c_str());
				nick = cmd->arguments[0];
				custom_send(ms, this);
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_ERRONEUSNICKNAME, {cmd->arguments[0]}, nick.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NONICKNAMEGIVEN, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::exec_registerMS() {
	std::string tmp;
	std::string ms;

	// * 001
	ms = reply_formating(servername.c_str(), RPL_WELCOME, std::vector<std::string>({nick, username, servername}), nick.c_str());
	custom_send(ms, this);

	// * 002
	std::string server;
	server += servername;
	server += "[";
	server += inet_ntoa(ev->sin.sin_addr);
	server += "/";
	server += std::to_string(htons(ev->sin.sin_port));
	server += "]";
	ms = reply_formating(servername.c_str(), RPL_YOURHOST, std::vector<std::string>({server, *ev->version}), nick.c_str());
	custom_send(ms, this);
	
	// * 003
	struct tm * timeinfo;
	timeinfo = localtime(&ev->start);
	std::vector<std::string> month = {"January", "February", "March", "April", "May", "June", "July", "August","September", "October", "November", "December"};
	std::string date;
	date += month[timeinfo->tm_mon];
	date += "-";
	date += std::to_string(timeinfo->tm_mday);
	date += "-";
	date += std::to_string(timeinfo->tm_year + 1900);
	date += " ";
	date += std::to_string(timeinfo->tm_hour);
	date += ":";
	date += std::to_string(timeinfo->tm_min);
	date += ":";
	date += std::to_string(timeinfo->tm_sec);
	ms = reply_formating(servername.c_str(), RPL_CREATED, {date}, nick.c_str());
	custom_send(ms, this);
	
	// * 004
	ms = reply_formating(servername.c_str(), RPL_MYINFO, std::vector<std::string>({servername, *ev->version, "wois", "ovptismlkqah"}), nick.c_str());
	custom_send(ms, this);
	
	type = FD_CLIENT; // * register for other user
	tmp = "LUSERS";
	tmp += CR;
	execute_parsed(parse(tmp));
	tmp = "MOTD";
	tmp += CR;
	execute_parsed(parse(tmp));
	tmp = "";
	tmp += ":";
	tmp += nick;
	tmp += " MODE ";
	tmp += nick;
	tmp += " +i";
	tmp += CR;
	execute_parsed(parse(tmp));
	tmp = "";
	tmp += ":";
	tmp += servername;
	tmp += " WALLOPS ";
	tmp += ": new User with nick : ";
	tmp += nick;
	tmp += " Join the server";
	tmp += CR;
	execute_parsed(parse(tmp));
	// TODO : need more functions (i will do)
}

void	Client::USER(Command *cmd) {
	std::string	ms;
	ev->cmd_count["USER"] += 1;

	if (!is_setup) {
		if (cmd->prefix.empty() && cmd->arguments.size() >= 4) {
			username = "~";
			username += cmd->arguments[0];
			if (cmd->arguments[1] != "0")
				hostname = cmd->arguments[1]; // * not used deprecated
			if (cmd->arguments[2] != "*")
				servername = cmd->arguments[2];
			for (size_t i = 3; i < cmd->arguments.size() - 1; i++) {
				realname += cmd->arguments[i];
				realname += " ";
			}
			realname += cmd->arguments[cmd->arguments.size() - 1];
			realname = std::string(&realname[1], &realname[realname.length()]);
			is_setup = true;
			if (nick_set)
				exec_registerMS();
		} else {
			ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
			custom_send(ms, this);
		}
	}
	else {
		ms = reply_formating(servername.c_str(), ERR_ALREADYREGISTRED, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::OPER(Command *cmd) {
	std::string ms;
	ev->cmd_count["OPER"] += 1;

	if (cmd->arguments.size() >= 2) {
		if (ev->accept_operators) {
			if (*ev->password == cmd->arguments[1] 
				&& *ev->serv == cmd->arguments[0]) {
					o_mode = true;
					ms = reply_formating(servername.c_str(), RPL_YOUREOPER, {}, nick.c_str());
					custom_send(ms, this);
			} else {
				ms = reply_formating(servername.c_str(), ERR_PASSWDMISMATCH, {}, nick.c_str());
				custom_send(ms, this);
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_NOOPERHOST, {}, nick.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
		custom_send(ms, this);
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
	ev->cmd_count["MODE"] += 1;

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
					custom_send(ms, this);
				} else {
					ms = reply_formating(servername.c_str(), ERR_UMODEUNKNOWNFLAG, {}, nick.c_str());
					custom_send(ms, this);
				}
			} else {
				ms = reply_formating(servername.c_str(), ERR_USERSDONTMATCH, {}, nick.c_str());
				custom_send(ms, this);
			}
		} else {
			ev->channels->mode(this, cmd->arguments);
		}
	} else {
		if (cmd->arguments.size() == 1) {
			if (!ev->channels->getChanModes(this, cmd->arguments)) {
				ms = get_userMODEs_ms();
				custom_send(ms, this);
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
			custom_send(ms, this);
		}
	}
}

void	Client::QUIT(Command *cmd) {
	(void)cmd;
	std::string ans;
	std::string ms;
	ev->cmd_count["QUIT"] += 1;

	if (cmd->arguments.size()) {
		for (size_t i = 0; i < cmd->arguments.size(); i++) {
			ms += cmd->arguments[i];
			ms += " ";
		}
	} else {
		ms = nick;
	}
	if (type == FD_CLIENT)
		ev->client_history.push_back(this);
	ans += ":";
	ans += nick;
	ans += " QUIT :";
	ans += ms;
	ans += CRLF;
	custom_send(ans, this);
	ev->clients_fd[sock] = new Fd();
	close(sock);
}

void	Client::PRIVMSG(Command *cmd) {
	std::string 		ms;
	std::vector<Fd *> 	tmp;
	int					good = 0;
	ev->cmd_count["PRIVMSG"] += 1;
	
	if (cmd->arguments.size() >= 2) {
		for (std::string targ : parse_comma(cmd->arguments[0])) {
			if (targ[0] == '#') {
				// TODO : chans
			} else if (targ[0] == '$') {
				// TODO : multi server
			} else {
				if (!(tmp = ev->search_list_nick(targ)).empty()) {
					Client *c = reinterpret_cast<Client *>(tmp[0]);

					ms += ":";
					ms += nick;
					ms += " PRIVMSG ";
					ms += targ;
					ms += " ";
					for (size_t i = 1; i < cmd->arguments.size(); i++) {
						ms += cmd->arguments[i];
						ms += " ";
					}
					ms += CRLF;
					custom_send(ms, c);
					good += 1;
					if (c->is_away) {
						ms = reply_formating(servername.c_str(), RPL_AWAY, std::vector<std::string>({c->nick, c->away_ms}), nick.c_str());
						custom_send(ms, this);
					}
				} else {
					ms = reply_formating(servername.c_str(), ERR_NOSUCHNICK, {targ}, nick.c_str());
					custom_send(ms, this);
				}
			}
		}
		if (good == 0) {
			ms = reply_formating(servername.c_str(), ERR_NORECIPIENT, {cmd->line}, nick.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NOTEXTTOSEND, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::NOTICE(Command *cmd) {
	std::string 		ms;
	std::vector<Fd *> 	tmp;
	ev->cmd_count["NOTICE"] += 1;
	
	if (cmd->arguments.size() >= 2) {
		for (std::string targ : parse_comma(cmd->arguments[0])) {
			if (targ[0] == '#') {
				// TODO : chans
			} else if (targ[0] == '$') {
				// TODO : multi server
			} else {
				if (!(tmp = ev->search_list_nick(targ)).empty()) {
					Client *c = reinterpret_cast<Client *>(tmp[0]);

					ms += ":";
					ms += nick;
					ms += " NOTICE ";
					ms += targ;
					ms += " ";
					for (size_t i = 1; i < cmd->arguments.size(); i++) {
						ms += cmd->arguments[i];
						ms += " ";
					}
					ms += CRLF;
					custom_send(ms, c);
				}
			}
		}
	}
}

void	Client::MOTD(Command *cmd) {
	(void)cmd;
	std::string motd;
	ev->cmd_count["MOTD"] += 1;

	motd += " :::       ::: :::::::::: :::        ::::::::   ::::::::  ::::    ::::  ::::::::: \n";
	motd += " :+:       :+: :+:        :+:       :+:    :+: :+:    :+: +:+:+: :+:+:+ :+:       \n";
	motd += " +:+       +:+ +:+        +:+       +:+        +:+    +:+ +:+ +:+:+ +:+ +:+       \n";
	motd += " +#+  +:+  +#+ +#++:++#   +#+       +#+        +#+    +:+ +#+  +:+  +#+ +#++:++#  \n";
	motd += " +#+ +#+#+ +#+ +#+        +#+       +#+        +#+    +#+ +#+       +#+ +#+       \n";
	motd += "  #+#+# #+#+#  #+#        #+#       #+#    #+# #+#    #+# #+#       #+# #+#       \n";
	motd += "   ###   ###   ########## ########## ########   ########  ###       ### ######### \n";
	motd += "                                                                                  \n";
	motd += "                          Welcome to FT_IRC a 42 project                          \n";
	motd += "                                                                                  \n";
	motd += "                                 444    22222222                                  \n";
    motd += "                                444    222    222                                 \n";
    motd += "                               444 444       222                                  \n";
    motd += "                              444  444     222                                    \n";
    motd += "                             44444444444 222                                      \n";
    motd += "                                   444  222                                       \n";
    motd += "                                   444 2222222222                                 \n";
	motd += "                                                                                  \n";

	std::string line;
	std::string rd = std::string(motd);
	std::istringstream iss(rd);

	line = reply_formating(servername.c_str(), RPL_MOTDSTART, {servername.c_str()}, nick.c_str());
	custom_send(line, this);

	while (std::getline(iss, line) && !line.empty()) {
		line = reply_formating(servername.c_str(), RPL_MOTD, {line}, nick.c_str());
		custom_send(line, this);
    }

	line = reply_formating(servername.c_str(), RPL_ENDOFMOTD, {}, nick.c_str());
	custom_send(line, this);
}

void	Client::LUSERS(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["LUSERS"] += 1;
	
	// * 251
	std::string clients = std::to_string(ev->search_list_nick("*").size());
	std::string clients_inv = std::to_string(ev->search_list_with_mode("", "", 'i').size());
	ms = reply_formating(servername.c_str(), RPL_LUSERCLIENT, std::vector<std::string>({clients, clients_inv, "1"}), nick.c_str());
	custom_send(ms, this);

    // * 252
	std::string clients_op = std::to_string(ev->search_list_with_mode("", "", 'o').size());
	ms = reply_formating(servername.c_str(), RPL_LUSEROP, std::vector<std::string>({clients_op}), nick.c_str());
	custom_send(ms, this);
	
    // * 253
	ms = reply_formating(servername.c_str(), RPL_LUSERUNKNOWN, std::vector<std::string>({"0"}), nick.c_str());
	custom_send(ms, this);

    // * 254
	// TODO : when merging channel branch
	ms = reply_formating(servername.c_str(), RPL_LUSERCHANNELS, std::vector<std::string>({"0"}), nick.c_str());
	custom_send(ms, this);
	
	// * 255
	ms = reply_formating(servername.c_str(), RPL_LUSERME, std::vector<std::string>({clients, "1"}), nick.c_str());
	custom_send(ms, this);
}

void	Client::VERSION(Command *cmd) {
	std::string ms;
	ev->cmd_count["VERSION"] += 1;

	if (cmd->arguments.size() > 0) {
		if (cmd->arguments[0] == servername || cmd->arguments[0] == "*") {
			ms = reply_formating(servername.c_str(), RPL_VERSION, std::vector<std::string>({*ev->version, "1", servername, "first server-to-client iterations of irc for 42 project"}), nick.c_str());
			custom_send(ms, this);
		} else {
			ms = reply_formating(servername.c_str(), ERR_NOSUCHSERVER, {cmd->arguments[0]}, nick.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = reply_formating(servername.c_str(), RPL_VERSION, std::vector<std::string>({*ev->version, "1", servername, "first server-to-client iterations of irc for 42 project"}), nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::STATS(Command *cmd) {
	std::string ms;
	ev->cmd_count["STATS"] += 1;

	if (cmd->arguments.size() >= 2) {
		if (cmd->arguments[1] == servername || cmd->arguments[1] == "*") {
			if (cmd->arguments[0] == "m") {
				for (std::pair<std::string, int> cm : ev->cmd_count) {
					ms = reply_formating(servername.c_str(), RPL_STATSCOMMANDS, std::vector<std::string>({cm.first, std::to_string(cm.second)}), nick.c_str());
					custom_send(ms, this);
				}
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"m"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "c") {
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"c"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "h") {
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"h"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "i") {
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"i"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "k") {
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"k"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "l") {
				for (Fd *f : ev->search_list_nick("*")) {
					Client *c = reinterpret_cast<Client *>(f);
					ms += c->nick;
					ms += "!";
					ms += c->username;
					ms += "@";
					ms += hostname;
					ms += " ";
					ms += std::to_string(sendq);
					ms += " ";
					ms += std::to_string(send_ms);
					ms += " ";
					ms += std::to_string(Kb_sent / 1000);
					ms += " ";
					ms += std::to_string(recv_ms);
					ms += " ";
					ms += std::to_string(Kb_recv / 1000);
					time_t now;
					time(&now);
					int diff = difftime(now, creation);
					ms += " :";
					ms += std::to_string(diff);
					ms = reply_formating(servername.c_str(), RPL_STATSLINKINFO, {ms}, nick.c_str());
					custom_send(ms, this);
				}
				// TODO : adding the same as above for servers when server-to-server
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"l"}, nick.c_str());
				custom_send(ms, this);
			}  else if (cmd->arguments[0] == "o") {
				ms = reply_formating(servername.c_str(), RPL_STATSOLINE, std::vector<std::string>({"", servername}), nick.c_str());
				custom_send(ms, this);
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"o"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "y") {
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"y"}, nick.c_str());
				custom_send(ms, this);
			} else if (cmd->arguments[0] == "u") {
				time_t now;
				time(&now);
				double diff = difftime(now, creation);
				std::string day = std::to_string(static_cast<long int>(diff / 86400));
				std::string	hour = std::to_string(static_cast<int>(diff / 3600) % 24);
				std::string	minutes = std::to_string(static_cast<int>(diff / 60) % 60);
				std::string	secs = std::to_string(static_cast<int>(diff) % 60);
				ms = reply_formating(servername.c_str(), RPL_STATSUPTIME, std::vector<std::string>({day, hour, minutes, secs}), nick.c_str());
				custom_send(ms, this);
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {"u"}, nick.c_str());
				custom_send(ms, this);
				// TODO : test long time after solving PING
			} else {
				ms = reply_formating(servername.c_str(), RPL_ENDOFSTATS, {std::string(&cmd->arguments[0][0], &cmd->arguments[0][1])}, nick.c_str());
				custom_send(ms, this); 
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_NOSUCHSERVER, {cmd->arguments[0]}, nick.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::LINKS(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["LINKS"] += 1;
	
	ms = reply_formating(servername.c_str(), RPL_ENDOFLINKS, {""}, nick.c_str());
	custom_send(ms, this);
}

void	Client::TIME(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["TIME"] += 1;
	time_t		now = time(NULL);

	ms = asctime(localtime(&now));
	ms = std::string(&ms[0], &ms[ms.size() - 1]);
	ms = reply_formating(servername.c_str(), RPL_TIME, std::vector<std::string>({servername, ms}), nick.c_str());
	custom_send(ms, this);
}

void	Client::ADMIN(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["ADMIN"] += 1;

	ms = reply_formating(servername.c_str(), RPL_ADMINME, {servername}, nick.c_str());
	custom_send(ms, this);
	ms = reply_formating(servername.c_str(), RPL_ADMINLOC1, {ev->loc1}, nick.c_str());
	custom_send(ms, this);
	ms = reply_formating(servername.c_str(), RPL_ADMINLOC2, {ev->loc2}, nick.c_str());
	custom_send(ms, this);
	for (std::string s : ev->emails) {
		ms = reply_formating(servername.c_str(), RPL_ADMINEMAIL, {s}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::INFO(Command *cmd) {
	(void)cmd;
	std::string inf;
	std::string buf;
	ev->cmd_count["INFO"] += 1;

	inf += " IRC --\n";
	inf += " Based on RFCs given on 42 subject page : \n";
	inf += " https://github.com/qfeuilla/ft_irc/blob/master/en.subject.pdf \n";
	inf += " \n";
	inf += " This program is free software; you can redistribute it and/or\n";
	inf += " modify it under the terms of the GNU General Public License as\n";
	inf += " published by the Free Software Foundation; either version 2, or\n";
	inf += " (at your option) any later version.\n";
	inf += " \n";
	inf += " ft_irc has been developed to meet the policy needs of the\n";
	inf += " RFCs IRC network. \n";
	inf += "  \n";
	inf += " ft_irc development is currently ongoin with developpers :\n";
	inf	+= " Quentin FEUILLADE--MONTIXI and Mayeul Le Monies De Sagazan\n";
	inf += " see our Github : \n";
	inf += " 	- https://github.com/qfeuilla \n";
	inf += " 	- https://github.com/mle-moni \n";
	inf += " \n";
	inf += " \n";
	inf += " On-line since:";
	buf = std::asctime(gmtime(&(ev->start)));
	inf += std::string(&buf[0], &buf[buf.size() - 1]);
	inf += " UTC \n";
	
	std::string line;
	std::string rd = std::string(inf);
	std::istringstream iss(rd);

	while (std::getline(iss, line) && !line.empty()) {
		line = reply_formating(servername.c_str(), RPL_INFO, {line}, nick.c_str());
		custom_send(line, this);
    }

	line = reply_formating(servername.c_str(), RPL_ENDOFINFO, {}, nick.c_str());
	custom_send(line, this);
}

void	Client::WHO(Command *cmd) {
	std::string ms;
	ev->cmd_count["WHO"] += 1;

	if (cmd->arguments.size() >= 1) {
		for (Fd *f: ev->search_list_nick(cmd->arguments[0])) {
			Client *c = reinterpret_cast<Client *>(f);
			// TODO : addind same channel checking
			if (c->o_mode || cmd->arguments.size() == 1
				|| cmd->arguments[1] != "o") {
				ms = c->username;
				ms += " ";
				ms += c->hostname;
				ms += " ";
				ms += c->servername;
				ms += " ";
				ms += c->nick;
				ms += " H :0 ";
				ms += c->realname;
				ms = reply_formating(servername.c_str(), RPL_WHOREPLY, {ms},nick.c_str());
				custom_send(ms, this);
			}
		}
	} else {
		for (Fd *f: ev->clients_fd) {
			if (f->type == FD_CLIENT) {
				Client *c = reinterpret_cast<Client *>(f);
				// TODO : addind same channel checking
				if (!c->i_mode) {
					ms += c->username;
					ms += " ";
					ms += c->hostname;
					ms += " ";
					ms += c->servername;
					ms += " ";
					ms += c->nick;
					ms += " H :0 ";
					ms += c->realname;
					ms = reply_formating(servername.c_str(), RPL_WHOREPLY, {ms},nick.c_str());
					custom_send(ms, this);
				}
			}
		}
	}
	ms = reply_formating(servername.c_str(), RPL_ENDOFWHO, {nick}, nick.c_str());
	custom_send(ms, this);
}

void	Client::WHOIS(Command *cmd) {
	// ! ignoring server parametre because no multi server for the moment
	std::string ms;
	ev->cmd_count["WHOIS"] += 1;
	std::vector<Fd *> tmp;

	if (cmd->arguments.size() >= 1) {
		for (std::string targ : parse_comma(cmd->arguments[0])) {
			if (!(tmp = ev->search_list_nick(targ)).empty()) {
				Client *c = reinterpret_cast<Client *>(tmp[0]);
				ms = reply_formating(servername.c_str(), RPL_WHOISUSER, std::vector<std::string>({c->nick, c->username, c->hostname, c->realname}), nick.c_str());
				custom_send(ms, this);
				ms = asctime(localtime(&c->last));
				ms = std::string(&ms[0], &ms[ms.size() - 1]);
				ms = reply_formating(servername.c_str(), RPL_WHOISSERVER, std::vector<std::string>({c->nick, c->servername, ms}), nick.c_str());
				custom_send(ms, this);
			} else {
				ms = reply_formating(servername.c_str(), ERR_NOSUCHNICK, {targ}, nick.c_str());
				custom_send(ms, this);
			}
		}
		ms = reply_formating(servername.c_str(), RPL_ENDOFWHOIS, {nick}, nick.c_str());
		custom_send(ms, this);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NONICKNAMEGIVEN, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::WHOWAS(Command *cmd) {
	// ! ignoring server parametre because no multi server for the moment
	std::string ms;
	ev->cmd_count["WHOWAS"] += 1;
	std::vector<Fd *> tmp;

	if (cmd->arguments.size() >= 1) {
		for (std::string targ : parse_comma(cmd->arguments[0])) {
			if (!(tmp = ev->search_history_nick(targ)).empty()) {
				Client *c = reinterpret_cast<Client *>(tmp[0]);
				ms = reply_formating(servername.c_str(), RPL_WHOWASUSER, std::vector<std::string>({c->nick, c->username, c->hostname, c->realname}), nick.c_str());
				custom_send(ms, this);
				ms = asctime(localtime(&c->last));
				ms = std::string(&ms[0], &ms[ms.size() - 1]);
				ms = reply_formating(servername.c_str(), RPL_WHOISSERVER, std::vector<std::string>({c->nick, c->servername, ms}), nick.c_str());
				custom_send(ms, this);
			} else {
				ms = reply_formating(servername.c_str(), ERR_NOSUCHNICK, {targ}, nick.c_str());
				custom_send(ms, this);
			}
		}
		ms = reply_formating(servername.c_str(), RPL_ENDOFWHOWAS, {nick}, nick.c_str());
		custom_send(ms, this);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NONICKNAMEGIVEN, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::KILL(Command *cmd) {
	std::string ms;
	ev->cmd_count["KILL"] += 1;
	std::vector<Fd *> tmp;

	if (cmd->arguments.size() >= 2) {
		if (o_mode) {
			if (!(tmp = ev->search_list_nick(cmd->arguments[0])).empty()) {
				Client *c = reinterpret_cast<Client *>(tmp[0]);
				
				ms = "QUIT ";
				for (size_t i = 1; i < cmd->arguments.size(); i++) {
					ms += cmd->arguments[i];
					ms += " ";
				}
				ms += CR;
				c->execute_parsed(parse(ms));
			} else {
				ms = reply_formating(servername.c_str(), ERR_NOSUCHNICK, {}, nick.c_str());
				custom_send(ms, this);
			}
		} else {
			ms = reply_formating(servername.c_str(), ERR_NOPRIVILEGES, {}, nick.c_str());
			custom_send(ms, this);
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::PING(Command *cmd) {
	std::string ms;
	ev->cmd_count["PING"] += 1;

	if (cmd->arguments.size() >= 1) {
		ms = ":";
		ms += servername;		
		ms += " PONG ";
		ms += servername;
		ms += " :";
		for (size_t i = 0; i < cmd->arguments.size(); i++) {
			ms += cmd->arguments[i];
			ms += " ";
		}
		ms += CRLF;
		custom_send(ms, this);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NOORIGIN, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::AWAY(Command *cmd) {
	std::string ms;
	ev->cmd_count["AWAY"] += 1;

	if (cmd->arguments.size() >= 1) {
		is_away = true;
		for (size_t i = 0; i < cmd->arguments.size(); i++) {
			ms += cmd->arguments[i];
			ms += " ";
		}
		away_ms = ms;
		ms = reply_formating(servername.c_str(), RPL_AWAY, std::vector<std::string>({nick, ms}), nick.c_str());
		custom_send(ms, this);
	} else {
		is_away = false;
		ms = reply_formating(servername.c_str(), RPL_UNAWAY, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::DIE(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["DIE"] += 1;

	if (o_mode) {
		ev->active = false;
	} else {
		ms = reply_formating(servername.c_str(), ERR_NOPRIVILEGES, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::SUMMON(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["SUMMON"] += 1;

	ms = reply_formating(servername.c_str(), ERR_SUMMONDISABLED, {}, nick.c_str());
	custom_send(ms, this);
}

void	Client::USERS(Command *cmd) {
	(void)cmd;
	std::string ms;
	ev->cmd_count["USERS"] += 1;

	ms = reply_formating(servername.c_str(), ERR_USERSDISABLED, {}, nick.c_str());
	custom_send(ms, this);
}

void	Client::WALLOPS(Command *cmd) {
	std::string ms;
	ev->cmd_count["WALLOPS"] += 1;

	if (!cmd->prefix.empty() && !check_nick(cmd->prefix)) {
		// * Command only send by a server
		if (cmd->arguments.size() >= 1) {
			ms = ":";
			ms += servername;
			ms += " WALLOPS ";
			ms += nick;
			ms += " ";
			for (size_t i = 0; i < cmd->arguments.size(); i++) {
				ms += cmd->arguments[i];
				ms += " ";
			}
			for (Fd *f : ev->search_list_with_mode("", "", 'w')) {
				Client *c = reinterpret_cast<Client *>(f);
				ms += CRLF;
				custom_send(ms, c);
			}
		}
	} else {
		ms = reply_formating(servername.c_str(), ERR_NOCLIENTCMD, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::USERHOST(Command *cmd) {
	std::string ms;
	ev->cmd_count["USERHOST"] += 1;
	std::vector<Fd *>		tmp;

	if (cmd->arguments.size() >= 1) {
		for (std::string targ : cmd->arguments) {
			if (!(tmp = ev->search_list_nick(targ)).empty()) {
				Client *c = reinterpret_cast<Client *>(tmp[0]);
				ms += " ";
				ms += c->nick;
				ms += "=";
				ms += c->username;
				ms += "@";
				ms += c->hostname;
				ms += "/";
				ms += std::to_string(htons(c->csin.sin_port));
			}
		}
		ms = reply_formating(servername.c_str(), RPL_USERHOST, {ms}, nick.c_str());
		custom_send(ms, this);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::ISON(Command *cmd) {
	std::string ms;
	ev->cmd_count["ISON"] += 1;
	std::vector<Fd *>		tmp;

	if (cmd->arguments.size() >= 1) {
		for (std::string targ : cmd->arguments) {
			if (!(tmp = ev->search_list_nick(targ)).empty()) {
				Client *c = reinterpret_cast<Client *>(tmp[0]);
				ms += " ";
				ms += c->nick;
			}
		}
		ms = reply_formating(servername.c_str(), RPL_ISON, {ms}, nick.c_str());
		custom_send(ms, this);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {cmd->line}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::JOIN(Command *cmd) {
	std::string ms;
	if (cmd->arguments.size() >= 1) {
		ev->channels->join(this, cmd->arguments, &channels);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::PART(Command *cmd) {
	std::string ms;
	if (cmd->arguments.size() >= 1) {
		ev->channels->leave(this, cmd->arguments);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::KICK(Command *cmd) {
	std::string ms;
	if (cmd->arguments.size() >= 2) {
			ev->channels->kick(this, cmd->arguments);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {}, nick.c_str());
		custom_send(ms, this);
	}
}

void	Client::TOPIC(Command *cmd) {
	std::string ms;
	if (cmd->arguments.size() >= 1) {
		ev->channels->topic(this, cmd->arguments);
	} else {
		ms = reply_formating(servername.c_str(), ERR_NEEDMOREPARAMS, {}, nick.c_str());
		custom_send(ms, this);
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
	// ! TODO INVITE
	case QUIT_CC:
		QUIT(parsed);
		break;
	case PRIVMSG_CC:
		PRIVMSG(parsed);
		break;
	case NOTICE_CC:
		NOTICE(parsed);
		break;
	case MOTD_CC:
		MOTD(parsed);
		break;
	case LUSERS_CC:
		LUSERS(parsed);
		break;
	case VERSION_CC:
		VERSION(parsed);
		break;
	case STATS_CC:
		STATS(parsed);
		break;
	case LINKS_CC:
		LINKS(parsed);
		break;
	case TIME_CC:
		TIME(parsed);
		break;
	case ADMIN_CC:
		ADMIN(parsed);
		break;
	case INFO_CC:
		INFO(parsed);
		break;
	case WHO_CC:
		WHO(parsed);
		break;
	case WHOIS_CC:
		WHOIS(parsed);
		break;
	case WHOWAS_CC:
		WHOWAS(parsed);
		break;
	case KILL_CC:
		KILL(parsed);
		break;
	case PING_CC:
		PING(parsed);
		break;
	case AWAY_CC:
		AWAY(parsed);
		break;
	case DIE_CC:
		DIE(parsed);
		break;
	case SUMMON_CC:
		SUMMON(parsed);
		break;
	case USERS_CC:
		USERS(parsed);
		break;
	case WALLOPS_CC:
		WALLOPS(parsed);
		break;
	case USERHOST_CC:
		USERHOST(parsed);
		break;
	case ISON_CC:
		ISON(parsed);
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
	time(&last);
	while (std::getline(iss, line) && !line.empty()) {
		Kb_sent += sizeof(line);
		parsed = parse(line);
		send_ms += 1;
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
	o << "IP addr : " << inet_ntoa(cl.csin.sin_addr) << "\n";
	o << "Port : " << htons(cl.csin.sin_port) << "\n"; 
	return (o);
}