include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= ms.c
OBJECTS= ms.o
TARGET= moraleserv.so
DISTFILES = $(SOURCES) Makefile ChangeLog ms_help.c

all:	ms

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

ms: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: ms
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(distdir)/$$files; \
	done



$(OBJECTS): Makefile
ms.o:		ms.c	ms_help.c
