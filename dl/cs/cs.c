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

static const char mode_netadmin[]="Network Administrator";
static const char mode_conetadmin[]="Co-Network Administrator";
static const char mode_techadmin[]="Network Technical Administrator";
static const char mode_cotechadmin[]="Network Co-Technical Administrator";
static const char mode_serveradmin[]="Server Administrator";
static const char mode_coserveradmin[]="Co-Server Administrator";
static const char mode_guestadmin[]="Guest Administrator";
static const char mode_servicesadmin[]="Services Administrator";
static const char mode_globop[]="global operator";
static const char mode_locop[]="local operator";
static const char mode_netservice[]="Network Service";
static const char mode_bot[]="Bot";

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

static int cs_new_user(char **av, int ac);
static int cs_user_modes(char **av, int ac);
#ifdef GOTUSERSMODES
static int cs_user_smodes(char **av, int ac);
#endif
static int cs_del_user(char **av, int ac);
static int cs_user_kill(char **av, int ac);
static int cs_user_nick(char **av, int ac);
static int cs_server_join(char **av, int ac);
static int cs_server_quit(char **av, int ac);

static void LoadConfig(void);

static int cs_about(User * u, char **av, int ac);
static int cs_version(User * u, char **av, int ac);

struct cs_cfg { 
	int sign_watch;
	int kill_watch;
	int mode_watch;
	int nick_watch;
	int serv_watch;
	int use_exc;
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
	NEOSTATS_VERSION,
	__DATE__,
	__TIME__
};

static bot_cmd cs_commands[]=
{
	{"ABOUT",	cs_about,	0, 	NS_ULEVEL_ADMIN,	cs_help_about, 	cs_help_about_oneline },
	{"VERSION",	cs_version,	0, 	NS_ULEVEL_ADMIN,	cs_help_version,cs_help_version_oneline },
	{NULL,		NULL,		0, 	0,					NULL, 			NULL}
};

