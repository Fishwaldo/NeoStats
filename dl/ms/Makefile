include ../../Makefile.inc

SRCS= ms.c ms_help.c
OBJS= ${SRCS:.c=.o}
TARGET= moraleserv.so
DISTFILES = $(SRCS) ms.h Makefile ChangeLog

all:	$(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(MODINCLUDES) -c $<

$(TARGET): $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
ms.o:		ms.c	ms_help.c
