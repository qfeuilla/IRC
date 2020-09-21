/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:51:25 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/21 11:53:37 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sstream>

Client::Client(Environment *e, int s) : ev(e) {
	type = FD_WAITC;
	is_setup = false;
	sock = s;
}

Client::~Client() {
	std::cout << "client destructed" << std::endl;
}

Client::Client(const Client &old) {
	*this = old;
}

int		Client::execute_parsed(Command *parsed) {
	switch (parsed->cmd_code())
	{
	case PASS_CC:
		if (parsed->prefix.empty() && parsed->arguments.size() == 1) {
			if (is_setup) {
				send(sock, "Already register the right PASS\n", 32, 0);
			} else {
				if (parsed->arguments[0] == *ev->password) {
					is_setup = true;
					type = FD_CLIENT;
					send(sock, "Good code you are register\n", 27, 0);
				}
				else
					send(sock, "Wrong code please try again\n", 28, 0);
			}
		} 
		else
			send(sock, "error format PASS\n", 18, 0);
		break;
	
	default:
		break;
	}
	return (0);
}

void	Client::read_func() {
	int	r;
	std::string resp;
	std::string resps;
	std::string dt;
	Command		*parsed;

	fcntl(sock, F_SETFL, O_NONBLOCK);
	memset(&buf_read, 0, BUF_SIZE + 1);
	r = recv(sock, &buf_read, BUF_SIZE, 0);
	std::string line;
	std::string rd = std::string(buf_read);
	std::istringstream iss(rd);
	while (std::getline(iss, line) && !line.empty()) {
		parsed = parse(line);
		std::cout << "Line : " << line << std::endl;
		std::cout << *parsed << std::endl;
		execute_parsed(parsed);
    }
}

void	Client::write_func() { }