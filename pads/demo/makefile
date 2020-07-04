CC=CC
CCC=cc
ARCH= -m32

LIB=../host/libpads.a
PADSTERM= "../termc/padsterm"

DEFINES= -DPADSTERM='$(PADSTERM)'
CFLAGS=-I. -I.. -g $(ARCH) $(DEFINES)

TARGETS= null now news misc stats istat

all: $(TARGETS)

null      :  ${LIB} null.o        ;  $(CC) $(ARCH) null.o -o $@ ${LIB}
now       :  ${LIB} now.o         ;  $(CC) $(ARCH) now.o -o $@ ${LIB}
news      :  ${LIB} news.o        ;  $(CC) $(ARCH) news.o -o $@ ${LDLIB} ${LIB}
misc      :  ${LIB} misc.o        ;  $(CC) $(ARCH) misc.o -o $@ ${LIB}
stats     :  ${LIB} stats.o       ;  $(CC) $(ARCH) stats.o -o $@ ${LIB}
istat     :  ${LIB} istat.o       ;  $(CC) $(ARCH) istat.o -o $@ ${LIB}

clean: ; rm -f $(TARGETS) *.o a.out core

${LIB}: frc ; cd ../host; $(MAKE)

install:

frc:
