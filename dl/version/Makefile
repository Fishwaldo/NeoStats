include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= version.c
OBJECTS= version.o
TARGET= version.so
DISTFILES= $(SOURCES) Makefile 

all:	version

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

version: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: version
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(distdir)/$$files; \
	done



$(OBJECTS): Makefile
version.o:		version.c	
