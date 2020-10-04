#include "Channel.hpp"
#include "defines.hpp"

Channel::Channel(): _name(), _users(), _modes(), _topic(), _srv_name(), _creator() {}
Channel::Channel(const std::string &name, Client *client, const std::string &srvName)
: _name(name), _users(), _modes(), _topic(), _srv_name(srvName), _creator(client->nick) {
	if (_name.at(0) != '+') {
		_modes.o.push_back(utils::ircLowerCase(client->nick));
		_modes.n = true;
	}
	std::cout << "Creating channel: " << _name << "\n\n";
	join(client, "");
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
std::string			Channel::getUsersNum() const
{
	return (std::to_string(_modes.users));
}
const std::string	&Channel::getCreator() const
{
	return (_creator);
}
const std::string	&Channel::getTopic() const
{
	return (_topic);
}

bool				Channel::setTopic(Client *client, const std::string &newTopic)
{
	std::string	ms;
	if (!isInChan(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
		return (!rplMsg(ms, client));
	}
	if (_modes.t && !_hasRights(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!rplMsg(ms, client));
	}
	_topic = std::string(newTopic);
	ms = ":" + client->nick + "!" + client->username + "@" + client->servername;
	ms += " TOPIC " + getName() + " :" +_topic;
	ms += CRLF;
	succesMsg(ms, client);
	broadcastMsg(client, ms);
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

bool				Channel::join(Client *client, const std::string &passwd)
{
	std::string	ms;
	if (!isInChan(client->nick)) {
		if (_modes.k != passwd) {
			ms = reply_formating(client->servername.c_str(), ERR_BADCHANNELKEY, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		if (_modes.l != -1 && _modes.users >= _modes.l) {
			ms = reply_formating(client->servername.c_str(), ERR_CHANNELISFULL, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		if (_modes.i && !_is_in_list(client->nick, _modes.invitation_list)) {
			ms = reply_formating(client->servername.c_str(), ERR_INVITEONLYCHAN, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		if (_modes.p) {
			ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		// on ajoute le client dans le channel, et on le retire de la liste d'invitations
		_users.insert(std::pair<std::string, Client*>(utils::ircLowerCase(client->nick), client));
		_modes.invitation_list.remove(utils::ircLowerCase(client->nick));
		_modes.users++;
		
		std::string	join_msg = ":" + client->nick;
		join_msg += "!" + client->username + "@" + _srv_name;
		join_msg += " JOIN :" + getName();
		join_msg += CRLF;

		rplMsg(join_msg, client);
		if (!_modes.q)
			broadcastMsg(client, join_msg);
		updateServsChan(client); // update servers chan
		return (true);
	}
	return (false);
}

bool				Channel::leave(Client *client, const std::string &reason, bool muted)
{
	std::string		ms;

	_users_map::iterator	user = _users.find(utils::ircLowerCase(client->nick));
	if (user == _users.end()) {
		ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
		if (!muted)
			rplMsg(ms, client);
		return (false);
	}
	ms = ":" + client->nick + "!a" + client->username + "@";
	ms += client->servername + " PART " + getName();
	ms += (reason != "") ? " :" + reason : "";
	ms += CRLF;
	if (!muted) {
		succesMsg(ms, client);
		if (!_modes.q)
			broadcastMsg(client, ms);
	}
	if (_is_in_list(user->first, _modes.o))
		_modes.o.remove(utils::ircLowerCase(user->first));
	if (_is_in_list(user->first, _modes.v))
		_modes.v.remove(utils::ircLowerCase(user->first));
	_users.erase(user);
	_modes.users--;
	return (true);
}

// modes
bool	Channel::mode_o(bool append, Client *client, const std::string &target)
{
	std::string ms;
	if (!isInChan(target))
		return (true);
	if (append && _hasRights(client->nick)) {
		if (_is_in_list(target, _modes.o)) // target is already chanop
			return (true);
		_modes.o.push_back(utils::ircLowerCase(target));
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +o " + utils::ircLowerCase(target);
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}

	if (!append && _hasRights(client->nick)) {
		if (!_is_in_list(target, _modes.o)) // target is not chanop
			return (true);
		_modes.o.remove(utils::ircLowerCase(target));
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -o " + utils::ircLowerCase(target);
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_v(bool append, Client *client, const std::string &target)
{
	std::string ms;
	if (!isInChan(target))
		return (true);
	if (append && _hasRights(client->nick)) {
		if (_is_in_list(target, _modes.v))
			return (true);
		_modes.v.push_back(utils::ircLowerCase(target));
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +v " + utils::ircLowerCase(target);
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}

	if (!append && _hasRights(client->nick)) {
		if (!_is_in_list(target, _modes.v))
			return (true);
		_modes.v.remove(utils::ircLowerCase(target));
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -v " + utils::ircLowerCase(target);
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_p(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.p = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +p";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.p = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -p";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_s(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.s = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +s";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.s = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -s";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_i(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.i = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +i";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.i = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -i";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_t(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.t = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +t";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.t = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -t";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_m(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.m = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +m";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.m = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -m";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_n(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.n = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +n";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.n = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -n";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_q(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.q = true;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +q";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.q = false;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -q";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_l(bool append, Client *client, int limit)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.l = limit;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +l " + std::to_string(limit);
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.l = -1;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -l";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::mode_k(bool append, Client *client, const std::string &passwd)
{
	std::string ms;
	if (!_hasRights(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!rplMsg(ms, client));
	}

	if (append) {
		if (_modes.k != "") {
			ms = reply_formating(client->servername.c_str(), ERR_KEYSET, {getName()}, client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		_modes.k = passwd;
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +k " + passwd;
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
	} else {
		_modes.k = "";
		ms = ":" + client->nick + "!" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -k";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (rplMsg(ms, client));
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
	modes += _modes.l != -1 ? "l" : "";
	modes += _modes.k != "" ? "k" : "";
	return (modes);
	
}

bool	Channel::getModeN() const
{
	return (_modes.n);
}

bool	Channel::kick(Client *client, const std::string &guyToKick, const std::string &reason)
{
	std::string	ms;
	_users_map::iterator	userToKick = _users.find(utils::ircLowerCase(guyToKick));

	if (_hasRights(client->nick)) {
		if (userToKick == _users.end()) {
			ms = reply_formating(client->servername.c_str(), ERR_USERNOTINCHANNEL, std::vector<std::string>({guyToKick, getName()}), client->nick.c_str());
			return (!rplMsg(ms, client));
		}
		ms = ":" + client->nick + "!a" + client->username + "@";
		ms += client->servername + " KICK " + getName() + " " + guyToKick;
		ms += (reason != "") ? " :" + reason : " :" + client->nick;
		ms += CRLF;
		succesMsg(ms, client);
		broadcastMsg(client, ms);
		_users.erase(userToKick);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!rplMsg(ms, client));
}

bool	Channel::invite(Client *client, const std::string &guyToInvite)
{
	std::string	ms;
	_users_map::iterator	userToInvite = _users.find(utils::ircLowerCase(guyToInvite));
	Client		*clientToInvite = client->getOtherClient(guyToInvite);

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
	if (!_is_in_list(guyToInvite, _modes.invitation_list))
		_modes.invitation_list.push_back(utils::ircLowerCase(guyToInvite));
	ms = ":" + client->nick + "!" + client->username + "@" + client->servername;
	ms += " INVITE " + utils::ircLowerCase(guyToInvite) + " :" + getName();
	ms += CRLF;
	succesMsg(ms, clientToInvite);
	ms = reply_formating(client->servername.c_str(), RPL_INVITING,
	std::vector<std::string>({utils::ircLowerCase(guyToInvite), getName()}), client->nick.c_str());
	return (rplMsg(ms, client));
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
	if (_modes.m) { // user need to be chanop OR to be in voice list
		if (_hasRights(client->nick))
			return (false); // he can send the message (he is a chan op)
		if (_is_in_list(client->nick, _modes.v))
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
	if (_is_in_list(oldNick, _modes.o)) {
		_modes.o.remove(utils::ircLowerCase(oldNick));
		_modes.o.push_back(utils::ircLowerCase(newNick));
	}
	if (_is_in_list(oldNick, _modes.v)) {
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
	ms = ":" + oldNick + "!" + client->username + "@" + client->servername;
	ms += " NICK :" + newNick + CRLF;
	if (!_modes.q)
		broadcastMsg(client, ms);
}

bool		Channel::quit(Client *client, const std::vector<std::string> &args)
{
	// QUIT :allez salut
	// :sdlfjJFFFF!~Oui_eneffet@CJ-eef.m3i.5tviju.IP QUIT :Quit: allez a +
	std::string	ms = ":" + client->nick + "!" + client->username + "@" + client->servername;
	ms += " QUIT ";
	for (size_t i = 0; i < args.size(); i++) {
		ms += args[i];
		if (i + 1 < args.size())
			ms += " ";
	}
	ms += CRLF;
	broadcastMsg(client, ms);
	leave(client, "", true);
	return (true);
}

// * this function always returns true
bool		Channel::rplMsg(std::string ms, Client *c)
{
	std::string	msg = ":" + c->nick + " CHAN_RPL ";
	if (c->sock == -1) { // we need to send msg to the server on which the client is connected
		msg += ms;
		OtherServ *srv = c->serv;
		if (!srv)
			return (true);
		custom_send(msg, srv);
		return (true);
	}
	custom_send(ms, c);
	return (true);
}

// this function always returns true
bool		Channel::succesMsg(std::string ms, Client *c)
{
	if (c->sock == -1) { // we need to send msg to the server on which the client is connected
		OtherServ *srv = c->serv;
		if (!srv)
			return (true);
		custom_send(ms, srv);
		return (true);
	}
	custom_send(ms, c);
	return (true);
}

void			Channel::updateServsChan(Client *c) const
{
	std::string	ms;

	if (getName().at(0) == '&')
		return ; // channel is local to this serv
	ms = "CHAN_CHG ";
	ms += getName() + "," + getUsersNum() + "," + getModes() + ",";
	ms += (getTopic() != "") ? getTopic() : "!";
	ms += CRLF;
	c->sendToAllServs(ms);
}
