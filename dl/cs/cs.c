/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "cs_help.c"
#include "cs.h"
#include "log.h"
#include "conf.h"

/* Uncomment this line to disable colours in ConnectServ 
   channel messages
*/
/* #define DISABLE_COLOUR_SUPPORT */

#ifdef DISABLE_COLOUR_SUPPORT
char msg_nickchange[]="\2NICK\2 %s (%s@%s) Changed their nick to %s";
char msg_signon[]="\2SIGNON\2 %s (%s@%s) has signed on at %s";
char msg_localkill[]="\2LOCAL KILL\2 %s (%s@%s) was killed by %s - Reason sighted: \2%s\2";
char msg_globalkill[]="\2GLOBAL KILL\2 %s (%s@%s) was Killed by %s - Reason sighted: \2%s\2";
char msg_serverkill[]="\2SERVER KILL\2 %s was Killed by the Server %s - Reason sighted: \2%s\2";  
char msg_signoff[]="\2SIGNOFF\2 %s (%s@%s) has signed off at %s - %s";
char msg_netadmin[]="\2NETADMIN\2 %s is Now a Network Administrator (+%c)";
char msg_netadminoff[]="\2NETADMIN\2 %s is No Longer a Network Administrator (-%c)";
char msg_conetadmin[]="\2CO-NETADMIN\2 %s is Now a Co-Network Administrator (+%c)";
char msg_conetadminoff[]="\2CO-NETADMIN\2 %s is No Longer a Co-Network Administrator (-%c)";
char msg_techadmin[]="\2TECHADMIN\2 %s is Now a Network Technical Administrator (+%c)";
char msg_techadminoff[]="\2TECHADMIN\2 %s is No Longer a Network Technical Administrator (-%c)";
char msg_cotechadmin[]="\2CO-TECHADMIN\2 %s is Now a Network Co-Technical Administrator (+%c)";
char msg_cotechadminoff[]="\2CO-TECHADMIN\2 %s is No Longer a Network Co-Technical Administrator (-%c)";
char msg_serveradmin[]="\2SERVERADMIN\2 %s is Now a Server Administrator (+%c) on %s";
char msg_serveradminoff[]="\2SERVERADMIN\2 %s is No Longer a Server Administrator (-%c) on %s";
char msg_coserveradmin[]="\2CO-SERVERADMIN\2 %s is Now a Co-Server Administrator (+%c) on %s";
char msg_coserveradminoff[]="\2CO-SERVERADMIN\2 %s is No Longer a Co-Server Administrator (-%c) on %s";
char msg_guestadmin[]="\2GUESTADMIN\2 %s is Now a Guest Administrator on (+%c) %s";
char msg_guestadminoff[]="\2GUESTADMIN\2 %s is No Longer a Guest Administrator (-%c) on %s";
char msg_servicesadmin[]="\2SERVICESADMIN\2 %s is Now a Services Administrator (+%c)";
char msg_servicesadminoff[]="\2SERVICESADMIN\2 %s is No Longer a Services Administrator (-%c)";
char msg_globop[]="\2OPER\2 %s is Now a global operator (+%c) on %s";
char msg_globopoff[]="\2OPER\2 %s is No Longer an global operator (-%c) on %s";
char msg_locop[]="\2LOCALOPER\2 %s is Now a local operator (+%c) on %s";
char msg_locopoff[]="\2LOCALOPER\2 %s is No Longer a local operator (-%c) on %s";
char msg_netservice[]="\2SERVICES\2 %s is Now a Network Service (+%c)";
char msg_netserviceoff[]="\2SERVICES\2 %s is No Longer a Network Service (-%c)";
char msg_bot[]="\2BOT\2 %s is Now a Bot (+%c)";
char msg_botoff[]="\2BOT\2 %s is No Longer a Bot (-%c)";
char msg_invisible[]="\2INVISIBLE\2 %s Is Using \2Invisible Mode\2 (+%c)";
char msg_invisibleoff[]="\2INVISIBLE\2 %s Is no longer using \2Invisible Mode\2 (-%c)";
#else
char msg_nickchange[]="\2\0037Nick Change\2 user: \2%s\2 (%s@%s) Changed their nick to \2%s\2\003"; 
char msg_signon[]="\2\0034SIGNED ON\2 user: \2%s\2 (%s@%s) at: \2%s\2\003";
char msg_localkill[]="\2LOCAL KILL\2 user: \2%s\2 (%s@%s) was Killed by: \2%s\2 - Reason sighted: \2%s\2";
char msg_globalkill[]="\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2\003";
char msg_serverkill[]="\2SERVER KILL\2 user: \2%s\2 was Killed by the Server \2%s\2 - Reason sighted: \2%s\2";
char msg_signoff[]="\2\0033Signed Off\2 user: %s (%s@%s) at: %s - %s\003";
char msg_netadmin[]="\2\00313%s\2 is \2Now\2 a \2Network Administrator\2 (+%c)\003";
char msg_netadminoff[]="\02\00313%s\2 is \2No Longer\2 a \2Network Administrator\2 (-%c)\003";
char msg_conetadmin[]="\2\00313%s\2 is \2Now\2 a \2Co-Network Administrator\2 (+%c)\003";
char msg_conetadminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Co-Network Administrator\2 (-%c)\003";
char msg_techadmin[]="\2\00313%s\2 is \2Now\2 a \2Network Technical Administrator\2 (+%c)\003";
char msg_techadminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Network Technical Administrator\2 (-%c)\003";
char msg_cotechadmin[]="\2\00313%s\2 is \2Now\2 a \2Network Co-Technical Administrator\2 (+%c)\003";
char msg_cotechadminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Network Co-Technical Administrator\2 (-%c)\003";
char msg_serveradmin[]="\2\00313%s\2 is \2Now\2 a \2Server Administrator\2 (+%c) on \2%s\2\003";
char msg_serveradminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Server Administrator\2 (-%c) on \2%s\2\003";
char msg_coserveradmin[]="\2\00313%s\2 is \2Now\2 a \2Co-Server Administrator\2 (+%c) on \2%s\2\003";
char msg_coserveradminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Co-Server Administrator\2 (-%c) on \2%s\2\003";
char msg_guestadmin[]="\2\00313%s\2 is \2Now\2 a \2Guest Administrator\2 (+%c) on \2%s\2\003";
char msg_guestadminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Guest Administrator\2 (-%c) on \2%s\2\003";
char msg_servicesadmin[]="\2\00313%s\2 is \2Now\2 a \2Services Administrator\2 (+%c)\003";
char msg_servicesadminoff[]="\2\00313%s\2 is \2No Longer\2 a \2Services Administrator\2 (-%c)\003";
char msg_globop[]="\2\00313%s\2 is \2Now\2 a \2Global Operator\2 (+%c) on \2%s\2\003";
char msg_globopoff[]="\2\00313%s\2 is \2No Longer\2 a \2Global Operator\2 (-%c) on \2%s\2\003";
char msg_locop[]="\2\00313%s\2 is \2Now\2 a \2Local Operator\2 (+%c) on \2%s\2\003";
char msg_locopoff[]="\2\00313%s\2 is \2No Longer\2 a \2Local Operator\2 (-%c) on \2%s\2\003";
char msg_netservice[]="\2\00313%s\2 is \2Now\2 a \2Network Service\2 (+%c)\003";
char msg_netserviceoff[]="\2\00313%s\2 is \2No Longer\2 a \2Network Service\2 (-%c)\003";
char msg_bot[]="\2\00313%s\2 is \2Now\2 a \2Bot\2 (+%c)\003";
char msg_botoff[]="\2\00313%s\2 is \2No Longer\2 a \2Bot\2 (-%c)\003";
char msg_invisible[]="\2%s\2 Is Using \2Invisible Mode\2 (+%c)";
char msg_invisibleoff[]="\2%s\2 Is no longer using \2Invisible Mode\2 (-%c)";
#endif

