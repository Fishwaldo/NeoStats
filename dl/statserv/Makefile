include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns -I.

SRCS= statserv.c ss_help.c database.c tld.c stats.c htmlstats.c 
OBJS= ${SRCS:.c=.o}
TARGET= statserv.so
GEOCFLAGS = $(CFLAGS) -DGEOIPDATADIR=\"$(DIRECTORY)/data\" -DSYSCONFDIR=\"$(DIRECTORY)/data/\"
GEOSRCS = GeoIPBitReader.c GeoIP.c GeoIPCity.c md5.c
GEOOBJS = GeoIPBitReader.o GeoIP.o GeoIPCity.o md5.o
HTML= html/index.tpl
DATA= GeoIP.dat
DISTFILES= $(SRCS) $(DATA) $(GEOSRCS) $(HTML) ChangeLog statserv.h m_stats.h Makefile sqlstats.h GeoIPBitReader.h GeoIP.h global.h GeoIPCity.h md5.h   

all:	statserv

.c.o:
	$(CC) -c $< $(GEOCFLAGS) $(INCLUDES) 

statserv: $(OBJS) $(GEOOBJS)
	ld $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS) $(GEOOBJS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: statserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl
	$(INSTALL) -m 644 $(HTML) $(DIRECTORY)/data
	$(INSTALL) -m 644 $(DATA) $(DIRECTORY)/data

dist:	$(DISTFILES)
	@mkdir $(DISTDIR)/html
	@chmod 777 $(DISTDIR)/html
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done


$(OBJS): Makefile
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
statserv.so: statserv.o ss_help.o database.o tld.o stats.o htmlstats.o md5.o GeoIPCity.o GeoIP.o GeoIPBitReader.c