pkglib_LTLIBRARIES	= bdb.la
bdb_la_SOURCES 		= bdb.c
bdb_la_LIBADD		= @BDB_LIBS@

AM_LDFLAGS		= -module -avoid-version @BDB_LDFLAGS@ 
AM_CFLAGS		= -I$(top_srcdir)/include @BDB_CPPFLAGS@ @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
