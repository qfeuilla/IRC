#include "utils.hpp"

static std::string	getNextWord(const std::string &filter, size_t pos)
{
	size_t i = pos;
	size_t len = 0;

	while (i < filter.size()) {
		if (filter[i] == '*') {
			break ;
		}
		++len;
		++i;
	}
	return (filter.substr(pos, len));
}

bool	utils::strMatch(const std::string &filter, const std::string &str)
{
	size_t		strI = 0;
	size_t		filterI = 0;
	std::string	toFind;

	while (strI < str.size() && filterI < filter.size()) {
		if (filter[filterI] == '*') {
			while (filterI < filter.size() && filter[filterI + 1] == '*')
				++filterI;
			if (filterI == filter.size() - 1) // if there is * at the end of filter
				return (true);
			toFind =  getNextWord(filter, filterI + 1);
			strI = str.find(toFind, strI); // jump to the next match after *
			if (strI == std::string::npos)
				return (false);
			++filterI; // next char after *
		}
		if (filter[filterI] != str[strI])
			return (false);
		++strI;
		++filterI;
	}
	while (filterI < filter.size() && filter[filterI] == '*')
		++filterI;
	if (strI < str.size())
		return (false);
	if (filterI < filter.size())
		return (false);
	return (true);
}

bool	utils::strCmp(const std::string &str1, const std::string &str2)
{
	std::string lowStr1 = utils::ircLowerCase(str1);
	std::string lowStr2 = utils::ircLowerCase(str2);

	return (lowStr1 == lowStr2);
}

std::string	utils::ircLowerCase(const std::string &str)
{
	std::string	newStr = "";
	size_t		i = 0;

	while (i < str.size())
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
			newStr += str[i] + ('a' - 'A');
		else if (str[i] == '[')
			newStr += '{';
		else if (str[i] == ']')
			newStr += '}';
		else if (str[i] == '\\')
			newStr += '|';
		else
			newStr += str[i];
		++i;
	}
	return (newStr);
}

std::string	utils::ircUpperCase(const std::string &str)
{
	std::string	newStr = "";
	size_t		i = 0;

	while (i < str.size())
	{
		if (str[i] >= 'a' && str[i] <= 'z')
			newStr += str[i] - ('a' - 'A');
		else if (str[i] == '{')
			newStr += '[';
		else if (str[i] == '}')
			newStr += ']';
		else if (str[i] == '|')
			newStr += '\\';
		else
			newStr += str[i];
		++i;
	}
	return (newStr);
}

// uppercase letters (ABCDEFGHIJKLMNOPQRSTUVWXYZ)
// lowercase letters (abcdefghijklmnopqrstuvwxyz
int		utils::isalpha( int ch )
{
	if (ch >= 'a' && ch <= 'z')
		return (ch);
	if (ch >= 'A' && ch <= 'Z')
		return (ch);
	return (0);
}

// digits (0123456789)
int		utils::isdigit( int ch )
{
	if (ch >= '0' && ch <= '9')
		return (ch);
	return (0);
}

// digits (0123456789)
// uppercase letters (ABCDEFGHIJKLMNOPQRSTUVWXYZ)
// lowercase letters (abcdefghijklmnopqrstuvwxyz
int		utils::isalnum( int ch )
{
	return (utils::isalpha(ch) || utils::isdigit(ch));
}

// void* dest, int ch, std::size_t count
void	*utils::memset(void *dest, int ch, std::size_t count)
{
	std::size_t	i;
	char		*str;

	str = (char*)dest;
	i = 0;
	while (i < count)
	{
		str[i] = (unsigned char)ch;
		++i;
	}
	return (dest);
}

//
bool	utils::strMatchToLower(const std::string &filter, const std::string &str)
{
	std::string	str1 = utils::ircLowerCase(filter);
	std::string	str2 = utils::ircLowerCase(str);

	return (utils::strMatch(str1, str2));
}

std::string	utils::delFirstWord(const std::string &str)
{
	std::string	newStr;
	size_t		pos = str.find(" ");
	if (pos == std::string::npos)
		return ("");
	++pos;
	newStr = str.substr(pos);
	return (newStr);
}

std::string	utils::strJoin(const std::vector<std::string> &vec, char sep)
{
	std::string	users;

	for (std::string str : vec) {
		users += str;
		users += sep;
	}
	if (users.size() == 0)
		return ("");
	return (users.substr(0, users.size() - 1));
}
