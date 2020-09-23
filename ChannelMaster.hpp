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
	typedef	std::map<std::string, std::list<Channel*>* >	_user_chan_map;
	
	_channel_list	*_channels;
	_user_chan_map	*_user_channels;
	// * _testChannelName returns true if the channelName is not correctly formatted
	static bool	_testChannelName(socket_t socket, const std::string &channelName);
	bool		_join_channel(std::string nick, socket_t socket, const std::string &channelName, const std::string &passwd);
	bool		_leave_channel(std::string nick, socket_t socket, const std::string &channelName, const std::string &reason);
	Channel		*_chan_exists(const std::string &channelName);
public:
	ChannelMaster();
	~ChannelMaster();

	// * join returns true on succes (false if socket was already in the channel before the call)
	bool	join(std::string nick, socket_t socket, const std::vector<std::string> &args, std::list<Channel*> *usrChans);
	// * leave returns true on succes
	bool	leave(std::string nick, socket_t socket, const std::vector<std::string> &args);
	// * mode returns true on succes
	bool	mode(std::string nick, socket_t socket, const std::vector<std::string> &args);
	// * kick returns true on succes
	bool	kick(std::string nick, socket_t socket, const std::vector<std::string> &args);
	// * topic returns true on succes
	bool	topic(std::string nick, socket_t socket, const std::vector<std::string> &args);

	// * send msg to everyone in the channel but the sender
	bool	broadcastMsg(const std::string &sender, socket_t socket, const std::string &chanName, const std::string &msg);
};

#endif