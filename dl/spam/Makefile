include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= spam.c
OBJECTS= spam.o
TARGET= spam.so

all:	spam

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

spam: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: spam
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
spam.o:		spam.c	
