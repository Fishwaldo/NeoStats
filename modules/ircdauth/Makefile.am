pkglib_LTLIBRARIES	= ircdauth.la
ircdauth_la_SOURCES	= ircdauth.c 
ircdauth_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/
include $(top_srcdir)/autotools/rules.mk