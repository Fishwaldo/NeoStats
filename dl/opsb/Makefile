include ../../Makefile.inc

INCLUDES = -I../.. -Iares/

SOURCES= opsb.c
OBJECTS= opsb.o libares.a
TARGET= opsb.so

all:	opsb ares

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

libares.a:	
	(cd ares; $(MAKE) $@)

opsb: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) ares/libares.a $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so *.a
	(cd ares; $(MAKE) $@)

install: spam
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
opsb.o:		opsb.c	
