#include "ChannelMaster.hpp"

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

bool	ChannelMaster::join(socket_t socket, const std::string &channelName)
{
	_channel_map::iterator	element = _channels.find(channelName);
	
	if (element == _channels.end()) {
		_testChannelName(channelName);
		_channels[channelName] = new Channel(channelName);
		element = _channels.find(channelName);
	}
	return ((*element).second->join(socket));
}

bool	ChannelMaster::leave(socket_t socket, const std::string &channelName)
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
