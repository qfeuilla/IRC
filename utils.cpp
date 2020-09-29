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
