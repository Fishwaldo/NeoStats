include ../../Makefile.inc

SRCS = $(extauth_src) 
OBJS = $(extauth_objs)
TARGET = extauth.so
DISTFILES = serviceroots.c Makefile

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
serviceroots.o:		serviceroots.c
