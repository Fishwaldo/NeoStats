include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= neoserv.c
OBJECTS= neoserv.o
TARGET= neoserv.so

all:	neoserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

neoserv: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: neoserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
neoserv.o:		neoserv.c	neoserv_help.c
