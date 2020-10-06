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


bool	ChannelMaster::join(Client *client, const std::vector<std::string> &args, std::list<Channel*> *usrChans)
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
		ret &= joinChannel(client, (*current).first, (*current).second);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::joinChannel(Client *client, const std::string &channelName, const std::string &passwd)
{
	Channel	*chan = getChannel(channelName);
	OtherServ			*serv;
	std::string			ms;

	if (!chan) {
		// if there is chan with this name in another serv, we forward the join message to this serv
		serv = client->getServByChannelName(channelName);
		if (serv) {
			ms = ":" + client->nick + " JOIN " + channelName + " " + passwd;
			custom_send(ms, serv);
			return (true); // stop the function here to prevent creating a local channel
		}
	}
	if (!chan) {
		if (_testChannelName(client, channelName))
			return (false);
		Channel	*nchan = new Channel(channelName, client, _srv_name);
		_channels->push_back(nchan);
		(*_user_channels)[client->nick]->push_back(nchan);
		return (true);
	}
	if (chan->join(client, passwd)) {
		if (client->sock != -1)
			(*_user_channels)[client->nick]->push_back(chan);
		return (true);
	}
	return (false);
}

bool	ChannelMaster::leaveChannel(Client *client, const std::string &channelName, const std::string &reason)
{
	Channel	*chan = getChannel(channelName);
	std::string		ms;
	OtherServ		*serv;

	if (!chan) {
		// if there is chan with this name in another serv, we forward the part message to this serv
		serv = client->getServByChannelName(channelName);
		if (serv) {
			ms = ":" + client->nick + " PART " + channelName  + " :" + reason;
			custom_send(ms, serv);
			return (true);
		}
	}
	if (!chan) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({channelName}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	if (chan->leave(client, reason)) {
		if (client->sock != -1)
			(*_user_channels)[client->nick]->remove(chan); // remove this chan from this user channels list 
		if (!chan->isEmpty())
			return (true);
		_channels->remove(chan); // channel is empty -> we delete it
		delete chan;
		return (true);
	}
	return (false);
}

bool	ChannelMaster::leave(Client *client, const std::vector<std::string> &args)
{
	bool	ret = true;
	std::vector<std::string>	names = splitComma(args[0]);
	std::string		reason = args.size() > 1 ? Channel::parseArg(1, args) : std::string();
	for (size_t i = 0; i < names.size(); ++i) {
		ret &= leaveChannel(client, names[i], reason);
	}
	return (ret);
}

bool	ChannelMaster::getChanModes(Client *client, const std::vector<std::string> &args)
{
	Channel		*chan;
	std::string	ms;
	std::string	modes;
	OtherServ	*serv;

	chan = getChannel(args[0]);
	if (!chan) {
		// if there is chan with this name in another serv, we forward the join message to this serv
		serv = client->getServByChannelName(args[0]);
		if (serv) {
			ms = ":" + client->nick + " MODE " + args[0];
			custom_send(ms, serv);
			return (true); // stop the function here to prevent creating a local channel
		}
	}
	if (!chan)
		return (false);
	modes = chan->getModes();
	std::vector<std::string>	params({chan->getName(), modes, ""});
	ms = reply_formating(client->servername.c_str(), RPL_CHANNELMODEIS, params, client->nick.c_str());
	return (Channel::rplMsg(ms, client));
}

bool	ChannelMaster::mode(Client *client, const std::vector<std::string> &args)
{
	Channel					*chan;
	std::string				operations = args[1]; // looks like "+o" 
	bool					append;
	OtherServ				*serv;
	std::string				ms;

	chan = getChannel(args[0]);
	
	if (!chan) {
		serv = client->getServByChannelName(args[0]);
		if (!serv)
			return (false);
		ms = ":" + client->nick + " MODE ";
		for (std::string str: args) {
			ms += str + " ";
		}
		custom_send(ms, serv);
		return (true);
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
			chan->mode_o(append, client, args[2]);
			break;
		case 'O':
			if (args.size() < 3)
				chan->showChanCreator(client);
			else
				chan->mode_O(append, client, args[2]);
			break;
		case 'e':
			if (args.size() < 3)
				chan->showExceptionlist(client);
			else
				chan->mode_e(append, client, args[2]);
			break;
		case 'v':
			if (args.size() < 3)
				return (false);
			chan->mode_v(append, client, args[2]);
			break;
		case 'b':
			if (args.size() < 3)
				chan->showBanlist(client);
			else
				chan->mode_b(append, client, args[2]);
			break;
		case 'I':
			if (args.size() < 3)
				chan->showInvitelist(client);
			else
				chan->mode_I(append, client, args[2]);
			break;
		case 'p':
			chan->mode_p(append, client);
			break;
		case 's':
			chan->mode_s(append, client);
			break;
		case 'i':
			chan->mode_i(append, client);
			break;
		case 't':
			chan->mode_t(append, client);
			break;
		case 'm':
			chan->mode_m(append, client);
			break;
		case 'n':
			chan->mode_n(append, client);
		case 'q':
			chan->mode_q(append, client);
			break;
		case 'l':
			if (args.size() == 2) {
				if (!append)
					chan->mode_l(append, client, -1);
			} else
				chan->mode_l(append, client, std::stoi(args[2]));
			break;
		case 'k':
			if (args.size() == 2) {
				if (!append) 
					chan->mode_k(append, client, "");
			} else
				chan->mode_k(append, client, args[2]);
			break;
		default:
			break;
		}
	}
	chan->updateServsChan(client);
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

bool	ChannelMaster::kick(Client *client, const std::vector<std::string> &args)
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
			ret &= kickFromChan(client, chanNames[0], nickname, reason);
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
		ret &= kickFromChan(client, (*current).first, (*current).second, reason);
		++current;
	}
	return (ret);
}

