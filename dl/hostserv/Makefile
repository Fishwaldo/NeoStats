include ../../Makefile.inc

INCLUDES = -I. -I../.. -I../../adns

SRCS= hostserv.c hs_help.c
OBJS= ${SRCS:.c=.o}
TARGET= hostserv.so
DISTFILES = $(SRCS) hostserv.h Makefile README ChangeLog

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

$(TARGET): $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(DIRECTORY)/dl

dist:	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done



$(OBJS): Makefile
hostserv.o:		hostserv.c
