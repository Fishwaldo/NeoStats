SUBDIRS		= protocol curl keeper adns libltdl


bin_PROGRAMS = neostats

neostats_SOURCES = auth.c bans.c bots.c channels.c commands.c conf.c db.c \
		dl.c dns.c dotconf.c exclude.c hash.c ircd.c ircstring.c \
		keeper.c list.c log.c main.c match.c misc.c modules.c \
		ns_help.c servers.c services.c signals.c sock.c support.c \
		timer.c transfer.c users.c
neostats_CFLAGS = -Ipcre -Iadns -Icurl -Ikeeper @INCLTDL@
neostats_LDADD = protocol/libprotocol.la curl/libcurl.la keeper/libkeeper.la \
		adns/libadns.la libltdl/libltdlc.la
