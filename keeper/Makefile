include ../Makefile.inc

INCLUDES = -I.. -DHAVE_CONFIG_H

SRCS= kp_cache.c kp_dir.c kp_enum.c kp_get.c kp_iface.c kp_recur.c kp_set.c kp_sort.c kp_util.c
OBJS= ${SRCS:.c=.o}

$(OBJS): Makefile ../Makefile.inc 

all:	$(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	/bin/rm -rf *.o *.lo *.so *.a

depend:
	$(CC) -MM $(INCLUDES) $(CFLAGS) $(SRCS) > .depend

install:

DISTFILES =Makefile *.c *.h 

dist:
	@for file in $(DISTFILES); do \
		cp -pr $$file $(DISTDIR)/$$file; \
	done

kp_cache.o: kp_cache.c kp_util.h keeper.h ../config.h
kp_dir.o: kp_dir.c kp_util.h keeper.h ../config.h
kp_enum.o: kp_enum.c kp_util.h keeper.h ../config.h
kp_get.o: kp_get.c kp_util.h keeper.h ../config.h
kp_iface.o: kp_iface.c kp_util.h keeper.h ../config.h
kp_recur.o: kp_recur.c kp_util.h keeper.h ../config.h
kp_set.o: kp_set.c kp_util.h keeper.h ../config.h
kp_sort.o: kp_sort.c keeper.h
kp_util.o: kp_util.c kp_util.h keeper.h ../config.h
