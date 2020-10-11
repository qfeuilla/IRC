/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/20 16:41:00 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/12 01:09:53 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"
#include "defines.hpp"
#include "utils.hpp"

Command::~Command() { }

Command::Command(std::vector<std::string> cmd, std::string ln) {
	std::string		tmp;
	
	if (cmd[0][0] == ':') {
		tmp = cmd[0];
		prefix = std::string(&tmp[1], &tmp[tmp.length()]);
		command = cmd[1];
		if (cmd.size() > 2)
			arguments = std::vector<std::string>(&cmd[2], &cmd[cmd.size()]);
	} else {
		prefix = "";
		command = cmd[0];
		if (cmd.size() > 1)
			arguments = std::vector<std::string>(&cmd[1], &cmd[cmd.size()]);
	}
	line = ln;
}

int		Command::cmd_code() const {
	std::string upperCaseCommand = utils::ircUpperCase(command);

	if (upperCaseCommand == "PASS" || upperCaseCommand == ":PASS")	
		return (PASS_CC);
	else if (upperCaseCommand == "NICK" || upperCaseCommand == ":NICK")
		return (NICK_CC);
	else if (upperCaseCommand == "USER" || upperCaseCommand == ":USER")
		return (USER_CC);
	else if (upperCaseCommand == "OPER" || upperCaseCommand == ":OPER")	
		return (OPER_CC);
	else if (upperCaseCommand == "MODE" || upperCaseCommand == ":MODE")	
		return (MODE_CC);
	else if (upperCaseCommand == "SERVICE" || upperCaseCommand == ":SERVICE")	
		return (SERVICE_CC);
	else if (upperCaseCommand == "QUIT" || upperCaseCommand == ":QUIT")	
		return (QUIT_CC);
	else if (upperCaseCommand == "SQUIT" || upperCaseCommand == ":SQUIT")	
		return (SQUIT_CC);
	else if (upperCaseCommand == "JOIN" || upperCaseCommand == ":JOIN")	
		return (JOIN_CC);
	else if (upperCaseCommand == "PART" || upperCaseCommand == ":PART")	
		return (PART_CC);
	else if (upperCaseCommand == "TOPIC" || upperCaseCommand == ":TOPIC")	
		return (TOPIC_CC);
	else if (upperCaseCommand == "NAMES" || upperCaseCommand == ":NAMES")	
		return (NAMES_CC);
	else if (upperCaseCommand == "LIST" || upperCaseCommand == ":LIST")	
		return (LIST_CC);
	else if (upperCaseCommand == "INVITE" || upperCaseCommand == ":INVITE")	
		return (INVITE_CC);
	else if (upperCaseCommand == "KICK" || upperCaseCommand == ":KICK")	
		return (KICK_CC);
	else if (upperCaseCommand == "PRIVMSG" || upperCaseCommand == ":PRIVMSG")	
		return (PRIVMSG_CC);
	else if (upperCaseCommand == "NOTICE" || upperCaseCommand == ":NOTICE")	
		return (NOTICE_CC);
	else if (upperCaseCommand == "MOTD" || upperCaseCommand == ":MOTD")	
		return (MOTD_CC);
	else if (upperCaseCommand == "LUSERS" || upperCaseCommand == ":LUSERS")	
		return (LUSERS_CC);
	else if (upperCaseCommand == "VERSION" || upperCaseCommand == ":VERSION")	
		return (VERSION_CC);
	else if (upperCaseCommand == "STATS" || upperCaseCommand == ":STATS")	
		return (STATS_CC);
	else if (upperCaseCommand == "LINKS" || upperCaseCommand == ":LINKS")	
		return (LINKS_CC);
	else if (upperCaseCommand == "TIME" || upperCaseCommand == ":TIME")	
		return (TIME_CC);
	else if (upperCaseCommand == "CONNECT" || upperCaseCommand == ":CONNECT")	
		return (CONNECT_CC);
	else if (upperCaseCommand == "TRACE" || upperCaseCommand == ":TRACE")	
		return (TRACE_CC);
	else if (upperCaseCommand == "ADMIN" || upperCaseCommand == ":ADMIN")	
		return (ADMIN_CC);
	else if (upperCaseCommand == "INFO" || upperCaseCommand == ":INFO")	
		return (INFO_CC);
	else if (upperCaseCommand == "SERVLIST" || upperCaseCommand == ":SERVLIST")	
		return (SERVLIST_CC);
	else if (upperCaseCommand == "SQUERY" || upperCaseCommand == ":SQUERY")	
		return (SQUERY_CC);
	else if (upperCaseCommand == "WHO" || upperCaseCommand == ":WHO")	
		return (WHO_CC);
	else if (upperCaseCommand == "WHOIS" || upperCaseCommand == ":WHOIS")	
		return (WHOIS_CC);
	else if (upperCaseCommand == "WHOWAS" || upperCaseCommand == ":WHOWAS")	
		return (WHOWAS_CC);
	else if (upperCaseCommand == "KILL" || upperCaseCommand == ":KILL")	
		return (KILL_CC);
	else if (upperCaseCommand == "PING" || upperCaseCommand == ":PING")	
		return (PING_CC);
	else if (upperCaseCommand == "PONG" || upperCaseCommand == ":PONG")	
		return (PONG_CC);
	else if (upperCaseCommand == "ERROR" || upperCaseCommand == ":ERROR")	
		return (ERROR_CC);
	else if (upperCaseCommand == "AWAY" || upperCaseCommand == ":AWAY")
		return (AWAY_CC);
	else if (upperCaseCommand == "REHASH" || upperCaseCommand == ":REHASH")	
		return (REHASH_CC);
	else if (upperCaseCommand == "DIE" || upperCaseCommand == ":DIE")	
		return (DIE_CC);
	else if (upperCaseCommand == "RESTART" || upperCaseCommand == ":RESTART")	
		return (RESTART_CC);
	else if (upperCaseCommand == "SUMMON" || upperCaseCommand == ":SUMMON")	
		return (SUMMON_CC);
	else if (upperCaseCommand == "USERS" || upperCaseCommand == ":USERS")	
		return (USERS_CC);
	else if (upperCaseCommand == "WALLOPS" || upperCaseCommand == ":WALLOPS")	
		return (WALLOPS_CC);
	else if (upperCaseCommand == "USERHOST" || upperCaseCommand == ":USERHOST")	
		return (USERHOST_CC);
	else if (upperCaseCommand == "ISON" || upperCaseCommand == ":ISON")	
		return (ISON_CC);
	else if (upperCaseCommand == "SERVER" || upperCaseCommand == ":SERVER")	
		return (SERVER_CC);
	else if (upperCaseCommand == "NJOIN" || upperCaseCommand == ":NJOIN")	
		return (NJOIN_CC);
	else if (upperCaseCommand == "ADDS" || upperCaseCommand == ":ADDS")
		return (ADDS_CC);
	else if (upperCaseCommand == "NSERV" || upperCaseCommand == ":NSERV")
		return (NSERV_CC);
	else if (upperCaseCommand == "DELS" || upperCaseCommand == ":DELS")
		return (DELS_CC);
	else if (upperCaseCommand == "TRACEUP" || upperCaseCommand == ":TRACEUP")
		return (TRACEUP_CC);
	else if (upperCaseCommand == "CHAN_CHG" || upperCaseCommand == ":CHAN_CHG")
		return (CHAN_CHG_CC);
	else if (upperCaseCommand == "CHAN_RPL" || upperCaseCommand == ":CHAN_RPL")
		return (CHAN_RPL_CC);
	else if (upperCaseCommand == "READY" || upperCaseCommand == ":READY")
		return (READY_CC);
	else if (upperCaseCommand == "351")
		return (RPL_351_CC);
	else if (upperCaseCommand == "364")
		return (RPL_364_CC);
	else if (upperCaseCommand == "365")
		return (RPL_365_CC);
	else
		return (WRONG_CMD);
}

std::ostream &			operator<<( std::ostream &o, Command const & cmd ) {
	o << "Line : " << cmd.line << "\n";
	if (!cmd.prefix.empty())
		o << "Prefix : " << cmd.prefix << "\n";
	o << "Command : " << cmd.command << "\n";
	o << "Arguments : [";
	if (cmd.arguments.size() >= 1) {
		for (size_t i = 0; i < cmd.arguments.size() - 1; i++)
				o << cmd.arguments[i] << ", ";
		o << cmd.arguments[cmd.arguments.size() - 1];
	}
	o << "]\n";
	return (o);
}