SRCS = 	main.cpp \
		Client.cpp \
		Environment.cpp \
		error_handling.cpp \
		Fd.cpp \
		Server.cpp \
		parser.cpp \
		Command.cpp \
		replies.cpp \
		checking.cpp \
		Channel.cpp \
		ChannelMaster.cpp \
		utils.cpp \
		OtherServ.cpp

OBJS := $(patsubst %.cpp,%.o,$(SRCS))
DEPENDS := $(patsubst %.cpp,%.d,$(SRCS))

# c++ flags
FLAGS = -Wall -Wextra -Werror 

# Compiler
CC = g++

# Output name
NAME = bircd

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

# .PHONY means these rules get executed even if files of those names exist.
.PHONY: all clean


# * more info here: https://stackoverflow.com/questions/52034997/how-to-make-makefile-recompile-when-a-header-file-is-changed