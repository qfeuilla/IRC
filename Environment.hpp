/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:54:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/22 00:55:14 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENVIRONMENT_HPP
# define ENVIRONMENT_HPP

#include "Fd.hpp"
#include <sys/select.h>
#include <vector>

class Environment {
public:
	Environment();
	~Environment();

	void	init_fds();
	
	// * search return the list of Fd that match conditions
	std::vector<Fd *>		search_history_nick(std::string);
	std::vector<Fd *>		search_list_nick(std::string);
	// TODO : for other params

	std::vector<Fd *>	clients_fd;
	std::vector<Fd *>	client_history; 
	fd_set				fd_read;
	fd_set				fd_write;
	int					maxfd;
	int					clients_num;
	std::string			*password;
	time_t				start;
};

#endif 