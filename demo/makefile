TERM    = blit
OPTIONS = -g -Xa $(ARCH)
DEFINES	= -DPOLL -DFREETYPE2
INCLUDE = -I../lib$(TERM) -I$(HOME)/include -I/usr/include/freetype2

MACH	= -m32	# default on Solaris
CFLAGS  = $(DEFINES) $(OPTIONS) $(INCLUDE)
LIB = ../lib$(TERM)/libblit.a
LIBS = ${LIB} -lXext -lX11
TESTS = t0 t1 t2 t3 t4 t5 t6
CLEANFILES = a.out core *.o null $(TESTS)

all:	$(TESTS)

t%:     t%.c $(LIB)
	$(CC) -o $@ $(CFLAGS) $< $(LIBS)

clean:
	rm -f $(CLEANFILES)

install:
