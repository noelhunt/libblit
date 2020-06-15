SUBDIRS= host termb termc demo

all clean install:
	for d in $(SUBDIRS); do \
		(cd $$d; echo Making $@ in $$d; $(MAKE) $@); \
	done
