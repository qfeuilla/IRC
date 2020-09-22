#include "ChannelMaster.hpp"
#include <string>
ChannelMaster::ChannelMaster() {}
ChannelMaster::~ChannelMaster() {
	_channel_map::iterator	current = _channels.begin();
	_channel_map::iterator	end = _channels.end();

	while (current != end) {
		delete (*current).second;
		++current;
	}
}
void	ChannelMaster::_testChannelName(const std::string &channelName)
{
	if (channelName.at(0) != '#' && channelName.at(0) != '&')
		throw Channel::badName(channelName, "Channel should start with # or &");
	if (channelName.length() > 200)
		throw Channel::badName(channelName, "Channel should not be more than 200 characters");
	if (channelName.find(" ") != std::string::npos)
		throw Channel::badName(channelName, "Channel should not contain space (' ')");
	if (channelName.find(",") != std::string::npos)
		throw Channel::badName(channelName, "Channel should not contain comma (',')");
	if (channelName.find("\7") != std::string::npos)
		throw Channel::badName(channelName, "Channel should not contain ^G ('\\7')");
}

std::vector<std::string>	splitComma(std::string const &str)
{
	std::vector<std::string>	ret;
	size_t startPos;
	size_t endPos = 0;
	std::string		chunk;

	while ((startPos = str.find_first_not_of(',', endPos)) != std::string::npos)
	{
		endPos = str.find(',', startPos);
		chunk = str.substr(startPos, endPos - startPos);
		ret.push_back(chunk);
	}
	return (ret);
}


bool	ChannelMaster::join(socket_t socket, const std::vector<std::string> &args)
{
	std::map<std::string, std::string>	name_pass_map;
	std::map<std::string, std::string>::iterator	current;
	std::map<std::string, std::string>::iterator	end;
	std::vector<std::string>	names;
	std::vector<std::string>	passwds;
	bool	ret = true;

	names = splitComma(args[0]);
	if (args.size() > 1)
		passwds = splitComma(args[1]);
	for (size_t i = 0; i < names.size(); ++i) {
		name_pass_map[names[i]] = "";
	}
	for (size_t i = 0; i < passwds.size(); ++i) {
		name_pass_map[names[i]] = passwds[i];
	}
	current = name_pass_map.begin();
	end = name_pass_map.end();
	while (current != end) {
		ret &= _join_channel(socket, (*current).first);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::_join_channel(socket_t socket, const std::string &channelName)
{
	_channel_map::iterator	element = _channels.find(channelName);
	
	if (element == _channels.end()) {
		_testChannelName(channelName);
		_channels[channelName] = new Channel(channelName);
		element = _channels.find(channelName);
	}
	return ((*element).second->join(socket));
}

bool	ChannelMaster::_leave_channel(socket_t socket, const std::string &channelName)
{
	_channel_map::iterator	element = _channels.find(channelName);

	if (element == _channels.end())
		return (false); // there is no channel with this name
	if ((*element).second->leave(socket))
	{
		std::cout << "channel erased\n\n";
		_channels.erase(element); // channel is empty -> we delete it
	}
	return (true);
}

bool	ChannelMaster::leave(socket_t socket, const std::vector<std::string> &args)
{
	bool	ret = true;
	std::vector<std::string>	names = splitComma(args[0]);
	for (size_t i = 0; i < names.size(); ++i) {
		ret &= _leave_channel(socket, names[i]);
	}
	return (ret);
}
