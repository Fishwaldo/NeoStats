pkglib_LTLIBRARIES	= statserv.la
statserv_la_SOURCES	= database.c GeoIPCity.c global.h md5.c stats.c \
			GeoIPBitReader.c GeoIPCity.h md5.h statserv.c \
			GeoIPBitReader.h htmlstats.c sqlstats.h \
			statserv.h GeoIP.c GeoIP.h ss_help.c tld.c
statserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/ -DGEOIPDATADIR=\"$(datadir)\" \
			-DSYSCONFDIR=\"$(sysconfdir)\"
dist_data_DATA		= GeoIP.dat html/index.tpl
include $(top_srcdir)/autotools/rules.mk