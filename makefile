SUBDIRS= libblit libjerq libXg pads demo jim

all clean install:
	for d in $(SUBDIRS); do \
		(cd $$d; echo Making $@ in $$d; $(MAKE) $@); \
	done

trans: trans.c ; $(CC) -g trans.c -o $@ -lXpm -lX11
