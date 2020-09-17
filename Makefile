# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2020/09/17 14:44:32 by qfeuilla          #+#    #+#              #
#    Updated: 2020/09/17 14:55:05 by qfeuilla         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

all: 
	clang++ -Wall -Werror -Wextra client/*.cpp -o client.bin
	clang++ -Wall -Werror -Wextra server/*.cpp -o server.bin

fclean:
	rm client.bin
	rm server.bin