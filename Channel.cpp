#include "Channel.hpp"
#include "defines.hpp"

Channel::Channel(): _name(), _users(), _modes(), _topic(), _creator() {}
Channel::Channel(const std::string &name, Client *client, OtherServ *srv)
: _name(name), _users(), _modes(), _topic(), _creator(client->nick) {
	if (_name.at(0) != '+') {
		if (!srv) // if it comes from another serv, it will be followed by a MODE command if needed
			_modes.o.push_back(utils::ircLowerCase(client->nick));
	}
	std::cout << "Creating channel: " << _name << "\n\n";
	join(client, "", srv);
}
Channel::~Channel() {
	std::cout << "channel erased: " << _name << "\n\n";
}
Channel	&Channel::operator=(const Channel& other)
{
	_name = other._name;
	_users = other._users;
	return (*this);
}

const std::string	&Channel::getName() const
{
	return (_name);
}
int			Channel::getUsersNum() const
{
	return (_modes.users);
}
const std::string	&Channel::getCreator() const
{
	return (_creator);
}
const std::string	&Channel::getTopic() const
{
	return (_topic);
}

bool				Channel::setTopic(Client *client, const std::string &newTopic, OtherServ *svFrom)
{
	std::string	ms;
	if (!isInChan(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
		return (!rplMsg(ms, client));
	}
	if (_modes.t && (!svFrom && !_hasRights(client->nick))) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!rplMsg(ms, client));
	}
	_topic = std::string(newTopic);
	ms = client->getFullMask();
	ms += " TOPIC " + getName() + " :" +_topic;
	rplMsg(ms, client);
	broadcastMsg(client, ms);
	ms = ":" + client->nick + " TOPIC " + getName() + " :" + _topic;
	client->sendToAllServs(ms, svFrom);
	return (true);
}

bool				Channel::broadcastMsg(Client *sender, const std::string &msg) const
{
	if (getModeN() && !isInChan(sender->nick))
		return (false);
	_users_map::const_iterator	current = _users.begin();
	_users_map::const_iterator	end = _users.end();
	while (current != end) {
		if (!utils::strCmp((*current).first, sender->nick)) { // * send msg to everyone but the sender
			rplMsg(msg, (*current).second);
		}
		++current;
	}
	return (true);
}

bool				Channel::join(Client *client, const std::string &passwd, OtherServ *svFrom)
{
	std::string	ms;

	if (!isInChan(client->nick)) {
		if (!svFrom) { // if it comes from a server, we must not test
			if (_modes.k != "" && _modes.k != passwd) {
				ms = reply_formating(client->servername.c_str(), ERR_BADCHANNELKEY, std::vector<std::string>({getName()}), client->nick.c_str());
				return (!rplMsg(ms, client));
			}
			if (_modes.l != -1 && _modes.users >= _modes.l) {
				ms = reply_formating(client->servername.c_str(), ERR_CHANNELISFULL, std::vector<std::string>({getName()}), client->nick.c_str());
				return (!rplMsg(ms, client));
			}
			if (_modes.i && !_isInvited(client)) {
				ms = reply_formating(client->servername.c_str(), ERR_INVITEONLYCHAN, std::vector<std::string>({getName()}), client->nick.c_str());
				return (!rplMsg(ms, client));
			}
			if (_modes.p) {
				ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
				return (!rplMsg(ms, client));
			}
			if (_isBanned(client)) {
				if (!_isInvited(client)) {
					ms = reply_formating(client->servername.c_str(), ERR_BANNEDFROMCHAN, std::vector<std::string>({getName()}), client->nick.c_str());
					return (!rplMsg(ms, client));
				}
			}
		}
		// on ajoute le client dans le channel, et on le retire de la liste d'invitations
		_users.insert(std::pair<std::string, Client*>(utils::ircLowerCase(client->nick), client));
		_modes.invitation_list.remove(utils::ircLowerCase(client->nick));
		_modes.users++;
		
		std::string	join_msg = client->getFullMask();
		join_msg += " JOIN :" + getName();

		rplMsg(join_msg, client);
		if (!_modes.q)
			broadcastMsg(client, join_msg);
		if (getTopic() != "") {
			ms = reply_formating(client->servername.c_str(), RPL_TOPIC, std::vector<std::string>({getName(), getTopic()}), client->nick.c_str());
			rplMsg(ms, client);
		}

		usrList(client);

		ms = ":" + client->nick + " JOIN " + getName();
		client->sendToAllServs(ms, svFrom);
		if (_modes.users == 1) { // channel was just created
			ms = ":" + client->nick + " MODE " + getName() + " +o " + client->nick;
			client->sendToAllServs(ms, svFrom);
		}
		
		return (true);
	}
	return (false);
}

