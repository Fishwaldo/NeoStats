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
#include "cs.h"

/* Uncomment this line to enable colours in ConnectServ 
   channel messages
*/
/* #define ENABLE_COLOUR_SUPPORT */

/** local structures */
struct cs_cfg { 
	int sign_watch;
	int kill_watch;
	int mode_watch;
	int nick_watch;
	int serv_watch;
	int use_exc;
} cs_cfg;

typedef struct ModeDef {
	unsigned int mask;
	unsigned int serverflag;
}ModeDef;

#ifndef ENABLE_COLOUR_SUPPORT
static char msg_nickchange[]="\2NICK\2 %s (%s@%s) changed their nick to %s";
static char msg_signon[]="\2SIGNON\2 %s (%s@%s - %s) has signed on at %s";
static char msg_signoff[]="\2SIGNOFF\2 %s (%s@%s - %s) has signed off at %s - %s";
static char msg_localkill[]="\2LOCAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
static char msg_globalkill[]="\2GLOBAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
static char msg_serverkill[]="\2SERVER KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";  
static char msg_mode[]="\2MODE\2 %s is %s a %s (%c%c)";
static char msg_mode_serv[]="\2MODE\2 %s is %s a %s (%c%c) on %s";
static char msg_bot[]="\2BOT\2 %s is %s a Bot (%c%c)";
#else
static char msg_nickchange[]="\2\0037NICK CHANGE\2 user: \2%s\2 (%s@%s) Changed their nick to \2%s\2\003"; 
static char msg_signon[]="\2\0034SIGNED ON\2 user: \2%s\2 (%s@%s - %s) at: \2%s\2\003";
static char msg_signoff[]="\2\0033SIGNED OFF\2 user: %s (%s@%s - %s) at: %s - %s\003";
static char msg_localkill[]="\2LOCAL KILL\2 user: \2%s\2 (%s@%s) was Killed by: \2%s\2 - Reason sighted: \2%s\2";
static char msg_globalkill[]="\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2\003";
static char msg_serverkill[]="\2SERVER KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2";
static char msg_mode[]="\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c)\003";
static char msg_mode_serv[]="\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c) on \2%s\2\003";
static char msg_bot[]="\2\00313%s\2 is \2%s\2 a \2Bot\2 (%c%c)\003";
#endif

/** Bot command function prototypes */
static int cs_event_signon (CmdParams* cmdparams);
static int cs_event_umode (CmdParams* cmdparams);
static int cs_event_smode (CmdParams* cmdparams);
static int cs_event_quit (CmdParams* cmdparams);
static int cs_event_kill (CmdParams* cmdparams);
static int cs_event_nick (CmdParams* cmdparams);
static int cs_event_server (CmdParams* cmdparams);
static int cs_event_squit (CmdParams* cmdparams);

static int cs_set_exclusions_cb (CmdParams* cmdparams);
static int cs_set_sign_watch_cb (CmdParams* cmdparams);
static int cs_set_kill_watch_cb (CmdParams* cmdparams);
static int cs_set_mode_watch_cb (CmdParams* cmdparams);
static int cs_set_nick_watch_cb (CmdParams* cmdparams);
static int cs_set_serv_watch_cb (CmdParams* cmdparams);

/** Bot pointer */
static Bot *cs_bot;

/** Module pointer */
static Module* cs_module;

/** Copyright info */
const char *cs_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"ConnectServ",
	"Connection monitoring service",
	cs_copyright,
	cs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Bot setting table */
static bot_setting cs_settings[]=
{
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "SignWatch",	NULL,	cs_help_set_signwatch, cs_set_sign_watch_cb, (void*)1 },
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "KillWatch",	NULL,	cs_help_set_killwatch, cs_set_kill_watch_cb, (void*)1 },
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ModeWatch",	NULL,	cs_help_set_modewatch, cs_set_mode_watch_cb, (void*)1 },
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "NickWatch",	NULL,	cs_help_set_nickwatch, cs_set_nick_watch_cb, (void*)1 },
	{"SERVWATCH",	&cs_cfg.serv_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ServWatch",	NULL,	cs_help_set_servwatch, cs_set_serv_watch_cb, (void*)1 },
	{"EXCLUSIONS",	&cs_cfg.use_exc,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "Exclusions",	NULL,	cs_help_set_exclusions, cs_set_exclusions_cb, (void*)1 },
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/** BotInfo */
static BotInfo cs_botinfo = 
{
	"ConnectServ", 
	"ConnectServ1", 
	"CS", 
	BOT_COMMON_HOST, 
	"Connection monitoring service", 	
	BOT_FLAG_SERVICEBOT|BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
	NULL, 
	cs_settings,
};

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_SIGNON,	cs_event_signon,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_UMODE,	cs_event_umode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SMODE,	cs_event_smode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_QUIT,	cs_event_quit,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_KILL,	cs_event_kill,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_NICK,	cs_event_nick,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SERVER,	cs_event_server,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SQUIT,	cs_event_squit,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_NULL,	NULL}
};

