#include "Channel.hpp"

Channel::Channel(): _name(), _users(), _modes(), _topic() {}
Channel::Channel(const std::string &name, std::string nick, socket_t socket): _name(name), _users(), _modes(), _topic() {
	_modes.o.push_back(nick);
	std::cout << "Creating channel: " << _name << "\n\n";
	join(nick, socket, "");
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
const std::string	&Channel::getTopic() const
{
	return (_topic);
}

bool				Channel::setTopic(std::string nick, socket_t socket, const std::string &newTopic)
{
	if (!_is_in_chan(nick))
		return (!sendMsgToSocket(socket, "You are not in this chan\n"));
	std::cout << "T mode = " << _modes.t << "\n\n";
	if (_modes.t && !_hasRights(nick))
		return (!sendMsgToSocket(socket, "Topic can only be set by an operator\n"));
	_topic = std::string(newTopic);
	sendMsgToSocket(socket, std::string("Topic set to: ") + newTopic + "\n");
	broadcastMsg(nick, socket, std::string("Topic set to: ") + newTopic + "\n");
	return (true);
}

bool				Channel::sendMsgToSocket(socket_t socket, const std::string &msg)
{
	send(socket, msg.c_str(), msg.length(), 0);
	return (true);
}

bool				Channel::sendMsgToUser(const std::string &userName, const std::string &msg)
{
	_users_map::iterator	user = _users.find(userName);

	if (user == _users.end())
		return (false);
	return (sendMsgToSocket((*user).second, msg));
}

bool				Channel::broadcastMsg(const std::string &sender, socket_t socket, const std::string &msg)
{
	if (!_is_in_chan(sender))
		return (!sendMsgToSocket(socket, "You are not on this channel\n"));
	_users_map::iterator	current = _users.begin();
	_users_map::iterator	end = _users.end();
	while (current != end) {
		if ((*current).first != sender) { // * send msg to everyone but the sender
			sendMsgToSocket((*current).second, msg);
		}
		++current;
	}
	return (true);
}

bool				Channel::join(std::string nick, socket_t socket, const std::string &passwd)
{
	if (!_is_in_chan(nick)) {
		if (_modes.k != passwd) {
			sendMsgToSocket(socket, "bad password\n");
			return (false);
		}
		if (_modes.l != -1 && _modes.l >= _modes.users) {
			sendMsgToSocket(socket, "channel is full\n");
			return (false);
		}
		if (_modes.i && !_is_in_list(nick, _modes.invitation_list)) {
			sendMsgToSocket(socket, "channel is invitation only\n");
			return (false);
		}
		_users.insert(std::pair<std::string, socket_t>(nick, socket));
		_modes.invitation_list.remove(nick);
		sendMsgToSocket(socket, "Channel joined\n");
		// * debug only
		_print_channel();
		return (true);
	}
	return (!sendMsgToSocket(socket, "You are already in this channel\n"));
}

// TODO broadcast leave msg (reason param)
bool				Channel::leave(std::string nick, socket_t socket, const std::string &reason, bool kicked)
{
	(void)reason;
	_users_map::iterator	user = _users.find(nick);
	if (user == _users.end()) // user was not in the channel
		return (!sendMsgToSocket(socket, nick + std::string(" is not is the channel ") + getName() + "\n"));
	if (kicked)
		sendMsgToSocket((*user).second, "You were kicked from the channel\n");
	else
		sendMsgToSocket((*user).second, "Channel left\n");
	_users.erase(user);
	// * debug only
	_print_channel();
	return (true);
}

// errors
std::string Channel::badName(const std::string &name, const std::string &reason) {
	std::string	errorMsg = "Bad channel name: ";
	errorMsg += reason;
	errorMsg += ": ";
	errorMsg += name;
	errorMsg += '\n';
	return (errorMsg);
}

// modes
bool	Channel::mode_o(bool append, std::string nick, socket_t socket, const std::string &target)
{
	(void)socket;
	if (append && _hasRights(nick) && _is_in_chan(target) && !_is_in_list(target, _modes.o)) {
		sendMsgToSocket(socket, "+o succes\n");
		_modes.o.push_back(target);
		return (true);
	}

	if (!append && _hasRights(nick) && _is_in_chan(target) && _is_in_list(target, _modes.o)) {
		sendMsgToSocket(socket, "-o succes\n");
		_modes.o.remove(target);
		return (true);
	}
	
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_v(bool append, std::string nick, socket_t socket, const std::string &target)
{
	(void)socket;
	if (append && _hasRights(nick) && _is_in_chan(target) && !_is_in_list(target, _modes.v)) {
		sendMsgToSocket(socket, "+v succes\n");
		_modes.v.push_back(target);
		return (true);
	}

	if (!append && _hasRights(nick) && _is_in_chan(target) && _is_in_list(target, _modes.v)) {
		sendMsgToSocket(socket, "-v succes\n");
		_modes.v.remove(target);
		return (true);
	}
	
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_p(bool append, std::string nick, socket_t socket)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		sendMsgToSocket(socket, "+p succes\n");
		_modes.p = true;
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, "-p succes\n");
		_modes.p = false;
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_s(bool append, std::string nick, socket_t socket)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		sendMsgToSocket(socket, "+s succes\n");
		_modes.s = true;
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, "-s succes\n");
		_modes.s = false;
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_i(bool append, std::string nick, socket_t socket)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		sendMsgToSocket(socket, "+i succes\n");
		_modes.i = true;
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, "-i succes\n");
		_modes.i = false;
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_t(bool append, std::string nick, socket_t socket)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		sendMsgToSocket(socket, "+t succes\n");
		_modes.t = true;
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, "-t succes\n");
		_modes.t = false;
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_m(bool append, std::string nick, socket_t socket)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		sendMsgToSocket(socket, "+m succes\n");
		_modes.m = true;
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, "-m succes\n");
		_modes.m = false;
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_l(bool append, std::string nick, socket_t socket, int limit)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		_modes.l = limit;
		sendMsgToSocket(socket, std::string("limit set to ") + std::to_string(limit) + "\n");
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, std::string("limit unset") + "\n");
		_modes.l = -1;
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::mode_k(bool append, std::string nick, socket_t socket, const std::string &passwd)
{
	(void)socket;
	if (append && _hasRights(nick)) {
		sendMsgToSocket(socket, std::string("password set to ") + std::string(passwd) + "\n");
		_modes.k = passwd;
		return (true);
	}
	if (!append && _hasRights(nick)) {
		sendMsgToSocket(socket, std::string("password unset ") + "\n");
		_modes.k = "";
		return (true);
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool	Channel::kick(std::string nick, socket_t socket, const std::string &guyToKick, const std::string &reason)
{
	if (_hasRights(nick)) {
		if (!_is_in_chan(guyToKick))
			return (!sendMsgToSocket(socket, guyToKick + " is not in chan\n"));
		return (leave(guyToKick, socket, reason, true));
	}
	return (false);
}

bool	Channel::invite(std::string nick, socket_t socket, const std::string &guyToInvite)
{
	if (_hasRights(nick)) {
		if (_is_in_chan(guyToInvite))
			return (!sendMsgToSocket(socket, guyToInvite + " is already in chan\n"));
		if (_is_in_list(guyToInvite, _modes.invitation_list))
			return (!sendMsgToSocket(socket, guyToInvite + " is already in the invitation list\n"));
		_modes.invitation_list.push_back(guyToInvite);
		sendMsgToUser(guyToInvite, std::string("You are invited in the ") + getName() + " channel\n");
		return (sendMsgToSocket(socket, "invitation sent\n"));
	}
	return (!sendMsgToSocket(socket, "You need to be chan op for this\n"));
}

bool				Channel::isEmpty() const
{
	return (_users.empty());
}
