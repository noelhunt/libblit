SRCS=	alloc.c file.c gcalloc.c jim.c msgs.c pattern.c string.c unix.c
OBJS=	${SRCS:.c=.o}
SNTX=	${SRCS:.c=.O}

CFLAGS=-g

%.O:	%.c
	cyntax -c $<

xjim:&	$(FILES)
	$(CC) $(CFLAGS) $(FILES) -o xjim

cyntax:	$(OFILES)
	cyntax $(OFILES) && touch cyntax

jim.O:	jim.c
	cyntax -c -Vdprintf:1 jim.c

ftest:	ftest.o string.o file.o
	$(CC) -o ftest ftest.o string.o file.o

xljim:	$(FILES)
	lcomp $(CFILES) && cp a.out xljim && rm a.out

$(SRCS):	jim.h
$(OBJS):	jim.h
file.o jim.o pattern.o string.o unix.o:	file.h
jim.o msgs.o:	msgs.h

lint:
	lint -h -u -x $(CFILES) 2>&1 | buffer

install:	xjim
	cp xjim /usr/jerq/bin/jim
	strip -g /usr/jerq/bin/jim
	cd term; make install

clean:
	rm -f xjim *.o
	cd term; make clean
