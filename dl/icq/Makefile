include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= icq.c icqlib.c
OBJECTS= icq.o icqlib.o
TARGET= icqserv.so

all:	icqserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

icqserv: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: icqserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
icq.o:		icq.c		icq_help.c	icq.h	icqlib.h
icqlib.o:	icqlib.c	icqlib.h