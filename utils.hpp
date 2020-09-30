#ifndef MYUTILS_HPP
#define MYUTILS_HPP

#include <string>

namespace utils
{
	bool	strMatch(const std::string &filter, const std::string &str);
	bool	strCmp(const std::string &str1, const std::string &str2);
	std::string	ircLowerCase(const std::string &str);

	int isalpha( int ch );
	int isdigit( int ch );
	int isalnum( int ch );

	void* memset( void* dest, int ch, std::size_t count );
}

#endif