include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= statserv.c ss_help.c database.c tld.c stats.c htmlstats.c
OBJECTS= statserv.o ss_help.o database.o tld.o stats.o htmlstats.o
TARGET= statserv.so

all:	statserv

.c.o:
#	$(CC) -c $< $(CFLAGS) $(INCLUDES) 2>> make.log
	$(CC) -c $< $(CFLAGS) $(INCLUDES)

statserv: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: statserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
statserv.o:		statserv.c	ss_help.c
ss_help.o:	ss_help.c
database.o:	database.c
tld.o:		tld.c
stats.o:	stats.c
htmlstats.o:	htmlstats.c