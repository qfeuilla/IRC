/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   defines.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/20 17:01:28 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/07 19:37:12 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEFINES_HPP
# define DEFINES_HPP

# define TLS_PORT		6697

// * utilitie for format

# define CRLF			"\r\n"
# define CR				"\r"

// * User types

# define FD_FREE		0
# define FD_SERVER		1
# define FD_CLIENT		2
# define FD_WAITC		3
# define FD_OTHER		4
# define FD_OCLIENT		5

# define BUF_SIZE		4096
# define CLIENTS_MAX	512

// * Command number for switch operations

# define WRONG_CMD		0
# define PASS_CC		1 // * done
# define NICK_CC		2 // * done
# define USER_CC		3 // * done
# define OPER_CC		4 // * done
# define MODE_CC		5 // * done
# define SERVICE_CC		6 // * Not implemented, used for chat bot
# define QUIT_CC		7 // * done
# define SQUIT_CC		8 // TODO : When server to server will be implmented
# define JOIN_CC		9 // TODO : When biniding channel to this branch
# define PART_CC		10 // TODO : When binding channel to this branch
# define TOPIC_CC		11 // TODO : When binding channel to this branch
# define NAMES_CC		12 // TODO : When binding channel to this branch
# define LIST_CC		13 // TODO : When binding channel to this branch
# define INVITE_CC		14 // TODO : When binding channel to this branch
# define KICK_CC		15 // TODO : When binding channel to this branch
# define PRIVMSG_CC		16 // TODO : extanding when multi server
# define NOTICE_CC		17 // TODO : same as PRIVMSG
# define MOTD_CC		18 // * done
# define LUSERS_CC		19 // TODO : extanding when multi server
# define VERSION_CC		20 // TODO : extanding when multi server
# define STATS_CC		21 // TODO : extanding when multi server
# define LINKS_CC		22 // TODO : extanding when multi server
# define TIME_CC		23 // TODO : extanding when multi server
# define CONNECT_CC		24 // TODO : when server to server
# define TRACE_CC		25 // TODO : when server to server
# define ADMIN_CC		26 // TODO : extanding when multi server
# define INFO_CC		27 // TODO : extanding when multi server
# define SERVLIST_CC	28 // TODO : when server to server
# define SQUERY_CC		29 // * only use for services (bots) so not implemented
# define WHO_CC			30 // TODO : extanding when multi server
# define WHOIS_CC		31 // TODO : extanding when multi server
# define WHOWAS_CC		32 // TODO : extanding when multi server
# define KILL_CC		33 // * done
# define PING_CC		34 // TODO : extanding when multi server
# define PONG_CC		35 // * Only reply
# define ERROR_CC		36 // TODO : when server to server
# define AWAY_CC		37 // * done
# define REHASH_CC		38 // * No config file for server 
# define DIE_CC			39 // * done
# define RESTART_CC		40 // * optionnal for security reason so not used
# define SUMMON_CC		41 // * done
# define USERS_CC		42 // * done
# define WALLOPS_CC		43 // * done
# define USERHOST_CC	44 // * done
# define ISON_CC		45 // * done
# define SERVER_CC		46 // TODO : when server to server
# define NJOIN_CC		47 // TODO : when server to server
# define ADDS_CC		48 // * Command used to notify adding of a new server
# define NSERV_CC		49 // * Command to notify entrying server of num of SERVERS
# define DELS_CC		50 // * Command used to notify deleting of a new server
# define TRACEUP_CC		51 // * Used for trace
# define CHAN_CHG_CC	52 // * Used between servs to notify the changes of a channel in the network
# define CHAN_RPL_CC	53 // * Used between servs to forward a RPL / ERR to the user that triggered it
# define READY_CC		54 // * used to notify  the a server that he is ready to get his config files


// * Messages necessay rights to execute

# define CLIENT_R 1
# define SERVER_R 2
# define BOTH_R 3

// * Numeric replies

