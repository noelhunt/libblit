BINDIR= $(HOME)/bin
LIBDIR= ../../libblit
INSTALL= install
OFILES= buttons.o cache.o cursor.o host.o lib.o lineops.o \
	master.o pad.o paint.o protocol.o scripthit.o

# -DHALFDUP -DTAGS -DSCROLL -DSAMTERM -DBLANCHEDALMOND
DEFS= -DHALFDUP
MACH	= -m32
INCLUDE = -I.. -I$(LIBDIR)
CFLAGS= -mt -g -c -xCC $(MACH) $(INCLUDE) $(DEFS)
LDFLAGS= -g $(MACH)

LIB = $(LIBDIR)/libblit.a
LDLIBS= $(LIB) -lX11

PROG=padsterm

CLEANFILES= $(PROG) $(OFILES) *.core files.cs cscope.out

all:  $(PROG)

$(PROG):  $(OFILES) ${LIB}
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(LDLIBS)

install: $(BINDIR)/$(PROG)

$(BINDIR)/$(PROG): $(PROG)
	rm -f $(BINDIR)/$(PROG)
	$(INSTALL) $(PROG) $(BINDIR)/$(PROG)

NUKEFILES=$(CLEANFILES) $(BINDIR)/$(TARG)

clean:
	rm -f $(CLEANFILES)

buttons.o: buttons.c univ.h ../pads.h
cache.o: cache.c univ.h ../pads.h
cursor.o: cursor.c univ.h ../pads.h
host.o: host.c univ.h ../pads.h
lib.o: lib.c univ.h ../pads.h
lineops.o: lineops.c univ.h ../pads.h
master.o: master.c univ.h ../pads.h
pad.o: pad.c univ.h ../pads.h
paint.o: paint.c univ.h ../pads.h
protocol.o: protocol.c univ.h ../pads.h
scripthit.o: scripthit.c univ.h ../pads.h
