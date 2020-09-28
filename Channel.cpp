#include "Channel.hpp"
#include "defines.hpp"

Channel::Channel(): _name(), _users(), _modes(), _topic(), _srv_name(), _creator() {}
Channel::Channel(const std::string &name, Client *client, const std::string &srvName)
: _name(name), _users(), _modes(), _topic(), _srv_name(srvName), _creator(client->nick) {
	_modes.o.push_back(client->nick);
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
		return (!custom_send(ms, client));
	}
	if (_modes.t && !_hasRights(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!custom_send(ms, client));
	}
	_topic = std::string(newTopic);
	ms = ":" + client->nick + "!~" + client->username + "@" + client->servername;
	ms += " TOPIC " + getName() + " :" +_topic;
	ms += CRLF;
	custom_send(ms, client);
	broadcastMsg(client, ms);
	return (true);
}

bool				Channel::broadcastMsg(Client *sender, const std::string &msg)
{
	if (!isInChan(sender->nick))
		return (false);
	_users_map::iterator	current = _users.begin();
	_users_map::iterator	end = _users.end();
	while (current != end) {
		if ((*current).first != sender->nick) { // * send msg to everyone but the sender
			custom_send(msg, (*current).second);
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
			return (!custom_send(ms, client));
		}
		if (_modes.l != -1 && _modes.l >= _modes.users) {
			ms = reply_formating(client->servername.c_str(), ERR_CHANNELISFULL, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!custom_send(ms, client));
		}
		if (_modes.i && !_is_in_list(client->nick, _modes.invitation_list)) {
			ms = reply_formating(client->servername.c_str(), ERR_INVITEONLYCHAN, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!custom_send(ms, client));
		}
		if (_modes.p) {
			ms = reply_formating(client->servername.c_str(), ERR_NOSUCHCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
			return (!custom_send(ms, client));
		}
		// on ajoute le client dans le channel, et on le retire de la liste d'invitations
		_users.insert(std::pair<std::string, Client*>(client->nick, client));
		_modes.invitation_list.remove(client->nick);
		
		std::string	join_msg = ":" + client->nick;
		join_msg += "!~" + client->username + "@" + _srv_name;
		join_msg += " JOIN :" + getName();
		join_msg += CRLF;

		custom_send(join_msg, client);
		broadcastMsg(client, join_msg);
		return (true);
	}
	return (false);
}

bool				Channel::leave(Client *client, const std::string &reason)
{
	std::string		ms;

	(void)reason;
	_users_map::iterator	user = _users.find(client->nick);
	if (user == _users.end()) {
		ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, std::vector<std::string>({getName()}), client->nick.c_str());
		return (!custom_send(ms, client));
	}
	ms = ":" + client->nick + "!~a" + client->username + "@";
	ms += client->servername + " PART " + getName();
	ms += (reason != "") ? " :" + reason : "";
	ms += CRLF;
	custom_send(ms, client);
	broadcastMsg(client, ms);
	_users.erase(user);
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
		_modes.o.push_back(target);
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +o " + target;
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}

	if (!append && _hasRights(client->nick)) {
		if (!_is_in_list(target, _modes.o)) // target is not chanop
			return (true);
		_modes.o.remove(target);
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -o " + target;
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_v(bool append, Client *client, const std::string &target)
{
	std::string ms;
	if (!isInChan(target))
		return (true);
	if (append && _hasRights(client->nick)) {
		if (_is_in_list(target, _modes.v))
			return (true);
		_modes.v.push_back(target);
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +v " + target;
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}

	if (!append && _hasRights(client->nick)) {
		if (!_is_in_list(target, _modes.v))
			return (true);
		_modes.v.remove(target);
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -v " + target;
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_p(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.p = true;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +p";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.p = false;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -p";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_s(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.s = true;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +s";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.s = false;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -s";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_i(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.i = true;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +i";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.i = false;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -i";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_t(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.t = true;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +t";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.t = false;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -t";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_m(bool append, Client *client)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.m = true;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +m";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.m = false;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -m";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_l(bool append, Client *client, int limit)
{
	std::string ms;
	if (append && _hasRights(client->nick)) {
		_modes.l = limit;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +l " + std::to_string(limit);
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	if (!append && _hasRights(client->nick)) {
		_modes.l = -1;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -l";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::mode_k(bool append, Client *client, const std::string &passwd)
{
	std::string ms;
	if (!_hasRights(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!custom_send(ms, client));
	}
	std::cout << append << " = append\n\n";

	if (append) {
		if (_modes.k != "") {
			ms = reply_formating(client->servername.c_str(), ERR_KEYSET, {getName()}, client->nick.c_str());
			return (!custom_send(ms, client));
		}
		_modes.k = passwd;
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " +k " + passwd;
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	} else {
		std::cout << "WE ENTER HERE\n\n";
		_modes.k = "";
		ms = ":" + client->nick + "!~" + client->username + "@";
		ms += client->servername + " MODE " + getName() + " -k";
		ms += CRLF;
		broadcastMsg(client, ms);
		return (custom_send(ms, client));
	}
	std::cout << "WE RETURN HERE (NEVER)\n\n";
}

void	Channel::getModes(Client *client)
{
	std::string	ms;
	std::string	modes = "+";
	modes += _modes.p ? "p" : "";
	modes += _modes.s ? "s" : "";
	modes += _modes.i ? "i" : "";
	modes += _modes.t ? "t" : "";
	modes += _modes.m ? "m" : "";
	modes += _modes.l != -1 ? "l" : "";
	modes += _modes.k != "" ? "k" : "";
	std::vector<std::string>	params({getName(), modes, ""});
	ms = reply_formating(client->servername.c_str(), RPL_CHANNELMODEIS, params, client->nick.c_str());
	custom_send(ms, client);
}

bool	Channel::kick(Client *client, const std::string &guyToKick, const std::string &reason)
{
	std::string	ms;
	_users_map::iterator	userToKick = _users.find(guyToKick);

	if (_hasRights(client->nick)) {
		if (userToKick == _users.end()) {
			ms = reply_formating(client->servername.c_str(), ERR_USERNOTINCHANNEL, std::vector<std::string>({guyToKick, getName()}), client->nick.c_str());
			return (!custom_send(ms, client));
		}
		ms = ":" + client->nick + "!~a" + client->username + "@";
		ms += client->servername + " KICK " + getName() + " " + guyToKick;
		ms += (reason != "") ? " :" + reason : " :" + client->nick;
		ms += CRLF;
		custom_send(ms, client);
		broadcastMsg(client, ms);
		_users.erase(userToKick);
		return (true);
	}
	ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
	return (!custom_send(ms, client));
}

bool	Channel::invite(Client *client, const std::string &guyToInvite)
{
	std::string	ms;
	_users_map::iterator	userToInvite = _users.find(guyToInvite);
	Client		*clientToInvite = client->getOtherClient(guyToInvite);

	if (!clientToInvite) {
		ms = reply_formating(client->servername.c_str(), ERR_NOSUCHNICK, {guyToInvite}, client->nick.c_str());
		return (!custom_send(ms, client));
	}
	if (!isInChan(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_NOTONCHANNEL, {getName()}, client->nick.c_str());
		return (!custom_send(ms, client));
	}
	if (!_hasRights(client->nick)) {
		ms = reply_formating(client->servername.c_str(), ERR_CHANOPRIVSNEEDED, {getName()}, client->nick.c_str());
		return (!custom_send(ms, client));
	}
	if (userToInvite != _users.end()) { // user is already in channel (no need to invite)
		ms = reply_formating(client->servername.c_str(), ERR_USERONCHANNEL, std::vector<std::string>({guyToInvite, getName()}), client->nick.c_str());
		return (!custom_send(ms, client));
	}
	if (!_is_in_list(guyToInvite, _modes.invitation_list))
		_modes.invitation_list.push_back(guyToInvite);
	ms = ":" + client->nick + "!~" + client->username + "@" + client->servername + " INVITE " + guyToInvite + " :" + getName();
	ms += CRLF;
	custom_send(ms, clientToInvite);
	ms = reply_formating(client->servername.c_str(), RPL_INVITING, std::vector<std::string>({guyToInvite, getName()}), client->nick.c_str());
	return (custom_send(ms, client));
}

bool				Channel::isEmpty() const
{
	return (_users.empty());
}

std::string			Channel::parseArg(size_t fromIndex, const std::vector<std::string> &args)
{
	if (args[fromIndex] == "")
		return ("");
	std::string	argToReturn = args[fromIndex].substr(1);
	++fromIndex;
	while (fromIndex < args.size()) {
		argToReturn += " ";
		argToReturn += args[fromIndex++];
	}
	return (argToReturn);
}

bool		Channel::isInChan(const std::string &userName)
{
	_users_map::iterator	user = _users.find(userName);
	return (user != _users.end());
}

bool		Channel::msgErrors(Client *client)
{
	std::string	ms;
	if (_modes.m) { // user need to be chanop OR to be in voice list
		if (_hasRights(client->nick))
			return (false); // he can send the message (he is a chan op)
		if (_is_in_list(client->nick, _modes.v))
			return (false); // he can send the message (he is in voice list)
		ms = reply_formating(client->servername.c_str(), ERR_CANNOTSENDTOCHAN, {getName()}, client->nick.c_str());
		return (custom_send(ms, client));
	}
	return (false); // user can send the message as the chan is not restricted
}
