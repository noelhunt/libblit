CFLAGS=-g -I../../libg
CC=cc
AR= /usr/ccs/bin/ar rcs
FILES=deltext.o fralloc.o frameop.o instext.o ptofchar.o
CFILES=deltext.c fralloc.c frameop.c instext.c ptofchar.c 

frame.a:	$(FILES)
	$(AR) libframe.a $(FILES)

print:
	@pr frame.h $(CFILES)

lint:
	lint -I/usr/jerq/include -DMUX $(CFILES) -lj 2>&1 | /usr/rob/bin/buffer

$(FILES):	frame.h

clean:
	rm -f *.o core
