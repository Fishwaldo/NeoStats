pkglib_LTLIBRARIES	= bdb.la gdbm.la 
bdb_la_SOURCES = bdb.c
gdbm_la_SOURCES = gdbm.c
AM_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= -I$(top_srcdir)/lib/adns/ -I$(top_srcdir)/lib/gdbm/ -I$(top_srcdir)/lib/keeper @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk