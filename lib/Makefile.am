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

SUBDIRS		= adns rta gdbm $(CURLDIR) $(PCREDIR)

include $(top_srcdir)/autotools/rules.mk