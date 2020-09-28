/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:28:49 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/28 13:09:55 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"
#include <iostream>

void	main_loop(Server *se)
{
	while (se->active())
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
	main_loop(s);
	return (0);
}