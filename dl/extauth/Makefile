include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= $(extauth_src) 
OBJS= $(extauth_objs)
TARGET= extauth.so
DISTFILES = serviceroots.c Makefile

all:	extauth

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

extauth: $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: extauth
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
serviceroots.o:		serviceroots.c
