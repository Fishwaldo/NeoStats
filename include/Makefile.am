noinst_SCRIPTS=version.sh
BUILT_SOURCES=version.h
CLEANFILES = version.h

include_HEADERS		=	neostats.h config.h version.h \
						../lib/prce/pcre.h ../lib/adns/adns.h \
						list.h hash.h support.h events.h numeric.h

version.h: version.sh
	@(if test -f version.sh ; then $(SHELL) version.sh; else echo > version.h; fi)

