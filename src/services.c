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
#include "exclude.h"
#include "services.h"
#include "bans.h"
#include "rtaserv.h"

static int ns_cmd_shutdown (CmdParams* cmdparams);
static int ns_cmd_reload (CmdParams* cmdparams);
static int ns_cmd_jupe (CmdParams* cmdparams);
static int ns_cmd_exclude (CmdParams* cmdparams);
#ifdef USE_RAW
static int ns_cmd_raw (CmdParams* cmdparams);
#endif
static int ns_cmd_userlist (CmdParams* cmdparams);
static int ns_cmd_serverlist (CmdParams* cmdparams);
static int ns_cmd_chanlist (CmdParams* cmdparams);
static int ns_cmd_banlist (CmdParams* cmdparams);
static int ns_cmd_status (CmdParams* cmdparams);
static int ns_cmd_level (CmdParams* cmdparams);
static int ns_cmd_load (CmdParams* cmdparams);
static int ns_cmd_unload (CmdParams* cmdparams);

tme me;

static char quitmsg[BUFSIZE];

static const char *ns_about[] = {
	"\2NeoStats\2 statistical services",
	NULL
};

/** Copyright info */
static const char *ns_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
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

/** Fake Module pointer for run level code */
Module ns_module = {
	&ns_module_info
};

/** Bot comand table */
static bot_cmd ns_commands[]=
{
	{"LEVEL",		ns_cmd_level,		0, 	0,					ns_help_level, 		ns_help_level_oneline},
	{"STATUS",		ns_cmd_status,		0, 	0,					ns_help_status, 	ns_help_status_oneline},
	{"SHUTDOWN",	ns_cmd_shutdown,	1, 	NS_ULEVEL_ADMIN, 	ns_help_shutdown, 	ns_help_shutdown_oneline},
	{"RELOAD",		ns_cmd_reload,		1, 	NS_ULEVEL_ADMIN, 	ns_help_reload,		ns_help_reload_oneline},
	{"MODLIST",		ns_cmd_modlist,		0, 	NS_ULEVEL_ADMIN,  	ns_help_modlist, 	ns_help_modlist_oneline},
	{"LOAD",		ns_cmd_load,		1, 	NS_ULEVEL_ADMIN, 	ns_help_load, 		ns_help_load_oneline},
	{"UNLOAD",		ns_cmd_unload,		1,	NS_ULEVEL_ADMIN, 	ns_help_unload, 	ns_help_unload_oneline},
	{"JUPE",		ns_cmd_jupe,		1, 	NS_ULEVEL_ADMIN, 	ns_help_jupe,		ns_help_jupe_oneline},
	{"EXCLUDE",		ns_cmd_exclude,		1,	NS_ULEVEL_ADMIN,	ns_help_exclude,	ns_help_exclude_oneline},
#ifdef USE_RAW																
	{"RAW",			ns_cmd_raw,			0, 	NS_ULEVEL_ADMIN, 	ns_help_raw, 		ns_help_raw_oneline},
#endif																	
	{"BOTLIST",		ns_cmd_botlist,		0, 	NS_ULEVEL_ROOT,  	ns_help_botlist,	ns_help_botlist_oneline},
	{"SOCKLIST",	ns_cmd_socklist,	0, 	NS_ULEVEL_ROOT,  	ns_help_socklist, 	ns_help_socklist_oneline},
	{"TIMERLIST",	ns_cmd_timerlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_timerlist, 	ns_help_timerlist_oneline},
	{"USERLIST",	ns_cmd_userlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_userlist, 	ns_help_userlist_oneline},
	{"CHANLIST",	ns_cmd_chanlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_chanlist, 	ns_help_chanlist_oneline},
	{"SERVERLIST",	ns_cmd_serverlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_serverlist, ns_help_serverlist_oneline},
	{"BANLIST",		ns_cmd_banlist,		0, 	NS_ULEVEL_ROOT,  	ns_help_banlist,	ns_help_banlist_oneline},
	{NULL,			NULL,			0, 	0,					NULL, 				NULL}
};

