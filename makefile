SUBDIRS= libblit pads demo

all clean install:
	for d in $(SUBDIRS); do \
		(cd $$d; echo Making $@ in $$d; $(MAKE) $@); \
	done
