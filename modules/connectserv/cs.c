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

#ifndef ENABLE_COLOUR_SUPPORT
static char msg_nickchange[]="\2NICK\2 %s (%s@%s) changed their nick to %s";
static char msg_signon[]="\2SIGNON\2 %s (%s@%s - %s) has signed on at %s";
static char msg_signoff[]="\2SIGNOFF\2 %s (%s@%s - %s) has signed off at %s - %s";
static char msg_localkill[]="\2LOCAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
static char msg_globalkill[]="\2GLOBAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
static char msg_serverkill[]="\2SERVER KILL\2 %s (%s@%s) was killed by the server %s - Reason sighted: \2%s\2";  
static char msg_mode[]="\2MODE\2 %s is %s a %s (%c%c)";
static char msg_mode_serv[]="\2MODE\2 %s is %s a %s (%c%c) on %s";
static char msg_bot[]="\2BOT\2 %s is %s a Bot (%c%c)";
#else
static char msg_nickchange[]="\2\0037NICK CHANGE\2 user: \2%s\2 (%s@%s) Changed their nick to \2%s\2\003"; 
static char msg_signon[]="\2\0034SIGNED ON\2 user: \2%s\2 (%s@%s - %s) at: \2%s\2\003";
static char msg_signoff[]="\2\0033SIGNED OFF\2 user: %s (%s@%s - %s) at: %s - %s\003";
static char msg_localkill[]="\2LOCAL KILL\2 user: \2%s\2 (%s@%s) was Killed by: \2%s\2 - Reason sighted: \2%s\2";
static char msg_globalkill[]="\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2\003";
static char msg_serverkill[]="\2SERVER KILL\2 user: \2%s\2 (%s@%s) was Killed by the Server \2%s\2 - Reason sighted: \2%s\2";
static char msg_mode[]="\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c)\003";
static char msg_mode_serv[]="\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c) on \2%s\2\003";
static char msg_bot[]="\2\00313%s\2 is \2%s\2 a \2Bot\2 (%c%c)\003";
#endif

static int cs_event_online(CmdParams* cmdparams);
static int cs_event_signon(CmdParams* cmdparams);
static int cs_event_umode(CmdParams* cmdparams);
static int cs_event_smode(CmdParams* cmdparams);
static int cs_event_quit(CmdParams* cmdparams);
static int cs_event_kill(CmdParams* cmdparams);
static int cs_event_nick(CmdParams* cmdparams);
static int cs_event_server(CmdParams* cmdparams);
static int cs_event_squit(CmdParams* cmdparams);

struct cs_cfg { 
	int sign_watch;
	int kill_watch;
	int mode_watch;
	int nick_watch;
	int serv_watch;
	int use_exc;
} cs_cfg;

static Bot *cs_bot;

static Module* cs_module;

const char *cs_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

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

static bot_setting cs_settings[]=
{
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "SignWatch",	NULL,	cs_help_set_signwatch, NULL, (void*)1 },
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "KillWatch",	NULL,	cs_help_set_killwatch, NULL, (void*)1 },
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ModeWatch",	NULL,	cs_help_set_modewatch, NULL, (void*)1 },
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "NickWatch",	NULL,	cs_help_set_nickwatch, NULL, (void*)1 },
	{"SERVWATCH",	&cs_cfg.serv_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ServWatch",	NULL,	cs_help_set_servwatch, NULL, (void*)1 },
	{"EXCLUSIONS",	&cs_cfg.use_exc,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "Exclusions",	NULL,	cs_help_set_exclusions, NULL, (void*)1 },
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

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

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	cs_event_online},
	{EVENT_SIGNON,	cs_event_signon},
	{EVENT_UMODE,	cs_event_umode},
	{EVENT_SMODE,	cs_event_smode},
	{EVENT_QUIT,	cs_event_quit},
	{EVENT_KILL,	cs_event_kill},
	{EVENT_NICK,	cs_event_nick},
	{EVENT_SERVER,	cs_event_server},
	{EVENT_SQUIT,	cs_event_squit},
	{EVENT_NULL,	NULL}
};

typedef struct ModeDef {
	unsigned int mask;
	unsigned int serverflag;
}ModeDef;

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

int ModInit(Module* mod_ptr)
{
	cs_module = mod_ptr;
	ModuleConfig(cs_settings);
	return 1;
}

void ModFini()
{

};

static int cs_event_online(CmdParams* cmdparams)
{
	cs_bot = init_bot (&cs_botinfo);
	return 1;
};

/* 
 * Echo signon
 */
static int cs_event_signon(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.sign_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	if (IsMe(cmdparams->source)) {
		/* its me, forget it */
		return 1;
	}
	/* Print Connection Notice */
	irc_chanalert(cs_bot, msg_signon,
		cmdparams->source->name, cmdparams->source->user->username, 
		cmdparams->source->user->hostname, cmdparams->source->info,
		cmdparams->source->user->server->name);
	return 1;
}

/* 
 * Echo signoff
 */
static int cs_event_quit(CmdParams* cmdparams)
{
	char *cmd, *lcl, *QuitMsg, *KillMsg;
	char **Quit;
	char **Local;
	int QuitCount = 0;
	int LocalCount = 0;

	SET_SEGV_LOCATION();
	if (!cs_module->synched) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	if (IsMe(cmdparams->source)) {
		/* its me, forget it */
		return 1;
	}
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
			return 1;
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
	return 1;
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
	return 1;
}

/* 
 * Echo oper mode changes
 */
static int cs_event_umode(CmdParams* cmdparams)
{
	int mask;
	int add = 1;
	char *modes;
	ModeDef* def;

	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.mode_watch) {
		return -1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	if (IsMe(cmdparams->source)) {
		/* its me, forget it */
		return 1;
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
	return 1;
}

/* smode support */
static int cs_event_smode(CmdParams* cmdparams)
{
	int mask;
	int add = 1;
	char *modes;
	ModeDef* def;

	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.mode_watch) {
		return -1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	if (IsMe(cmdparams->source)) {
		/* its me, forget it */
		return 1;
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
	return 1;
}

/* 
 * Echo kills
 */
static int cs_event_kill(CmdParams* cmdparams)
{
	char *cmd, *GlobalMsg;
	char **Kill;
	int KillCount = 0;

	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.kill_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	if (IsMe(cmdparams->source)) {
		/* its me, forget it */
		return 1;
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
	return 1;
}

/* 
 * Echo nick changes
 */
static int cs_event_nick(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.nick_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	if (IsMe(cmdparams->source)) {
		/* its me, forget it */
		return 1;
	}
	irc_chanalert(cs_bot, msg_nickchange, cmdparams->param, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->name);
	return 1;
}

static int cs_event_server(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.serv_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	irc_chanalert (cs_bot, "\2SERVER\2 %s has joined the network at %s",
		cmdparams->source->name, cmdparams->source->uplink);
	return 1;
}

static int cs_event_squit(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_module->synched || !cs_cfg.serv_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source)) {
		return 1;
	}
	irc_chanalert (cs_bot, "\2SERVER\2 %s has left the network at %s for %s",
		cmdparams->source->name, cmdparams->source->uplink, 
		cmdparams->param ? cmdparams->param : "");
	return 1;
}
