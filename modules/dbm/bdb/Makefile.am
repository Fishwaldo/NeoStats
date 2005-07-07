pkglib_LTLIBRARIES	= bdb.la
bdb_la_SOURCES = bdb.c

AM_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= -I$(top_srcdir)/lib/adns/ @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk