include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= cs.c
OBJECTS= cs.o
TARGET= connectserv.so
DISTFILES = $(SOURCES) cs.h Makefile ChangeLog cs_help.c

all:	cs

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

cs: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: cs
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist: 	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(distdir)/$$files; \
	done



$(OBJECTS): Makefile
cs.o:		cs.c	cs_help.c
