BUILT_SOURCES=version.h
CLEANFILES = version.h

include_HEADERS		=	neostats.h config.h version.h \
						list.h hash.h support.h events.h numeric.h 

noinst_HEADERS		=	commands.h dns.h servers.h users.h conf.h \
				dotconf.h log.h services.h modes.h signals.h \
				auth.h config.h.in  exclude.h  modules.h sock.h \
				bans.h base64.h ctcp.h ircd.h ns_help.h support.h \
				bots.h dcc.h ircstring.h numeric.h timer.h \
				channels.h dl.h lang.h rtaserv.h transfer.h nsdbm.h \
				nsdba.h main.h numerics.h settings.h botinfo.h

version.h:
	@(if test -f version.sh ; then $(SHELL) version.sh; else echo > version.h; fi)

include $(top_srcdir)/autotools/rules.mk