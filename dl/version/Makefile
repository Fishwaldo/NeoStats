include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= version.c
OBJECTS= version.o
TARGET= version.so

all:	version

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

version: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: version
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
version.o:		version.c	
