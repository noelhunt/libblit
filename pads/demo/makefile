CC=CC
CCC=cc

LIB=../host/libpads.a
PADSTERM= "../termc/padsterm"

DEFINES= -DPADSTERM='$(PADSTERM)'
CFLAGS=-I. -I.. -g $(DEFINES)

TARGETS= null now news misc stats istat

all: $(TARGETS)

null      :  ${LIB} null.o        ;  $(CC) null.o -o $@ ${LIB}
now       :  ${LIB} now.o         ;  $(CC) now.o -o $@ ${LIB}
news      :  ${LIB} news.o        ;  $(CC) news.o -o $@ ${LDLIB} ${LIB}
misc      :  ${LIB} misc.o        ;  $(CC) misc.o -o $@ ${LIB}
stats     :  ${LIB} stats.o       ;  $(CC) stats.o -o $@ ${LIB}
istat     :  ${LIB} istat.o       ;  $(CC) istat.o -o $@ ${LIB}

clean: ; rm -f $(TARGETS) *.o a.out core

${LIB}: frc ; cd ../host; $(MAKE)

frc:
