/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   OtherServ.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/24 21:36:23 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/24 23:20:19 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OTHERSERV_HPP
# define OTHERSERV_HPP

#include "ft_irc.hpp"
#include "Fd.hpp"

class OtherServ : public Fd {
public:
	OtherServ(int);

	~OtherServ();

	virtual void	read_func();
	virtual void	write_func();
};

#endif