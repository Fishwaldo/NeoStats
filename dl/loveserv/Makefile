include ../../Makefile.inc

SRCS= loveserv.c ls_help.c 
OBJS= ${SRCS:.c=.o}
TARGET= loveserv.so
DISTFILES = $(SRCS) loveserv.h Makefile ChangeLog 

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(MODINCLUDES) -c $<

$(TARGET): $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so

install: $(TARGET)
	$(INSTALL_MOD) -m 644 $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
loveserv.o:		loveserv.c	ls_help.c