static int cs_new_user(char **av, int ac);
static int cs_user_modes(char **av, int ac);
#ifdef GOTUSERSMODES
static int cs_user_smodes(char **av, int ac);
#endif
static int cs_del_user(char **av, int ac);
static int cs_user_kill(char **av, int ac);
static int cs_user_nick(char **av, int ac);

static void LoadConfig(void);

static int cs_about(User * u, char **av, int ac);
static int cs_version(User * u, char **av, int ac);

struct cs_cfg { 
	int sign_watch;
	int kill_watch;
	int mode_watch;
	int nick_watch;
	int modnum;
	char user[MAXUSER];
	char host[MAXHOST];
	char rname[MAXREALNAME];
} cs_cfg;

static char s_ConnectServ[MAXNICK];
static int cs_online = 0;
static ModUser *cs_bot;

ModuleInfo __module_info = {
	"ConnectServ",
	"Network Connection & Mode Monitoring Service",
	"1.11",
	__DATE__,
	__TIME__
};

static int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(RPL_VERSION, origin,
		     "Module ConnectServ Loaded, Version: %s %s %s",
			 __module_info.module_version, __module_info.module_build_date,
			 __module_info.module_build_time);
	return 0;
}

Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1}	,
#ifdef GOTTOKENSUPPORT
	{TOK_VERSION, new_m_version, 1}	,
