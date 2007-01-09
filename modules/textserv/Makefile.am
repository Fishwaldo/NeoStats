pkglib_LTLIBRARIES	= textserv.la
textserv_la_SOURCES	= main.c textserv.h help.c
textserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@

tsdbdir			= $(datadir)/TSDB/
dist_tsdb_DATA		= data/barmaid data/icecream data/loveserv data/moraleserv data/waitress
include $(top_srcdir)/autotools/rules.mk
