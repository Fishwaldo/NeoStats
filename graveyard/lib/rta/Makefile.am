noinst_LIBRARIES	= librtadb.a
noinst_HEADERS		=	do_sql.h parse.tab.h rta.h
librtadb_a_SOURCES	= parse.y token.l api.c do_sql.c rtatables.c 
AM_CFLAGS 		= -I../pcre -I../adns -I../curl 
CLEANFILES		= parse.c token.c
include $(top_srcdir)/autotools/rules.mk