#endif
	{NULL, NULL, 0}
};

static bot_cmd cs_commands[]=
{
	{"ABOUT",	cs_about,	0, 	NS_ULEVEL_ADMIN,	cs_help_about, 	cs_help_about_oneline },
	{"VERSION",	cs_version,	0, 	NS_ULEVEL_ADMIN,	cs_help_version,cs_help_version_oneline },
	{NULL,		NULL,		0, 	0,					NULL, 			NULL}
};

static bot_setting cs_settings[]=
{
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "SignWatch",	NULL,	cs_help_set	},
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "KillWatch",	NULL,	NULL	},
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ModeWatch",	NULL,	NULL	},
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "NickWatch",	NULL,	NULL	},
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

static int cs_about(User * u, char **av, int ac)
{
	privmsg_list(u->nick, s_ConnectServ, cs_help_about);
	return 1;
}

static int Online(char **av, int ac)
{
	cs_bot = init_mod_bot(s_ConnectServ, cs_cfg.user, cs_cfg.host, cs_cfg.rname, 
		services_bot_modes, BOT_FLAG_RESTRICT_OPERS, cs_commands, cs_settings, __module_info.module_name);
	if(cs_bot)
		cs_online = 1;
	return 1;
};

EventFnList __module_events[] = {
	{EVENT_ONLINE,		Online},
	{EVENT_SIGNON,		cs_new_user},
	{EVENT_UMODE,		cs_user_modes},
#ifdef GOTUSERSMODES
	{EVENT_SMODE,		cs_user_smodes},
#endif
	{EVENT_SIGNOFF,		cs_del_user},
	{EVENT_KILL,		cs_user_kill},
	{EVENT_NICKCHANGE,	cs_user_nick},
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
	LoadConfig();
	return 1;
}

void __ModFini()
{

};

/* 
 * VERSION
 */
static int cs_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, s_ConnectServ, "\2%s Version Information\2", s_ConnectServ);
	prefmsg(u->nick, s_ConnectServ, "%s Version: %s Compiled %s at %s", s_ConnectServ,
		__module_info.module_version, __module_info.module_build_date, __module_info.module_build_time);
	prefmsg(u->nick, s_ConnectServ, "http://www.neostats.net");
	return 1;
}

/* 
 * Echo signon
 */
static int cs_new_user(char **av, int ac)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!cs_online)
		return 1;

	u = finduser(av[0]);
	if (!u) /* User not found */
		return 1;
	if (!strcasecmp(u->server->name, me.name)) {
		/* its me, forget it */
		return 1;
	}

	/* Print Connection Notice */
	if (cs_cfg.sign_watch) {
		chanalert(s_ConnectServ, msg_signon,
			  u->nick, u->username, u->hostname,
			  u->server->name);
	}
	return 1;
}

/* 
 * Echo signoff
 */
static int cs_del_user(char **av, int ac)
{
	char *cmd, *lcl, *QuitMsg, *KillMsg;
	User *u;
	char **Quit;
	char **Local;
	int QuitCount = 0;
	int LocalCount = 0;

	SET_SEGV_LOCATION();

	if (!cs_online)
		return 1;

	u = finduser(av[0]);
	if (!u) /* User not found */
		return 1;

	if (!strcasecmp(u->server->name, me.name)) {
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
			chanalert(s_ConnectServ,
				  msg_localkill,
				  u->nick, u->username, u->hostname,
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
		chanalert(s_ConnectServ,
			msg_signoff,
			  u->nick, u->username, u->hostname,
			  u->server->name, QuitMsg);
	}
	free(QuitMsg);
	free(cmd);
	free(lcl);
	free(Quit);
	return 1;
}

/* 
 * Echo oper mode changes
 */
static int cs_user_modes(char **av, int ac)
{
	int add = 1;
	char *modes;
	User *u;

	SET_SEGV_LOCATION();

	if (!cs_online)
		return 1;

	if (cs_cfg.mode_watch != 1)
		return -1;

	u = finduser(av[0]);
	if (!u) {
		nlog(LOG_WARNING, LOG_MOD,
		     "Changing modes for unknown user: %s", u->nick);
		return -1;
	}
	if (!strcasecmp(u->server->name, me.name)) {
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
#ifdef GOTUSERSMODES
/* these modes in Ultimate3 are Smodes */
#ifdef UMODE_CH_NETADMIN
		case UMODE_CH_NETADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_netadmin,
					  u->nick, UMODE_CH_NETADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_netadminoff,
					  u->nick, UMODE_CH_NETADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_CONETADMIN
		case UMODE_CH_CONETADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_conetadmin,
					  u->nick, UMODE_CH_CONETADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_conetadminoff,
					  u->nick, UMODE_CH_CONETADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_TECHADMIN
		case UMODE_CH_TECHADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_techadmin,
					  u->nick, UMODE_CH_TECHADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_techadminoff,
					  u->nick, UMODE_CH_TECHADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_ADMIN
		case UMODE_CH_ADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_serveradmin,
					  u->nick, UMODE_CH_ADMIN, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_serveradminoff,
					  u->nick, UMODE_CH_ADMIN, u->server->name);
			}
			break;
#endif
#ifdef UMODE_CH_COADMIN
		case UMODE_CH_COADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_coserveradmin,
					  u->nick, UMODE_CH_COADMIN, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_coserveradminoff,
					  u->nick, UMODE_CH_COADMIN, u->server->name);
			}
			break;