bool	ChannelMaster::kickFromChan(Client *client, const std::string &chanName,
const std::string &guyToKick, const std::string &reason)
{
	std::string	ms;
	Channel		*channel = getChannel(chanName);
	
	OtherServ			*serv;

	if (!channel) {
		// if there is chan with this name in another serv, we forward the kick message to this serv
		serv = client->getServByChannelName(chanName);
		if (serv) {
			ms = ":" + client->nick + " KICK " + chanName + " " + guyToKick + " :" + reason;
			custom_send(ms, serv);
			return (true);
		}
	}

	if (!channel) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL,
		std::vector<std::string>({chanName}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	if (channel->kick(client, guyToKick, reason)) {
		if (client->sock != -1)
			(*_user_channels)[client->nick]->remove(channel);
		return (true);
	}
	return (false);
}


bool	ChannelMaster::broadcastMsg(Client *client, const std::string &chanName, const std::vector<std::string> &args, bool sendErrors)
{
	std::string	ms;
	Channel		*channel = getChannel(chanName);
	std::string	msgToSend = Channel::parseArg(1, args);
	OtherServ	*serv;

	if (!channel) {
		// if there is chan with this name in another serv, we forward the topic message to this serv
		serv = client->getServByChannelName(chanName);
		if (serv) {
			ms = ":" + client->nick + (sendErrors ? " PRIVMSG " : " NOTICE ") + chanName + " :" + msgToSend;
			custom_send(ms, serv);
			return (true);
		}
	}

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
	return (channel->broadcastMsg(client, ms));
}

bool	ChannelMaster::topic(Client *client, const std::vector<std::string> &args)
{
	std::string	ms;
	std::string	newTopic = "";
	std::string	topicIs;
	OtherServ	*serv;

	Channel		*channel = getChannel(args[0]);

	if (args.size() > 1)
		newTopic = Channel::parseArg(1, args);

	if (!channel) {
		// if there is chan with this name in another serv, we forward the topic message to this serv
		serv = client->getServByChannelName(args[0]);
		if (serv) {
			ms = ":" + client->nick + " TOPIC " + args[0];
			if (newTopic != "")
				ms += " :" + newTopic;
			custom_send(ms, serv);
			return (true);
		}
	}

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
	return (channel->setTopic(client, newTopic));
}

bool	ChannelMaster::invite(Client *client, const std::vector<std::string> &args)
{
	std::string	ms;
	std::string	userName = args[0];
	Channel		*channel = getChannel(args[1]);
	OtherServ	*serv;

	if (!channel) {
		// if there is chan with this name in another serv, we forward the invite message to this serv
		serv = client->getServByChannelName(args[1]);
		if (serv) {
			ms = ":" + client->nick + " INVITE " + args[0] + " " + args[1];
			custom_send(ms, serv);
			return (true);
		}
	}

	if (!channel) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({args[1]}), client->nick.c_str());
		return (!Channel::rplMsg(ms, client));
	}
	return (channel->invite(client, userName));
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
	std::vector<Chan>	serverChans = client->getServsChans();
	
	if (args.size() >= 1)
		names = splitComma(args[0], true);

	// begin list
	std::string	ms;
	ms = ":" + client->servername + " 321 " + client->nick + " Channel :Users Name";
	custom_send(ms, client);

	// * local chans
	// :chatjunkies.org 322 adwonno #linuxdojo 10 :[+nt]
	// :chatjunkies.org 322 adwonno #trax 7 :[+nt] #trax museum : bring back the only demo art
	for (Channel *chan : *_channels) {
		chanModes = chan->getModes();
		if (chanModes.find("s") != std::string::npos)
			continue ; // this chan is secret: skip it
		if (chanModes.find("p") != std::string::npos)
			continue ; // this chan is private: skip it
		chanName = chan->getName();
		numUsersVisible = chan->getUsersNum();
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

	// * others servers chans
	for (Chan &chan : serverChans) {
		chanModes = chan.modes;
		if (chanModes.find("s") != std::string::npos)
			continue ; // this chan is secret: skip it
		if (chanModes.find("p") != std::string::npos)
			continue ; // this chan is private: skip it
		chanName = chan.name;
		numUsersVisible = chan.usersNum;
		topic = "[" + chanModes + "]";
		if (chan.topic != "" && chan.topic != "!") {
			topic += " " + chan.topic;
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

std::vector<Chan>	ChannelMaster::getChans() const
{
	std::vector<Chan>	vec;

	for (Channel *nextChannel : *_channels) {
		if (nextChannel->getName()[0] == '&')
			continue ;
		vec.push_back(Chan(
			std::string(nextChannel->getName()),
			std::string(nextChannel->getUsersNum()),
			std::string(nextChannel->getModes()),
			std::string(nextChannel->getTopic()),
			std::vector<std::string>(nextChannel->getUsersVec())
		));
	}
	return (vec);
}

void		ChannelMaster::doQuit(Client *client, const std::vector<std::string> &args)
{
	for (Channel *nextChannel : *_channels) {
		if (nextChannel->isInChan(client->nick)) {
			nextChannel->quit(client, args);
		}
	}
}
