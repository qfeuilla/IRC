/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   defines.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/20 17:01:28 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/20 20:19:55 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DEFINES_HPP
#define DEFINES_HPP

// * User types

# define FD_FREE		0
# define FD_SERVER		1
# define FD_CLIENT		2
# define FD_WAITC		3

# define BUF_SIZE		4096

// * Command number for switch operations

# define WRONG_CMD 0
# define PASS_CC 1
# define NICK_CC 2
# define USER_CC 3
# define SERVER_CC 4
# define OPER_CC 5
# define QUIT_CC 7
# define SQUIT_CC 6
# define JOIN_CC 8
# define PART_CC 9
# define MODE_CC 10
# define TOPIC_CC 11
# define NAMES_CC 12
# define LIST_CC 13
# define INVITE_CC 14
# define KICK_CC 15
# define VERSION_CC 16
# define STATS_CC 17
# define LINKS_CC 18
# define TIME_CC 19
# define CONNECT_CC 21
# define TRACE_CC 22
# define ADMIN_CC 23
# define INFO_CC 24
# define PRIVMSG_CC 25
# define NOTICE_CC 26
# define WHO_CC 27
# define WHOIS_CC 28
# define WHOWAS_CC 29
# define KILL_CC 30
# define PING_CC 31
# define PONG_CC 32
# define ERROR_CC 33

// * Messages necessay rights to execute

# define CLIENT_R 1
# define SERVER_R 2
# define BOTH_R 3


#endif