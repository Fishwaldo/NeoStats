include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= operme.c
OBJECTS= operme.o
TARGET= operme.so

all:	operme

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

operme: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: cs
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
operme.o:		operme.c	om_help.c
