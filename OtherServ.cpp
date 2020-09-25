/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:03 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/24 23:20:27 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "OtherServ.hpp"

OtherServ::OtherServ(int socket) {
	sock = socket;
}

OtherServ::~OtherServ() { }

void	OtherServ::read_func() { }

void	OtherServ::write_func() { }