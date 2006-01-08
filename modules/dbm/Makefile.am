SUBDIRS		= bdb gdbm

bin_PROGRAMS 	= convertdb 

convertdb_SOURCES = convertdb.c
AM_CFLAGS         = @PCRE_CFLAGS@ @CURL_CFLAGS@

include $(top_srcdir)/autotools/rules.mk

