/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:54:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/23 14:38:27 by qfeuilla         ###   ########.fr       */
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
	std::vector<Fd *>		match_mask_serv(std::string);
	std::vector<Fd *>		match_mask_host(std::string); 
	// TODO : for other params

	std::vector<Fd *>	clients_fd;
	std::vector<Fd *>	client_history; 
	fd_set				fd_read;
	fd_set				fd_write;
	int					maxfd;
	int					clients_num;
	std::string			*password;
	std::string			*username_oper;
	time_t				start;
	bool				accept_operators;
	std::string			*serv;
	std::string			*version;
	struct sockaddr_in	sin;
};

#endif 