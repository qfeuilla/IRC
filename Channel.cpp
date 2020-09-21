#include "Channel.hpp"

Channel::Channel(const std::string &name): _name(name) {
	std::cout << "Creating channel: " << _name << "\n\n";
}
Channel::~Channel() {
	_users_map::iterator	current = _users.begin();
	_users_map::iterator	end = _users.end();

	while (current != end) {
		delete (*current).second;
		++current;
	}
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

bool				Channel::join(socket_t socket)
{
	bool	ret;

	std::pair<socket_t, _User_modes*>	new_el(socket, new _User_modes());
	ret = _users.insert(new_el).second;

	// * debug only
	_print_channel();
	return (ret);
}
bool				Channel::leave(socket_t socket)
{
	_users_map::iterator	user = _users.find(socket);
	if (user == _users.end())
		return (_users.empty()); // user was not in the channel
	delete (*user).second;
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
