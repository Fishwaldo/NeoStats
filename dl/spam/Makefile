include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= spam.c
OBJECTS= spam.o
TARGET= spam.so
DISTFILES= $(SOURCES) Makefile

all:	spam

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

spam: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: spam
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(distdir)/$$files; \
	done



$(OBJECTS): Makefile
spam.o:		spam.c	
