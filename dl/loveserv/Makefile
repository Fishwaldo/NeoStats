include ../../Makefile.inc

SRCS= loveserv.c ls_help.c 
OBJS= ${SRCS:.c=.o}
TARGET= loveserv.so
DISTFILES = $(SRCS) loveserv.h Makefile ChangeLog 

all:	loveserv

.c.o:
	$(CC) $(CFLAGS) $(MODINCLUDES) -c $<

loveserv: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so

install: loveserv
	$(INSTALLMOD) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
loveserv.o:		loveserv.c	ls_help.c
