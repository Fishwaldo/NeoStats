pkglib_LTLIBRARIES	= moraleserv.la
moraleserv_la_SOURCES	= ms.c ms.h ms_help.c
moraleserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/
include $(top_srcdir)/autotools/rules.mk