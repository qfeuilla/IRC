# ft_irc
IRC server implementation in cpp, a 42 school project

This project was a group project, our group was: https://github.com/qfeuilla + https://github.com/mle-moni
We finished this project in approx. 3 weeks

HOW TO LAUNCH THE SERVER:

linux: tested with debian, ubuntu, change the apt-get calls to the pacman equivalent to get it to work for ARCH
```zsh
git clone https://github.com/mle-moni/ft_irc
cd ft_irc
make install # install dependencies and an IRC client, you might want to change that in the Makefile
make # build the project
```

after this, you get an executable nammed "ircserv"

`Usage: ./ircserv [host:port:password] <port> <password>`

to launch a simple IRC server waiting for non encrypted connections on port 6667, and 6668 for TLS connections:
```zsh
./ircserv 6667 MySecret
```
replace "MySecret" by whatever you want, it will be usefull to promote an IRC client as an IRC operator

for testing purposes we put a self signed certificate with a password "toto". This password will be asked when you launch the server

You should use your own certificate if you want to use the server with real clients

HOW TO CONNECT A CLIENT TO THE SERVER:

open hexchat or any other IRC client, then configure it to connect to the server
in hexchat: 
hit CTRL+S, then click on "add", you can enter a name for this new server config, then click on "edit", change the address field on top "newserver/6667" by the actual address ("localhost/6667")
then you can close the config page, and click "connect" and VOILA!
