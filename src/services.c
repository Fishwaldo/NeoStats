/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "services.h"
#include "bans.h"

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
static int ns_bandump (CmdParams* cmdparams);
static int ns_status (CmdParams* cmdparams);
static int ns_level (CmdParams* cmdparams);
static int ns_load (CmdParams* cmdparams);
static int ns_unload (CmdParams* cmdparams);

tme me;

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
	VERSION,
	VERSION,
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
	{"BOTLIST",		list_bots,		0, 	NS_ULEVEL_ROOT,  	ns_help_botlist,	ns_help_botlist_oneline},
	{"SOCKLIST",	list_sockets,	0, 	NS_ULEVEL_ROOT,  	ns_help_socklist, 	ns_help_socklist_oneline},
	{"TIMERLIST",	list_timers,	0, 	NS_ULEVEL_ROOT,  	ns_help_timerlist, 	ns_help_timerlist_oneline},
	{"USERDUMP",	ns_userdump,	0, 	NS_ULEVEL_ROOT,  	ns_help_userdump, 	ns_help_userdump_oneline},
	{"CHANDUMP",	ns_chandump,	0, 	NS_ULEVEL_ROOT,  	ns_help_chandump, 	ns_help_chandump_oneline},
	{"SERVERDUMP",	ns_serverdump,	0, 	NS_ULEVEL_ROOT,  	ns_help_serverdump, ns_help_serverdump_oneline},
	{"BANDUMP",		ns_bandump,		0, 	NS_ULEVEL_ROOT,  	ns_help_bandump,	ns_help_bandump_oneline},
	{NULL,			NULL,			0, 	0,					NULL, 				NULL}
};

