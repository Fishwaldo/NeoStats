include ../../Makefile.inc

SRCS = cs.c cs_help.c
OBJS = ${SRCS:.c=.o}
TARGET = connectserv.so
DISTFILES = $(SRCS) cs.h Makefile ChangeLog

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(MODINCLUDES) -c $<

$(TARGET): $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)

dist: 	$(DISTFILES)
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done

$(OBJS): Makefile

cs.o:		cs.c	cs_help.c
