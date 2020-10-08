/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Environment.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:54:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/10/08 17:06:17 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENVIRONMENT_HPP
# define ENVIRONMENT_HPP

#include "Fd.hpp"
#include <sys/select.h>
#include <vector>
#include "ChannelMaster.hpp"
#include "OtherServ.hpp"

class ChannelMaster;
class OtherServ;

class Environment {
public:
	Environment();
	~Environment();

	void	init_fds();
	
	// * search return the list of Fd that match conditions
	std::vector<Fd *>		search_history_nick(std::string);
	std::vector<Fd *>		search_list_nick(std::string);
	std::vector<OtherServ *>	search_othersrv_nick(std::string);
	std::vector<OtherServ *>	search_othersrv_history_nick(std::string);
	std::vector<Fd *>		match_mask_serv(std::string);
	std::vector<Fd *>		match_mask_host(std::string); 
	
	std::vector<Fd *>		search_list_with_mode(std::string, std::string, char);

	Client	*getOtherServClientByNick(const std::string &nick);

	std::vector<Fd *>	clients_fd;
	std::vector<Fd *>	client_history;
	std::vector<Fd *>	trash;
	fd_set				fd_read;
	fd_set				fd_write;
	int					maxfd;
	int					clients_num;
	std::string			*password;
	time_t				start;
	bool				accept_operators;
	std::string			*serv;
	std::string			*version;
	std::map<std::string, int> cmd_count;
	struct sockaddr_in	sin;

	int							servport;
	std::vector<std::string>	emails;
	std::string					loc1;
	std::string					loc2;

	std::vector<OtherServ *>	otherServers;
	std::vector<OtherServ *>	lostServers;

	bool						active = true;
	
	ChannelMaster				*channels;
};

#endif 