static bot_setting ns_settings[]=
{
	{"PINGTIME",		&config.pingtime,	SET_TYPE_INT,		0, 0, 	NS_ULEVEL_ADMIN, "pingtime",	NULL,	ns_help_set_pingtime, NULL, (void*)120 },
	{"VERSIONSCAN",		&config.versionscan,SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "versionscan",	NULL,	ns_help_set_versionscan, NULL, (void*)1 },
	{"SERVICECMODE",	&me.servicescmode,	SET_TYPE_STRING,	0, 64, 	NS_ULEVEL_ADMIN, "servicescmode",	NULL,	ns_help_set_servicecmode, NULL, NULL },
	{"SERVICEUMODE",	&me.servicesumode,	SET_TYPE_STRING,	0, 64, 	NS_ULEVEL_ADMIN, "servicesumode",	NULL,	ns_help_set_serviceumode, NULL, NULL },
	{"LOGLEVEL",		&config.loglevel,	SET_TYPE_INT,		1, 6, 	NS_ULEVEL_ADMIN, "loglevel",	NULL,	ns_help_set_loglevel, NULL, (void*)5 },
	{"DEBUG",			&config.debug,		SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debug, NULL, (void*)0 },
	{"DEBUGLEVEL",		&config.debuglevel,	SET_TYPE_INT,		1, 10, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debuglevel, NULL, (void*)0 },
	{"DEBUGCHAN",		&config.debugchan,	SET_TYPE_STRING,	0, MAXCHANLEN, 	NS_ULEVEL_ADMIN, "debugchan",	NULL,	ns_help_set_debugchan, NULL, (void*)"#debug" },
	{"DEBUGTOCHAN",		&config.debugtochan,SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debugtochan, NULL, (void*)0 },
	{"DEBUGMODULE",		&config.debugmodule,SET_TYPE_STRING,	0, MAX_MOD_NAME, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debugmodule, NULL, (void*)"all" },
	{NULL,				NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

Bot* ns_botptr = NULL;

BotInfo ns_botinfo = {
	"NeoStats",
	"NeoStats1",
	"Neo",
	BOT_COMMON_HOST,
	"",
	/* 0x80000000 is a "hidden" flag to identify the core bot */
	0x80000000|BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	ns_commands, 
	ns_settings,
};

/** @brief InitServices
 *
 *  init NeoStats core
 *
 * @return none
 */
void InitServices(void)
{
	/* if all bots should join the chan */
	if (GetConf ((void *) &config.allbots, CFGINT, "AllBotsJoinChan") <= 0) {
		config.allbots = 0;
	}
	/* */
	ModuleConfig(ns_settings);
}

/** @brief init_services_bot
 *
 *  init NeoStats bot
 *
 * @return none
 */
int 
init_services_bot (void)
{
	SET_SEGV_LOCATION();
	ircsnprintf (ns_botinfo.realname, MAXREALNAME, "/msg %s \2HELP\2", ns_botinfo.nick);
	if(config.onlyopers) 
		ns_botinfo.flags |= BOT_FLAG_ONLY_OPERS;
	ns_botinfo.flags |= BOT_FLAG_DEAF;
	ns_botptr = init_bot (&ns_botinfo);
	me.onchan = 1;
	SendAllModuleEvent (EVENT_ONLINE, NULL);
	RequestServerUptimes();	
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
		ns_do_exclude_add(cmdparams->source, cmdparams->av[0], cmdparams->av[1]);
	} else if (!ircstrcasecmp(cmdparams->av[0], "DEL")) {
		if (cmdparams->ac < 2) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		ns_do_exclude_del(cmdparams->source, cmdparams->av[1]);
	} else if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		ns_do_exclude_list(cmdparams->source, ns_botptr);
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
	irc_chanalert (ns_botptr, "%s requested SHUTDOWN for %s", cmdparams->source->name, cmdparams->av[cmdparams->ac-1]);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested SHUTDOWN for %s.", 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, cmdparams->av[cmdparams->ac-1]);
	irc_globops (ns_botptr, "%s", quitmsg);
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
	irc_chanalert (ns_botptr, "%s requested RELOAD for %s", cmdparams->source->name, cmdparams->av[cmdparams->ac - 1]);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested RELOAD for %s.", 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, cmdparams->av[cmdparams->ac - 1]);
	irc_globops (ns_botptr, "%s", quitmsg);
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
	ircsnprintf (infoline, 255, "[jupitered by %s]", cmdparams->source->name);
	irc_server (cmdparams->av[0], 1, infoline);
	nlog (LOG_NOTICE, "%s!%s@%s jupitered %s", cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, cmdparams->av[0]);
	irc_chanalert (ns_botptr, "%s jupitered %s", cmdparams->source->name, cmdparams->av[0]);
	irc_prefmsg(ns_botptr, cmdparams->source, "%s has been jupitered", cmdparams->av[0]);
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
#ifndef DEBUG
	if (!config.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
#endif
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
#ifndef DEBUG
	if (!config.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
#endif
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
#ifndef DEBUG
	if (!config.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
#endif
	ChanDump ((cmdparams->ac < 1)? NULL : cmdparams->av[0]);
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
ns_bandump (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
#ifndef DEBUG
	if (!config.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, "\2Error:\2 debug mode disabled");
	   	return NS_FAILURE;
	}
#endif
	BanDump ();
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
	irc_prefmsg (ns_botptr, cmdparams->source, "%s status:", ns_botptr->name);
	if (uptime > 86400) {
		irc_prefmsg (ns_botptr, cmdparams->source, "%s up \2%d\2 day%s, \2%02d:%02d\2", ns_botptr->name, uptime / 86400, (uptime / 86400 == 1) ? "" : "s", (uptime / 3600) % 24, (uptime / 60) % 60);
	} else if (uptime > 3600) {
		irc_prefmsg (ns_botptr, cmdparams->source, "%s up \2%d hour%s, %d minute%s\2", ns_botptr->name, uptime / 3600, uptime / 3600 == 1 ? "" : "s", (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	} else if (uptime > 60) {
		irc_prefmsg (ns_botptr, cmdparams->source, "%s up \2%d minute%s, %d second%s\2", ns_botptr->name, uptime / 60, uptime / 60 == 1 ? "" : "s", uptime % 60, uptime % 60 == 1 ? "" : "s");
	} else {
		irc_prefmsg (ns_botptr, cmdparams->source, "%s up \2%d second%s\2", ns_botptr->name, uptime, uptime == 1 ? "" : "s");
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "Sent %ld messages, %ld bytes", me.SendM, me.SendBytes);
	irc_prefmsg (ns_botptr, cmdparams->source, "Received %ld messages, %ld Bytes", me.RcveM, me.RcveBytes);
	irc_prefmsg (ns_botptr, cmdparams->source, "Reconnect time: %d", config.r_time);
	irc_prefmsg (ns_botptr, cmdparams->source, "Requests: %d", me.requests);
	irc_prefmsg (ns_botptr, cmdparams->source, "Max sockets: %d (in use: %d)", me.maxsocks, me.cursocks);
	if (config.debug)
		irc_prefmsg (ns_botptr, cmdparams->source, "Debugging mode enabled");
	else
		irc_prefmsg (ns_botptr, cmdparams->source, "Debugging mode disabled");
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
		irc_prefmsg (ns_botptr, cmdparams->source, "Your level is %d", UserLevel (cmdparams->source));
	} else {
		Client * otheruser;
		otheruser = find_user(cmdparams->av[0]);
		if(!otheruser) {
			irc_prefmsg (ns_botptr, cmdparams->source, "User %s not found", cmdparams->av[0]);
			return NS_FAILURE;
		}
		irc_prefmsg (ns_botptr, cmdparams->source, "User level for %s is %d", otheruser->name, UserLevel (otheruser));
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
	if (load_module (cmdparams->av[0], cmdparams->source)) {
		irc_chanalert (ns_botptr, "%s loaded module %s", cmdparams->source->name, cmdparams->av[0]);
	} else {
		irc_chanalert (ns_botptr, "%s tried to load module %s, but load failed", cmdparams->source->name, cmdparams->av[0]);
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
	if (unload_module (cmdparams->av[0], cmdparams->source) > 0) {
		irc_chanalert (ns_botptr, "%s unloaded module %s", cmdparams->source->name, cmdparams->av[0]);
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
	irc_chanalert (ns_botptr, "\2RAW COMMAND\2 \2%s\2 issued a raw command!(%s)", cmdparams->source->name, message);
	nlog (LOG_NORMAL, "RAW COMMAND %s issued a raw command!(%s)", cmdparams->source->name, message);
	send_cmd ("%s", message);
	sfree (message);
   	return NS_SUCCESS;
}
#endif
