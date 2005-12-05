BUILT_SOURCES=version.h
CLEANFILES = version.h

include_HEADERS		=	neostats.h config.h version.h \
				list.h hash.h support.h events.h numeric.h \
				adns.h protocol.h perlmod.h \
				updates.h MiniMessage.h MiniMessageGateway.h MuscleSupport.h NeoNet.h


noinst_HEADERS		=	commands.h dns.h servers.h users.h conf.h \
				log.h services.h modes.h signals.h nsevents.h \
				auth.h config.h.in exclude.h modules.h sock.h \
				bans.h base64.h ctcp.h helpstrings.h support.h \
				ircprotocol.h ircsend.h ircrecv.h \
				bots.h dcc.h ircstring.h numeric.h timer.h \
				channels.h dl.h lang.h transfer.h nsdbm.h \
				nsdba.h main.h numerics.h settings.h botinfo.h confuse.h \
				adnsdlist.h adnsinternal.h adnstvarith.h 

version.h:	stamp-h1
	@(if test -f version.sh ; then $(SHELL) version.sh; else echo > version.h; fi)

include $(top_srcdir)/autotools/rules.mk
