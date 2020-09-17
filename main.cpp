/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:28:49 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 22:41:48 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"
#include <iostream>

void	main_loop(Server *se)
{
	while (1)
	{
		se->init_fd();
		se->serv_select();
		se->do_actions();
	}
}

int	main(int ac, char **av)
{
	Server *s = new Server();

	s->load_options(ac, av);
	s->create();
	main_loop(s);
	return (0);
}