include ../../../Makefile.inc

INCLUDES = -I../..
RANLIB=ranlib


SOURCES= ares__close_sockets.c ares__get_hostent.c ares__read_line.c \
        ares_destroy.c ares_expand_name.c ares_fds.c ares_free_errmem.c \
        ares_free_hostent.c ares_free_string.c ares_gethostbyaddr.c \
        ares_gethostbyname.c ares_init.c ares_mkquery.c ares_parse_a_reply.c \
        ares_parse_ptr_reply.c ares_process.c ares_query.c ares_search.c \
        ares_send.c ares_strerror.c ares_timeout.c
OBJECTS= ares__close_sockets.o ares__get_hostent.o ares__read_line.o \
        ares_destroy.o ares_expand_name.o ares_fds.o ares_free_errmem.o \
        ares_free_hostent.o ares_free_string.o ares_gethostbyaddr.o \
        ares_gethostbyname.o ares_init.o ares_mkquery.o ares_parse_a_reply.o \
        ares_parse_ptr_reply.o ares_process.o ares_query.o ares_search.o \
        ares_send.o ares_strerror.o ares_timeout.o

all:	libares.a

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

libares.a: $(OBJECTS)
	ar cru $@ ${OBJECTS}
	${RANLIB} $@
	cp libares.a ../
clean:
	/bin/rm -rf *.o *.lo *.so *.a

install: 




$(OBJECTS): Makefile
