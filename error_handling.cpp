/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 20:51:33 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/17 21:00:30 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include <iostream>

int	x_int(int err, int res, std::string str, std::string file, int line)
{
  	if (res == err)
	{
		std::cerr << str << " error (" << file << "," << line << "): " << errno << std::endl; 
		exit (EXIT_FAILURE);
	} 
  	return (res);
}

void	*x_void(void *err, void *res, std::string str, std::string file, int line)
{
  	if (res == err)
	{
		std::cerr << str << " error (" << file << "," << line << "): " << errno << std::endl;
		exit (EXIT_FAILURE);
	} 
  	return (res);
}
