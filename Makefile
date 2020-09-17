
SRCS = 	main.cpp \
		Client.cpp \
		Environment.cpp \
		error_handling.cpp \
		Fd.cpp \
		Server.cpp

OBJS = ${SRCS:.cpp=.o}

NAME = bircd

CFLAGS = -I. -g3 -Wall -Werror
LDFLAGS = 

CC = clang++
RM = rm -f

${NAME}:	${OBJS}
		${CC} -o ${NAME} ${OBJS} ${LDFLAGS}

all:		${NAME}

clean:
		${RM} ${OBJS} *~ #*#

fclean:		clean
		${RM} ${NAME}

re:		fclean all