#endif
#ifdef UMODE_CH_GUESTADMIN
		case UMODE_CH_GUESTADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_guestadmin,
					  u->nick, UMODE_CH_GUESTADMIN, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_guestadminoff,
					  u->nick, UMODE_CH_GUESTADMIN, u->server->name);
			}
			break;
/* these modes are not used in Ultimate3 */
#endif
#ifdef UMODE_CH_BOT
		case UMODE_CH_BOT:
			if (add) {
				chanalert(s_ConnectServ,
					msg_bot,
					  u->nick, UMODE_CH_BOT);
			} else {
				chanalert(s_ConnectServ,
					msg_botoff,
					  u->nick, UMODE_CH_BOT);
			}
			break;
#endif
#endif
#ifdef UMODE_CH_SADMIN
		case UMODE_CH_SADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_servicesadmin,
					  u->nick, UMODE_CH_SADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_servicesadminoff,
					  u->nick, UMODE_CH_SADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_OPER
		case UMODE_CH_OPER:
			if (add) {
				chanalert(s_ConnectServ,
					msg_globop,
					  u->nick, UMODE_CH_OPER, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_globopoff,
					  u->nick, UMODE_CH_OPER, u->server->name);
			}
			break;
#endif
#ifdef UMODE_CH_LOCOP
		case UMODE_CH_LOCOP:
			if (add) {
				chanalert(s_ConnectServ,
					msg_locop,
					  u->nick, UMODE_CH_LOCOP, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_locopoff,
					  u->nick, UMODE_CH_LOCOP, u->server->name);
			}
			break;
#endif
#ifdef UMODE_CH_SERVICES
		case UMODE_CH_SERVICES:
			if (add) {
				chanalert(s_ConnectServ,
					msg_netservice,
					  u->nick, UMODE_CH_SERVICES);
			} else {
				chanalert(s_ConnectServ,
					msg_netserviceoff,
					  u->nick, UMODE_CH_SERVICES);
			}
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
static int cs_user_smodes(char **av, int ac)
{
	int add = 1;
	char *modes;
	User *u;

	SET_SEGV_LOCATION();

	if (cs_cfg.mode_watch != 1)
		return -1;

	u = finduser(av[0]);
	if (!u) {
		nlog(LOG_WARNING, LOG_MOD,
		     "Changing modes for unknown user: %s", u->nick);
		return -1;
	}

	if (!strcasecmp(u->server->name, me.name)) {
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
#ifdef UMODE_CH_NETADMIN
		case UMODE_CH_NETADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_netadmin,
					  u->nick, UMODE_CH_NETADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_netadminoff,
					  u->nick, UMODE_CH_NETADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_CONETADMIN
		case UMODE_CH_CONETADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_conetadmin,
					  u->nick, UMODE_CH_CONETADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_conetadminoff,
					  u->nick, UMODE_CH_CONETADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_TECHADMIN
		case UMODE_CH_TECHADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_techadmin,
					  u->nick, UMODE_CH_TECHADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_techadminoff,
					  u->nick, UMODE_CH_TECHADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_COTECHADMIN
		case UMODE_CH_COTECHADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_cotechadmin,
					  u->nick, UMODE_CH_COTECHADMIN);
			} else {
				chanalert(s_ConnectServ,
					msg_cotechadminoff,
					  u->nick, UMODE_CH_COTECHADMIN);
			}
			break;
