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

SUBDIRS		= adns keeper $(CURLDIR) $(PCREDIR) rta

include $(top_srcdir)/autotools/rules.mk