pkglib_LTLIBRARIES	= quoteserv.la
quoteserv_la_SOURCES	= main.c quoteserv.h help.c
quoteserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@
dist_data_DATA		= data/*
include $(top_srcdir)/autotools/rules.mk