include ../../Makefile.inc

SRCS = template.c
OBJS = ${SRCS:.c=.o}
TARGET = template.so
DISTFILES = $(SRCS) Makefile

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

template.o:		template.c	
