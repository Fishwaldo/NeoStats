include ../Makefile.inc

INCLUDES = -I..
RANLIB=ranlib
CFLAGS2=-O2 -ggdb -fPIC -DPIC -I..

SOURCES= check.c  event.c  general.c  parse.c  query.c  reply.c  setup.c  transmit.c  types.c
OBJECTS= check.o  event.o  general.o  parse.o  query.o  reply.o  setup.o  transmit.o  types.o
all:	libadns.a

.c.o:
	$(CC) $(CFLAGS2) $(INCLUDES) -c $<

libadns.a: $(OBJECTS)
	ar cru $@ ${OBJECTS}
	${RANLIB} $@

clean:
	/bin/rm -rf *.o *.lo *.so *.a

install: 




$(OBJECTS): Makefile ../Makefile.inc

event.o:	internal.h	tvarith.h	adns.h	dlist.h config.h
check.o:	internal.h	adns.h	dlist.h config.h
general.o:	internal.h	adns.h  dlist.h config.h
parse.o:	internal.h	adns.h  dlist.h config.h
query.o:	internal.h	adns.h  dlist.h config.h
reply.o:	internal.h	adns.h  dlist.h config.h
setup.o:	internal.h	adns.h  dlist.h config.h
transmit.o:	internal.h	tvarith.h	adns.h  dlist.h config.h
types.o:	internal.h	adns.h  dlist.h config.h
