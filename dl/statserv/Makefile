include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= statserv.c ss_help.c
OBJECTS= statserv.o ss_help.o
TARGET= statserv.so

all:	statserv

.c.o:
	$(CC) -c $< $(CFLAGS) $(INCLUDES) 2>> make.log

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
