include ../Makefile.inc

INCLUDES = -I.. -DHAVE_CONFIG_H
RANLIB=ranlib

SOURCES= kp_cache.c kp_dir.c kp_enum.c kp_get.c kp_iface.c kp_recur.c kp_set.c kp_sort.c kp_util.c
OBJECTS= kp_cache.o kp_dir.o kp_enum.o kp_get.o kp_iface.o kp_recur.o kp_set.o kp_sort.o kp_util.o
all:	libkeeper.a

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

libkeeper.a: $(OBJECTS)
	ar cru $@ ${OBJECTS}
	${RANLIB} $@

clean:
	/bin/rm -rf *.o *.lo *.so *.a

install: 




$(OBJECTS): Makefile ../Makefile.inc

