/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "servers.h"
#include "channels.h"
#include "ircd.h"
#include "hash.h"
#include "exclude.h"

static int ns_set_debug (CmdParams* cmdparams);
static int ns_shutdown (CmdParams* cmdparams);
static int ns_reload (CmdParams* cmdparams);
static int ns_jupe (CmdParams* cmdparams);
static int ns_exclude (CmdParams* cmdparams);
#ifdef USE_RAW
static int ns_raw (CmdParams* cmdparams);
#endif
static int ns_userdump (CmdParams* cmdparams);
static int ns_serverdump (CmdParams* cmdparams);
static int ns_chandump (CmdParams* cmdparams);
static int ns_status (CmdParams* cmdparams);
static int ns_level (CmdParams* cmdparams);
static int ns_load (CmdParams* cmdparams);
static int ns_unload (CmdParams* cmdparams);

static char quitmsg[BUFSIZE];

static const char *ns_about[] = {
	"\2NeoStats\2 statistical services",
	NULL
};

ModuleInfo ns_module_info = {
	"NeoStats",
	"NeoStats Statistical services", 	
	ns_copyright,
	ns_about,
	NEOSTATS_VERSION,
	NEOSTATS_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

Module ns_module = {
	&ns_module_info
};

static bot_cmd ns_commands[]=
{
	{"LEVEL",		ns_level,		0, 	0,					ns_help_level, 		ns_help_level_oneline},
	{"STATUS",		ns_status,		0, 	0,					ns_help_status, 	ns_help_status_oneline},
	{"SHUTDOWN",	ns_shutdown,	1, 	NS_ULEVEL_ADMIN, 	ns_help_shutdown, 	ns_help_shutdown_oneline},
	{"RELOAD",		ns_reload,		1, 	NS_ULEVEL_ADMIN, 	ns_help_reload,		ns_help_reload_oneline},
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
 *  init NeoStats bot
 *
 * @return none
 */
int 
init_services_bot (void)
{
	unsigned int flags;

	SET_SEGV_LOCATION();
	/* if all bots should join the chan */
	if (GetConf ((void *) &me.allbots, CFGINT, "AllBotsJoinChan") <= 0) {
		me.allbots = 0;
	}
	if (GetConf ((void *) &me.pingtime, CFGINT, "PingServerTime") <= 0) {
		me.pingtime = 120;
	}
	ircsnprintf (ns_botinfo.realname, MAXREALNAME, "/msg %s \2HELP\2", ns_botinfo.nick);
	flags = me.onlyopers ? BOT_FLAG_ONLY_OPERS : 0;
	flags |= BOT_FLAG_DEAF;
	ns_botptr = init_bot (&ns_botinfo, services_bot_modes, flags, ns_commands, NULL);
	me.onchan = 1;
	SendAllModuleEvent (EVENT_ONLINE, NULL);
	return NS_SUCCESS;
}

/** @brief EXCLUDE command handler
 *
 *  maintain global exclusion list, which modules can take advantage off
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */

static int
ns_exclude (CmdParams* cmdparams) 
{
	if (!ircstrcasecmp(cmdparams->av[0], "ADD")) {
		if (cmdparams->ac < 3) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		ns_do_exclude_add(cmdparams->source.user, cmdparams->av[0], cmdparams->av[1]);
	} else if (!ircstrcasecmp(cmdparams->av[0], "DEL")) {
		if (cmdparams->ac < 2) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		ns_do_exclude_del(cmdparams->source.user, cmdparams->av[1]);
	} else if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		ns_do_exclude_list(cmdparams->source.user, ns_botptr->nick);
	} else {
		return NS_ERR_SYNTAX_ERROR;
	}
	return NS_SUCCESS;
}
/** @brief SHUTDOWN command handler
 *
 *  Shutdown NeoStats
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_shutdown (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	chanalert (ns_botptr->nick, "%s requested SHUTDOWN for %s", cmdparams->source.user->nick, cmdparams->av[cmdparams->ac-1]);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested SHUTDOWN for %s.", 
		cmdparams->source.user->nick, cmdparams->source.user->username, cmdparams->source.user->hostname, cmdparams->av[cmdparams->ac-1]);
	globops (ns_botptr->nick, "%s", quitmsg);
	nlog (LOG_NOTICE, "%s", quitmsg);
	do_exit (NS_EXIT_NORMAL, quitmsg);
   	return NS_SUCCESS;
}

/** @brief RELOAD command handler
 *
 *  Reload NeoStats
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_reload (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	chanalert (ns_botptr->nick, "%s requested RELOAD for %s", cmdparams->source.user->nick, cmdparams->av[cmdparams->ac - 1]);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested RELOAD for %s.", 
		cmdparams->source.user->nick, cmdparams->source.user->username, cmdparams->source.user->hostname, cmdparams->av[cmdparams->ac - 1]);
	globops (ns_botptr->nick, "%s", quitmsg);
	nlog (LOG_NOTICE, "%s", quitmsg);
	do_exit (NS_EXIT_RELOAD, quitmsg);
   	return NS_SUCCESS;
}

/** @brief JUPE command handler
 *
 *  Jupiter a server
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_jupe (CmdParams* cmdparams)
{
	static char infoline[255];

	SET_SEGV_LOCATION();
	ircsnprintf (infoline, 255, "[jupitered by %s]", cmdparams->source.user->nick);
	sserver_cmd (cmdparams->av[0], 1, infoline);
	nlog (LOG_NOTICE, "%s!%s@%s jupitered %s", cmdparams->source.user->nick, cmdparams->source.user->username, cmdparams->source.user->hostname, cmdparams->av[0]);
	chanalert (ns_botptr->nick, "%s jupitered %s", cmdparams->source.user->nick, cmdparams->av[0]);
	prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "%s has been jupitered", cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief DEBUG command handler
 *
 *  Set debug mode on/off 
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_set_debug (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!ircstrcasecmp(cmdparams->av[0], "ON")) {
		me.debug_mode = 1;
		globops (me.name, "\2DEBUG MODE\2 enabled by %s", cmdparams->source.user->nick);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Debug mode enabled!");
	} else if (!ircstrcasecmp(cmdparams->av[0], "OFF")) {
		me.debug_mode = 0;
		globops (me.name, "\2DEBUG MODE\2 disabled by %s", cmdparams->source.user->nick);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Debug mode disabled");
	} else {
	   	return NS_ERR_SYNTAX_ERROR;
	}
   	return NS_SUCCESS;
}

/** @brief USERDUMP command handler
 *
 *  Dump user list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_userdump (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
	UserDump ((cmdparams->ac < 1)? NULL : cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief SERVERDUMP command handler
 *
 *  Dump server list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_serverdump (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
	ServerDump ((cmdparams->ac < 1)? NULL : cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief CHANDUMP command handler
 *
 *  Dump channel list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_chandump (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
	ChanDump ((cmdparams->ac < 1)? NULL : cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief STATUS command handler
 *
 *  Display NeoStats status
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int 
ns_status (CmdParams* cmdparams)
{
	int uptime = me.now - me.t_start;

	SET_SEGV_LOCATION();
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s status:", ns_botptr->nick);
	if (uptime > 86400) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s up \2%d\2 day%s, \2%02d:%02d\2", ns_botptr->nick, uptime / 86400, (uptime / 86400 == 1) ? "" : "s", (uptime / 3600) % 24, (uptime / 60) % 60);
	} else if (uptime > 3600) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s up \2%d hour%s, %d minute%s\2", ns_botptr->nick, uptime / 3600, uptime / 3600 == 1 ? "" : "s", (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	} else if (uptime > 60) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s up \2%d minute%s, %d second%s\2", ns_botptr->nick, uptime / 60, uptime / 60 == 1 ? "" : "s", uptime % 60, uptime % 60 == 1 ? "" : "s");
	} else {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s up \2%d second%s\2", ns_botptr->nick, uptime, uptime == 1 ? "" : "s");
	}
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Sent %ld messages, %ld bytes", me.SendM, me.SendBytes);
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Received %ld messages, %ld Bytes", me.RcveM, me.RcveBytes);
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Reconnect time: %d", me.r_time);
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Requests: %d", me.requests);
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Max sockets: %d (in use: %d)", me.maxsocks, me.cursocks);
	if (me.debug_mode)
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Debugging mode enabled");
	else
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Debugging mode disabled");
	return NS_SUCCESS;
}

/** @brief LEVEL command handler
 *
 *  Display user level
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int 
ns_level (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if(cmdparams->ac < 1) {
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Your level is %d", UserLevel (cmdparams->source.user));
	} else {
		User * otheruser;
		otheruser = finduser(cmdparams->av[0]);
		if(!otheruser) {
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "User %s not found", cmdparams->av[0]);
			return NS_FAILURE;
		}
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "User level for %s is %d", otheruser->nick, UserLevel (otheruser));
	}
	return NS_SUCCESS;
}

/** @brief LOAD command handler
 *
 *  Load module
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int 
ns_load (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (load_module (cmdparams->av[0], cmdparams->source.user)) {
		chanalert (ns_botptr->nick, "%s loaded module %s", cmdparams->source.user->nick, cmdparams->av[0]);
	} else {
		chanalert (ns_botptr->nick, "%s tried to load module %s, but load failed", cmdparams->source.user->nick, cmdparams->av[0]);
	}
   	return NS_SUCCESS;
}

/** @brief UNLOAD command handler
 *
 *  Unload module
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int 
ns_unload (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (unload_module (cmdparams->av[0], cmdparams->source.user) > 0) {
		chanalert (ns_botptr->nick, "%s unloaded module %s", cmdparams->source.user->nick, cmdparams->av[0]);
	}
   	return NS_SUCCESS;
}

/** @brief RAW command handler
 *
 *  issue a RAW command
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
#ifdef USE_RAW
static int
ns_raw (CmdParams* cmdparams)
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf (cmdparams->av, cmdparams->ac, 1);
	chanalert (ns_botptr->nick, "\2RAW COMMAND\2 \2%s\2 issued a raw command!(%s)", cmdparams->source.user->nick, message);
	nlog (LOG_INFO, "RAW COMMAND %s issued a raw command!(%s)", cmdparams->source.user->nick, message);
	send_cmd ("%s", message);
	free (message);
   	return NS_SUCCESS;
}
#endif
