/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id$
*/

#include "neostats.h"
#include "modules.h"
#include "bots.h"
#include "timer.h"
#include "commands.h"
#include "sock.h"
#include "ns_help.h"
#include "users.h"
#include "server.h"
#include "chans.h"
#include "ircd.h"
#include "hash.h"
#include "exclude.h"

static int ns_set_debug (User * u, char **av, int ac);
static int ns_shutdown (User * u, char **av, int ac);
static int ns_reload (User * u, char **av, int ac);
static int ns_logs (User * u, char **av, int ac);
static int ns_jupe (User * u, char **av, int ac);
static int ns_exclude (User * u, char **av, int ac);
#ifdef USE_RAW
static int ns_raw (User * u, char **av, int ac);
#endif
static int ns_userdump (User * u, char **av, int ac);
static int ns_serverdump (User * u, char **av, int ac);
static int ns_chandump (User * u, char **av, int ac);
static int ns_status (User * u, char **av, int ac);
static int ns_version (User * u, char **av, int ac);
static int ns_level (User * u, char **av, int ac);
static int ns_load (User * u, char **av, int ac);
static int ns_unload (User * u, char **av, int ac);

static char quitmsg[BUFSIZE];
static char no_reason[]="no reason given";

static bot_cmd ns_commands[]=
{
	{"LEVEL",		ns_level,		0, 	0,					ns_help_level, 		ns_help_level_oneline},
	{"STATUS",		ns_status,		0, 	0,					ns_help_status, 	ns_help_status_oneline},
	{"VERSION",		ns_version,		0, 	0,					ns_help_version, 	ns_help_version_oneline},
	{"SHUTDOWN",	ns_shutdown,	1, 	NS_ULEVEL_ADMIN, 	ns_help_shutdown, 	ns_help_shutdown_oneline},
	{"RELOAD",		ns_reload,		1, 	NS_ULEVEL_ADMIN, 	ns_help_reload,		ns_help_reload_oneline},
	{"LOGS",		ns_logs,		0, 	NS_ULEVEL_OPER, 	ns_help_logs,		ns_help_logs_oneline},
	{"MODLIST",		list_modules,	0, 	NS_ULEVEL_ADMIN,  	ns_help_modlist, 	ns_help_modlist_oneline},
	{"LOAD",		ns_load,		1, 	NS_ULEVEL_ADMIN, 	ns_help_load, 		ns_help_load_oneline},
	{"UNLOAD",		ns_unload,		1,	NS_ULEVEL_ADMIN, 	ns_help_unload, 	ns_help_unload_oneline},
	{"JUPE",		ns_jupe,		1, 	NS_ULEVEL_ADMIN, 	ns_help_jupe,		ns_help_jupe_oneline},
	{"EXCLUDE",		ns_exclude,		1,	NS_ULEVEL_ADMIN,	ns_help_exclude,	ns_help_exclude_oneline},
#ifdef USE_RAW																
	{"RAW",			ns_raw,			0, 	NS_ULEVEL_ADMIN, 	ns_help_raw, 		ns_help_raw_oneline},
#endif																	
	{"DEBUG",		ns_set_debug,	1, 	NS_ULEVEL_ROOT,  	ns_help_debug,		ns_help_debug_oneline},
	{"BOTLIST",		list_bots,		0, 	NS_ULEVEL_ROOT,  	ns_help_botlist,	ns_help_botlist_oneline},
	{"SOCKLIST",	list_sockets,	0, 	NS_ULEVEL_ROOT,  	ns_help_socklist, 	ns_help_socklist_oneline},
	{"TIMERLIST",	list_timers,	0, 	NS_ULEVEL_ROOT,  	ns_help_timerlist, 	ns_help_timerlist_oneline},
	{"BOTCHANLIST",	list_bot_chans,	0, 	NS_ULEVEL_ROOT,  	ns_help_botchanlist,ns_help_botchanlist_oneline},
	{"USERDUMP",	ns_userdump,	0, 	NS_ULEVEL_ROOT,  	ns_help_userdump, 	ns_help_userdump_oneline},
	{"CHANDUMP",	ns_chandump,	0, 	NS_ULEVEL_ROOT,  	ns_help_chandump, 	ns_help_chandump_oneline},
	{"SERVERDUMP",	ns_serverdump,	0, 	NS_ULEVEL_ROOT,  	ns_help_serverdump, ns_help_serverdump_oneline},
	{NULL,			NULL,			0, 	0,					NULL, 				NULL}
};

