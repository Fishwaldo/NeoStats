lib_LTLIBRARIES		= libadns.la
libadns_la_SOURCES	= adns.h adnscheck.c adnsdlist.h adnsevent.c adnsgeneral.c \
			adnsinternal.h adnsparse.c adnsquery.c adnsreply.c adnssetup.c \
			adnstransmit.c adnstvarith.h adnstypes.c
include_HEADERS		=	adns.h

AM_CFLAGS 	= -I../pcre

include $(top_srcdir)/autotools/rules.mk
