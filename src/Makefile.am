SUBDIRS		= protocol

bin_PROGRAMS = neostats

neostats_SOURCES = auth.c bans.c bots.c channels.c commands.c conf.c db.c \
		dl.c dns.c dotconf.c exclude.c hash.c ircd.c ircstring.c \
		keeper.c list.c log.c main.c match.c misc.c modules.c \
		ns_help.c servers.c services.c signals.c sock.c support.c \
		timer.c transfer.c users.c modes.c ctcp.c dcc.c
AM_CFLAGS = -I$(top_srcdir)/lib/adns -I$(top_srcdir)/lib/keeper \
		@PCRE_CFLAGS@ @CURL_CFLAGS@ -DNEOSTATSCORE
neostats_LDADD = -ldl $(top_srcdir)/lib/keeper/libkeeper.a \
		$(top_srcdir)/lib/adns/libadns.la @PCRE_LIBS@ @CURL_LIBS@
neostats_LDFLAGS = --export-dynamic -rdynamic

include $(top_srcdir)/autotools/rules.mk 
