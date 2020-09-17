/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 13:35:59 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 17:39:53 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

#define PORT 8080

int	handle_msgs(int client_socket) {
	char		buffer[1024] = {0};
	int			valread;

	if (!(valread = read(client_socket, buffer, 1024)))
		return (1);
	std::string resp = "Server has read : " + std::string(buffer);
	std::cout << resp << std::endl;
	send(client_socket, resp.c_str(), resp.length(), 0);
	return (0);
}


int main() {
	int					opt = 1;
	int					server_socket, new_socket, client_socket[30],
						max_clients = 30, activity, i, sd;
	int					max_sd;
	struct sockaddr_in	address;
	int					addrlen;

	fd_set				readfds;

	// set clien sockets
	for (int i = 0; i < max_clients; i++) {
		client_socket[i] = 0;
	}

	// Socket FD
	/*
	**	AF_INET is IPv4, if you want to use IPv6 use AF_INET6
	**	SOCK_STREAM specified that we used a TCP connection, use SOCK_DGRAM for UDP
	*/
	if (!(server_socket = socket(AF_INET, SOCK_STREAM, 0)))	{		
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
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("socket opt change failed\n");
		return (EXIT_FAILURE);
	}

	// setup adress
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // localhost
	address.sin_port = htons(PORT);

	// bind the socket to the adress
	if (bind(server_socket, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) < 0) {
		perror("bind failed\n");
		exit(EXIT_FAILURE);
	}
	std::cout << "Listener on port : " << PORT << std::endl;

	if (listen(server_socket, 3) < 0) {
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address);
	std::cout << "Waiting for connection" << std::endl;

	while (true) {

		// clear the read set
		FD_ZERO(&readfds);

		//add master socket to the set
		FD_SET(server_socket, &readfds);
		max_sd = server_socket;

		//add client sockets to the set
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];

			if (sd > 0)
				FD_SET(sd, &readfds);
			
			if (sd > max_sd)
				max_sd = sd;
		}

		// see man for parameters descriptions
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		// check select error
		if ((activity < 0) && (errno!=EINTR))
			std::cout << "select enconter an error. Stand still, select is restarting" << std::endl;
		
		// Check if something happened on server socket,
		// If someting happened then it's an incomming connection
		if (FD_ISSET(server_socket, &readfds)) {
			// accept the incoming connection
			if ((new_socket = accept(server_socket, 
							reinterpret_cast<struct sockaddr *>(&address), 
							reinterpret_cast<socklen_t*>(&addrlen))) < 0) {
				perror("accept failed\n");
				exit(EXIT_FAILURE);
			}

			// connection information
			std::cout << "New Connection, socker fd is : " << new_socket
				<< ", ip is : " << inet_ntoa(address.sin_addr)
				<< ", on port : " << ntohs(address.sin_port) << std::endl;
			
			// add new socket to the array for next loop
			for (i = 0; i < max_clients; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					std::cout << "User has been add at index : " << i << std::endl;
					break;
				}
			}
		}

		//check each client sockets for activity
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				// if return is true then client is disconnecting
				if (handle_msgs(sd)) {
					// function is forbiden for 42, TODO : find an alternative (maybe getaddrinfo)
					getpeername(sd, 
							reinterpret_cast<struct sockaddr*>(&address), 
							reinterpret_cast<socklen_t*>(&addrlen));
					std::cout << "Host disconnected, ip : " << inet_ntoa(address.sin_addr) <<
							", port : " << ntohs(address.sin_port) << std::endl;
						
					// close and set the fd to zero for space reuse
					close (sd);
					client_socket[i] = 0;
				}
			}
		}
	}
	
	return (EXIT_SUCCESS);
}