ModeDef OperUmodes[]=
{
	{UMODE_NETADMIN,	0},
	{UMODE_TECHADMIN,	0},
	{UMODE_ADMIN,		1},
	{UMODE_COADMIN,		1},
	{UMODE_SADMIN,		0},
	{UMODE_OPER,		1},
	{UMODE_LOCOP,		1},
	{UMODE_SERVICES,	0},
	{0, 0},
};

ModeDef OperSmodes[]=
{
	{SMODE_NETADMIN,	0},
	{SMODE_CONETADMIN,	0},
	{SMODE_TECHADMIN,	0},
	{SMODE_COTECHADMIN, 0},
	{SMODE_ADMIN,		1},
	{SMODE_COADMIN,		1},
	{SMODE_GUESTADMIN,	1},
	{0, 0},
};

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit (Module *mod_ptr)
{
	cs_module = mod_ptr;
	ModuleConfig (cs_settings);
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch (void)
{
	cs_bot = init_bot (&cs_botinfo);
	if (!cs_bot) {
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return none
 */

void ModFini (void)
{
}

/* 
 * Echo signon
 */
static int cs_event_signon (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_cfg.sign_watch) {
		return NS_SUCCESS;
	}
	/* Print Connection Notice */
	irc_chanalert(cs_bot, msg_signon,
		cmdparams->source->name, cmdparams->source->user->username, 
		cmdparams->source->user->hostname, cmdparams->source->info,
		cmdparams->source->user->server->name);
	return NS_SUCCESS;
}

/* 
 * Echo signoff
 */
static int cs_event_quit (CmdParams* cmdparams)
{
	char *cmd, *lcl, *QuitMsg, *KillMsg;
	char **Quit;
	char **Local;
	int QuitCount = 0;
	int LocalCount = 0;

	SET_SEGV_LOCATION();
	cmd = sstrdup(recbuf);
	lcl = sstrdup(recbuf);
	QuitCount = split_buf(cmd, &Quit, 0);
	QuitMsg = joinbuf(Quit, QuitCount, 2);
	/* Local Kill Watch For Signoff */
	if (cs_cfg.kill_watch) {
		if (strstr(cmd, "Local kill by") && strstr(cmd, "[")
		    && strstr(cmd, "]")) {
			LocalCount = split_buf(lcl, &Local, 0);
			KillMsg = joinbuf(Local, LocalCount, 7);
			irc_chanalert(cs_bot, msg_localkill,
				  cmdparams->source->name, cmdparams->source->user->username, 
				  cmdparams->source->user->hostname,
				  Local[6], KillMsg);
			sfree(KillMsg);
			sfree(QuitMsg);
			sfree(cmd);
			sfree(lcl);
			return NS_SUCCESS;
		}
	}
	/* Print Disconnection Notice */
	if (cs_cfg.sign_watch) {
		irc_chanalert(cs_bot, msg_signoff,
			  cmdparams->source->name, cmdparams->source->user->username, 
			  cmdparams->source->user->hostname, cmdparams->source->info,
			  cmdparams->source->user->server->name, QuitMsg);
	}
	sfree(QuitMsg);
	sfree(cmd);
	sfree(lcl);
	sfree(Quit);
	return NS_SUCCESS;
}

/* 
 * report mode change
 */

static int cs_report_mode (const char* modedesc, int serverflag, Client * u, int mask, int add, char mode)
{
	if(serverflag) {
		irc_chanalert(cs_bot, msg_mode_serv, u->name, 
			add?"now":"no longer", 
			modedesc,
			add?'+':'-',
			mode, u->user->server->name);
	} else {
		irc_chanalert(cs_bot, msg_mode, u->name, 
			add?"now":"no longer", 
			modedesc,
			add?'+':'-',
			mode);
	}
	return NS_SUCCESS;
}

/* 
 * Echo oper mode changes
 */
static int cs_event_umode (CmdParams* cmdparams)
{
	int mask;
	int add = 1;
	char *modes;
	ModeDef* def;

	SET_SEGV_LOCATION();
	if (!cs_cfg.mode_watch) {
		return -1;
	}
	modes = cmdparams->param;
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			mask = GetUmodeMask (*modes);
			if (mask == UMODE_BOT) {
				irc_chanalert (cs_bot, msg_bot, cmdparams->source->name, 
					add?"now":"no longer", add?'+':'-', *modes);			
			} else {
				def = OperUmodes;
				while(def->mask) {
					if(def->mask == mask) 
					{
						cs_report_mode(GetUmodeDesc(def->mask), def->serverflag, cmdparams->source, mask, add, *modes);
						break;
					}
					def ++;
				}
			}
			break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/* smode support */
static int cs_event_smode (CmdParams* cmdparams)
{
	int mask;
	int add = 1;
	char *modes;
	ModeDef* def;

	SET_SEGV_LOCATION();
	if (!cs_cfg.mode_watch) {
		return -1;
	}
	modes = cmdparams->param;
	while (*modes) {
		switch (*modes) {
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			default:
				mask = GetSmodeMask (*modes);
				def = OperSmodes;
				while(def->mask) {
					if(def->mask == mask) 
					{
						cs_report_mode(GetSmodeDesc(def->mask), def->serverflag, cmdparams->source, mask, add, *modes);
						break;
					}
					def ++;
				}

				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/* 
 * Echo kills
 */
static int cs_event_kill (CmdParams* cmdparams)
{
	char *cmd, *GlobalMsg;
	char **Kill;
	int KillCount = 0;

	SET_SEGV_LOCATION();
	if (!cs_cfg.kill_watch) {
		return NS_SUCCESS;
	}
	cmd = sstrdup(recbuf);
	KillCount = split_buf(cmd, &Kill, 0);
	GlobalMsg = joinbuf(Kill, KillCount, 4);
	if (find_user(Kill[2])) {
		/* it was a User who was killed */
		irc_chanalert(cs_bot, msg_globalkill,
			cmdparams->source->name, cmdparams->source->user->username, 
			cmdparams->source->user->hostname,
			Kill[0], GlobalMsg);
	} else if (find_server(Kill[2])) {
		irc_chanalert(cs_bot, msg_serverkill,
			cmdparams->source->name, cmdparams->source->user->username, 
			cmdparams->source->user->hostname,
			Kill[0], GlobalMsg);
	}
	sfree(cmd);
	sfree(GlobalMsg);
	return NS_SUCCESS;
}

/* 
 * Echo nick changes
 */
static int cs_event_nick (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_cfg.nick_watch) {
		return NS_SUCCESS;
	}
	irc_chanalert(cs_bot, msg_nickchange, cmdparams->param, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->name);
	return NS_SUCCESS;
}

static int cs_event_server (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_cfg.serv_watch) {
		return NS_SUCCESS;
	}
	irc_chanalert (cs_bot, "\2SERVER\2 %s has joined the network at %s",
		cmdparams->source->name, cmdparams->source->uplink);
	return NS_SUCCESS;
}

static int cs_event_squit (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_cfg.serv_watch) {
		return NS_SUCCESS;
	}
	irc_chanalert (cs_bot, "\2SERVER\2 %s has left the network at %s for %s",
		cmdparams->source->name, cmdparams->source->uplink, 
		cmdparams->param ? cmdparams->param : "");
	return NS_SUCCESS;
}

static int cs_set_exclusions_cb (CmdParams* cmdparams)
{
	SetAllEventFlags (EVENT_FLAG_USE_EXCLUDE, cs_cfg.use_exc);
	return NS_SUCCESS;
}

static int cs_set_sign_watch_cb (CmdParams* cmdparams)
{
	if (cs_cfg.sign_watch) {
		EnableEvent (EVENT_SIGNON);
		EnableEvent (EVENT_QUIT);
	} else {
		DisableEvent (EVENT_SIGNON);
		DisableEvent (EVENT_QUIT);
	}
	return NS_SUCCESS;
}

static int cs_set_kill_watch_cb (CmdParams* cmdparams)
{
	if (cs_cfg.kill_watch) {
		EnableEvent (EVENT_KILL);
	} else {
		DisableEvent (EVENT_KILL);
	}
	return NS_SUCCESS;
}

static int cs_set_mode_watch_cb (CmdParams* cmdparams)
{
	if (cs_cfg.mode_watch) {
		EnableEvent (EVENT_UMODE);
		EnableEvent (EVENT_SMODE);
	} else {
		DisableEvent (EVENT_UMODE);
		DisableEvent (EVENT_SMODE);
	}
	return NS_SUCCESS;
}

static int cs_set_nick_watch_cb (CmdParams* cmdparams)
{
	if (cs_cfg.nick_watch) {
		EnableEvent (EVENT_NICK);
	} else {
		DisableEvent (EVENT_NICK);
	}
	return NS_SUCCESS;
}

static int cs_set_serv_watch_cb (CmdParams* cmdparams)
{
	if (cs_cfg.serv_watch) {
		EnableEvent (EVENT_SERVER);
		EnableEvent (EVENT_SQUIT);
	} else {
		DisableEvent (EVENT_SERVER);
		DisableEvent (EVENT_SQUIT);
	}
	return NS_SUCCESS;
}
