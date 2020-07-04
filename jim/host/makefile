CFLAGS=-g -I../../libblit
CC=cc
SRCS:sh=	ls *.c
OBJS=		${SRCS:.c=.o}
CLEANFILES=	jim ${OBJS} core

jim: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f $(CLEANFILES)
