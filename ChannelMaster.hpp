#ifndef CHANNELMASTER_H
#define CHANNELMASTER_H

#include "Channel.hpp"

class ChannelMaster
{
private:
	typedef	Channel::socket_t	socket_t;
	typedef	std::map<std::string, Channel*>	_channel_map;
	
	_channel_map	_channels;
	static void	_testChannelName(const std::string &channelName);
public:
	ChannelMaster();
	~ChannelMaster();

	// * join returns true on succes (false if socket was already in the channel before the call)
	bool	join(socket_t socket, const std::string &channelName);
	// * leave returns true on succes
	bool	leave(socket_t socket, const std::string &channelName);
};

#endif