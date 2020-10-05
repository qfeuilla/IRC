/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:24:57 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/23 17:16:02 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_IRC_H
# define FT_IRC_H

# include <sys/select.h>
# include <string>
# include <vector>
# include <iostream>
# include "defines.hpp"
# include "Command.hpp"
# include "structs.hpp"
# include <ctime>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <map>
# include <algorithm>

// TODO check what we can withdraw from the list of includes
# include <sstream>
# include <stdio.h>
# include <unistd.h>
# include <sys/socket.h>
# include <cstring>
# include <sys/types.h>
# include <fcntl.h>

# define Xv(err,res,str)	(x_void(err,res,str,__FILE__,__LINE__))
# define X(err,res,str)		(x_int(err,res,str,__FILE__,__LINE__))
# define MAX(a,b)			((a > b) ? a : b)
# define reply_formating(server, err, params, user) _reply_formating(server, err, params, user, __FILE__, __LINE__)

int							x_int(int, int, std::string, std::string, int);
void						*x_void(void *, void *, std::string, std::string, int);
Command						*parse(std::string cmd);
std::string					_reply_formating(const char *, Error, 
								std::vector<std::string>, const char *,
								const char*, int);
bool						is_special(char c);
std::vector<std::string>	parse_comma(std::string cmd);

struct Chan
{
	std::string	name;
	std::string	usersNum;
	std::string	modes;
	std::string	topic;
	std::vector<std::string>	nicknames;
	Chan(const std::string &nm, const std::string &usrNm, const std::string &md, const std::string &tpc,
	std::vector<std::string> nicks): name(nm), usersNum(usrNm), modes(md), topic(tpc), nicknames(nicks) {}
};

#endif
