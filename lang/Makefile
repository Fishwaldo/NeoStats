# Makefile for language module

#include ../Makefile.inc


LANGOBJS = en
LANGSRCS = $(LANGOBJS:%=%.l)

LANGCOMP = ./langcomp
#LANGCOMP = ./langcomp -w


all: $(LANGOBJS)


install: all
	mkdir -p "$(DATDEST)/languages"
ifdef RUNGROUP
	chgrp "$(RUNGROUP)" "$(DATDEST)/languages"
	chmod 770 "$(DATDEST)/languages"
else
	chmod 700 "$(DATDEST)/languages"
endif
	cp $(LANGOBJS) "$(DATDEST)/languages"
ifdef RUNGROUP
	chgrp $(RUNGROUP) "$(DATDEST)/languages/"*
	chmod 660 "$(DATDEST)/languages/"*
else
	chmod 600 "$(DATDEST)/languages/"*
endif


clean:
	rm -f $(LANGOBJS) langcomp

spotless: clean
	rm -f langstrs.h


$(LANGOBJS): %: %.l langcomp index langstrs.h
	$(LANGCOMP) $<

langcomp: langcomp.c
	$(CC) $(CFLAGS) langcomp.c -o $@


langstrs.h: index Makefile
	@perl -e <index >$@.new '\
		print STDERR "Generating langstrs.h... "; \
		$$i=0; \
		while (<>) { \
			chop; \
			printf "#define %-32s %d\n", $$_, $$i++; \
		} \
		print "\n#define NUM_STRINGS $$i\n"; \
		print STDERR "$$i strings\n";'
	@if cmp $@ $@.new >/dev/null 2>&1 ; then \
		echo "$@ unchanged" ; \
		rm -f $@.new ; \
	else \
		mv -f $@.new $@ ; \
	fi

index: en.l
	grep '^[A-Z]' <en.l >$@
