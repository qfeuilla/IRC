#ifndef CHANNELMASTER_H
#define CHANNELMASTER_H

#include "ft_irc.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include <vector>
#include <map>
#include <string>
#include <list>
#include "Client.hpp"
#include "utils.hpp"

#include "OtherServ.hpp"

class Channel;
class Client;

class ChannelMaster
{
private:
	typedef	int					socket_t;
	typedef	std::list<Channel*>	_channel_list;
	typedef	std::map<std::string, std::list<Channel*>* >	_user_chan_map;
	
	_channel_list	*_channels;
	_user_chan_map	*_user_channels;
	// * _testChannelName returns true if the channelName is not correctly formatted
	static bool	_testChannelName(Client *client, const std::string &channelName);

	std::string		_srv_name;
public:
	ChannelMaster(const std::string &srvName = std::string());
	~ChannelMaster();

	void	setSrvName(const std::string &srvName);

	// * join returns true on succes
	bool	join(Client *client, const std::vector<std::string> &args, std::list<Channel*> *usrChans, OtherServ *svFrom = nullptr);
	// * leave returns true on succes
	bool	leave(Client *client, const std::vector<std::string> &args, OtherServ *svFrom = nullptr);
	// * mode returns true on succes
	bool	mode(Client *client, const std::vector<std::string> &args, OtherServ *svFrom = nullptr);
	// * kick returns true on succes
	bool	kick(Client *client, const std::vector<std::string> &args, OtherServ *svFrom = nullptr);
	// * topic returns true on succes
	bool	topic(Client *client, const std::vector<std::string> &args, OtherServ *svFrom = nullptr);
	// * invite returns true on succes
	bool	invite(Client *client, const std::vector<std::string> &args, OtherServ *svFrom = nullptr);
	// * list always return true
	bool	list(Client *client, const std::vector<std::string> &args);

	bool	names(Client *client, const std::vector<std::string> &args);
	bool	chanNames(Client *client, const std::string &channelName);

	bool	getChanModes(Client *client, const std::vector<std::string> &args);

	size_t	size() const;

	// * send msg to everyone in the channel but the sender 
	bool	broadcastMsg(OtherServ *svFrom, Client *client, const std::string &chanName, const std::vector<std::string> &args, bool sendErrors = true);

	Channel				*getChannel(const std::string &channelName);
	bool		joinChannel(Client *client, const std::string &channelName, const std::string &passwd, OtherServ *svFrom);
	bool		leaveChannel(Client *client, const std::string &channelName, const std::string &reason, OtherServ *svFrom);
	bool	kickFromChan(Client *client, const std::string &chanName,
	const std::string &guyToKick, const std::string &reason, OtherServ *svFrom);

	void		doQuit(Client *client, const std::vector<std::string> &args);
	bool		localChanWHO(Client *client, const std::vector<std::string> &args);

	bool		delChanIfEmpty(Channel *chan);

	void		changeNick(const std::string &oldNick, const std::string &newNick);

	void		shareAll(OtherServ *sv) const;

	void		changeChanListNick(std::list<Channel*> *usrChans, const std::string &newNick);
};

#endif