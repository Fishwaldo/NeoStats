pkglib_LTLIBRARIES	= bahamut14.la bahamut18.la client.la hybrid7.la \
						ircup10.la plexus.la \
						ultimate2.la ultimate3.la unreal31.la unreal32.la
bahamut14_la_SOURCES = bahamut14.c bahamut14.h
bahamut18_la_SOURCES = bahamut18.c bahamut18.h
client_la_SOURCES = client.c client.h
hybrid7_la_SOURCES = hybrid7.c hybrid7.h
ircup10_la_SOURCES = ircup10.c ircup10.h
plexus_la_SOURCES = plexus.c plexus.h
ultimate2_la_SOURCES = ultimate2.c ultimate2.h
ultimate3_la_SOURCES = ultimate3.c ultimate3.h
unreal31_la_SOURCES = unreal31.c unreal31.h
unreal32_la_SOURCES = unreal32.c unreal32.h
AM_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= -I$(top_srcdir)/lib/adns/ -I$(top_srcdir)/lib/keeper @PCRE_CFLAGS@ @CURL_CFLAGS@
include $(top_srcdir)/autotools/rules.mk