include ../Makefile.inc

INCLUDES = -I..
RANLIB=ranlib


SOURCES= check.c  event.c  general.c  parse.c  query.c  reply.c  setup.c  transmit.c  types.c
OBJECTS= check.o  event.o  general.o  parse.o  query.o  reply.o  setup.o  transmit.o  types.o
all:	libadns.a

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

libadns.a: $(OBJECTS)
	ar cru $@ ${OBJECTS}
	${RANLIB} $@

clean:
	/bin/rm -rf *.o *.lo *.so *.a

install: 




$(OBJECTS): Makefile

event.o:	internal.h	tvarith.h	adns.h	dlist.h
check.o:	internal.h	adns.h	dlist.h
general.o:	internal.h	adns.h  dlist.h
parse.o:	internal.h	adns.h  dlist.h
query.o:	internal.h	adns.h  dlist.h
reply.o:	internal.h	adns.h  dlist.h
setup.o:	internal.h	adns.h  dlist.h
transmit.o:	internal.h	tvarith.h	adns.h  dlist.h
types.o:	internal.h	adns.h  dlist.h
