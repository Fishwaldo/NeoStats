lib_LTLIBRARIES		= connectserv.la
connectserv_la_SOURCES	= cs.c cs.h cs_help.c
connectserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= -I../.. -I../../pcre -I../../curl -I../../adns
include $(top_srcdir)/autotools/rules.mk