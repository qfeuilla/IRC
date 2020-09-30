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
