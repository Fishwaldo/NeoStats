# Module Makefile.inc
# utility macros used to build, clean, install and dist modules

# Include NeoStats Makefile.inc
# pwd is the module so we must map directories from there
include ../../Makefile.inc

# Create the list of objects we will build. 
# Module makefile must supply SRCS
OBJS = ${SRCS:.c=.o}

# The target we are building
# Module makefile must supply TARGET
all: $(TARGET)

# Include dependency info
-include $(OBJS:.o=.d)

# How to build objects from C source
# Module makefile can optionally specify MODCFLAGS
.c.o:
	$(CC) $(CFLAGS) $(MODCFLAGS) $(MODINCLUDES) -c $<
	$(CC) -MM $(MODINCLUDES) -c $< > $*.d

# How to build our target modules
$(TARGET): $(OBJS)
	$(LD) $(MODLDFLAGS) -o $(TARGET) $(LIBS) $(OBJS)

# How to clean up files created by the build process
clean:
	$(RM) *.o *.lo *.so *.d *.dll

distclean:
	$(RM) *.o *.lo *.so *.d *.dll

# How to install this module
# TARGET will be that defined for building in the Module makefile
# Module makefile can optionally specify DATA for data files to install
install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)

# How to create dist of this module
# Module makefile must supply DISTFILES with the files to be distributed 
# Module makefile can optionally specify SUBDIRS for subdirectories it uses
dist: 	$(DISTFILES)
	@for subdir in $(SUBDIRS); do \
		mkdir $(DISTDIR)/$$subdir; \
		chmod 777 $(DISTDIR)/$$subdir; \
	done
	@for files in $(DISTFILES); do \
		cp $$files $(DISTDIR)/$$files; \
	done

# Give all objects a dependency on the Makefiles used for build
$(OBJS): Makefile ../Makefile.inc ../../Makefile.inc
