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



SUBDIRS		= protocol $(CURLDIR) keeper adns libltdl $(PCREDIR) . modules


bin_PROGRAMS = neostats

neostats_SOURCES = auth.c bans.c bots.c channels.c commands.c conf.c db.c \
		dl.c dns.c dotconf.c exclude.c hash.c ircd.c ircstring.c \
		keeper.c list.c log.c main.c match.c misc.c modules.c \
		ns_help.c servers.c services.c signals.c sock.c support.c \
		timer.c transfer.c users.c
AM_CFLAGS = @PCRE_CFLAGS@ -I$(top_srcdir)/lang -Iadns -Ikeeper @INCLTDL@ @CURL_CFLAGS@ -DNEOSTATSCORE
neostats_LDADD = keeper/libkeeper.a \
		adns/libadns.la libltdl/libltdlc.la @PCRE_LIBS@ @CURL_LIBS@
neostats_LDFLAGS = --export-dynamic -rdynamic

include $(top_srcdir)/autotools/rules.mk 
