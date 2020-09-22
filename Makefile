
SRCS = 	main.cpp \
		Client.cpp \
		Environment.cpp \
		error_handling.cpp \
		Fd.cpp \
		Server.cpp \
		parser.cpp \
		Command.cpp \
		replies.cpp \
		checking.cpp

OBJS = ${SRCS:.cpp=.o}

NAME = bircd

CPPFLAGS = -I. -g3 -Wall -Werror -Wextra
LDFLAGS =

CC = clang++
RM = rm -f

.cpp.o:
	${CC} ${CPPFLAGS} -c $< -o ${<:.cpp=.o}

${NAME}:	${OBJS}
		${CC} -o ${NAME} ${OBJS} ${LDFLAGS}

all:		${NAME}

clean:
		${RM} ${OBJS} *~ #*#

fclean:		clean
		${RM} ${NAME}

re:		fclean all
