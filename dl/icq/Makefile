INCLUDES = -I../..

SOURCES= icq.c
OBJECTS= icq.lo
TARGET= icq.so

CC=gcc
CFLAGS=-O2 -Wall -ggdb -fPIC
LINKER=ld

all:
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OBJECTS) -c $(SOURCES)
	$(LINKER) -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so