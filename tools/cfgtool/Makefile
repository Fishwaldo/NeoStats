include ../../Makefile.inc

TOOLSCFLAGS = $(CFLAGS) $(INSTALLDIR) -DHAVE_CONFIG_H 

INCLUDES = -I../.. -I../../keeper/ -Ipixmaps
GCFLAGS = `gtk-config --cflags`
GLDFLAGS = `gtk-config --libs`
KP_SRCS= gkeeper.c tree.c edit.c menu.c dialog.c
KP_OBJS= gkeeper.o tree.o edit.o menu.o dialog.o
KEEPER_OBJS= ../../keeper/kp_cache.o ../../keeper/kp_dir.o ../../keeper/kp_enum.o ../../keeper/kp_get.o \
			../../keeper/kp_iface.o ../../keeper/kp_recur.o ../../keeper/kp_set.o ../../keeper/kp_sort.o \
			../../keeper/kp_util.o
all: 	configtool

.c.o:
	$(CC) $(TOOLSCFLAGS) $(GCFLAGS) $(INCLUDES) -c $<


configtool:	$(KP_OBJS)
	$(CC) $(GLDFLAGS) $(KP_OBJS) $(KEEPER_OBJS) -o $@

clean:
	/bin/rm -rf *.o *.lo *.so *.a configtool

install: 

depend:
	$(CC) -MM $(INCLUDES) $(CFLAGS) $(KP_SRCS) > .depend



$(OBJS): Makefile ../Makefile.inc

gkeeper.o: gkeeper.c gkeeper.h ../../config.h ../../keeper/keeper.h
tree.o: tree.c gkeeper.h ../../config.h ../../keeper/keeper.h pixmaps/folder_open.xpm pixmaps/folder_closed.xpm pixmaps/book_open.xpm pixmaps/book_closed.xpm pixmaps/string.xpm pixmaps/data.xpm pixmaps/float.xpm pixmaps/num.xpm
edit.o: edit.c gkeeper.h ../../config.h ../../keeper/keeper.h
menu.o: menu.c gkeeper.h ../../config.h ../../keeper/keeper.h
dialog.o: dialog.c gkeeper.h ../../config.h ../../keeper/keeper.h pixmaps/warning.xpm
