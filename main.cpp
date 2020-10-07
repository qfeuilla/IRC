/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:28:49 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/03 17:46:02 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"
#include <iostream>

void	free_all(Server *se) {
	Environment *tmp;

	tmp = se->ev;
	std::cout << "ok" << std::endl;
	delete tmp->password;
	delete tmp->serv;
	delete tmp->version;
	for (OtherServ *sv : tmp->otherServers) {
		delete sv;
	} for (OtherServ *sv : tmp->lostServers) {
		delete sv;
	}
	for (Fd *f : tmp->clients_fd) {
		if (f->type == FD_WAITC || f->type == FD_CLIENT) {
			close(f->sock);
		}
		if (f->type != FD_OTHER) {
			delete f;
		}
	} for (Fd *f : tmp->client_history) {
		delete f;
	}
	delete tmp;
}

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
	free_all(s);
	return (0);
}