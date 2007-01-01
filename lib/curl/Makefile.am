#
# $Id: Makefile.am,v 1.72 2003/10/24 21:26:05 bagder Exp $
#

AUTOMAKE_OPTIONS = foreign nostdinc

EXTRA_DIST = getdate.y 

lib_LTLIBRARIES = libcurl.la

# we use srcdir/include for the static global include files
# we use builddir/lib for the generated lib/config.h file to get found
# we use srcdir/lib for the lib-private header files
INCLUDES = -I$(top_srcdir)/include 

VERSION=-version-info 2:2:0

# This flag accepts an argument of the form current[:revision[:age]]. So,
# passing -version-info 3:12:1 sets current to 3, revision to 12, and age to
# 1.
#
# If either revision or age are omitted, they default to 0. Also note that age
# must be less than or equal to the current interface number.
#
# Here are a set of rules to help you update your library version information: 
#
# 1.Start with version information of 0:0:0 for each libtool library.
#
# 2.Update the version information only immediately before a public release of
# your software. More frequent updates are unnecessary, and only guarantee
# that the current interface number gets larger faster.
#
# 3.If the library source code has changed at all since the last update, then
# increment revision (c:r:a becomes c:r+1:a).
#
# 4.If any interfaces have been added, removed, or changed since the last
# update, increment current, and set revision to 0.
#
# 5.If any interfaces have been added since the last public release, then
# increment age.
#
# 6.If any interfaces have been removed since the last public release, then
# set age to 0.
#

#if NO_UNDEFINED
# The -no-undefined flag is CRUCIAL for this to build fine on Cygwin.
#UNDEF = -no-undefined
#endif

#if MIMPURE
# This is for gcc on Solaris (8+ ?) to avoid "relocations remain against
# allocatable but non-writable sections" problems.
#MIMPURE = -mimpure-text
#endif

libcurl_la_LDFLAGS = $(VERSION)

libcurl_la_SOURCES = file.c timeval.c base64.c hostip.c progress.c formdata.c	\
  cookie.c http.c sendf.c ftp.c url.c dict.c if2ip.c speedcheck.c	\
  ldap.c ssluse.c version.c getenv.c escape.c mprintf.c telnet.c	\
  netrc.c getinfo.c transfer.c strequal.c easy.c security.c krb4.c	\
  memdebug.c http_chunks.c strtok.c connect.c llist.c hash.c multi.c	\
  content_encoding.c share.c http_digest.c md5.c http_negotiate.c	\
  http_ntlm.c inet_pton.c strtoofft.c strerror.c hostares.c hostasyn.c	\
  hostip4.c hostip6.c hostsyn.c hostthre.c inet_ntop.c parsedate.c	\
  select.c gtls.c sslgen.c tftp.c splay.c strdup.c socks.c

noinst_HEADERS = arpa_telnet.h netrc.h file.h timeval.h base64.h hostip.h	\
  progress.h formdata.h cookie.h http.h sendf.h ftp.h url.h dict.h	\
  if2ip.h speedcheck.h urldata.h ldap.h ssluse.h escape.h telnet.h	\
  getinfo.h strequal.h krb4.h memdebug.h inet_ntoa_r.h \
  http_chunks.h strtok.h connect.h llist.h hash.h content_encoding.h	\
  share.h md5.h http_digest.h http_negotiate.h http_ntlm.h ca-bundle.h	\
  inet_pton.h strtoofft.h strerror.h inet_ntop.h curlx.h memory.h	\
  setup.h transfer.h select.h easyif.h multiif.h parsedate.h sslgen.h   \
  gtls.h tftp.h sockaddr.h splay.h strdup.h setup_once.h socks.h

BUILT_SOURCES = $(srcdir)/ca-bundle.h

$(srcdir)/ca-bundle.h: Makefile.in Makefile
	@if test -f $@; then \
	  chmod 0644 $@; \
	fi
	@echo "/* The file is generated automaticly */" > $@
	@echo '#undef CURL_CA_BUNDLE /* unknown */' >> $@

include $(top_srcdir)/autotools/rules.mk
