noinst_LTLIBRARIES	= libpcre.la
noinst_PROGRAMS		= dftables
libpcre_la_SOURCES	= chartables.c get.c pcre.c study.c internal.h pcre.h
libpcre_la_LDFLAGS	= -static
dftables_SOURCES	= dftables.c internal.h pcre.h
dftables_DEPENDENCES 	= maketables.c


AM_CFLAGS 		= -I../../include -I.. -I../pcre -I../curl

chartables.c: dftables
	$(top_builddir)/src/pcre/dftables > $@

include $(top_srcdir)/autotools/rules.mk
