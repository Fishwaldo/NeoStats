include ../../Makefile.inc

OBJS = ${SRCS:.c=.o}

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(MODCFLAGS) $(MODINCLUDES) -c $<

$(TARGET): $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

clean:
	$(RM) *.o *.lo *.so *.d

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)
	$(INSTALL_DATA) $(DATA) $(DATADIRECTORY)

dist: 	$(DISTFILES)
	@for subdir in $(SUBDIRS); do \
		mkdir $(DISTDIR)/$$subdir; \
		chmod 777 $(DISTDIR)/$$subdir; \
	done
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done

$(OBJS): Makefile

