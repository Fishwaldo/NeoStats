include ../../Makefile.inc

INCLUDES = -I../.. -Iares/ -I../../adns

SOURCES= opsb.c proxy.c opsb_help.c
OBJECTS= opsb.o proxy.o opsb_help.o
TARGET= opsb.so

all:	opsb

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

opsb: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so *.a
	(cd ares; $(MAKE) $@)

install: spam
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
opsb.o:		opsb.c		opsb.h
proxy.o:	proxy.c		opsb.h
