pkglib_LTLIBRARIES	= statserv.la
statserv_la_SOURCES	= database.c stats.c statserv.c htmlstats.c \
			statsrta.c statsrta.h statserv.h ss_help.c tld.c \
			geoip/GeoIPCity.c geoip/GeoIPCity.h \
			geoip/GeoIPBitReader.c geoip/GeoIPBitReader.h \
			geoip/GeoIP.c geoip/GeoIP.h \
			geoip/global.h geoip/md5.c geoip/md5.h 
statserv_la_LDFLAGS	= -module -avoid-version -no-undefined
AM_CFLAGS		= @PCRE_CFLAGS@ @CURL_CFLAGS@ -I$(top_srcdir)/lib/keeper/ -I$(top_srcdir)/lib/adns/ -Igeoip -DGEOIPDATADIR=\"$(datadir)\" \
			-DSYSCONFDIR=\"$(sysconfdir)\"
dist_data_DATA		= GeoIP.dat html/index.tpl
include $(top_srcdir)/autotools/rules.mk