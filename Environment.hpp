/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:54:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/19 22:48:35 by qfeuilla         ###   ########.fr       */
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

	std::vector<Fd *>	clients_fd;
	fd_set				fd_read;
	fd_set				fd_write;
	int					maxfd;
	int					clients_num;
	std::string			*password;
};

#endif 