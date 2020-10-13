SRCS = 	srcs/main.cpp \
		srcs/Client.cpp \
		srcs/Environment.cpp \
		srcs/error_handling.cpp \
		srcs/Fd.cpp \
		srcs/Server.cpp \
		srcs/parser.cpp \
		srcs/Command.cpp \
		srcs/replies.cpp \
		srcs/checking.cpp \
		srcs/Channel.cpp \
		srcs/ChannelMaster.cpp \
		srcs/utils.cpp \
		srcs/OtherServ.cpp

OBJS := $(patsubst %.cpp,%.o,$(SRCS))
DEPENDS := $(patsubst %.cpp,%.d,$(SRCS))

# c++ flags
FLAGS = -Wall -Wextra -Werror

# Compiler
CC = g++

# Output name
NAME = ircserv

# Default rule (so make, make all, and make $(NAME) are the same)
all: $(NAME)

# Linking the executable ($@ = target) from the object files (here $^ = .o files)
$(NAME): $(OBJS)
	$(CC) $(FLAGS) $^ -o $@ -L/usr/local/ssl/lib -lssl -lcrypto

-include $(DEPENDS)

# generate .d files (dependencies)
%.o: %.cpp Makefile
	$(CC) $(FLAGS) -MMD -MP -c $< -o $@

clean:
	$(RM) $(OBJS) $(DEPENDS)

fclean:		clean
	$(RM) $(NAME)

re:		fclean all

install:
	sudo apt-get update && sudo apt-get install libssl-dev hexchat ngircd

conf:
	cp servlist.conf ~/.config/hexchat/

# .PHONY means these rules get executed even if files of those names exist.
.PHONY: all clean


# * more info here: https://stackoverflow.com/questions/52034997/how-to-make-makefile-recompile-when-a-header-file-is-changed