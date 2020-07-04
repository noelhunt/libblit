#	Copyright (c) 1992 AT&T - All rights reserved.
#
AR	= /usr/ccs/bin/ar rcs
AR	= ar rcs
DEFINES	= -DPOLL
INCLUDES = -I. -I/usr/X11R6/include
XLIBS	= -L/usr/lib -lX11
CC	= cc
MACH	= -m32	# default on Solaris
CFLAGS	= $(DEFINES) $(INCLUDES) -g
LIB	= libblit.a

SRCS:sh= ls *.c

OBJS=	$(SRCS:.c=.o)

all install:	$(LIB)
compile:	$(LIB)
test:	test.o $(LIB)
	$(CC) -o $@ test.o $(LIB) $(XLIBS) -lm
	echo try running test
clean:
	rm -f *.o $(LIB) test

nuke:	clean
	rm -f $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(LIB) $(OBJS)

$(LIB)(%.o): %.o

$(OBJS):	blit.h libgint.h
