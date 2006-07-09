noinst_LTLIBRARIES	= libnxml.la libmrss.la
libnxml_la_SOURCES 	= nxml_download.c nxml_easy.c nxml_entity.c nxml_free.c nxml_namespace.c nxml_string.c nxml_utf.c nxml_dtd.c \
       			  nxml_edit.c nxml_error.c nxml_init.c nxml_parser.c nxml_tools.c nxml_write.c
libmrss_la_SOURCES	= mrss_download.c mrss_edit.c mrss_free.c mrss_generic.c mrss_parser.c mrss_write.c
include_HEADERS 	= nxml.h mrss.h
noinst_HEADERS		= nxml_internal.h mrss_internal.h
INCLUDES 		= -I../curl/

EXTRA_DIST = 

include $(top_srcdir)/autotools/rules.mk


