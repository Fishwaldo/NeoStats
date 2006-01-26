pkglib_LTLIBRARIES	= nstest.la
nstest_la_SOURCES	= main.c 
nstest_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
