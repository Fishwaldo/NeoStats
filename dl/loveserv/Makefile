include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= loveserv.c
OBJECTS= loveserv.o
TARGET= loveserv.so
DISTFILES = $(SOURCES) ls_help.c Makefile ChangeLog 

all:	loveserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

loveserv: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: loveserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(distdir)/$$files; \
	done



$(OBJECTS): Makefile
loveserv.o:		loveserv.c	ls_help.c
