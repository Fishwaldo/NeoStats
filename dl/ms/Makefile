include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= ms.c
OBJS= ${SRCS:.c=.o}
TARGET= moraleserv.so
DISTFILES = $(SRCS) Makefile ChangeLog ms_help.c

all:	ms

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

ms: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: ms
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
ms.o:		ms.c	ms_help.c
