#	Copyright (c) 1992 AT&T - All rights reserved.
#
OS=
INCS=-I. -I/usr/X11R6/include
XLIBS=-L/usr/lib -lXt -lX11
RANLIB=:
AR=ar
LIB=libXg.a
CFLAGS=$(OS) $(INCS) -g
CC=cc

OBJS=	arc.o arith.o balloc.o bitblt.o bitbltclip.o border.o bscreenrect.o\
	circle.o clipline.o clipr.o copymasked.o cursorset.o cursorswitch.o\
	disc.o ellipse.o font.o gcs.o getrect.o gwin.o ldconvert.o latin1.o\
	mkfont.o menuhit.o point.o polysegment.o rdbitmap.o rdbitmapfile.o\
	rdfontfile.o rectclip.o rune.o segment.o string.o strwidth.o texture.o\
	wrbitmap.o wrbitmapfile.o xtbinit.o

all install:	$(LIB)
compile:	$(LIB)
test:	test.o $(LIB)
	$(CC) -o $@ test.o $(LIB) $(XLIBS) -lm
	echo try running test
clean:
	rm -f *.o test

nuke:	clean
	rm -f $(LIB)

$(LIB):	$(OBJS)
	$(AR) rv $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

$(LIB)(%.o): %.o

$(OBJS):	libg.h libgint.h libc.h
