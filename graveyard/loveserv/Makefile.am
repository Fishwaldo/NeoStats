pkglib_LTLIBRARIES	= loveserv.la
loveserv_la_SOURCES	= loveserv.c loveserv.h ls_help.c
loveserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/
include $(top_srcdir)/autotools/rules.mk