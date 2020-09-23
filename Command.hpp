/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/20 16:41:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/23 11:11:55 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <string>
# include <vector>
# include <ostream>

// TODO : implement a struct prefix for user formating (like qfeu!~qfeuilla@einstein.ft.irc)
// TODO 	used for multi server

class Command {
public:
	Command(std::vector<std::string>, std::string);
	~Command();

	int	cmd_code() const;

	std::string						line;
	std::string						prefix;
	std::string						command;
	std::vector<std::string>		arguments;
};

std::ostream &			operator<<( std::ostream & o, Command const & i );

#endif