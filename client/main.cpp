/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 14:15:21 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 17:23:18 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>

#define PORT 8080

int		handle_msgs(int sock) {
	std::string	msg;
	int			valread;
	char		buffer[1024] = {0};

	std::getline(std::cin, msg);
	send( sock, msg.c_str(), msg.length(), 0 );
	valread = read( sock, buffer, 1024);
	std::cout << buffer << std::endl;
	if (msg == "exit")
		return (1);
	return (0);
}

int main() {
	int					sock = 0;
	struct sockaddr_in	serv_addr;
	std::string			greeting = "Hello I am a new client";
	
	// See server implementation for explenations
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socker creation error\n");
		return (EXIT_FAILURE);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	
	// convert address from IPv4 text to binary inside the adress struct
	if (inet_pton(serv_addr.sin_family, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("invalid address, or not supported\n");
		return (EXIT_FAILURE);
	}

	if (connect(sock, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
		perror("Connection to the server failed\n");
		return(EXIT_FAILURE);
	}
	
	std::cout << "You are successfully connected" << std::endl;
	
	while (true) {
		if (handle_msgs(sock))
			break ;
	}

	return (EXIT_SUCCESS);
}