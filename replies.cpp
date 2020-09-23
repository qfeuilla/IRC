/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   replies.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/21 15:45:47 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/23 17:16:54 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

// you need to check beforehand that size of params is the same as 
// num of brackets. The fonction is not secured volontarely
std::string		fast_format(std::string str, std::vector<std::string> params) {
	std::string tmp;
	size_t		i = 0;
	int			vector_index = 0;

	while (i < str.length())
		if (str[i] == '{') {
			tmp += params[vector_index++];
			i += 2;
		}
		else
			tmp += str[i++];
	return (tmp);
}

std::string		_reply_formating(const char *server, Error err, std::vector<std::string> params, const char *user, const char* file, int line) {
	std::string reply;

	if (params.size() != err.param_num) {
		std::cerr << "Wrong number of argument file : " << file << ", line : " << line << std::endl;
		std::cerr << params.size() << " " << err.param_num << std::endl;
		return ("Err");
	}
	reply += ":";
	reply += server;
	reply += " ";
	reply += err.error_code;
	reply += " ";
	reply += user;
	reply += " ";
	reply += fast_format(err.to_format, params);
	reply += CRLF;
	return (reply);
}