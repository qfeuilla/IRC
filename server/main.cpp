/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 13:35:59 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 16:11:53 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <iostream>

#define PORT 8080

int	handle_msgs(int client_socket) {
	char		buffer[1024] = {0};
	int			valread;

	if (!(valread = read(client_socket, buffer, 1024)))
		return (1);
	std::string resp = "Server has read : " + std::string(buffer) + " with code : " + std::to_string(valread);
	std::cout << resp << std::endl;
	send(client_socket, resp.c_str(), resp.length(), 0);
	return (0);
}


int main() {
	int					server_fd, new_socket;
	struct sockaddr_in	address;
	int					opt = 1;
	int					addrlen = sizeof(address);
	std::string			greeting = "Hello from server";

	// Socket FD
	/*
	**	AF_INET is IPv4, if you want to use IPv6 use AF_INET6
	**	SOCK_STREAM specified that we used a TCP connection, use SOCK_DGRAM for UDP
	*/
	if (!(server_fd = socket(AF_INET, SOCK_STREAM, 0)))	{		
		perror("socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	// Attach socket to port
	/*
	**	SOL_SOCKET specify that the option to change is in the Socket level 
	**	(see https://www.gnu.org/software/libc/manual/html_node/Socket_002dLevel-Options.html
	**	 for more socket level specification ) 
	**	SO_REUSE* are option that specified that the same port and adress can be used by other
	**	server (see https://man7.org/linux/man-pages/man7/socket.7.html)
	*/
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("socket opt change failed\n");
		return (EXIT_FAILURE);
	}

	// setup adress
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // localhost
	address.sin_port = htons(PORT);

	// bind the socket to the adress
	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) < 0) {
		perror("bind failed\n");
		exit(EXIT_FAILURE);
	}

	while (true) {
		// listen to a maximum 3 request in pending queue
		if (listen(server_fd, 3) < 0) {
			perror("listen failed\n");
			exit(EXIT_FAILURE);
		}

		// accept the incoming connection
		if ((new_socket = accept(server_fd, 
						reinterpret_cast<struct sockaddr *>(&address), 
						reinterpret_cast<socklen_t*>(&addrlen))) < 0) {
			perror("accept failed\n");
			exit(EXIT_FAILURE);
		}

		std::cout << "next client accepted" << std::endl;

		while (true) {
			if (handle_msgs(new_socket))
				break ;
		}

		std::cout << "Client disconnected" << std::endl;
	}
	return (EXIT_SUCCESS);
}