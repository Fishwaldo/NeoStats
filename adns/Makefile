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

depend:
	$(CC) -MM $(INCLUDES) $(CFLAGS) $(SOURCES) > .depend


install: 




$(OBJECTS): Makefile ../Makefile.inc

check.o: check.c internal.h config.h adns.h dlist.h
event.o: event.c internal.h config.h adns.h dlist.h tvarith.h
general.o: general.c internal.h config.h adns.h dlist.h
parse.o: parse.c internal.h config.h adns.h dlist.h
query.o: query.c internal.h config.h adns.h dlist.h
reply.o: reply.c internal.h config.h adns.h dlist.h
setup.o: setup.c internal.h config.h adns.h dlist.h
transmit.o: transmit.c internal.h config.h adns.h dlist.h tvarith.h
types.o: types.c internal.h config.h adns.h dlist.h
