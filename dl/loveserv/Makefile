# Generated automatically from Makefile.in by configure.
# Generated automatically from Makefile.in by configure.
INCLUDES = -I../..

SOURCES= loveserv.c
OBJECTS= loveserv.lo
TARGET= loveserv.so

CC=gcc
CFLAGS= -O2 -Wall -fPIC
LINKER=ld

all:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OBJECTS) -c $(SOURCES)
	$(LINKER) -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET) Makefile
	/bin/rm -rf *.o *.lo *.so
