/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Fd.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:40:52 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/20 16:32:43 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FD_HPP
# define FD_HPP

#include "ft_irc.hpp"
#include <string>

class Fd {
public:
	Fd();
	virtual ~Fd();

	virtual void	read_func();
	virtual void	write_func();

	// Server or client
	int		type;
	char	buf_read[BUF_SIZE + 1];
	char	buf_write[BUF_SIZE + 1];
	int		sock;
};

#endif