include ../../Makefile.inc

SRCS = hostserv.c hs_help.c
OBJS = ${SRCS:.c=.o}
TARGET = hostserv.so
DISTFILES = $(SRCS) hostserv.h Makefile ChangeLog

all: $(TARGET)

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
hostserv.o:		hostserv.c
