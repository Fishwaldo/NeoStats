SUBDIRS = dbm protocol

bin_PROGRAMS = neostats

neostats_SOURCES = auth.c bans.c bots.c channels.c commands.c confuse.c confuselexer.c nsdba.c \
		conf.c dl.c dns.c exclude.c hash.c ircstring.c \
		ircprotocol.c ircsend.c ircrecv.c \
		lang.c list.c log.c main.c match.c misc.c modules.c \
		helpstrings.c servers.c services.c signals.c sock.c support.c \
		timer.c transfer.c users.c modes.c ctcp.c dcc.c nsmemory.c \
		base64.c numerics.c settings.c botinfo.c \
		adnscheck.c adnsevent.c adnsgeneral.c adnsparse.c adnsquery.c \
		adnsreply.c adnssetup.c adnstransmit.c adnstypes.c \
		oscalls.c ossocket.c osfile.c updates.c
if DO_PERL
neostats_SOURCES += perl.c
else
neostats_SOURCES +=
endif

AM_CFLAGS = @PCRE_CFLAGS@ @CURL_CFLAGS@ -DNEOSTATSCORE @PERL_CFLAGS@ \
		-I$(top_srcdir)/lib/event
neostats_LDADD = @PCRE_LIBS@ @CURL_LIBS@ @LIBDB@ @PERL_LDFLAGS@ \
		$(top_srcdir)/lib/event/libevent.a

neostats_LDFLAGS = --export-dynamic -rdynamic
noinst_HEADERS          =       confuselexer.l


include $(top_srcdir)/autotools/rules.mk 
