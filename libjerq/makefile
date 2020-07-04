#	Copyright (c) 1992 AT&T - All rights reserved.
#
AR	= /usr/ccs/bin/ar rcs
AR	= ar rcs
DEFINES	= -DPOLL -DFREETYPE2
INCLUDES = -I. -I$(HOME)/include -I/usr/include/freetype2
XLIBS	= $(HOME)/lib/libXft.a -L/usr/lib -lXext -lX11
CC	= cc
MACH	= -m32
CFLAGS	= $(DEFINES) $(INCLUDES) $(MACH) -g
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
