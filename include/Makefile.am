noinst_SCRIPTS=version.sh
BUILT_SOURCES=version.h
CLEANFILES = version.h

version.h: Makefile
	@(if test -f version.sh ; then $(SHELL) version.sh; else echo > version.h; fi)
	