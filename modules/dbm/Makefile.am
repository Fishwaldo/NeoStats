SUBDIRS		= bdb gdbm

bin_PROGRAMS 	= convertdb 

convertdb_SOURCES = convertdb.c ../../src/dl.c ../../src/hash.c ../../src/list.c ../../src/ircstring.c ../../src/nsmemory.c ../../src/osfile.c ../../src/support.c
convertdb_LDFLAGS = --export-dynamic @RDYNAMIC_FLAG@
AM_CFLAGS         = @PCRE_CFLAGS@ @CURL_CFLAGS@

include $(top_srcdir)/autotools/rules.mk

