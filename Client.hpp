/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qfeuilla <qfeuilla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/09/17 19:50:59 by qfeuilla          #+#    #+#             */
/*   Updated: 2020/09/20 20:34:46 by qfeuilla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Fd.hpp"
# include "Environment.hpp"

class Client : public Fd {
public:
	Client(Environment *, int);
	~Client();

	Client(const Client &);

	virtual void	read_func();
	virtual void	write_func();

	int				execute_parsed(Command *parsed);

	bool			is_setup;

private:
	Environment			*ev;
};

#endif