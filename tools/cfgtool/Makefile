include ../../Makefile.inc

INCLUDES = -I../.. -I../../keeper/ -Ipixmaps -DHAVE_CONFIG_H 
GCFLAGS = `gtk-config --cflags`
GLDFLAGS = `gtk-config --libs`
KP_SOURCES= gkeeper.c tree.c edit.c menu.c dialog.c
KP_OBJECTS= gkeeper.o tree.o edit.o menu.o dialog.o
all: 	configtool

.c.o:
	$(CC) $(GCFLAGS) $(CFLAGS) $(INCLUDES) -c $<


configtool:	$(KP_OBJECTS)
	$(CC) $(GLDFLAGS) $(KP_OBJECTS) ../../keeper/libkeeper.a -o $@

clean:
	/bin/rm -rf *.o *.lo *.so *.a configtool

install: 

depend:
	$(CC) -MM $(INCLUDES) $(CFLAGS) $(KP_SOURCES) > .depend



$(OBJECTS): Makefile ../Makefile.inc

gkeeper.o: gkeeper.c gkeeper.h ../../config.h ../../keeper/keeper.h
tree.o: tree.c gkeeper.h ../../config.h ../../keeper/keeper.h pixmaps/folder_open.xpm pixmaps/folder_closed.xpm pixmaps/book_open.xpm pixmaps/book_closed.xpm pixmaps/string.xpm pixmaps/data.xpm pixmaps/float.xpm pixmaps/num.xpm
edit.o: edit.c gkeeper.h ../../config.h ../../keeper/keeper.h
menu.o: menu.c gkeeper.h ../../config.h ../../keeper/keeper.h
dialog.o: dialog.c gkeeper.h ../../config.h ../../keeper/keeper.h pixmaps/warning.xpm
