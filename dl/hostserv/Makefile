include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= hostserv.c
OBJS= ${SRCS:.c=.o}
TARGET= hostserv.so
DISTFILES = $(SRCS) hs_help.c Makefile README ChangeLog

all:	hostserv 

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

hostserv: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	/bin/rm -rf ../$(TARGET) 
	/bin/rm -rf *.o *.lo *.so

install: hostserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
hostserv.o:		hostserv.c	hs_help.c	
