include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= statserv.c ss_help.c database.c tld.c stats.c htmlstats.c
OBJECTS= statserv.o ss_help.o database.o tld.o stats.o htmlstats.o
TARGET= statserv.so
HTML= html/index.tpl

all:	statserv

.c.o:
	$(CC) -c $< $(CFLAGS) $(INCLUDES) 
	$(CC) -c $< $(CFLAGS) $(INCLUDES)

statserv: $(OBJECTS)
	ld $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: statserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl
	$(INSTALL) -m 644 $(HTML) $(DIRECTORY)/data



$(OBJECTS): Makefile
statserv.o:		statserv.c	ss_help.c	statserv.h 
ss_help.o:	ss_help.c	statserv.h
database.o:	database.c	statserv.h
tld.o:		tld.c	statserv.h
stats.o:	stats.c	statserv.h	m_stats.h
htmlstats.o:	htmlstats.c	statserv.h
