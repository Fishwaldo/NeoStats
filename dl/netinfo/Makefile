include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= netinfo.c
OBJECTS= netinfo.o
TARGET= netinfo.so

all:	netinfo

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

netinfo: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: netinfo
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
netinfo.o:		netinfo.c	ni_help.c
