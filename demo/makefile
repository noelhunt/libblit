INCLUDE = -I../libblit
OPTIONS = -g -Xa $(ARCH)
DEFINES =
CFLAGS  = $(DEFINES) $(OPTIONS) $(INCLUDE)
LIB = ../libblit/libblit.a
LIBS = $(LIB) -lX11
TESTS = t0 t1 t2 t3 t4 t5 t6
CLEANFILES = a.out core *.o null $(TESTS)

all:	$(TESTS)

t%:     t%.c $(LIB)
	$(CC) -o $@ $(CFLAGS) $< $(LIBS)

clean:
	rm -f $(CLEANFILES)
