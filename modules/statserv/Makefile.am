pkglib_LTLIBRARIES	= statserv.la
statserv_la_SOURCES	= database.c stats.c statserv.c htmlstats.c \
			sqlstats.h statserv.h ss_help.c tld.c \
			GeoIP/GeoIPCity.c GeoIP/GeoIPCity.h \
			GeoIP/GeoIPBitReader.c GeoIP/GeoIPBitReader.h \
			GeoIP/GeoIP.c GeoIP/GeoIP.h \
			GeoIP/global.h GeoIP/md5.c GeoIP/md5.h 
statserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/ -Igeoip -DGEOIPDATADIR=\"$(datadir)\" \
			-DSYSCONFDIR=\"$(sysconfdir)\"
dist_data_DATA		= GeoIP.dat html/index.tpl
include $(top_srcdir)/autotools/rules.mk