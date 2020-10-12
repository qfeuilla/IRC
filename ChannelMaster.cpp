#include "ChannelMaster.hpp"
ChannelMaster::ChannelMaster(const std::string &srvName): _srv_name(srvName) {
	_channels = new _channel_list();
	_user_channels = new _user_chan_map();
}
ChannelMaster::~ChannelMaster() {
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
bool	ChannelMaster::_testChannelName(Client *client, const std::string &channelName)
{
	bool badName = false;
	std::string	ms;

	if (channelName.at(0) != '#' && channelName.at(0) != '&' && channelName.at(0) != '+' && channelName.at(0) != '!')
		badName = true;
	if (channelName.length() > 50)
		badName = true;
	if (channelName.find(" ") != std::string::npos)
		badName = true;
	if (channelName.find(",") != std::string::npos)
		badName = true;
	if (channelName.find("\7") != std::string::npos)
		badName = true;
	if (badName) {
		(void)client;
		return (true);
	}
	return (false); // channelName is well formatted
}

std::vector<std::string>	splitComma(std::string const &str, bool asLowerCase = false)
{
	std::vector<std::string>	ret;
	size_t startPos;
	size_t endPos = 0;
	std::string		chunk;

	while ((startPos = str.find_first_not_of(',', endPos)) != std::string::npos)
	{
		endPos = str.find(',', startPos);
		chunk = str.substr(startPos, endPos - startPos);
		if (asLowerCase)
			ret.push_back(utils::ircLowerCase(chunk));
		else
			ret.push_back(chunk);
	}
	return (ret);
}


bool	ChannelMaster::join(Client *client, const std::vector<std::string> &args,
std::list<Channel*> *usrChans, OtherServ *svFrom)
{
	std::map<std::string, std::string>	name_pass_map;
	std::map<std::string, std::string>::iterator	current;
	std::map<std::string, std::string>::iterator	end;
	std::vector<std::string>	names;
	std::vector<std::string>	passwds;
	bool	ret = true;

	(*_user_channels)[client->nick] = usrChans;
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
		ret &= joinChannel(client, (*current).first, (*current).second, svFrom);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::joinChannel(Client *client, const std::string &channelName,
const std::string &passwd, OtherServ *svFrom)
{
	Channel	*chan = getChannel(channelName);
	std::string			ms;

	if (!chan) {
		if (_testChannelName(client, channelName))
			return (false);
		Channel	*nchan = new Channel(channelName, client, svFrom);
		_channels->push_back(nchan);
		(*_user_channels)[client->nick]->push_back(nchan);
		return (true);
	}
	if (chan->join(client, passwd, svFrom)) {
		(*_user_channels)[client->nick]->push_back(chan);
		return (true);
	}
	return (false);
}

bool	ChannelMaster::leaveChannel(Client *client, const std::string &channelName,
const std::string &reason, OtherServ *svFrom)
{
	Channel	*chan = getChannel(channelName);
	std::string		ms;

	if (!chan) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({channelName}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	if (chan->leave(client, reason, svFrom)) {
		(*_user_channels)[client->nick]->remove(chan); // remove this chan from this user channels list 
		if (!chan->isEmpty())
			return (true);
		_channels->remove(chan); // channel is empty -> we delete it
		delete chan;
		return (true);
	}
	return (false);
}

bool	ChannelMaster::leave(Client *client, const std::vector<std::string> &args, OtherServ *svFrom)
{
	bool	ret = true;
	std::vector<std::string>	names = splitComma(args[0]);
	std::string		reason = args.size() > 1 ? Channel::parseArg(1, args) : std::string();
	for (size_t i = 0; i < names.size(); ++i) {
		ret &= leaveChannel(client, names[i], reason, svFrom);
	}
	return (ret);
}

bool	ChannelMaster::getChanModes(Client *client, const std::vector<std::string> &args)
{
	Channel		*chan;
	std::string	ms;
	std::string	modes;

	chan = getChannel(args[0]);
	if (!chan)
		return (false);
	modes = chan->getModes();
	std::vector<std::string>	params({chan->getName(), modes, ""});
	ms = reply_formating(client->servername.c_str(), RPL_CHANNELMODEIS, params, client->nick.c_str());
	return (Channel::rplMsg(ms, client));
}

bool	ChannelMaster::mode(Client *client, const std::vector<std::string> &args, OtherServ *svFrom)
{
	Channel					*chan;
	std::string				operations = args[1]; // looks like "+o" 
	bool					append;
	std::string				ms;

	chan = getChannel(args[0]);
	
	if (!chan) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL,
		std::vector<std::string>({args[0]}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	if (operations[0] == 'O') {
		chan->showChanCreator(client);
		return (true);
	} else if (operations[0] == 'I') {
		chan->showInvitelist(client);
	} else if (operations[0] == 'e') {
		chan->showExceptionlist(client);
	} else if (operations[0] == 'b') {
		chan->showBanlist(client);
	}
	if (!(operations[0] == '+' || operations[0] == '-'))
		return (false);
	append = operations[0] == '+';
	for (size_t i = 1; i < operations.size(); ++i) {
		switch (operations[i])
		{
		case 'o':
			if (args.size() < 3)
				return (false);
			chan->mode_o(svFrom, append, client, args[2]);
			break;
		case 'O':
			if (args.size() < 3)
				chan->showChanCreator(client);
			else
				chan->mode_O(svFrom, append, client, args[2]);
			break;
		case 'e':
			if (args.size() < 3)
				chan->showExceptionlist(client);
			else
				chan->mode_e(svFrom, append, client, args[2]);
			break;
		case 'v':
			if (args.size() < 3)
				return (false);
			chan->mode_v(svFrom, append, client, args[2]);
			break;
		case 'b':
			if (args.size() < 3)
				chan->showBanlist(client);
			else
				chan->mode_b(svFrom, append, client, args[2]);
			break;
		case 'I':
			if (args.size() < 3)
				chan->showInvitelist(client);
			else
				chan->mode_I(svFrom, append, client, args[2]);
			break;
		case 'p':
			chan->mode_p(svFrom, append, client);
			break;
		case 's':
			chan->mode_s(svFrom, append, client);
			break;
		case 'i':
			chan->mode_i(svFrom, append, client);
			break;
		case 't':
			chan->mode_t(svFrom, append, client);
			break;
		case 'm':
			chan->mode_m(svFrom, append, client);
			break;
		case 'n':
			chan->mode_n(svFrom, append, client);
			break;
		case 'q':
			chan->mode_q(svFrom, append, client);
			break;
		case 'l':
			if (args.size() == 2) {
				if (!append)
					chan->mode_l(svFrom, append, client, -1);
			} else
				chan->mode_l(svFrom, append, client, std::stoi(args[2]));
			break;
		case 'k':
			if (args.size() == 2) {
				if (!append) 
					chan->mode_k(svFrom, append, client, "");
			} else
				chan->mode_k(svFrom, append, client, args[2]);
			break;
		default:
			break;
		}
	}
	// TODO chan->updateServsChan(client);
	return (true);
}

Channel		*ChannelMaster::getChannel(const std::string &channelName)
{
	_channel_list::iterator	current = _channels->begin();
	_channel_list::iterator	end = _channels->end();

	while (current != end) {
		if (utils::strCmp(channelName, (*current)->getName()))
			return (*current);
		++current;
	}
	return (nullptr);
}

bool	ChannelMaster::kick(Client *client, const std::vector<std::string> &args, OtherServ *svFrom)
{
	std::map<std::string, std::string>	name_pass_map;
	std::map<std::string, std::string>::iterator	current;
	std::map<std::string, std::string>::iterator	end;
	std::vector<std::string>	chanNames;
	std::vector<std::string>	guysToKick;
	std::string	reason = "";
	bool	ret = true;

	chanNames = splitComma(args[0]);
	guysToKick = splitComma(args[1]);
	if (args.size() > 2)
		reason = Channel::parseArg(2, args);

	if (chanNames.size() == 1) {
		for (std::string nickname : guysToKick) {
			ret &= kickFromChan(client, chanNames[0], nickname, reason, svFrom);
		}
		return (ret);
	}
	if (chanNames.size() != guysToKick.size())
		return (false);

	for (size_t i = 0; i < chanNames.size(); ++i) {
		name_pass_map[chanNames[i]] = guysToKick[i];
	}
	current = name_pass_map.begin();
	end = name_pass_map.end();
	while (current != end) {
		ret &= kickFromChan(client, (*current).first, (*current).second, reason, svFrom);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::kickFromChan(Client *client, const std::string &chanName,
const std::string &guyToKick, const std::string &reason, OtherServ *svFrom)
{
	std::string	ms;
	Channel		*channel = getChannel(chanName);

	if (!channel) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL,
		std::vector<std::string>({chanName}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	if (channel->kick(client, guyToKick, reason, svFrom)) {
		(*_user_channels)[client->nick]->remove(channel);
		return (true);
	}
	return (false);
}


bool	ChannelMaster::broadcastMsg(OtherServ *svFrom, Client *client, const std::string &chanName, const std::vector<std::string> &args, bool sendErrors)
{
	std::string	ms;
	Channel		*channel = getChannel(chanName);
	std::string	msgToSend = Channel::parseArg(1, args);

	if (!channel) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, {chanName}, client->nick.c_str());
		if (sendErrors)
			Channel::rplMsg(ms, client);
		return (false);
	}
	if (channel->getModeN() && !channel->isInChan(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CANNOTSENDTOCHAN, {chanName}, client->nick.c_str());
		if (sendErrors)
			Channel::rplMsg(ms, client);
		return (false);
	}
	if (channel->msgErrors(client, sendErrors))
		return (false);
	ms = ":" + client->nick + "!" + client->username + "@" + client->servername;
	ms += sendErrors ? " PRIVMSG " : " NOTICE ";
	ms += channel->getName() + " :" + msgToSend;
	bool	ret = channel->broadcastMsg(client, ms);
	if (ret) {
		ms = ":" + client->nick;
		ms += sendErrors ? " PRIVMSG " : " NOTICE ";
		ms += channel->getName() + " :" + msgToSend;
		client->sendToAllServs(ms, svFrom);
	}
	return (ret);
}

bool	ChannelMaster::topic(Client *client, const std::vector<std::string> &args, OtherServ *svFrom)
{
	std::string	ms;
	std::string	newTopic = "";
	std::string	topicIs;

	Channel		*channel = getChannel(args[0]);

	if (args.size() > 1)
		newTopic = Channel::parseArg(1, args);

	if (!channel) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({args[0]}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	if (args.size() == 1) {
		topicIs = channel->getTopic();
		if (topicIs == "") {
			ms = reply_formating(client->servername.c_str(), RPL_NOTOPIC, std::vector<std::string>({channel->getName()}), client->nick.c_str());
			return (Channel::rplMsg(ms, client));
		}
		ms = reply_formating(client->servername.c_str(), RPL_TOPIC, std::vector<std::string>({channel->getName(), topicIs}), client->nick.c_str());
		return (Channel::rplMsg(ms, client));
	}
	return (channel->setTopic(client, newTopic, svFrom));
}

bool	ChannelMaster::invite(Client *client, const std::vector<std::string> &args, OtherServ *svFrom)
{
	std::string	ms;
	std::string	userName = args[0];
	Channel		*channel = getChannel(args[1]);

	if (!channel) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({args[1]}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	return (channel->invite(client, userName, svFrom));
}

void	ChannelMaster::setSrvName(const std::string &srvName)
{
	_srv_name = std::string(srvName);
}

// USER mayo 0 * :realname (pour se connecter avec nc)
bool	ChannelMaster::list(Client *client, const std::vector<std::string> &args)
{
	std::string	chanName;
	std::string	numUsersVisible;
	std::string	topic;
	std::string	chanModes;
	std::vector<std::string>	names;
	
	if (args.size() >= 1)
		names = splitComma(args[0], true);

	// begin list
	std::string	ms;
	ms = ":" + client->servername + " 321 " + client->nick + " Channel :Users Name";
	custom_send(ms, client);

	// * list chans
	// :chatjunkies.org 322 adwonno #linuxdojo 10 :[+nt]
	// :chatjunkies.org 322 adwonno #trax 7 :[+nt] #trax museum : bring back the only demo art
	for (Channel *chan : *_channels) {
		chanModes = chan->getModes();
		if (chanModes.find("s") != std::string::npos)
			continue ; // this chan is secret: skip it
		if (chanModes.find("p") != std::string::npos)
			continue ; // this chan is private: skip it
		chanName = chan->getName();
		numUsersVisible = std::to_string(chan->getUsersNum());
		topic = "[" + chanModes + "]";
		if (chan->getTopic() != "") {
			topic += " " + chan->getTopic();
		}
		std::vector<std::string> params({chanName, numUsersVisible, topic});
		ms = reply_formating(client->servername.c_str(), RPL_LIST, params, client->nick.c_str());
		if (args.size() < 1)
			custom_send(ms, client);
		else {
			if (std::find(names.begin(), names.end(), utils::ircLowerCase(chanName)) != names.end())
				custom_send(ms, client);
		}
	}

	// end list
	ms = ":" + client->servername + " 323 " + client->nick + " :End of channel list";
	custom_send(ms, client);
	return (true);
}

size_t	ChannelMaster::size() const
{
	_channel_list::const_iterator	current = _channels->begin();
	_channel_list::const_iterator	end = _channels->end();
	size_t	size = 0;

	while (current != end) {
		++current;
		++size;
	}
	return (size);
}

void		ChannelMaster::doQuit(Client *client, const std::vector<std::string> &args)
{
	_channel_list::iterator	current = _channels->begin();
	Channel *nextChannel;

	while (current != _channels->end()) {
		nextChannel = *current;
		if (nextChannel->isInChan(client->nick)) {
			nextChannel->quit(client, args);
			++current;
			delChanIfEmpty(nextChannel);
		} else {
			++current;
		}
	}
}

bool		ChannelMaster::localChanWHO(Client *client, const std::vector<std::string> &args)
{
	Channel *chan = getChannel(args[0]);

	if (!chan)
		return (false);
	return (chan->who(client));
}

bool		ChannelMaster::names(Client *client, const std::vector<std::string> &args)
{
	bool	ret = true;
	if (args.size() == 0) {
		for (Channel *chan : *_channels) {
			chan->usrList(client); // names message for local channels
		}
		return (true);
	}
	std::vector<std::string>	names = splitComma(args[0]);
	for (size_t i = 0; i < names.size(); ++i) {
		ret &= chanNames(client, names[i]);
	}
	return (ret);
}

bool	ChannelMaster::chanNames(Client *client, const std::string &channelName)
{
	Channel	*chan = getChannel(channelName);
	std::string		ms;

	if (!chan) {
		return (false);
	}
	return (chan->usrList(client));
}

bool		ChannelMaster::delChanIfEmpty(Channel *chan)
{
	if (chan->isEmpty()) {
		_channels->remove(chan); // channel is empty -> we delete it
		delete chan;
		return (true);
	}
	return (false);
}

void		ChannelMaster::changeNick(const std::string &oldNick, const std::string &newNick)
{
	for (Channel *chan : *_channels) {
		if (chan->isInChan(oldNick))
			chan->changeNick(oldNick, newNick);
	}
}

void		ChannelMaster::shareAll(OtherServ *sv) const
{
	std::string	ms;

	for (Channel *chan : *_channels) {
		std::vector<std::string>	users = chan->getUsersVec();
		for (std::string &nick : users) {
			ms = ":" + nick + " JOIN " + chan->getName();
			custom_send(ms, sv);
		}
		std::vector<std::string>	opers = chan->getUsersVec();
		for (std::string &nick : opers) {
			ms = ":" + nick + " MODE " + chan->getName() + " +o " + nick;
			custom_send(ms, sv);
		}
		std::vector<std::string>	voiced = chan->getUsersVec();
		for (std::string &nick : voiced) {
			ms = ":" + nick + " MODE " + chan->getName() + " +v " + nick;
			custom_send(ms, sv);
		}
		if (chan->getTopic() != "") {
			ms = ":" + users[0] + " TOPIC "  + chan->getName() + " :" + chan->getTopic();
			custom_send(ms, sv);
		}
		std::string	modes = chan->getModes().substr(1);
		for (char c : modes) {
			ms = ":" + users[0] + " MODE " + chan->getName() + " +" + c;
			custom_send(ms, sv);
		}
	}
}
