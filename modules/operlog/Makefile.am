pkglib_LTLIBRARIES	= operlog.la
operlog_la_SOURCES	= main.c help.c operlog.h
operlog_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk
