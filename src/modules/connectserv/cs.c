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
#include "cs.h"

/* Uncomment this line to disable colours in ConnectServ 
   channel messages
*/
/* #define DISABLE_COLOUR_SUPPORT */

static const char mode_netadmin[]="network administrator";
static const char mode_conetadmin[]="co network administrator";
static const char mode_techadmin[]="network technical administrator";
static const char mode_cotechadmin[]="network co technical administrator";
static const char mode_serveradmin[]="server administrator";
static const char mode_coserveradmin[]="co server administrator";
static const char mode_guestadmin[]="guest administrator";
static const char mode_servicesadmin[]="services administrator";
static const char mode_globop[]="global operator";
static const char mode_locop[]="local operator";
static const char mode_netservice[]="network service";
static const char mode_bot[]="bot";

#ifdef DISABLE_COLOUR_SUPPORT
static char msg_nickchange[]="\2NICK\2 %s (%s@%s) changed their nick to %s";
static char msg_signon[]="\2SIGNON\2 %s (%s@%s - %s) has signed on at %s";
static char msg_signoff[]="\2SIGNOFF\2 %s (%s@%s - %s) has signed off at %s - %s";
static char msg_localkill[]="\2LOCAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
static char msg_globalkill[]="\2GLOBAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
static char msg_serverkill[]="\2SERVER KILL\2 %s (%s@%s) was killed by the server %s - Reason sighted: \2%s\2";  
static char msg_mode[]="\2MODE\2 %s is %s a %s (%c%c)";
static char msg_mode_serv[]="\2MODE\2 %s is %s a %s (%c%c) on %s";
#ifdef UMODE_CH_BOT
static char msg_bot[]="\2BOT\2 %s is %s a Bot (%c%c)";
#endif
#else
static char msg_nickchange[]="\2\0037Nick Change\2 user: \2%s\2 (%s@%s) Changed their nick to \2%s\2\003"; 
static char msg_signon[]="\2\0034SIGNED ON\2 user: \2%s\2 (%s@%s - %s) at: \2%s\2\003";
static char msg_signoff[]="\2\0033Signed Off\2 user: %s (%s@%s - %s) at: %s - %s\003";
static char msg_localkill[]="\2LOCAL KILL\2 user: \2%s\2 (%s@%s) was Killed by: \2%s\2 - Reason sighted: \2%s\2";
static char msg_globalkill[]="\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2\003";
static char msg_serverkill[]="\2SERVER KILL\2 user: \2%s\2 (%s@%s) was Killed by the Server \2%s\2 - Reason sighted: \2%s\2";
static char msg_mode[]="\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c)\003";
static char msg_mode_serv[]="\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c) on \2%s\2\003";
#ifdef UMODE_CH_BOT
static char msg_bot[]="\2\00313%s\2 is \2%s\2 a \2Bot\2 (%c%c)\003";
#endif
#endif

static int cs_event_online(CmdParams* cmdparams);
static int cs_event_signon(CmdParams* cmdparams);
static int cs_event_umode(CmdParams* cmdparams);
#ifdef GOTUSERSMODES
static int cs_event_smode(CmdParams* cmdparams);
#endif
static int cs_event_quit(CmdParams* cmdparams);
static int cs_event_kill(CmdParams* cmdparams);
static int cs_event_nick(CmdParams* cmdparams);
static int cs_event_server(CmdParams* cmdparams);
static int cs_event_squit(CmdParams* cmdparams);

static void LoadConfig(void);

struct cs_cfg { 
	int sign_watch;
	int kill_watch;
	int mode_watch;
	int nick_watch;
	int serv_watch;
	int use_exc;
} cs_cfg;

static int cs_online = 0;
static Bot *cs_bot;
static BotInfo cs_botinfo = 
{
#ifdef HYBRID7
	"CS", 
#else
	"ConnectServ", 
#endif
	"ConnectServ", 
	"CS", 
	"", 
	"Connection monitoring service", 	
};

static Module* cs_module;

ModuleInfo module_info = {
	"ConnectServ",
	"Connection monitoring service", 	
	ns_copyright,
	cs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

static bot_cmd cs_commands[]=
{
	{NULL,		NULL,		0, 	0,					NULL, 			NULL}
};

static bot_setting cs_settings[]=
{
	{"NICK",		&cs_botinfo.nick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick, NULL, (void*)"ConnectServ" },
	{"USER",		&cs_botinfo.user,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user, NULL, (void*)"CS" },
	{"HOST",		&cs_botinfo.host,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host, NULL, (void*)"" },
	{"REALNAME",	&cs_botinfo.realname,SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname, NULL, (void*)"" },
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "SignWatch",	NULL,	cs_help_set_signwatch, NULL, (void*)1 },
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "KillWatch",	NULL,	cs_help_set_killwatch, NULL, (void*)1 },
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ModeWatch",	NULL,	cs_help_set_modewatch, NULL, (void*)1 },
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "NickWatch",	NULL,	cs_help_set_nickwatch, NULL, (void*)1 },
	{"SERVWATCH",	&cs_cfg.serv_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ServWatch",	NULL,	cs_help_set_servwatch, NULL, (void*)1 },
	{"EXCLUSIONS",	&cs_cfg.use_exc,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "Exclusions",	NULL,	cs_help_set_exclusions, NULL, (void*)1 },
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	cs_event_online},
	{EVENT_SIGNON,	cs_event_signon},
	{EVENT_UMODE,	cs_event_umode},
#ifdef GOTUSERSMODES
	{EVENT_SMODE,	cs_event_smode},
#endif
	{EVENT_QUIT,	cs_event_quit},
	{EVENT_KILL,	cs_event_kill},
	{EVENT_NICK,	cs_event_nick},
	{EVENT_SERVER,	cs_event_server},
	{EVENT_SQUIT,	cs_event_squit},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* mod_ptr)
{
	cs_module = mod_ptr;
	LoadConfig();
	return 1;
}

void ModFini()
{

};

static int cs_event_online(CmdParams* cmdparams)
{
	cs_bot = init_bot (cs_module, &cs_botinfo, services_bot_modes, BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
		cs_commands, cs_settings);
	cs_online = 1;
	return 1;
};

/* 
 * Echo signon
 */
static int cs_event_signon(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_online)	{
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.user)) {
		return 1;
	}
	if (IsMe(cmdparams->source.user)) {
		/* its me, forget it */
		return 1;
	}
	/* Print Connection Notice */
	if (cs_cfg.sign_watch) {
		chanalert(cs_bot->nick, msg_signon,
			  cmdparams->source.user->nick, cmdparams->source.user->username, 
			  cmdparams->source.user->hostname, cmdparams->source.user->realname,
			  cmdparams->source.user->server->name);
	}
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
	if (!cs_online) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.user)) {
		return 1;
	}
	if (IsMe(cmdparams->source.user)) {
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
			chanalert(cs_bot->nick, msg_localkill,
				  cmdparams->source.user->nick, cmdparams->source.user->username, 
				  cmdparams->source.user->hostname,
				  Local[6], KillMsg);
			free(KillMsg);
			free(QuitMsg);
			free(cmd);
			free(lcl);
			return 1;

		}
	}
	/* Print Disconnection Notice */
	if (cs_cfg.sign_watch) {
		chanalert(cs_bot->nick, msg_signoff,
			  cmdparams->source.user->nick, cmdparams->source.user->username, 
			  cmdparams->source.user->hostname, cmdparams->source.user->realname,
			  cmdparams->source.user->server->name, QuitMsg);
	}
	free(QuitMsg);
	free(cmd);
	free(lcl);
	free(Quit);
	return 1;
}

/* 
 * report mode change
 */

static int cs_report_mode(User* u, int add, char mode, const char* mode_desc, int serverinfo)
{
	if(serverinfo) {
		chanalert(cs_bot->nick, msg_mode_serv, u->nick, 
			add?"now":"no longer", 
			mode_desc,
			add?'+':'-',
			mode, u->server->name);
	} else {
		chanalert(cs_bot->nick, msg_mode, u->nick, 
			add?"now":"no longer", 
			mode_desc,
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
	int add = 1;
	char *modes;

	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.mode_watch) {
		return -1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.user)) {
		return 1;
	}
	if (IsMe(cmdparams->source.user)) {
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
#ifdef UMODE_CH_NETADMIN
		case UMODE_CH_NETADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_NETADMIN, mode_netadmin, 0);
			break;
#endif
#ifdef UMODE_CH_CONETADMIN
		case UMODE_CH_CONETADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_CONETADMIN, mode_conetadmin, 0);
			break;
#endif
#ifdef UMODE_CH_TECHADMIN
		case UMODE_CH_TECHADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_TECHADMIN, mode_techadmin, 0);
			break;
#endif
#ifdef UMODE_CH_ADMIN
		case UMODE_CH_ADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_ADMIN, mode_serveradmin, 1);
			break;
#endif
#ifdef UMODE_CH_COADMIN
		case UMODE_CH_COADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_COADMIN, mode_coserveradmin, 1);
			break;
#endif
#ifdef UMODE_CH_GUESTADMIN
		case UMODE_CH_GUESTADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_GUESTADMIN, mode_guestadmin, 1);
			break;
#endif
#ifdef UMODE_CH_BOT
		case UMODE_CH_BOT:
			chanalert(cs_bot->nick, msg_bot, cmdparams->source.user->nick, add?"now":"no longer", add?'+':'-', UMODE_CH_BOT);			
			break;
#endif
#ifdef UMODE_CH_SADMIN
		case UMODE_CH_SADMIN:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_SADMIN, mode_servicesadmin, 0);
			break;
#endif
#ifdef UMODE_CH_OPER
		case UMODE_CH_OPER:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_OPER, mode_globop, 1);
			break;
#endif
#ifdef UMODE_CH_LOCOP
		case UMODE_CH_LOCOP:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_LOCOP, mode_locop, 1);
			break;
#endif
#ifdef UMODE_CH_SERVICES
		case UMODE_CH_SERVICES:
			cs_report_mode(cmdparams->source.user, add, UMODE_CH_SERVICES, mode_netservice, 0);
			break;
