include ../Makefile.inc

OBJS   = api.o token.o parse.tab.o do_sql.o rtatables.o
SQLINCLUDES = -I. -I.. -I../pcre -I../adns
LIBS   = 

all: librtadb.a

redoparser:
	$(LEX) -otoken.c token.l 
	$(YACC) -dv -bparse parse.y 

.c.o:
	$(CC) $(SQLINCLUDES) $(CFLAGS) -c $<

librtadb.a: $(OBJS)
	ar -rcs librtadb.a $(OBJS)

install:
	$(INSTALL_DATA) rta.h $(INCDIRECTORY)

DISTFILES =*.c *.h *.y *.l README Makefile.in

dist:
	@for file in $(DISTFILES); do \
		cp -pr $$file $(DISTDIR)/$$file; \
	done

clean: 
	rm -rf *.o lib* Makefile parse.output parse.save

distclean:
	rm -rf *.o lib* Makefile parse.output parse.save

token.o: parse.tab.c do_sql.h
api.o: api.c rta.h do_sql.h
do_sql.o: do_sql.c do_sql.h rta.h
rtatables.o: rtatables.c do_sql.h rta.h
