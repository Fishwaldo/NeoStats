include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= template.c
OBJS= ${SRCS:.c=.o}
TARGET= template.so
DISTFILES= $(SRCS) Makefile

all:	template

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

template: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: template
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
template.o:		template.c	
