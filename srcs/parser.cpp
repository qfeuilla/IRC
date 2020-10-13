/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/19 19:15:55 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/22 22:32:11 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include <iostream>

Command		*parse(std::string cmd) {
	std::vector<std::string>	container;
	size_t						last = 0;
	size_t						i;

	for (i = 0; i < cmd.length(); i++) {	
		if (cmd[i] == ' ') {
			container.push_back(std::string(&cmd[last], &cmd[i]));
			while (cmd[i] == ' ') 
				i++;
			last = i;
			if (i >= cmd.size())
				break ; // so we keep last == i
		}
	}
	if (last < i)
		container.push_back(cmd.substr(last));
	return (new Command(container, std::string(cmd)));
}

std::vector<std::string>	parse_comma(std::string cmd) {
	std::vector<std::string>	container;
	size_t						last = 0;
	size_t						i;

	for (i = 0; i < cmd.length(); i++) {	
		if (cmd[i] == ',') {
			container.push_back(std::string(&cmd[last], &cmd[i]));
			while (cmd[i] == ',') 
				i++;
			last = i;
		}
	}
	if (last < i)
		container.push_back(std::string(&cmd[last], &cmd[i]));
	return container;
}