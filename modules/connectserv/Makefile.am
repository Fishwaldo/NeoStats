pkglib_LTLIBRARIES	= connectserv.la
connectserv_la_SOURCES	= cs.c cs.h cs_help.c
connectserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/
include $(top_srcdir)/autotools/rules.mk
