pkglib_LTLIBRARIES	= textserv.la
textserv_la_SOURCES	= main.c textserv.h help.c
textserv_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@

tsdbdir			= $(datadir)/TSDB/
dist_tsdb_DATA		= data/*
include $(top_srcdir)/autotools/rules.mk
