/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:51:25 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 23:36:31 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include "algorithm"
#include <string.h>

Client::Client(Environment *e) : ev(e) {
	type = FD_CLIENT;
}

Client::~Client() { 
	std::cout << "client destructed" << std::endl;
}

void	Client::read_func(int cs) {
	int	r;
	int	i;
	std::string resp;

	memset(&buf_read, 0, BUF_SIZE + 1);
	r = recv(cs, &buf_read, BUF_SIZE, 0);
	if (r <= 0) {
		close(cs);
		type = FD_FREE;
		std::cout << "client #" << cs << " gone away" << std::endl;
	}
	else {
		resp = "client #" + std::to_string(cs) + ": " + buf_read;
		std::cout << resp << std::endl;
		i = 0;
		while (i < ev->clients_num) {
			if ((ev->clients_fd[i]->type == FD_CLIENT) && (i != cs))
				send(i, resp.c_str(), resp.length(), 0);
			i++;
		}
    }
}

void	Client::write_func(int cs) { }