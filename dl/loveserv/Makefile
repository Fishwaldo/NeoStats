include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= loveserv.c
OBJECTS= loveserv.o
TARGET= loveserv.so

all:	loveserv

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

loveserv: $(OBJECTS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: loveserv
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
loveserv.o:		loveserv.c	ls_help.c