static bot_setting cs_settings[]=
{
	{"NICK",		&s_ConnectServ,		SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick },
	{"USER",		&cs_cfg.user,		SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user },
	{"HOST",		&cs_cfg.host,		SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host },
	{"REALNAME",	&cs_cfg.rname,		SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname },
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "SignWatch",	NULL,	cs_help_set_signwatch },
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "KillWatch",	NULL,	cs_help_set_killwatch },
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ModeWatch",	NULL,	cs_help_set_modewatch },
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "NickWatch",	NULL,	cs_help_set_nickwatch },
	{"SERVWATCH",	&cs_cfg.serv_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ServWatch",	NULL,	cs_help_set_servwatch },
	{"USEEXCLUSIONS", &cs_cfg.use_exc,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "Exclusions",	NULL,	cs_help_set_exclusions },
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
		services_bot_modes, BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, cs_commands, cs_settings, __module_info.module_name);
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
	{EVENT_SERVER,		cs_server_join},
	{EVENT_SQUIT,		cs_server_quit},
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
	/* Check that our compiled version if compatible with the calling version of NeoStats */
	if(	ircstrncasecmp (me.version, NEOSTATS_VERSION, VERSIONSIZE) !=0) {
		return NS_ERR_VERSION;
	}
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
	prefmsg(u->nick, s_ConnectServ, "%s Version: %s Compiled %s at %s", __module_info.module_name,
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

	if (cs_cfg.use_exc && IsExcluded(u)) 
		return 1;

	if (IsMe(u)) {
		/* its me, forget it */
		return 1;
	}

	/* Print Connection Notice */
	if (cs_cfg.sign_watch) {
		chanalert(s_ConnectServ, msg_signon,
			  u->nick, u->username, u->hostname, u->realname,
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

	if (cs_cfg.use_exc && IsExcluded(u)) 
		return 1;
	if (IsMe(u)) {
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
			  u->nick, u->username, u->hostname, u->realname,
			  u->server->name, QuitMsg);
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
		chanalert(s_ConnectServ, msg_mode_serv, u->nick, 
			add?"now":"no longer", 
			mode_desc,
			add?'+':'-',
			mode, u->server->name);
	} else {
		chanalert(s_ConnectServ, msg_mode, u->nick, 
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
static int cs_user_modes(char **av, int ac)
{
	int add = 1;
	char *modes;
	User *u;

	SET_SEGV_LOCATION();

	if (!cs_online || !cs_cfg.mode_watch)
		return -1;

	u = finduser(av[0]);
	if (!u) {
		return -1;
	}

	if (cs_cfg.use_exc && IsExcluded(u)) 
		return 1;

	if (IsMe(u)) {
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
			cs_report_mode(u, add, UMODE_CH_NETADMIN, mode_netadmin, 0);
			break;
#endif
#ifdef UMODE_CH_CONETADMIN
		case UMODE_CH_CONETADMIN:
			cs_report_mode(u, add, UMODE_CH_CONETADMIN, mode_conetadmin, 0);
			break;
#endif
#ifdef UMODE_CH_TECHADMIN
		case UMODE_CH_TECHADMIN:
			cs_report_mode(u, add, UMODE_CH_TECHADMIN, mode_techadmin, 0);
			break;
#endif
#ifdef UMODE_CH_ADMIN
		case UMODE_CH_ADMIN:
			cs_report_mode(u, add, UMODE_CH_ADMIN, mode_serveradmin, 1);
			break;
#endif
#ifdef UMODE_CH_COADMIN
		case UMODE_CH_COADMIN:
			cs_report_mode(u, add, UMODE_CH_COADMIN, mode_coserveradmin, 1);
			break;
#endif
#ifdef UMODE_CH_GUESTADMIN
		case UMODE_CH_GUESTADMIN:
			cs_report_mode(u, add, UMODE_CH_GUESTADMIN, mode_guestadmin, 1);
			break;
#endif
#ifdef UMODE_CH_BOT
		case UMODE_CH_BOT:
			chanalert(s_ConnectServ, msg_bot, u->nick, add?"now":"no longer", add?'+':'-', UMODE_CH_BOT);			
			break;
#endif
#ifdef UMODE_CH_SADMIN
		case UMODE_CH_SADMIN:
			cs_report_mode(u, add, UMODE_CH_SADMIN, mode_servicesadmin, 0);
			break;
#endif
#ifdef UMODE_CH_OPER
		case UMODE_CH_OPER:
			cs_report_mode(u, add, UMODE_CH_OPER, mode_globop, 1);
			break;
#endif
#ifdef UMODE_CH_LOCOP
		case UMODE_CH_LOCOP:
			cs_report_mode(u, add, UMODE_CH_LOCOP, mode_locop, 1);
			break;
#endif
#ifdef UMODE_CH_SERVICES
		case UMODE_CH_SERVICES:
			cs_report_mode(u, add, UMODE_CH_SERVICES, mode_netservice, 0);
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

	if (!cs_online || !cs_cfg.mode_watch)
		return -1;

	u = finduser(av[0]);
	if (!u) {
		return -1;
	}


	if (cs_cfg.use_exc && IsExcluded(u)) 
		return 1;

	if (IsMe(u)) {
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
			cs_report_mode(u, add, SMODE_CH_NETADMIN, mode_netadmin, 0);
			break;
#endif
#ifdef SMODE_CH_CONETADMIN
		case SMODE_CH_CONETADMIN:
			cs_report_mode(u, add, SMODE_CH_CONETADMIN, mode_conetadmin, 0);
			break;
#endif
#ifdef SMODE_CH_TECHADMIN
		case SMODE_CH_TECHADMIN:
			cs_report_mode(u, add, SMODE_CH_TECHADMIN, mode_techadmin, 0);
			break;
#endif
#ifdef SMODE_CH_COTECHADMIN
		case SMODE_CH_COTECHADMIN:
			cs_report_mode(u, add, SMODE_CH_COTECHADMIN, mode_cotechadmin, 0);
			break;
#endif
#ifdef SMODE_CH_ADMIN
		case SMODE_CH_ADMIN:
			cs_report_mode(u, add, SMODE_CH_ADMIN, mode_serveradmin, 1);
			break;
#endif
#ifdef SMODE_CH_COADMIN
		case SMODE_CH_COADMIN:
			cs_report_mode(u, add, SMODE_CH_COADMIN, mode_coserveradmin, 1);
			break;
#endif
#ifdef SMODE_CH_GUESTADMIN
		case SMODE_CH_GUESTADMIN:
			cs_report_mode(u, add, SMODE_CH_GUESTADMIN, mode_guestadmin, 1);
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

	if (!cs_online || !cs_cfg.kill_watch)
		return 1;

	u = finduser(av[0]);
	if (!u) /* User not found */
		return 1;
	

	if (cs_cfg.use_exc && IsExcluded(u)) 
		return 1;

	if (IsMe(u)) {
		/* its me, forget it */
		return 1;
	}

	cmd = sstrdup(recbuf);
	KillCount = split_buf(cmd, &Kill, 0);
	GlobalMsg = joinbuf(Kill, KillCount, 4);

	if (finduser(Kill[2])) {
		/* it was a User who was killed */
		chanalert(s_ConnectServ, msg_globalkill,
			u->nick, u->username, u->hostname,
			Kill[0], GlobalMsg);
	} else if (findserver(Kill[2])) {
		chanalert(s_ConnectServ, msg_serverkill,
			u->nick, u->username, u->hostname,
			Kill[0], GlobalMsg);
	}
	free(cmd);
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

	if (!cs_online || !cs_cfg.nick_watch)
		return 1;

	u = finduser(av[1]);
	if (!u)
		return -1;

	if (cs_cfg.use_exc && IsExcluded(u)) 
		return 1;

	if (IsMe(u)) {
		/* its me, forget it */
		return 1;
	}
	chanalert(s_ConnectServ, msg_nickchange,
		av[0], u->username, u->hostname, av[1]);
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
	if(GetConf((void *) &cs_cfg.serv_watch, CFGBOOL, "ServWatch")<= 0) {
		cs_cfg.serv_watch = 1;
	}
	if(GetConf((void *) &cs_cfg.use_exc, CFGBOOL, "Exclusions")<= 0) {
		cs_cfg.serv_watch = 0;
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

static int cs_server_join(char **av, int ac)
{
	Server *s;

	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.serv_watch)
		return 1;

	s = findserver(av[0]);
	if (!s)
		return 0;

	if (cs_cfg.use_exc && IsExcluded(s)) 
		return 1;

	chanalert (s_ConnectServ, "\2SERVER\2 %s has joined the Network at %s",
		s->name, s->uplink);

	return 1;
}
static int cs_server_quit(char **av, int ac)
{
	Server *s;

	SET_SEGV_LOCATION();
	if (!cs_online || !cs_cfg.serv_watch)
		return 1;

	s = findserver(av[0]);
	if (!s)
		return 0;

	if (cs_cfg.use_exc && IsExcluded(s)) 
		return 1;

	chanalert (s_ConnectServ, "\2SERVER\2 %s has left the Network at %s for %s",
		s->name, s->uplink, (ac == 2) ? av[1] : "");
	return 1;
}