Bot* ns_botptr;
BotInfo ns_botinfo = {
	"NeoStats",
	"NeoStats-",
	"",
	"",
	"",
};

/** @brief init_services_bot
 *
 * 
 *
 * @return none
 */
int 
init_services_bot (void)
{
	char **av;
	int ac = 0;
	long Umode;
	unsigned int flags;

	SET_SEGV_LOCATION();
	/* init NeoStats bot */
	ircsnprintf (ns_botinfo.realname, MAXREALNAME, "/msg %s \2HELP\2", ns_botinfo.nick);
	flags = me.onlyopers ? BOT_FLAG_ONLY_OPERS : 0;
	flags |= BOT_FLAG_DEAF;
	ns_botptr = init_bot (NULL, &ns_botinfo, services_bot_modes, flags, ns_commands, NULL);
#ifdef EXTAUTH
	/* load extauth if we need to */
	load_module ("extauth", NULL);
	InitExtAuth();
#endif
	me.onchan = 1;
	AddStringToList (&av, me.uplink, &ac);
	SendModuleEvent (EVENT_ONLINE, av, ac);
	free (av);
	return NS_SUCCESS;
}

/** @brief EXCLUDE command handler
 *
 *  maintain global exclusion list, which modules can take advantage off
 *   
 *  @param u user
 *  @param av list of arguments
 *  @param ac number of arguments
 *  @returns none
 */

