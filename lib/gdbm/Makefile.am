lib_LTLIBRARIES		= libgdbm.la libgdbm_compat.la
libgdbm_la_SOURCES	=  gdbmopen.c gdbmdelete.c gdbmfetch.c  gdbmstore.c gdbmclose.c \
	gdbmreorg.c gdbmseq.c gdbmsync.c gdbmerrno.c gdbmexists.c gdbmfdesc.c \
	gdbmsetopt.c bucket.c falloc.c findkey.c global.c hash.c update.c \
	version.c

libgdbm_compat_la_SOURCES = dbminit.c delete.c fetch.c store.c seq.c close.c \
	dbmopen.c dbmdelete.c dbmfetch.c dbmstore.c dbmseq.c \
	dbmclose.c dbmdirfno.c dbmpagfno.c dbmrdonly.c

include $(top_srcdir)/autotools/rules.mk
