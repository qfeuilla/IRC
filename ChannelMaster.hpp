#ifndef CHANNELMASTER_H
#define CHANNELMASTER_H

#include "Channel.hpp"
#include "Command.hpp"
#include <vector>
#include <map>
#include <string>
#include <list>

class ChannelMaster
{
private:
	typedef	Channel::socket_t	socket_t;
	typedef	std::list<Channel*>	_channel_list;
	
	_channel_list	*_channels;
	static void	_testChannelName(const std::string &channelName);
	bool		_join_channel(std::string nick, socket_t socket, const std::string &channelName, const std::string &passwd);
	bool		_leave_channel(std::string nick, socket_t socket, const std::string &channelName);
	Channel		*_chan_exists(const std::string &channelName);
public:
	ChannelMaster();
	~ChannelMaster();

	// * join returns true on succes (false if socket was already in the channel before the call)
	bool	join(std::string nick, socket_t socket, const std::vector<std::string> &args);
	// * leave returns true on succes
	bool	leave(std::string nick, socket_t socket, const std::vector<std::string> &args);
	// * mode returns true on succes
	bool	mode(std::string nick, socket_t socket, const std::vector<std::string> &args);
};

#endif