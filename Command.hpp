/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/20 16:41:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/21 12:19:01 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <string>
# include <vector>
# include <ostream>

class Command {
public:
	Command(std::vector<std::string>);
	~Command();

	int	cmd_code() const;

	std::string						prefix;
	std::string						command;
	std::vector<std::string>		arguments;
};

std::ostream &			operator<<( std::ostream & o, Command const & i );

#endif