static int
ns_exclude (User *u, char **av, int ac) 
{
	if (!ircstrcasecmp(av[2], "ADD")) {
		if (ac < 4) {
			prefmsg(u->nick, ns_botptr->nick, "Syntax error. /msg %s help exclude", ns_botptr->nick);
			return NS_FAILURE;
		}
		ns_do_exclude_add(u, av[2], av[3]);
	} else if (!ircstrcasecmp(av[1], "DEL")) {
		if (ac < 3) {
			prefmsg(u->nick, ns_botptr->nick, "Syntax error. /msg %s help exclude", ns_botptr->nick);
			return NS_FAILURE;
		}
		ns_do_exclude_del(u, av[2]);
	} else if (!ircstrcasecmp(av[1], "LIST")) {
		ns_do_exclude_list(u, ns_botptr->nick);
	} else {
		prefmsg(u->nick, ns_botptr->nick, "Syntax error. /msg %s help exclude", ns_botptr->nick);
	}
	return 1;
}
/** @brief SHUTDOWN command handler
 *
 *  Shutdown NeoStats
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_shutdown (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert (ns_botptr->nick, "%s requested SHUTDOWN for %s", u->nick, av[ac-1]);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested SHUTDOWN for %s.", 
		u->nick, u->username, u->hostname, av[ac-1]);
	globops (ns_botptr->nick, "%s", quitmsg);
	nlog (LOG_NOTICE, "%s", quitmsg);
	do_exit (NS_EXIT_NORMAL, quitmsg);
   	return 1;
}

/** @brief RELOAD command handler
 *
 *  Reload NeoStats
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_reload (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert (ns_botptr->nick, "%s requested RELOAD for %s", u->nick, av[ac - 1]);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested RELOAD for %s.", 
		u->nick, u->username, u->hostname, av[ac - 1]);
	globops (ns_botptr->nick, "%s", quitmsg);
	nlog (LOG_NOTICE, "%s", quitmsg);
	do_exit (NS_EXIT_RELOAD, quitmsg);
   	return 1;
}

/** @brief LOGS command handler
 *
 *  Send logs to nick
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_logs (User * u, char **av, int ac)
{
#ifdef DEBUG
	prefmsg (u->nick, ns_botptr->nick, "This command is disabled while in DEBUG.");
#else
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	fp = fopen ("logs/NeoStats.log", "r");
	if (!fp) {
		prefmsg (u->nick, ns_botptr->nick, "Unable to open neostats.log");
		return 1;
	}
	while (fgets (buf, BUFSIZE, fp)) {
		buf[strlen (buf)] = '\0';
		prefmsg (u->nick, ns_botptr->nick, buf);
	}
	fclose (fp);
#endif
   	return 1;
}

/** @brief JUPE command handler
 *
 *  Jupiter a server
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_jupe (User * u, char **av, int ac)
{
	static char infoline[255];

	SET_SEGV_LOCATION();
	ircsnprintf (infoline, 255, "[jupitered by %s]", u->nick);
	sserver_cmd (av[1], 1, infoline);
	nlog (LOG_NOTICE, "%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, av[1]);
	chanalert (ns_botptr->nick, "%s jupitered %s", u->nick, av[1]);
	prefmsg(u->nick, ns_botptr->nick, "%s has been jupitered", av[1]);
   	return 1;
}

/** @brief DEBUG command handler
 *
 *  Set debug mode on/off 
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_set_debug (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if ((!ircstrcasecmp(av[1], "YES")) || (!ircstrcasecmp(av[1], "ON"))) {
		me.debug_mode = 1;
		globops (me.name, "\2DEBUG MODE\2 enabled by %s", u->nick);
		prefmsg (u->nick, ns_botptr->nick, "Debug mode enabled!");
	} else if ((!ircstrcasecmp(av[1], "NO")) || (!ircstrcasecmp(av[1], "OFF"))) {
		me.debug_mode = 0;
		globops (me.name, "\2DEBUG MODE\2 disabled by %s", u->nick);
		prefmsg (u->nick, ns_botptr->nick, "Debug mode disabled");
	} else {
		prefmsg(u->nick, ns_botptr->nick,
			"Syntax Error: /msg %s HELP DEBUG for more info", ns_botptr->nick);
		   	return 0;
	}
   	return 1;
}

/** @brief USERDUMP command handler
 *
 *  Dump user list
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_userdump (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (u->nick, ns_botptr->nick, "\2Error:\2 debug mode disabled");
	   	return 0;
	}
	if(ac < 2) {
		chanalert (ns_botptr->nick, "\2DEBUG\2 \2%s\2 requested a user dump", u->nick);
		UserDump (NULL);
	} else {
		chanalert (ns_botptr->nick, "\2DEBUG\2 \2%s\2 requested a user dump for %s", u->nick, av[1]);
		UserDump (av[1]);
	}
   	return 1;
}

/** @brief SERVERDUMP command handler
 *
 *  Dump server list
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_serverdump (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (u->nick, ns_botptr->nick, "\2Error:\2 debug mode disabled");
	   	return 0;
	}
	if(ac < 2) {
		chanalert (ns_botptr->nick, "\2DEBUG\2 \2%s\2 requested a server dump", u->nick);
		ServerDump (NULL);
	} else {
		chanalert (ns_botptr->nick, "\2DEBUG\2 \2%s\2 requested a server dump for %s", u->nick, av[1]);
		ServerDump (av[1]);
	}
   	return 1;
}

/** @brief CHANDUMP command handler
 *
 *  Dump channel list
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_chandump (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (u->nick, ns_botptr->nick, "\2Error:\2 debug mode disabled");
	   	return 0;
	}
	if(ac < 2) {
		chanalert (ns_botptr->nick, "\2DEBUG\2 \2%s\2 requested a channel dump", u->nick);
		ChanDump (NULL);
	} else {
		chanalert (ns_botptr->nick, "\2DEBUG\2 \2%s\2 requested a channel dump for %s", u->nick, av[1]);
		ChanDump (av[1]);
	}
   	return 1;
}

/** @brief STATUS command handler
 *
 *  Display NeoStats status
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int 
ns_status (User * u, char **av, int ac)
{
	int uptime = me.now - me.t_start;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, ns_botptr->nick, "%s status:", ns_botptr->nick);
	if (uptime > 86400) {
		prefmsg (u->nick, ns_botptr->nick, "%s up \2%d\2 day%s, \2%02d:%02d\2", ns_botptr->nick, uptime / 86400, (uptime / 86400 == 1) ? "" : "s", (uptime / 3600) % 24, (uptime / 60) % 60);
	} else if (uptime > 3600) {
		prefmsg (u->nick, ns_botptr->nick, "%s up \2%d hour%s, %d minute%s\2", ns_botptr->nick, uptime / 3600, uptime / 3600 == 1 ? "" : "s", (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	} else if (uptime > 60) {
		prefmsg (u->nick, ns_botptr->nick, "%s up \2%d minute%s, %d second%s\2", ns_botptr->nick, uptime / 60, uptime / 60 == 1 ? "" : "s", uptime % 60, uptime % 60 == 1 ? "" : "s");
	} else {
		prefmsg (u->nick, ns_botptr->nick, "%s up \2%d second%s\2", ns_botptr->nick, uptime, uptime == 1 ? "" : "s");
	}
	prefmsg (u->nick, ns_botptr->nick, "Sent %ld messages, %ld bytes", me.SendM, me.SendBytes);
	prefmsg (u->nick, ns_botptr->nick, "Received %ld messages, %ld Bytes", me.RcveM, me.RcveBytes);
	prefmsg (u->nick, ns_botptr->nick, "Reconnect time: %d", me.r_time);
	prefmsg (u->nick, ns_botptr->nick, "Requests: %d", me.requests);
	prefmsg (u->nick, ns_botptr->nick, "Max sockets: %d (in use: %d)", me.maxsocks, me.cursocks);
	if (me.debug_mode)
		prefmsg (u->nick, ns_botptr->nick, "Debugging mode enabled");
	else
		prefmsg (u->nick, ns_botptr->nick, "Debugging mode disabled");
	return 0;
}

/** @brief VERSION command handler
 *
 *  NeoStats Version
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int
ns_version (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg (u->nick, ns_botptr->nick, "\2NeoStats version\2");
	prefmsg (u->nick, ns_botptr->nick, "NeoStats version: %s", me.versionfull);
	prefmsg (u->nick, ns_botptr->nick, "http://www.neostats.net");
   	return 1;
}

/** @brief LEVEL command handler
 *
 *  Display user level
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int 
ns_level (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if(ac > 1) {
		User * otheruser;
		otheruser = finduser(av[1]);
		if(otheruser) {
			prefmsg (u->nick, ns_botptr->nick, "User level for %s is %d", otheruser->nick, UserLevel (otheruser));
		}
	} else {
		prefmsg (u->nick, ns_botptr->nick, "Your level is %d", UserLevel (u));
	}
	return 1;
}

/** @brief LOAD command handler
 *
 *  Load module
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int 
ns_load (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (load_module (av[1], u) == NS_SUCCESS) {
		chanalert (ns_botptr->nick, "%s loaded module %s", u->nick, av[1]);
	} else {
		chanalert (ns_botptr->nick, "%s tried to load module %s, but load failed", u->nick, av[1]);
	}
   	return 1;
}

/** @brief UNLOAD command handler
 *
 *  Unload module
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
static int 
ns_unload (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (unload_module (av[1], u) > 0) {
		chanalert (ns_botptr->nick, "%s unloaded module %s", u->nick, av[1]);
	}
   	return 1;
}

/** @brief RAW command handler
 *
 *  issue a RAW command
 *   
 *  @param user
 *  @param list of arguments
 *  @param number of arguments
 *  @returns none
 */
#ifdef USE_RAW
static int
ns_raw (User * u, char **av, int ac)
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf (av, ac, 1);
	chanalert (ns_botptr->nick, "\2RAW COMMAND\2 \2%s\2 issued a raw command!(%s)", u->nick, message);
	nlog (LOG_INFO, "RAW COMMAND %s issued a raw command!(%s)", u->nick, message);
	send_cmd ("%s", message);
	free (message);
   	return 1;
}
#endif
