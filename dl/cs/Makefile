include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= cs.c
OBJS= ${SRCS:.c=.o}
TARGET= connectserv.so
DISTFILES = $(SRCS) cs.h Makefile ChangeLog cs_help.c

all:	cs

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

cs: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: cs
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist: 	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
cs.o:		cs.c	cs_help.c