// "Welcome to the Internet Relay Network <nick>!<user>@<host>"
# define RPL_WELCOME           {"001", "Welcome to the Internet Relay Network {}!{}@{}", 3}
// "Your host is <servername>, running version <ver>"
# define RPL_YOURHOST          {"002", "Your host is {}, running version {}", 2}
// "This server was created <date>"
# define RPL_CREATED           {"003", "This server was created {}", 1}
// "<servername> <version> <available user modes> <available channel modes>"
# define RPL_MYINFO            {"004", "{} {} {} {}", 4}
// "Try server <server name>, port <port number>"
# define RPL_BOUNCE            {"005", "Try server {}, port {}", 2}
// ":*1<reply> *( " " <reply> )" // * -> one arg
# define RPL_USERHOST          {"302", ":{}", 1}
// ":*1<nick> *( " " <nick> )" // *-> one arg
# define RPL_ISON              {"303", ":{}", 1}
//  "<nick> :<away message>"
# define RPL_AWAY              {"301", "{} :{}", 2}
// 
# define RPL_UNAWAY            {"305", ":You are no longer marked as being away", 0}
// 
# define RPL_NOWAWAY           {"306", ":You have been marked as being away", 0}
// "<nick> <user> <host> * :<real name>"
# define RPL_WHOISUSER         {"311", "{} {} {} * :{}", 4}
// "<nick> <server> :<server info>"
# define RPL_WHOISSERVER       {"312", "{} {} :{}", 3}
// "<nick> :is an IRC operator"
# define RPL_WHOISOPERATOR     {"313", "{} :is an IRC operator", 1}
// "<nick> <integer> :seconds idle"
# define RPL_WHOISIDLE         {"317", "{} {} :seconds idle", 2}
// "<nick> :End of WHOIS list"
# define RPL_ENDOFWHOIS        {"318", "{} :End of WHOIS list", 1}
// "<nick> :*( ( "@" / "+" ) <channel> " " )" // *-> one arg
# define RPL_WHOISCHANNELS     {"319", "{}", 1}
// "<nick> <user> <host> * :<real name>"
# define RPL_WHOWASUSER        {"314", "{} {} {} * :{}", 4}
// "<nick> :End of WHOWAS"
# define RPL_ENDOFWHOWAS       {"369", "{} :End of WHOWAS", 1}
// "<channel> <# visible> :<topic>"
# define RPL_LIST              {"322", "{} {} :{}", 3}
// 
# define RPL_LISTEND           {"323", ":End of LIST", 0}
// "<channel> <nickname>"
# define RPL_UNIQOPIS          {"325", "{} {}", 2}
// "<channel> <mode> <mode params>"
# define RPL_CHANNELMODEIS     {"324", "{} {} {}", 3}
// "<channel> :No topic is set"
# define RPL_NOTOPIC           {"331", "{} :No topic is set", 1}
// "<channel> :<topic>"
# define RPL_TOPIC             {"332", "{} :{}", 2}
// "<channel> <nick>"
# define RPL_INVITING          {"341", "{} {}", 2}
// "<user> :Summoning user to IRC"
# define RPL_SUMMONING         {"342", "{} :Summoning user to IRC", 1}
// "<channel> <invitemask>"
# define RPL_INVITELIST        {"346", "{} {}", 2}
// "<channel> :End of channel invite list"
# define RPL_ENDOFINVITELIST   {"347", "{} :End of channel invite list", 1}
// "<channel> <exceptionmask>"
# define RPL_EXCEPTLIST        {"348", "{} {}", 2}
// "<channel> :End of channel exception list"
# define RPL_ENDOFEXCEPTLIST   {"349", "{} :End of channel exception list", 1}
// "<version>.<debuglevel> <server> :<comments>"
# define RPL_VERSION           {"351", "{}.{} {} :{}", 4}
// "<channel> <user> <host> <server> <nick> 
// ( "H" / "G" > ["*"] [ ( "@" / "+" ) ]:<hopcount> <real name>" // * -> one arg
# define RPL_WHOREPLY          {"352", "{}", 1}
// "<name> :End of WHO list"
# define RPL_ENDOFWHO          {"315", "{} :End of WHO list", 1}
// "( "=" / "*" / "@" ) <channel>
// :[ "@" / "+" ] <nick> *( " " [ "@" / "+" ] <nick> )" // * -> one arg
# define RPL_NAMREPLY          {"353", "{}", 1}
// "<channel> :End of NAMES list"
# define RPL_ENDOFNAMES        {"366", "{} :End of NAMES list", 1}
// "<mask> <server> :<hopcount> <server info>"
# define RPL_LINKS             {"364", "{} {} :{} {}", 4}
// "<mask> :End of LINKS list"
# define RPL_ENDOFLINKS        {"365", "{} :End of LINKS list", 1}
// "<channel> <banmask>"
# define RPL_BANLIST           {"367", "{} {}", 2}
// "<channel> :End of channel ban list"
# define RPL_ENDOFBANLIST      {"368", "{} :End of channel ban list", 1}
// ":<string>"
# define RPL_INFO              {"371", ":{}", 1}
// 
# define RPL_ENDOFINFO         {"374", ":End of INFO list", 0}
// ":- {} Message of the day - "
# define RPL_MOTDSTART         {"375", ":- {} Message of the day - ", 1}
// ":- <text>"
# define RPL_MOTD              {"372", ":- {}", 1}
// 
# define RPL_ENDOFMOTD         {"376", ":End of MOTD command", 0}
// 
# define RPL_YOUREOPER         {"381", ":You are now an IRC operator", 0}
// "<config file> :Rehashing"
# define RPL_REHASHING         {"382", "{} : Rehashing", 1}
// "You are service <servicename>"
# define RPL_YOURESERVICE      {"383", "You are service {}", 1}
// "<server> :<string showing server's local time>"
# define RPL_TIME              {"391", "{} :{}", 2}
// 
# define RPL_USERSSTART        {"392", ":UserID   Terminal  Host", 0}
// ":<username> <ttyline> <hostname>"
# define RPL_USERS             {"393", ":{} {} {}", 3}
// 
# define RPL_ENDOFUSERS        {"394", ":End of users", 0}
// 
# define RPL_NOUSERS           {"395", ":Nobody logged in", 0}
// "Link <version & debug level> <destination> 
// <next server> V<protocol version> <link uptime in seconds>
// <backstream sendq> <upstream sendq>" // * -> one arg
# define RPL_TRACELINK         {"200", "{}", 1}
// "Try. <class> <server>"
# define RPL_TRACECONNECTING   {"201", "Try. {} {}", 2}
// "H.S. <class> <server>"
# define RPL_TRACEHANDSHAKE    {"202", "H.S. {} {}", 2}
// "???? <class> [<client IP address in dot form>]"
# define RPL_TRACEUNKNOWN      {"203", "???? {} {}", 2}
// "Oper <class> <nick>"
# define RPL_TRACEOPERATOR     {"204", "Oper {} {}", 2}
// "User <class> <info>"
// <info> : nick[username@servername] (hostname) hop :since_creation
# define RPL_TRACEUSER         {"205", "User {} {}", 2}
// "Serv <class> <int>S <int>C <server>
// <nick!user|*!*>@<host|server> V<protocol version>" // * -> one arg
# define RPL_TRACESERVER       {"206", "{}", 1}
// "Service <class> <name> <type> <active type>"
# define RPL_TRACESERVICE      {"207", "Service {} {} {} {}", 4}
// "<newtype> 0 <client name>"
# define RPL_TRACENEWTYPE      {"208", "{} 0 {}", 2}
// "Class <class> <count>"
# define RPL_TRACECLASS        {"209", "Class {} {}", 2}
// "File <logfile> <debug level>"
# define RPL_TRACELOG          {"261", "File {} {}", 2}
// "<server name> <version & debug level> :End of TRACE"
# define RPL_TRACEEND          {"262", "{} {} :End of TRACE", 2}
// "<linkname> <sendq> <sent messages>
//	<sent Kbytes> <received messages>
//	<received Kbytes> <time open>" // * -> one arg
# define RPL_STATSLINKINFO     {"211", "{}", 1}
// "<command> <count>"
# define RPL_STATSCOMMANDS     {"212", "{} {}", 2}
// "C <name> <port>"
#define RPL_STATSCLINE		   {"213", "C {}:{}", 2}
// "<stats letter> :End of STATS report"
# define RPL_ENDOFSTATS        {"219", "{} :End of STATS report", 1}
// ":Server Up <days> days <hours>:<minutes>:<seconds>"
# define RPL_STATSUPTIME       {"242", ":Server Up {} days {}:{}:{}", 4}
// "O <hostmask> * <name>"
# define RPL_STATSOLINE        {"243", "O {} * {}", 2}
// "<user mode string>"
# define RPL_UMODEIS           {"221", "{}", 1}
// "<name> <server> <mask> <type> <hopcount> <info>"
# define RPL_SERVLIST          {"234", "{} {} {} {} {} {}", 6}
// "<mask> <type> :End of service listing"
# define RPL_SERVLISTEND       {"235", "{} {} :End of service listing", 2}
// ":There are <integer> users and <integer> services on <integer> servers"
# define RPL_LUSERCLIENT       {"251", ":There are {} users and {} invisible on {} servers", 3}
// "<integer> :operator(s) online"
# define RPL_LUSEROP           {"252", "{} :operator(s) online", 1}
// "<integer> :unknown connection(s)"
# define RPL_LUSERUNKNOWN      {"253", "{} :unknown connection(s)", 1}
// "<integer> :channels formed"
# define RPL_LUSERCHANNELS     {"254", "{} :channels formed", 1}
// ":I have <integer> clients and <integer> servers"
# define RPL_LUSERME           {"255", ":I have {} clients and {} servers", 2}
// "<server> :Administrative info"
# define RPL_ADMINME           {"256", "{} :Administrative info", 1}
// ":<admin info>"
# define RPL_ADMINLOC1         {"257", ":{}", 1}
// ":<admin info>"
# define RPL_ADMINLOC2         {"258", ":{}", 1}
// ":<admin info>"
# define RPL_ADMINEMAIL        {"259", ":{}", 1}
// "<command> :Please wait a while and try again."
# define RPL_TRYAGAIN          {"263", "{} :Please wait a while and try again.", 1}
// "<nickname> :No such nick/channel"
# define ERR_NOSUCHNICK        {"401", "{} :No such nick/channel", 1}
// "<server name> :No such server"
# define ERR_NOSUCHSERVER      {"402", "{} :No such server", 1}
// "<channel name> :No such channel"
# define ERR_NOSUCHCHANNEL     {"403", "{} :No such channel", 1}
// "<channel name> :Cannot send to channel"
# define ERR_CANNOTSENDTOCHAN  {"404", "{} :Cannot send to channel", 1}
// "<channel name> :You have joined too many channels"
# define ERR_TOOMANYCHANNELS   {"405", "{} :You have joined too many channels", 1}
// "<nickname> :There was no such nickname"
# define ERR_WASNOSUCHNICK     {"406", "{} :There was no such nickname", 1}
// "<target> :<error code> recipients. <abort message>"
# define ERR_TOOMANYTARGETS    {"407", "{} :{} recipients. {}", 3}
// "<service name> :No such service"
# define ERR_NOSUCHSERVICE     {"408", "{} :No such service", 1}
//
# define ERR_NOORIGIN          {"409", ":No origin specified", 0}
// ":No recipient given <command>"
# define ERR_NORECIPIENT       {"411", ":No recipient given {}", 1}
//
# define ERR_NOTEXTTOSEND      {"412", ":No text to send", 0}
// "<mask> :No toplevel domain specified"
# define ERR_NOTOPLEVEL        {"413", "{} :No toplevel domain specified", 1}
// "<mask> :Wildcard in toplevel domain"
# define ERR_WILDTOPLEVEL      {"414", "{} :Wildcard in toplevel domain", 1}
// "<mask> :Bad Server/host mask"
# define ERR_BADMASK           {"415", "{} :Bad Server/host mask", 1}
// "<command> :Unknown command"
# define ERR_UNKNOWNCOMMAND    {"421", "{} :Unknown command", 1}
// 
# define ERR_NOMOTD            {"422", ":MOTD File is missing", 0}
// 
# define ERR_NOADMININFO       {"423", "{} :No administrative info available", 1}
// ":File error doing <file op> on <file>"
# define ERR_FILEERROR         {"424", ":File error doing {} on {}", 2}
// 
# define ERR_NONICKNAMEGIVEN   {"431", ":No nickname given", 0}
// "<nick> :Erroneous nickname"
# define ERR_ERRONEUSNICKNAME  {"432", "{} :Erroneous nickname", 1}
// "<nick> :Nickname is already in use"
# define ERR_NICKNAMEINUSE     {"433", "{} :Nickname is already in use", 1}
// "<nick> :Nickname collision KILL from <user>@<host>"
# define ERR_NICKCOLLISION     {"436", "{} :Nickname collision KILL from {}@{}", 3}
// "<nick/channel> :Nick/channel is temporarily unavailable"
# define ERR_UNAVAILRESOURCE   {"437", "{} :Nick/channel is temporarily unavailable", 1}
// "<nick> <channel> :They aren't on that channel"
# define ERR_USERNOTINCHANNEL  {"441", "{} {} :They aren't on that channel", 2}
// "<channel> :You're not on that channel"
# define ERR_NOTONCHANNEL      {"442", "{} :You're not on that channel", 1}
// "<user> <channel> :is already on channel"
# define ERR_USERONCHANNEL     {"443", "{} {} :is already on channel", 2}
// "<user> :User not logged in"
# define ERR_NOLOGIN           {"444", "{} :User not logged in", 1}
// 
# define ERR_SUMMONDISABLED    {"445", ":SUMMON has been disabled", 0}
// 
# define ERR_USERSDISABLED     {"446", ":USERS has been disabled please use WHOIS to querry user informations", 0}
// 
# define ERR_CONNECTDISABLED   {"447", ":CONNECT has been disabled for security reasons", 0}
//
# define ERR_NOTREGISTERED     {"451", ":You have not registered", 0}
// "<command> :Not enough parameters"
# define ERR_NEEDMOREPARAMS    {"461", "{} :Not enough parameters", 1}
// 
# define ERR_ALREADYREGISTRED  {"462", ":Unauthorized command (already registered)", 0}
// 
# define ERR_NOPERMFORHOST     {"463", ":Your host isn't among the privileged", 0}
// 
# define ERR_PASSWDMISMATCH    {"464", ":Password incorrect", 0}
// 
# define ERR_YOUREBANNEDCREEP  {"465", ":You are banned from this server", 0}
// 
# define ERR_YOUWILLBEBANNED   {"466", "", 0}
// "<channel> :Channel key already set"
# define ERR_KEYSET            {"467", "{} :Channel key already set", 1}
// "<channel> :Cannot join channel (+l)"
# define ERR_CHANNELISFULL     {"471", "{} :Cannot join channel (+l)", 1}
// "<char> :is unknown mode char to me for <channel>"
# define ERR_UNKNOWNMODE       {"472", "{} :is unknown mode char to me for {}", 2}
// "<channel> :Cannot join channel (+i)"
# define ERR_INVITEONLYCHAN    {"473", "{} :Cannot join channel (+i)", 1}
// "<channel> :Cannot join channel (+b)"
# define ERR_BANNEDFROMCHAN    {"474", "{} :Cannot join channel (+b)", 1}
// "<channel> :Cannot join channel (+k)"
# define ERR_BADCHANNELKEY     {"475", "{} :Cannot join channel (+k)", 1}
// "<channel> :Bad Channel Mask"
# define ERR_BADCHANMASK       {"476", "{} :Bad Channel Mask", 1}
// "<channel> :Channel doesn't support modes"
# define ERR_NOCHANMODES       {"477", "{} :Channel doesn't support modes", 1}
// "<channel> <char> :Channel list is full"
# define ERR_BANLISTFULL       {"478", "{} {} :Channel list is full", 2}
// 
# define ERR_NOPRIVILEGES      {"481", ":Permission Denied- You're not an IRC operator", 0}
// "<channel> :You're not channel operator"
# define ERR_CHANOPRIVSNEEDED  {"482", "{} :You're not channel operator", 1}
//
# define ERR_CANTKILLSERVER    {"483", ":You can't kill a server!", 0}
// 
# define ERR_RESTRICTED        {"484", ":Your connection is restricted!", 0}
//
# define ERR_UNIQOPPRIVSNEEDED {"485", ":You're not the original channel operator", 0}
// 
# define ERR_NOCLIENTCMD	   {"486", ":This Command is not allowed to be emmited by a client", 0}
// 
# define ERR_NOOPERHOST        {"491", ":No O-lines for your host", 0}
// 
# define ERR_UMODEUNKNOWNFLAG  {"501", ":Unknown MODE flag", 0}
// 
# define ERR_USERSDONTMATCH    {"502", ":Cannot change mode for other users", 0}

#endif