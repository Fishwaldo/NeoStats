include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= $(extauth_src) 
OBJECTS= $(extauth_objs)
TARGET= extauth.so

all:	statserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

statserv: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: statserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
serviceroots.o:		serviceroots.c
