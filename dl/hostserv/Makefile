include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= hostserv.c
OBJECTS= hostserv.o
TARGET= hostserv.so

all:	hostserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

hostserv: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET) 
	/bin/rm -rf *.o *.lo *.so

install: hostserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
hostserv.o:		hostserv.c	hs_help.c	