/** Bot setting table */
static bot_setting ns_settings[]=
{
	{"JOINSERVICESCHAN",&nsconfig.joinserviceschan, SET_TYPE_BOOLEAN,		0, 0, 	NS_ULEVEL_ADMIN, "joinserviceschan",	NULL,	ns_help_set_joinserviceschan, NULL, (void*)1 },
	{"PINGTIME",		&nsconfig.pingtime,	SET_TYPE_INT,		0, 0, 	NS_ULEVEL_ADMIN, "pingtime",	NULL,	ns_help_set_pingtime, NULL, (void*)120 },
	{"VERSIONSCAN",		&nsconfig.versionscan,SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "versionscan",	NULL,	ns_help_set_versionscan, NULL, (void*)1 },
	{"SERVICECMODE",	&me.servicescmode,	SET_TYPE_STRING,	0, 64, 	NS_ULEVEL_ADMIN, "servicescmode",	NULL,	ns_help_set_servicecmode, NULL, NULL },
	{"SERVICEUMODE",	&me.servicesumode,	SET_TYPE_STRING,	0, 64, 	NS_ULEVEL_ADMIN, "servicesumode",	NULL,	ns_help_set_serviceumode, NULL, NULL },
	{"CMDCHAR",			&nsconfig.cmdchar,	SET_TYPE_STRING,	0, 2, 	NS_ULEVEL_ADMIN, "cmdchar",	NULL,	ns_help_set_cmdchar, NULL, (void*)"!" },
	{"CMDREPORT",		&nsconfig.cmdreport,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "cmdreport",	NULL,	ns_help_set_cmdreport, NULL, (void*)1 },
	{"LOGLEVEL",		&nsconfig.loglevel,	SET_TYPE_INT,		1, 6, 	NS_ULEVEL_ADMIN, "loglevel",	NULL,	ns_help_set_loglevel, NULL, (void*)5 },
	{"DEBUG",			&nsconfig.debug,		SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debug, NULL, (void*)0 },
	{"DEBUGLEVEL",		&nsconfig.debuglevel,	SET_TYPE_INT,		1, 10, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debuglevel, NULL, (void*)0 },
	{"DEBUGCHAN",		&nsconfig.debugchan,	SET_TYPE_STRING,	0, MAXCHANLEN, 	NS_ULEVEL_ADMIN, "debugchan",	NULL,	ns_help_set_debugchan, NULL, (void*)"#debug" },
	{"DEBUGTOCHAN",		&nsconfig.debugtochan,SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debugtochan, NULL, (void*)0 },
	{"DEBUGMODULE",		&nsconfig.debugmodule,SET_TYPE_STRING,	0, MAX_MOD_NAME, 	NS_ULEVEL_ADMIN, NULL,	NULL,	ns_help_set_debugmodule, NULL, (void*)"all" },
	{NULL,				NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/** Bot pointer */
Bot* ns_botptr = NULL;

BotInfo ns_botinfo = {
	"NeoStats",
	"NeoStats1",
	"Neo",
	BOT_COMMON_HOST,
	"",
	/* 0x80000000 is a "hidden" flag to identify the core bot */
	0x80000000|BOT_FLAG_ONLY_OPERS|BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF,
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
	if (GetConf ((void *) &nsconfig.allbots, CFGINT, "AllBotsJoinChan") <= 0) {
		nsconfig.allbots = 0;
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
	if(nsconfig.onlyopers) 
		ns_botinfo.flags |= BOT_FLAG_ONLY_OPERS;
	ns_module.insynch = 1;
	ns_botptr = AddBot (&ns_botinfo);
	ns_module.synched = 1;
	me.synched = 1;
	SynchAllModules ();
	RequestServerUptimes();	
	rtaserv_init ();
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
ns_cmd_exclude (CmdParams* cmdparams) 
{
	if (!ircstrcasecmp(cmdparams->av[0], "ADD")) {
		return ns_cmd_exclude_add(cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "DEL")) {
		return ns_cmd_exclude_del(cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		return ns_cmd_exclude_list(cmdparams);
	}
	return NS_ERR_SYNTAX_ERROR;
}
/** @brief SHUTDOWN command handler
 *
 *  Shutdown NeoStats
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_cmd_shutdown (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	irc_chanalert (ns_botptr, _("%s requested SHUTDOWN for %s"), cmdparams->source->name, cmdparams->av[cmdparams->ac-1]);
	ircsnprintf (quitmsg, BUFSIZE, _("%s [%s](%s) requested SHUTDOWN for %s."), 
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
ns_cmd_reload (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	irc_chanalert (ns_botptr, _("%s requested RELOAD for %s"), cmdparams->source->name, cmdparams->av[cmdparams->ac - 1]);
	ircsnprintf (quitmsg, BUFSIZE, _("%s [%s](%s) requested RELOAD for %s."), 
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
ns_cmd_jupe (CmdParams* cmdparams)
{
	static char infoline[255];

	SET_SEGV_LOCATION();
	ircsnprintf (infoline, 255, "[jupitered by %s]", cmdparams->source->name);
	irc_server (cmdparams->av[0], 1, infoline);
	nlog (LOG_NOTICE, "%s!%s@%s jupitered %s", cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, cmdparams->av[0]);
	irc_chanalert (ns_botptr, _("%s jupitered %s"), cmdparams->source->name, cmdparams->av[0]);
	irc_prefmsg(ns_botptr, cmdparams->source, __("%s has been jupitered", cmdparams->source), cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief USERLIST command handler
 *
 *  Dump user list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_cmd_userlist (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
#ifndef DEBUG
	if (!nsconfig.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("\2Error:\2 debug mode disabled", cmdparams->source));
	   	return NS_FAILURE;
	}
#endif
	UserDump (cmdparams, (cmdparams->ac < 1)? NULL : cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief SERVERLIST command handler
 *
 *  Dump server list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_cmd_serverlist (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
#ifndef DEBUG
	if (!nsconfig.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("\2Error:\2 debug mode disabled", cmdparams->source));
	   	return NS_FAILURE;
	}
#endif
	ServerDump ((cmdparams->ac < 1)? NULL : cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief CHANLIST command handler
 *
 *  Dump channel list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_cmd_chanlist (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
#ifndef DEBUG
	if (!nsconfig.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("\2Error:\2 debug mode disabled", cmdparams->source));
	   	return NS_FAILURE;
	}
#endif
	ListChannels (cmdparams, (cmdparams->ac < 1)? NULL : cmdparams->av[0]);
   	return NS_SUCCESS;
}

/** @brief USERLIST command handler
 *
 *  Dump user list
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */
static int
ns_cmd_banlist (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
#ifndef DEBUG
	if (!nsconfig.debug) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("\2Error:\2 debug mode disabled", cmdparams->source));
	   	return NS_FAILURE;
	}
#endif
	ListBans ();
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
ns_cmd_status (CmdParams* cmdparams)
{
	time_t uptime = me.now - me.t_start;

	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("%s status:", cmdparams->source), ns_botptr->name);
	if (uptime > 86400) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("%s up \2%ld\2 day%s, \2%02ld:%02ld\2", cmdparams->source), ns_botptr->name, uptime / 86400, (uptime / 86400 == 1) ? "" : "s", (uptime / 3600) % 24, (uptime / 60) % 60);
	} else if (uptime > 3600) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("%s up \2%ld hour%s, %ld minute%s\2", cmdparams->source), ns_botptr->name, uptime / 3600, uptime / 3600 == 1 ? "" : "s", (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	} else if (uptime > 60) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("%s up \2%ld minute%s, %ld second%s\2", cmdparams->source), ns_botptr->name, uptime / 60, uptime / 60 == 1 ? "" : "s", uptime % 60, uptime % 60 == 1 ? "" : "s");
	} else {
		irc_prefmsg (ns_botptr, cmdparams->source, __("%s up \2%d second%s\2", cmdparams->source), ns_botptr->name, (int)uptime, uptime == 1 ? "" : "s");
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("Sent %ld messages, %ld bytes", cmdparams->source), me.SendM, me.SendBytes);
	irc_prefmsg (ns_botptr, cmdparams->source, __("Received %ld messages, %ld Bytes", cmdparams->source), me.RcveM, me.RcveBytes);
	irc_prefmsg (ns_botptr, cmdparams->source, __("Reconnect time: %d", cmdparams->source), nsconfig.r_time);
	irc_prefmsg (ns_botptr, cmdparams->source, __("Requests: %d",cmdparams->source), me.requests);
	irc_prefmsg (ns_botptr, cmdparams->source, __("Max sockets: %d (in use: %d)", cmdparams->source), me.maxsocks, me.cursocks);
	if (nsconfig.debug)
		irc_prefmsg (ns_botptr, cmdparams->source, __("Debugging mode enabled", cmdparams->source));
	else
		irc_prefmsg (ns_botptr, cmdparams->source, __("Debugging mode disabled", cmdparams->source));
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
ns_cmd_level (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if(cmdparams->ac < 1) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("Your level is %d", cmdparams->source), UserLevel (cmdparams->source));
	} else {
		Client * otheruser;
		otheruser = find_user(cmdparams->av[0]);
		if(!otheruser) {
			irc_prefmsg (ns_botptr, cmdparams->source, __("User %s not found", cmdparams->source), cmdparams->av[0]);
			return NS_FAILURE;
		}
		irc_prefmsg (ns_botptr, cmdparams->source, __("User level for %s is %d", cmdparams->source), otheruser->name, UserLevel (otheruser));
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
ns_cmd_load (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (load_module (cmdparams->av[0], cmdparams->source)) {
		irc_chanalert (ns_botptr, _("%s loaded module %s"), cmdparams->source->name, cmdparams->av[0]);
	} else {
		irc_chanalert (ns_botptr, _("%s tried to load module %s, but load failed"), cmdparams->source->name, cmdparams->av[0]);
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
ns_cmd_unload (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (unload_module (cmdparams->av[0], cmdparams->source) > 0) {
		irc_chanalert (ns_botptr, _("%s unloaded module %s"), cmdparams->source->name, cmdparams->av[0]);
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
ns_cmd_raw (CmdParams* cmdparams)
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf (cmdparams->av, cmdparams->ac, 1);
	irc_chanalert (ns_botptr, _("\2RAW COMMAND\2 \2%s\2 issued a raw command!(%s)"), cmdparams->source->name, message);
	nlog (LOG_NORMAL, "RAW COMMAND %s issued a raw command!(%s)", cmdparams->source->name, message);
	send_cmd ("%s", message);
	ns_free (message);
   	return NS_SUCCESS;
}
#endif
