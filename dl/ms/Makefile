include ../../Makefile.inc

SRCS= ms.c ms_help.c
OBJS= ${SRCS:.c=.o}
TARGET= moraleserv.so
DISTFILES = $(SRCS) ms.h Makefile ChangeLog

all:	ms

.c.o:
	$(CC) $(CFLAGS) $(MODINCLUDES) -c $<

ms: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so

install: ms
	$(INSTALLMOD) $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
ms.o:		ms.c	ms_help.c
