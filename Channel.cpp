#include "Channel.hpp"

Channel::Channel(): _name(), _users(), _modes() {}
Channel::Channel(const std::string &name, std::string nick, socket_t socket): _name(name), _users(), _modes() {
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

void				Channel::sendMsgToSocket(int socket, const std::string &msg)
{
	send(socket, msg.c_str(), msg.length(), 0);
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
		// * debug only
		_print_channel();
		return (true);
	}
	return (false);
}
bool				Channel::leave(std::string nick)
{
	_users_map::iterator	user = _users.find(nick);
	if (user == _users.end())
		return (_users.empty()); // user was not in the channel
	_users.erase(user);
	// * debug only
	_print_channel();
	return (_users.empty());
}

// exceptions
Channel::badName::badName(const std::string &name, const std::string &reason) : _name(name), _reason(reason) {
	_errorMsg = "Bad channel name: ";
	_errorMsg += _reason;
	_errorMsg += ": ";
	_errorMsg += _name;
	_errorMsg += '\n';
}
const char*	Channel::badName::what() const throw()
{
	return (_errorMsg.c_str());
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
	
	return (false);
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
	
	return (false);
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
	return (false);
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
	return (false);
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
	return (false);
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
	return (false);
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
	return (false);
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
	return (false);
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
	return (false);
}