bool				Channel::leave(Client *client, const std::string &reason, OtherServ *svFrom, bool muted)
{
	std::string		ms;

	_users_map::iterator	user = _users.find(utils::ircLowerCase(client->nick));
	if (user == _users.end()) {
		ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
		if (!muted)
			rplMsg(ms, client);
		return (false);
	}
	ms = client->getFullMask();
	ms += " PART " + getName();
	ms += (reason != "") ? " :" + reason : "";
	if (!muted) {
		rplMsg(ms, client);
		if (!_modes.q)
			broadcastMsg(client, ms);
	}
	if (_isInList(user->first, _modes.o))
		_modes.o.remove(utils::ircLowerCase(user->first));
	if (_isInList(user->first, _modes.v))
		_modes.v.remove(utils::ircLowerCase(user->first));
	_users.erase(user);
	_modes.users--;
	if (!muted) {
		ms = ":" + client->nick + " PART " + getName();
		ms += (reason != "") ? " :" + reason : "";
		client->sendToAllServs(ms, svFrom);
	}
	return (true);
}

// modes

bool	Channel::mode_O(OtherServ *svFrom, bool append, Client *client, const std::string &target)
{
	std::string ms;
	if (!isInChan(target))
		return (true);
	if (append && (svFrom || utils::strCmp(client->nick, _creator))) {
		_creator = target;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +O " + utils::ircLowerCase(target);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +O " + utils::ircLowerCase(target);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}

	if (!append) {
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_o(OtherServ *svFrom, bool append, Client *client, const std::string &target)
{
	std::string ms;
	if (!isInChan(target))
		return (true);
	if (append && (svFrom || _hasRights(client->nick))) {
		if (_isInList(target, _modes.o)) // target is already chanop
			return (true);
		_modes.o.push_back(utils::ircLowerCase(target));
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +o " + utils::ircLowerCase(target);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +o " + utils::ircLowerCase(target);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}

	if (!append && (svFrom || _hasRights(client->nick))) {
		if (!_isInList(target, _modes.o)) // target is not chanop
			return (true);
		_modes.o.remove(utils::ircLowerCase(target));
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -o " + utils::ircLowerCase(target);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -o " + utils::ircLowerCase(target);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_v(OtherServ *svFrom, bool append, Client *client, const std::string &target)
{
	std::string ms;
	if (!isInChan(target))
		return (true);
	if (append && (svFrom || _hasRights(client->nick))) {
		if (_isInList(target, _modes.v))
			return (true);
		_modes.v.push_back(utils::ircLowerCase(target));
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +v " + utils::ircLowerCase(target);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +v " + utils::ircLowerCase(target);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}

	if (!append && (svFrom || _hasRights(client->nick))) {
		if (!_isInList(target, _modes.v))
			return (true);
		_modes.v.remove(utils::ircLowerCase(target));
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -v " + utils::ircLowerCase(target);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -v " + utils::ircLowerCase(target);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_b(OtherServ *svFrom, bool append, Client *client, const std::string &mask)
{
	std::string ms;
	time_t		time;
	std::string	fullmask = client->getFullMask();

	std::time(&time);
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.b[utils::ircLowerCase(mask)] = std::pair<std::string, time_t>(fullmask, time);
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +b " + utils::ircLowerCase(mask);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +b " + utils::ircLowerCase(mask);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}

	if (!append && (svFrom || _hasRights(client->nick))) {
		if (_modes.b.erase(utils::ircLowerCase(mask)) == 0)
			return (true);
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -b " + utils::ircLowerCase(mask);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -b " + utils::ircLowerCase(mask);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_e(OtherServ *svFrom, bool append, Client *client, const std::string &mask)
{
	std::string ms;
	time_t		time;
	std::string	fullmask = client->getFullMask();

	std::time(&time);
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.e[utils::ircLowerCase(mask)] = std::pair<std::string, time_t>(fullmask, time);
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +e " + utils::ircLowerCase(mask);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +e " + utils::ircLowerCase(mask);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}

	if (!append && (svFrom || _hasRights(client->nick))) {
		if (_modes.e.erase(utils::ircLowerCase(mask)) == 0)
			return (true);
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -e " + utils::ircLowerCase(mask);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -e " + utils::ircLowerCase(mask);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_I(OtherServ *svFrom, bool append, Client *client, const std::string &mask)
{
	std::string ms;
	time_t		time;
	std::string	fullmask = client->getFullMask();

	std::time(&time);
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.I[utils::ircLowerCase(mask)] = std::pair<std::string, time_t>(fullmask, time);
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +I " + utils::ircLowerCase(mask);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +I " + utils::ircLowerCase(mask);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}

	if (!append && (svFrom || _hasRights(client->nick))) {
		if (_modes.I.erase(utils::ircLowerCase(mask)) == 0)
			return (true);
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -I " + utils::ircLowerCase(mask);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -I " + utils::ircLowerCase(mask);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_p(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.p = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +p";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +p";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.p = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -p";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -p";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_s(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.s = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +s";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +s";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.s = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -s";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -s";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_i(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.i = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +i";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +i";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.i = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -i";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -i";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_t(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.t = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +t";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +t";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.t = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -t";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -t";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_m(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.m = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +m";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +m";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.m = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -m";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -m";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_n(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.n = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +n";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +n";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.n = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -n";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -n";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_q(OtherServ *svFrom, bool append, Client *client)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.q = true;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +q";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +q";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.q = false;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -q";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -q";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_l(OtherServ *svFrom, bool append, Client *client, int limit)
{
	std::string ms;
	if (append && (svFrom || _hasRights(client->nick))) {
		_modes.l = limit;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +l " + std::to_string(limit);
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +l " + std::to_string(limit);
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	if (!append && (svFrom || _hasRights(client->nick))) {
		_modes.l = -1;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -l";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -l";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_k(OtherServ *svFrom, bool append, Client *client, const std::string &passwd)
{
	std::string ms;
	if (!svFrom && !_hasRights(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!rplMsg(ms, client));
	}

	if (append) {
		if (_modes.k != "") {
			ms = reply_formating(client->servername.c_str(), ERR_KEYSET, {getName()}, client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		_modes.k = passwd;
		ms = client->getFullMask();
		ms += " MODE " + getName() + " +k " + passwd;
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " +k " + passwd;
		client->sendToAllServs(ms, svFrom);
		return (true);
	} else {
		_modes.k = "";
		ms = client->getFullMask();
		ms += " MODE " + getName() + " -k";
		broadcastMsg(client, ms);
		rplMsg(ms, client);
		ms = ":" + client->nick + " MODE " + getName() + " -k";
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
}

std::string	Channel::getModes() const
{
	std::string	modes = "+";
	modes += _modes.p ? "p" : "";
	modes += _modes.s ? "s" : "";
	modes += _modes.i ? "i" : "";
	modes += _modes.t ? "t" : "";
	modes += _modes.m ? "m" : "";
	modes += _modes.n ? "n" : "";
	modes += _modes.q ? "q" : "";
	modes += _modes.l != -1 ? "l" : "";
	modes += _modes.k != "" ? "k" : "";
	return (modes);
	
}

bool	Channel::getModeN() const
{
	return (_modes.n);
}

bool	Channel::kick(Client *client, const std::string &guyToKick, const std::string &reason, OtherServ *svFrom)
{
	std::string	ms;
	_users_map::iterator	userToKick = _users.find(utils::ircLowerCase(guyToKick));

	if (svFrom || _hasRights(client->nick)) {
		if (userToKick == _users.end()) {
			ms = reply_formating(client->servername.c_str(), ERR_USERNOTINCHANNEL, std::vector<std::string>({guyToKick, getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		ms = client->getFullMask();
		ms += " KICK " + getName() + " " + guyToKick;
		ms += (reason != "") ? " :" + reason : " :" + client->nick;
		rplMsg(ms, client);
		broadcastMsg(client, ms);

		if (_isInList(userToKick->first, _modes.o))
			_modes.o.remove(utils::ircLowerCase(userToKick->first));
		if (_isInList(userToKick->first, _modes.v))
			_modes.v.remove(utils::ircLowerCase(userToKick->first));
		_users.erase(userToKick);
		_modes.users--;
		
		ms = ":" + client->nick + " KICK " + getName() + " " + guyToKick;
		ms += (reason != "") ? " :" + reason : " :" + client->nick;
		client->sendToAllServs(ms, svFrom);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::invite(Client *client, const std::string &guyToInvite, OtherServ *svFrom)
{
	std::string	ms;
	_users_map::iterator	userToInvite = _users.find(utils::ircLowerCase(guyToInvite));
	Client		*clientToInvite = client->getOtherClient(guyToInvite);

	if (!svFrom) {
		if (!clientToInvite) {
			ms = reply_formating(client->servername.c_str(), ERR_NOSUCHNICK, {guyToInvite}, client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		if (!isInChan(client->nick)) {
			ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, {getName()}, client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		if (!_hasRights(client->nick)) {
			ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		if (userToInvite != _users.end()) { // user is already in channel (no need to invite)
			ms = reply_formating(client->servername.c_str(), ERR_USERONCHANNEL, std::vector<std::string>({guyToInvite, getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
	}
	if (!_isInList(guyToInvite, _modes.invitation_list))
		_modes.invitation_list.push_back(utils::ircLowerCase(guyToInvite));
	ms = client->getFullMask();
	ms += " INVITE " + utils::ircLowerCase(guyToInvite) + " :" + getName();
	rplMsg(ms, clientToInvite);
	ms = reply_formating(client->servername.c_str(), RPL_INVITING,
	std::vector<std::string>({utils::ircLowerCase(guyToInvite), getName()}), client->nick.c_str());
	rplMsg(ms, client);

	ms = ":" + client->nick + " INVITE " + utils::ircLowerCase(guyToInvite) + " :" + getName();
	client->sendToAllServs(ms, svFrom);
	return (true);
}

bool				Channel::isEmpty() const
{
	return (_users.empty());
}

std::string			Channel::parseArg(size_t fromIndex, const std::vector<std::string> &args)
{
	if (args[fromIndex] == "")
		return ("");
	std::string	argToReturn;
	if (args[fromIndex].at(0) == ':')
		argToReturn = args[fromIndex].substr(1);
	else
		argToReturn = std::string(args[fromIndex]);
	++fromIndex;
	while (fromIndex < args.size()) {
		argToReturn += " ";
		argToReturn += args[fromIndex++];
	}
	return (argToReturn);
}

bool		Channel::isInChan(const std::string &userName) const
{
	_users_map::const_iterator	user = _users.find(utils::ircLowerCase(userName));
	return (user != _users.end());
}

bool		Channel::msgErrors(Client *client, bool sendErrors) const
{
	std::string	ms;
	if (_modes.m || _isBanned(client)) { // user need to be chanop OR to be in voice list
		if (_hasRights(client->nick))
			return (false); // he can send the message (he is a chan op)
		if (_isInList(client->nick, _modes.v))
			return (false); // he can send the message (he is in voice list)
		ms = reply_formating(client->servername.c_str(), ERR_CANNOTSENDTOCHAN, {getName()}, client->nick.c_str());
		if (sendErrors)
			rplMsg(ms, client);
		return (true);
	}
	return (false); // user can send the message as the chan is not restricted
}

void		Channel::changeNick(const std::string &oldNick, const std::string &newNick)
{
	Client		*client;
	std::string	ms;
	if (_isInList(oldNick, _modes.o)) {
		_modes.o.remove(utils::ircLowerCase(oldNick));
		_modes.o.push_back(utils::ircLowerCase(newNick));
	}
	if (_isInList(oldNick, _modes.v)) {
		_modes.v.remove(utils::ircLowerCase(oldNick));
		_modes.v.push_back(utils::ircLowerCase(newNick));
	}
	_users_map::iterator	user = _users.find(utils::ircLowerCase(oldNick));
	if (user == _users.end())
		return ; // should not happen
	client = user->second;
	_users.erase(user);
	_users.insert(std::pair<std::string, Client*>(utils::ircLowerCase(newNick), client));
	// * not in RFC but usefull
	// :paprika!~pokemon@ip-46.net-80-236-89.joinville.rev.numericable.fr NICK :patrick-2
	ms = client->getFullMask();
	ms += " NICK :" + newNick;
	std::cout << "miaw new nick is == " << newNick << "\n\n";
	if (!_modes.q)
		broadcastMsg(client, ms);
	rplMsg(ms, client);
}

bool		Channel::quit(Client *client, const std::vector<std::string> &args)
{
	// QUIT :allez salut
	// :sdlfjJFFFF!~Oui_eneffet@CJ-eef.m3i.5tviju.IP QUIT :Quit: allez a +
	std::string	ms = client->getFullMask();
	ms += " QUIT ";
	for (size_t i = 0; i < args.size(); i++) {
		ms += args[i];
		if (i + 1 < args.size())
			ms += " ";
	}
	broadcastMsg(client, ms);
	leave(client, "", nullptr, true);
	return (true);
}

// * this function always returns true
bool		Channel::rplMsg(std::string ms, Client *c)
{
	if (c->sock == -1) {
		return (true);
	}
	custom_send(ms, c);
	return (true);
}

std::string		Channel::getUsersStr() const
{
	std::string	users;

	for (std::pair<std::string, Client *> pair: _users) {
		users += utils::ircLowerCase(pair.first);
		users += ",";
	}
	if (users.size() == 0)
		return ("nobody ");
	users[users.size() - 1] = ' '; // replace the last comma (',') by a space (' ')
	return (users);
}

std::vector<std::string>	Channel::getUsersVec() const
{
	std::vector<std::string>	users;
	for (std::pair<std::string, Client *> pair: _users) {
		users.push_back(std::string(pair.first));
	}
	return (users);
}

std::vector<std::string>	Channel::getOpersVec() const
{
	std::vector<std::string>	users;
	for (std::string nick: _modes.o) {
		users.push_back(std::string(nick));
	}
	return (users);
}

std::vector<std::string>	Channel::getVoicedVec() const
{
	std::vector<std::string>	users;
	for (std::string nick: _modes.v) {
		users.push_back(std::string(nick));
	}
	return (users);
}

// returns true if user is banned
bool		Channel::_isBanned(Client *client) const
{
	std::string	fullmask = client->getFullMask();
	for (std::pair<std::string, std::pair<std::string, time_t> > pair : _modes.b) {
		if (utils::strMatchToLower(pair.first, fullmask))
			return (!_isInExceptionList(client));
	}
	return (false);
}

// returns true if user is in the execption list
bool		Channel::_isInExceptionList(Client *client) const
{
	std::string	fullmask = client->getFullMask();
	for (std::pair<std::string, std::pair<std::string, time_t> > pair : _modes.e) {
		if (utils::strMatchToLower(pair.first, fullmask))
			return (true);
	}
	return (false);
}

bool		Channel::_isInvited(Client *client) const
{
	std::string	fullmask = client->getFullMask();
	if (_isInList(client->nick, _modes.invitation_list))
		return (true);
	for (std::pair<std::string, std::pair<std::string, time_t> > pair : _modes.I) {
		if (utils::strMatchToLower(pair.first, fullmask))
			return (true);
	}
	return (false);
}

void		Channel::showBanlist(Client *client) const
{
	std::string	ms;
	for (std::pair<std::string, std::pair<std::string, time_t> > pair : _modes.b) {
		ms = ":" + client->servername + " 367 " + client->nick + " " + getName() + " " + pair.first;
		ms += " " + pair.second.first + " " + std::to_string(pair.second.second);
		//:barjavel.freenode.net 367 ratata #superchan *super*!*@* ratata!~pokemon@ip-46.net-80-236-89.joinville.rev.numericable.fr 1601899407
		rplMsg(ms, client);
	}
	ms = reply_formating(client->servername.c_str(), RPL_ENDOFBANLIST, {getName()}, client->nick.c_str());
	rplMsg(ms, client);
}

void		Channel::showInvitelist(Client *client) const
{
	std::string	ms;
	for (std::pair<std::string, std::pair<std::string, time_t> > pair : _modes.I) {
		ms = ":" + client->servername + " 346 " + client->nick + " " + getName() + " " + pair.first;
		ms += " " + pair.second.first + " " + std::to_string(pair.second.second);
		rplMsg(ms, client);
	}
	ms = reply_formating(client->servername.c_str(), RPL_ENDOFINVITELIST, {getName()}, client->nick.c_str());
	rplMsg(ms, client);
}

void		Channel::showExceptionlist(Client *client) const
{
	std::string	ms;
	for (std::pair<std::string, std::pair<std::string, time_t> > pair : _modes.e) {
		ms = ":" + client->servername + " 348 " + client->nick + " " + getName() + " " + pair.first;
		ms += " " + pair.second.first + " " + std::to_string(pair.second.second);
		rplMsg(ms, client);
	}
	ms = reply_formating(client->servername.c_str(), RPL_ENDOFEXCEPTLIST, {getName()}, client->nick.c_str());
	rplMsg(ms, client);
}

void		Channel::showChanCreator(Client *client) const
{
	std::string ms;

	ms = reply_formating(client->servername.c_str(), RPL_UNIQOPIS,
	std::vector<std::string>({getName(), getCreator()}), client->nick.c_str());
	rplMsg(ms, client);
}

// :orwell.freenode.net 353 adwonno__ @ #superxd :adwonno__ ratata
// :wilhelm.freenode.net 353 ratata @ #superxd :ratata adwonno
bool		Channel::usrList(Client *client) const
{
	std::string	ms;
	if (!isInChan(client->nick)) {
		ms = reply_formating(client->servername.c_str(), RPL_ENDOFNAMES, {getName()}, client->nick.c_str());
		rplMsg(ms, client);
		return (true);
	}
	ms = ":" + client->servername + " 353 " + client->nick + " @ " + getName() + " :";
	for (std::pair<std::string, Client*> pair : _users) {
		Client *c = pair.second;
		if (_hasRights(c->nick))
			ms += "@";
		if (_isInList(c->nick, _modes.v))
			ms += "+";
		ms += c->nick + " ";
	}
	rplMsg(ms, client);
	ms = reply_formating(client->servername.c_str(), RPL_ENDOFNAMES, {getName()}, client->nick.c_str());
	rplMsg(ms, client);
	return (true);
}


// :192.168.0.13 352 bob #1 ~adwonno 127.0.0.1 192.168.0.13 bob H :0 boby
// :orwell.freenode.net 352 adwonno__ #tructruc ~pokemon ip-46.net-80-236-89.joinville.rev.numericable.fr orwell.freenode.net adwonno__ H :0 tortipouss
// :wilhelm.freenode.net 315 ratata #superxd :End of /WHO list.
bool		Channel::who(Client *client) const
{
	std::string	ms;
	if (!isInChan(client->nick)) {
		ms = ":" + client->servername + " 315 " + client->nick + " " + getName() + " :End of /WHO list";
		custom_send(ms, client);
		return (true);
	}
	for (std::pair<std::string, Client*> pair : _users) {
		Client *c = pair.second;
		ms = ":" + client->servername + " 352 " + client->nick + " " + getName() + " ";
		ms += c->username + " " + c->hostname + " " + c->servername + " " + c->nick;
		ms += " H" + (_hasRights(c->nick) ? std::string("@") : std::string(""));
		ms += (_isInList(c->nick, _modes.v)) ? std::string("+") : std::string("");
		ms += " :" + std::to_string(c->hop_count) + " " + c->realname;
		custom_send(ms, client);
	}
	ms = ":" + client->servername + " 315 " + client->nick + " " + getName() + " :End of /WHO list";
	custom_send(ms, client);
	return (true);
}
