pkglib_LTLIBRARIES	= gdbm.la 
gdbm_la_SOURCES = gdbm.c \
	../../../lib/gdbm/gdbmopen.c ../../../lib/gdbm/gdbmdelete.c \
	../../../lib/gdbm/gdbmfetch.c ../../../lib/gdbm/gdbmstore.c \
	../../../lib/gdbm/gdbmclose.c ../../../lib/gdbm/gdbmreorg.c \
	../../../lib/gdbm/gdbmseq.c ../../../lib/gdbm/gdbmsync.c \
	../../../lib/gdbm/gdbmerrno.c ../../../lib/gdbm/gdbmexists.c \
	../../../lib/gdbm/gdbmfdesc.c ../../../lib/gdbm/gdbmsetopt.c \
	../../../lib/gdbm/bucket.c ../../../lib/gdbm/falloc.c \
	../../../lib/gdbm/findkey.c ../../../lib/gdbm/global.c \
	../../../lib/gdbm/hash.c ../../../lib/gdbm/update.c \
	../../../lib/gdbm/version.c 


AM_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= -I$(top_srcdir)/lib/adns/ -I$(top_srcdir)/lib/gdbm/ @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk