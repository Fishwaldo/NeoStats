include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= loveserv.c
OBJS= ${SRCS:.c=.o}
TARGET= loveserv.so
DISTFILES = $(SRCS) ls_help.c Makefile ChangeLog 

all:	loveserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

loveserv: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: loveserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
loveserv.o:		loveserv.c	ls_help.c