#endif
		default:
			break;
		}
		modes++;
	}
	return 1;
}

#ifdef GOTUSERSMODES
/* smode support for Ultimate3 */
static int cs_event_smode(CmdParams* cmdparams)
{
	int add = 1;
	char *modes;

	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.mode_watch) {
		return -1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.user)) {
		return 1;
	}
	if (IsMe(cmdparams->source.user)) {
		/* its me, forget it */
		return 1;
	}
	modes = (char *)(av[1]);
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
#ifdef SMODE_CH_NETADMIN
		case SMODE_CH_NETADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_NETADMIN, mode_netadmin, 0);
			break;
#endif
#ifdef SMODE_CH_CONETADMIN
		case SMODE_CH_CONETADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_CONETADMIN, mode_conetadmin, 0);
			break;
#endif
#ifdef SMODE_CH_TECHADMIN
		case SMODE_CH_TECHADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_TECHADMIN, mode_techadmin, 0);
			break;
#endif
#ifdef SMODE_CH_COTECHADMIN
		case SMODE_CH_COTECHADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_COTECHADMIN, mode_cotechadmin, 0);
			break;
#endif
#ifdef SMODE_CH_ADMIN
		case SMODE_CH_ADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_ADMIN, mode_serveradmin, 1);
			break;
#endif
#ifdef SMODE_CH_COADMIN
		case SMODE_CH_COADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_COADMIN, mode_coserveradmin, 1);
			break;
#endif
#ifdef SMODE_CH_GUESTADMIN
		case SMODE_CH_GUESTADMIN:
			cs_report_mode(cmdparams->source.user, add, SMODE_CH_GUESTADMIN, mode_guestadmin, 1);
			break;
#endif
		default:
			break;
		}
		modes++;
	}
	return 1;
}
#endif

/* 
 * Echo kills
 */
static int cs_event_kill(CmdParams* cmdparams)
{
	char *cmd, *GlobalMsg;
	char **Kill;
	int KillCount = 0;

	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.kill_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.user)) {
		return 1;
	}
	if (IsMe(cmdparams->source.user)) {
		/* its me, forget it */
		return 1;
	}
	cmd = sstrdup(recbuf);
	KillCount = split_buf(cmd, &Kill, 0);
	GlobalMsg = joinbuf(Kill, KillCount, 4);
	if (finduser(Kill[2])) {
		/* it was a User who was killed */
		chanalert(cs_bot->nick, msg_globalkill,
			cmdparams->source.user->nick, cmdparams->source.user->username, 
			cmdparams->source.user->hostname,
			Kill[0], GlobalMsg);
	} else if (findserver(Kill[2])) {
		chanalert(cs_bot->nick, msg_serverkill,
			cmdparams->source.user->nick, cmdparams->source.user->username, 
			cmdparams->source.user->hostname,
			Kill[0], GlobalMsg);
	}
	free(cmd);
	free(GlobalMsg);
	return 1;
}

/* 
 * Echo nick changes
 */
static int cs_event_nick(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.nick_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.user)) {
		return 1;
	}
	if (IsMe(cmdparams->source.user)) {
		/* its me, forget it */
		return 1;
	}
	chanalert(cs_bot->nick, msg_nickchange, cmdparams->param, 
		cmdparams->source.user->username, cmdparams->source.user->hostname, 
		cmdparams->source.user->nick);
	return 1;
}

/* 
 * Load ConnectServ Configuration file and set defaults if does not exist
 */
static void LoadConfig(void)
{
	char *temp = NULL;

	GetConf((void *) &cs_cfg.sign_watch, CFGBOOL, "SignWatch");
	GetConf((void *) &cs_cfg.kill_watch, CFGBOOL, "KillWatch");
	GetConf((void *) &cs_cfg.mode_watch, CFGBOOL, "ModeWatch");
	GetConf((void *) &cs_cfg.nick_watch, CFGBOOL, "NickWatch");
	GetConf((void *) &cs_cfg.serv_watch, CFGBOOL, "ServWatch");
	GetConf((void *) &cs_cfg.use_exc, CFGBOOL, "Exclusions";
	if(GetConf((void *) &temp, CFGSTR, "Nick") > 0) {
		strlcpy(cs_botinfo.nick, temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") > 0) {
		strlcpy(cs_botinfo.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") > 0) {
		strlcpy(cs_botinfo.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") > 0) {
		strlcpy(cs_botinfo.realname, temp, MAXREALNAME);
		free(temp);
	}
}

static int cs_event_server(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.serv_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.server)) {
		return 1;
	}
	chanalert (cs_bot->nick, "\2SERVER\2 %s has joined the network at %s",
		cmdparams->source.server->name, cmdparams->source.server->uplink);
	return 1;
}

static int cs_event_squit(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.serv_watch) {
		return 1;
	}
	if (cs_cfg.use_exc && IsExcluded(cmdparams->source.server)) {
		return 1;
	}
	chanalert (cs_bot->nick, "\2SERVER\2 %s has left the network at %s for %s",
		cmdparams->source.server->name, cmdparams->source.server->uplink, 
		cmdparams->param ? cmdparams->param : "");
	return 1;
}
