include ../../Makefile.inc

INCLUDES = -I../.. -I../../adns

SRCS= $(extauth_src) 
OBJS= $(extauth_objs)
TARGET= extauth.so
DISTFILES = serviceroots.c Makefile

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
serviceroots.o:		serviceroots.c
