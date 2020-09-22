#include "ChannelMaster.hpp"
ChannelMaster::ChannelMaster() {
	_channels = new _channel_list();
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


bool	ChannelMaster::join(std::string nick, socket_t socket, const std::vector<std::string> &args)
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
		ret &= _join_channel(nick, socket, (*current).first, (*current).second);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::_join_channel(std::string nick, socket_t socket, const std::string &channelName, const std::string &passwd)
{
	Channel	*chan = _chan_exists(channelName);
	if (!chan) {
		_testChannelName(channelName);
		Channel	*nchan = new Channel(channelName, nick, socket);
		_channels->push_back(nchan);
		return (true);
	}
	return (chan->join(nick, socket, passwd));
}

bool	ChannelMaster::_leave_channel(std::string nick, socket_t socket, const std::string &channelName)
{
	Channel	*chan = _chan_exists(channelName);

	(void)socket;
	if (!chan)
		return (false); // there is no channel with this name
	if (chan->leave(nick))
	{
		_channels->remove(chan); // channel is empty -> we delete it
		delete chan;
	}
	return (true);
}

bool	ChannelMaster::leave(std::string nick, socket_t socket, const std::vector<std::string> &args)
{
	bool	ret = true;
	std::vector<std::string>	names = splitComma(args[0]);
	for (size_t i = 0; i < names.size(); ++i) {
		ret &= _leave_channel(nick, socket, names[i]);
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
	try {
		_testChannelName(args[0]);
	} catch(const std::exception& e) {
		return (false);
	}
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
