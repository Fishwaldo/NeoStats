include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= $(extauth_src) 
OBJECTS= $(extauth_objs)
TARGET= extauth.so

all:	extauth

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

extauth: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: extauth
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
serviceroots.o:		serviceroots.c
