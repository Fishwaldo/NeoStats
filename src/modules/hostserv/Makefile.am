pkglib_LTLIBRARIES	= hostserv.la
hostserv_la_SOURCES	= hostserv.c hostserv.h hs_help.c
hostserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= -I../.. -I../../pcre -I../../curl -I../../adns
include $(top_srcdir)/autotools/rules.mk