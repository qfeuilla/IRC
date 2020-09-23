#include "ChannelMaster.hpp"
ChannelMaster::ChannelMaster() {
	_channels = new _channel_list();
	_user_channels = new _user_chan_map();
	std::cout << "created channel master\n";
}
ChannelMaster::~ChannelMaster() {
	std::cout << "deleting channel master\n";
	_channel_list::iterator	current = _channels->begin();
	_channel_list::iterator	end = _channels->end();
	Channel					*chan;

	while (current != end) {
		chan = *current;
		++current;
		delete chan;
	}
	delete _channels;
	delete _user_channels;
}
bool	ChannelMaster::_testChannelName(socket_t socket, const std::string &channelName)
{
	if (channelName.at(0) != '#' && channelName.at(0) != '&')
		return (Channel::sendMsgToSocket(socket, Channel::badName(channelName, "Channel should start with # or &")));
	if (channelName.length() > 200)
		return (Channel::sendMsgToSocket(socket, Channel::badName(channelName, "Channel should not be more than 200 characters")));
	if (channelName.find(" ") != std::string::npos)
		return (Channel::sendMsgToSocket(socket, Channel::badName(channelName, "Channel should not contain space (' ')")));
	if (channelName.find(",") != std::string::npos)
		return (Channel::sendMsgToSocket(socket, Channel::badName(channelName, "Channel should not contain comma (',')")));
	if (channelName.find("\7") != std::string::npos)
		return (Channel::sendMsgToSocket(socket, Channel::badName(channelName, "Channel should not contain ^G ('\\7')")));
	return (false); // channelName is well formatted
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


bool	ChannelMaster::join(std::string nick, socket_t socket, const std::vector<std::string> &args, std::list<Channel*> *usrChans)
{
	std::map<std::string, std::string>	name_pass_map;
	std::map<std::string, std::string>::iterator	current;
	std::map<std::string, std::string>::iterator	end;
	std::vector<std::string>	names;
	std::vector<std::string>	passwds;
	bool	ret = true;

	(*_user_channels)[nick] = usrChans;
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
		ret &= _join_channel(nick, socket, (*current).first, (*current).second);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::_join_channel(std::string nick, socket_t socket, const std::string &channelName, const std::string &passwd)
{
	Channel	*chan = _chan_exists(channelName);
	if (!chan) {
		if (_testChannelName(socket, channelName))
			return (false);
		Channel	*nchan = new Channel(channelName, nick, socket);
		_channels->push_back(nchan);
		(*_user_channels)[nick]->push_back(nchan);
		return (true);
	}
	if (chan->join(nick, socket, passwd)) {
		(*_user_channels)[nick]->push_back(chan);
		return (true);
	}
	return (false);
}

bool	ChannelMaster::_leave_channel(std::string nick, socket_t socket, const std::string &channelName, const std::string &reason)
{
	Channel	*chan = _chan_exists(channelName);

	if (!chan) {
		Channel::sendMsgToSocket(socket, "There is no chan with this name\n");
		return (false); // there is no channel with this name
	}
	if (chan->leave(nick, socket, reason)) {
		(*_user_channels)[nick]->remove(chan); // remove this chan from this user channels list 
		if (!chan->isEmpty())
			return (true);
		_channels->remove(chan); // channel is empty -> we delete it
		delete chan;
		return (true);
	}
	return (false);
}

bool	ChannelMaster::leave(std::string nick, socket_t socket, const std::vector<std::string> &args)
{
	bool	ret = true;
	std::vector<std::string>	names = splitComma(args[0]);
	std::string		reason = args.size() > 1 ? args[1] : std::string(); 
	for (size_t i = 0; i < names.size(); ++i) {
		ret &= _leave_channel(nick, socket, names[i], reason);
	}
	return (ret);
}

bool	ChannelMaster::mode(std::string nick, socket_t socket, const std::vector<std::string> &args)
{
	Channel					*chan;
	std::string				operations = args[1]; // looks like "+o"
	bool					append;

	if (operations.size() < 2 || !(operations[0] == '+' || operations[0] == '-'))
		return (false);
	append = operations[0] == '+';
	if (_testChannelName(socket, args[0]))
		return (false);
	chan = _chan_exists(args[0]);
	if (!chan)
		return (false);
	for (size_t i = 1; i < operations.size(); ++i) {
		switch (operations[i])
		{
		case 'o':
			if (args.size() < 3)
				return (false);
			chan->mode_o(append, nick, socket, args[2]);
			break;
		case 'v':
			if (args.size() < 3)
				return (false);
			chan->mode_v(append, nick, socket, args[2]);
			break;
		case 'p':
			chan->mode_p(append, nick, socket);
			break;
		case 's':
			chan->mode_s(append, nick, socket);
			break;
		case 'i':
			chan->mode_i(append, nick, socket);
			break;
		case 't':
			chan->mode_t(append, nick, socket);
			break;
		case 'm':
			chan->mode_m(append, nick, socket);
			break;
		case 'l':
			if (args.size() < 3)
				return (false);
			chan->mode_l(append, nick, socket, std::stoi(args[2]));
			break;
		case 'k':
			if (args.size() < 3)
				return (false);
			chan->mode_k(append, nick, socket, args[2]);
			break;
		default:
			break;
		}
	}
	return (true);
}

Channel		*ChannelMaster::_chan_exists(const std::string &channelName)
{
	_channel_list::iterator	current = _channels->begin();
	_channel_list::iterator	end = _channels->end();

	while (current != end) {
		if ((*current)->getName() == channelName)
			return (*current);
		++current;
	}
	return (nullptr);
}

bool	ChannelMaster::kick(std::string nick, socket_t socket, const std::vector<std::string> &args)
{
	Channel		*channel = _chan_exists(args[0]);
	std::string	guyToKick = args[1];
	std::string	reason = args.size() > 3 ? args[2] : std::string();
	if (!channel)
		return (!Channel::sendMsgToSocket(socket, "no chan with this name\n"));
	if (channel->kick(nick, socket, guyToKick, reason)) {
		(*_user_channels)[nick]->remove(channel);
		return (true);
	}
	return (false);
}


bool	ChannelMaster::broadcastMsg(const std::string &sender, socket_t socket, const std::string &chanName, const std::string &msg)
{
	Channel		*channel = _chan_exists(chanName);

	if (!channel)
		return (!Channel::sendMsgToSocket(socket, "no chan with this name\n"));
	return (channel->broadcastMsg(sender, socket, msg));
}

bool	ChannelMaster::topic(std::string nick, socket_t socket, const std::vector<std::string> &args)
{
	std::string	newTopic;
	Channel		*channel = _chan_exists(args[0]);

	if (!channel)
		return (!Channel::sendMsgToSocket(socket, "no chan with this name\n"));
	if (args.size() == 1)
		return (!Channel::sendMsgToSocket(socket, std::string("Topic is: ") + channel->getTopic() + "\n"));
	newTopic = args[1];
	return (channel->setTopic(nick, socket, newTopic));
}
