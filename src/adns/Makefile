include ../Makefile.inc

INCLUDES = -I.. -I../adns -I../pcre

SRCS = check.c event.c general.c parse.c query.c reply.c setup.c transmit.c types.c
OBJS = ${SRCS:.c=.o}

all: $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	$(RM) *.o *.lo *.so *.a

distclean:
	$(RM) *.o *.lo *.so *.a

depend:
	$(CC) -MM $(INCLUDES) $(CFLAGS) $(SRCS) > .depend

install:
	$(INSTALL_DATA) adns.h $(INCDIRECTORY)

DISTFILES =*.c adns.h dlist.h internal.h tvarith.h Makefile

dist:
	@for file in $(DISTFILES); do \
		cp -pr $$file $(DISTDIR)/$$file; \
	done

$(OBJS): Makefile ../Makefile.inc

check.o: check.c internal.h adns.h dlist.h
event.o: event.c internal.h adns.h dlist.h tvarith.h
general.o: general.c internal.h adns.h dlist.h
parse.o: parse.c internal.h adns.h dlist.h
query.o: query.c internal.h adns.h dlist.h
reply.o: reply.c internal.h adns.h dlist.h
setup.o: setup.c internal.h adns.h dlist.h
transmit.o: transmit.c internal.h adns.h dlist.h tvarith.h
types.o: types.c internal.h adns.h dlist.h