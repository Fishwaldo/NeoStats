include ../../Makefile.inc

INCLUDES = -I../..

SOURCES= ms.c
OBJECTS= ms.o
TARGET= ms.so

all:	ms

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

ms: $(OBJECTS)
	$(CC) -nostdlib -shared -o $(TARGET) $(LIBS) $(OBJECTS)
	/bin/cp $(TARGET) ../

clean:
	/bin/rm -rf ../$(TARGET)
	/bin/rm -rf *.o *.lo *.so

install: ms
	$(INSTALL) -m 644 $(TARGET) $(DIRECTORY)/dl




$(OBJECTS): Makefile
ms.o:		ms.c	ms_help.c
