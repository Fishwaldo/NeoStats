if BUILD_PCRE
PCREDIR = pcre
else
PCREDIR =
endif
                                                                                
if BUILD_CURL
CURLDIR = curl
else
CURLDIR =
endif

SUBDIRS		= adns rta $(CURLDIR) $(PCREDIR)

EXTRA_DIST	= gdbm/autoconf.h.in gdbm/gdbmdefs.h gdbm/gdbmerrno.h gdbm/extern.h \
		  gdbm/systems.h gdbm/gdbmconst.h gdbm/proto.h

include $(top_srcdir)/autotools/rules.mk