pkglib_LTLIBRARIES	= perlexttest.la
perlexttest_la_SOURCES	= main.c 
perlexttest_la_LDFLAGS	= -module -avoid-version
AM_CFLAGS		= -I$(top_srcdir)/include @PCRE_CFLAGS@ @CURL_CFLAGS@ @PERL_CFLAGS@

extra_DIST		= extension.ple test.pl

include $(top_srcdir)/autotools/rules.mk
