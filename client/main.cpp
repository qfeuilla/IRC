/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 14:15:21 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 14:54:16 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

#define PORT 8080

int main() {
	int					sock = 0, valread;
	struct sockaddr_in	serv_addr;
	std::string			greeting = "Hello I am a new client";
	char				buffer[1024] = {0};
	
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
	send(sock, greeting.c_str(), greeting.length(), 0);
	printf("Client has send greetings\n");
	valread = read( sock, buffer, 1024);
	printf("Client has read : %s\n", buffer);

	return (EXIT_SUCCESS);
}