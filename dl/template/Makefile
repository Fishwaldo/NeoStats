include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SOURCES= template.c
OBJECTS= template.o
TARGET= template.so
DISTFILES= $(SOURCES) Makefile

all:	template

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

template: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: template
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(distdir)/$$files; \
	done



$(OBJECTS): Makefile
template.o:		template.c	
