SRCS = statserv.c ss_help.c database.c tld.c stats.c htmlstats.c \
       GeoIPBitReader.c GeoIP.c GeoIPCity.c md5.c
TARGET = statserv.so
MODCFLAGS = -DGEOIPDATADIR=\"$(DATADIRECTORY)\" -DSYSCONFDIR=\"$(DATADIRECTORY)\"
DATA = html/index.tpl GeoIP.dat
DISTFILES = $(SRCS) $(DATA) ChangeLog statserv.h m_stats.h Makefile sqlstats.h \
       GeoIPBitReader.h GeoIP.h global.h GeoIPCity.h md5.h
SUBDIRS = html

include ../Makefile.inc

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)
	$(INSTALL_DATA) $(DATA) $(DATADIRECTORY)

