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




$(OBJECTS): Makefile ../Makefile.inc
