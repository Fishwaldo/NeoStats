pkglib_LTLIBRARIES	= templateauth.la
templateauth_la_SOURCES	= templateauth.c 
templateauth_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/
include $(top_srcdir)/autotools/rules.mk