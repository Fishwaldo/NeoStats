SRCS = statserv.c ss_help.c database.c tld.c stats.c htmlstats.c \
       GeoIPBitReader.c GeoIP.c GeoIPCity.c md5.c
TARGET = statserv.so
MODCFLAGS = -DGEOIPDATADIR=\"$(DATADIRECTORY)\" -DSYSCONFDIR=\"$(DATADIRECTORY)\"
DATA = html/index.tpl GeoIP.dat
DISTFILES= $(SRCS) $(DATA) ChangeLog statserv.h m_stats.h Makefile sqlstats.h GeoIPBitReader.h GeoIP.h global.h GeoIPCity.h md5.h
SUBDIRS= html

include ../Makefile.inc

statserv.o:		statserv.c	ss_help.c	statserv.h  sqlstats.h
ss_help.o:	ss_help.c	statserv.h
database.o:	database.c	statserv.h
tld.o:		tld.c	statserv.h
stats.o:	stats.c	statserv.h	m_stats.h
htmlstats.o:	htmlstats.c	statserv.h
md5.o: md5.c global.h md5.h
GeoIPCity.o: GeoIPCity.c
GeoIP.o: GeoIP.c GeoIP.h
GeoIPBitReader.o: GeoIPBitReader.c GeoIPBitReader.h
