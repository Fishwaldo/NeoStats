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

depend:
	$(CC) -MM $(INCLUDES) $(CFLAGS) $(SOURCES) > .depend


install: 




$(OBJECTS): Makefile ../Makefile.inc

kp_cache.o: kp_cache.c kp_util.h keeper.h ../config.h
kp_dir.o: kp_dir.c kp_util.h keeper.h ../config.h
kp_enum.o: kp_enum.c kp_util.h keeper.h ../config.h
kp_get.o: kp_get.c kp_util.h keeper.h ../config.h
kp_iface.o: kp_iface.c kp_util.h keeper.h ../config.h
kp_recur.o: kp_recur.c kp_util.h keeper.h ../config.h
kp_set.o: kp_set.c kp_util.h keeper.h ../config.h
kp_sort.o: kp_sort.c keeper.h
kp_util.o: kp_util.c kp_util.h keeper.h ../config.h
