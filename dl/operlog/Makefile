include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= operlog.c
OBJECTS= operlog.o
TARGET= operlog.so

all:	operlog

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

operlog: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: operlog
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
operlog.o:		operlog.c	ol_help.c
