bin_PROGRAMS = stats
stats_SOURCES = chans.c conf.c dl.c dotconf.c hash.c ircd.c list.c main.c ns_help.c server.c services.c  sock.c timer.c users.c
stats_LDADD = @IRCD_FILES_SRC@ 
stats_LDFLAGS = -export-dynamic 
SUBDIRS=. dl data doc
sysconf_DATA=stats.cfg.dist chkstats makeconf README BUGS INSTALL AUTHORS BUGS COPYING
include_HEADERS = stats.h dl.h Unreal.h Ultimate.h hash.h list.h config.h dotconf.h 
