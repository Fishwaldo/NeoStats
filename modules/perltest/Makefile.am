pkglib_LTLIBRARIES	= perlexttest.la
perlexttest_la_SOURCES	= main.c 
perlexttest_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ @PERL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
