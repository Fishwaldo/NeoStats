lib_LTLIBRARIES		= libadns.la
libadns_la_SOURCES	= adns.h check.c dlist.h event.c general.c internal.h \
			parse.c query.c reply.c setup.c transmit.c tvarith.h \
			types.c
include_HEADERS		=	adns.h

AM_CFLAGS 	= -I../pcre

include $(top_srcdir)/autotools/rules.mk
