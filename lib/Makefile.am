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

if BUILD_SQL
SQLSRVDIR = sqlsrv
else
SQLSRVDIR =
endif


SUBDIRS		= adns keeper $(CURLDIR) $(PCREDIR) $(SQLSRVDIR)

include $(top_srcdir)/autotools/rules.mk