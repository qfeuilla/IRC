/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 18:24:57 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/20 17:19:35 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_IRC_H
# define FT_IRC_H

# include <sys/select.h>
# include <string>
# include <vector>
# include "defines.hpp"
# include "Command.hpp"

# define Xv(err,res,str)	(x_void(err,res,str,__FILE__,__LINE__))
# define X(err,res,str)		(x_int(err,res,str,__FILE__,__LINE__))
# define MAX(a,b)			((a > b) ? a : b)

int							x_int(int, int, std::string, std::string, int);
void						*x_void(void *, void *, std::string, std::string, int);
Command						*parse(std::string cmd);

#endif