#endif
#ifdef UMODE_CH_ADMIN
		case UMODE_CH_ADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_serveradmin,
					  u->nick, UMODE_CH_ADMIN, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_serveradminoff,
					  u->nick, UMODE_CH_ADMIN, u->server->name);
			}
			break;
#endif
#ifdef UMODE_CH_COADMIN
		case UMODE_CH_COADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_coserveradmin,
					  u->nick,UMODE_CH_COADMIN,  u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_coserveradminoff,
					  u->nick, UMODE_CH_COADMIN, u->server->name);
			}
			break;
#endif
#ifdef UMODE_CH_GUESTADMIN
		case UMODE_CH_GUESTADMIN:
			if (add) {
				chanalert(s_ConnectServ,
					msg_guestadmin,
					  u->nick, UMODE_CH_GUESTADMIN, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					msg_guestadminoff,
					  u->nick, UMODE_CH_GUESTADMIN, u->server->name);
			}
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
static int cs_user_kill(char **av, int ac)
{
	char *cmd, *GlobalMsg;
	User *u;
	char **Kill;
	int KillCount = 0;

	SET_SEGV_LOCATION();

	if (!cs_online)
		return 1;

	u = finduser(av[0]);
	if (!u) /* User not found */
		return 1;
	
	if (!strcasecmp(u->server->name, me.name)) {
		/* its me, forget it */
		return 1;
	}

	cmd = sstrdup(recbuf);
	KillCount = split_buf(cmd, &Kill, 0);
	GlobalMsg = joinbuf(Kill, KillCount, 4);

	if (finduser(Kill[2])) {
		/* it was a User who was killed */
		if (cs_cfg.kill_watch)
			chanalert(s_ConnectServ,
			      msg_globalkill,
				  u->nick, u->username, u->hostname,
				  Kill[0], GlobalMsg);
	} else if (findserver(Kill[2])) {
		if (cs_cfg.kill_watch)
			chanalert(s_ConnectServ,
				msg_serverkill,
				  u->nick, Kill[0], GlobalMsg);
	}
	free(GlobalMsg);
	return 1;
}

/* 
 * Echo nick changes
 */
static int cs_user_nick(char **av, int ac)
{
	User *u;

	SET_SEGV_LOCATION();

	if (!cs_online)
		return 1;

	if (cs_cfg.nick_watch) {
		u = finduser(av[1]);
		if (!u)
			return -1;
		if (!strcasecmp(u->server->name, me.name)) {
			/* its me, forget it */
			return 1;
		}
		chanalert(s_ConnectServ,
			msg_nickchange,
			  av[0], u->username, u->hostname, av[1]);
	}
	return 1;
}

/* 
 * Load ConnectServ Configuration file and set defaults if does not exist
 */
static void LoadConfig(void)
{
	char *temp = NULL;
	SET_SEGV_LOCATION();
	if(GetConf((void *) &cs_cfg.sign_watch, CFGBOOL, "SignWatch")<= 0) {
		cs_cfg.sign_watch = 1;
	}
	if(GetConf((void *) &cs_cfg.kill_watch, CFGBOOL, "KillWatch")<= 0) {
		cs_cfg.kill_watch = 1;
	}
	if(GetConf((void *) &cs_cfg.mode_watch, CFGBOOL, "ModeWatch")<= 0) {
		cs_cfg.mode_watch = 1;
	}
	if(GetConf((void *) &cs_cfg.nick_watch, CFGBOOL, "NickWatch")<= 0) {
		cs_cfg.nick_watch = 1;
	}
	if(GetConf((void *) &temp, CFGSTR, "Nick") < 0) {
#if !defined(HYBRID7)
		strlcpy(s_ConnectServ , "ConnectServ", MAXNICK);
#else
		/* just to be safe on hyrbid, keep connectservs nick less than 9 */
		strlcpy(s_ConnectServ, "CS", MAXNICK);
#endif
	}
	else {
		strlcpy(s_ConnectServ, temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") < 0) {
		strlcpy(cs_cfg.user, "CS", MAXUSER);
	}
	else {
		strlcpy(cs_cfg.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") < 0) {
		strlcpy(cs_cfg.host, me.name, MAXHOST);
	}
	else {
		strlcpy(cs_cfg.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") < 0) {
		strlcpy(cs_cfg.rname, "Connection Monitoring Service", MAXREALNAME);
	}
	else {
		strlcpy(cs_cfg.rname, temp, MAXREALNAME);
		free(temp);
	}
}
