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
** $Id: cs.c,v 1.32 2003/09/19 12:13:43 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "cs_help.c"
#include "cs.h"
#include "log.h"
#include "conf.h"

const char csversion_date[] = __DATE__;
const char csversion_time[] = __TIME__;
char *s_ConnectServ;

extern const char *cs_help[];
int cs_new_user(char **av, int ac);
int cs_user_modes(char **av, int ac);
int cs_user_smodes(char **av, int ac);
int cs_del_user(char **av, int ac);
int cs_user_kill(char **av, int ac);
int cs_user_nick(char **av, int ac);
void do_set(User * u, char **av, int ac);

static void cs_version(User * u);
static void cs_set_list(User * u, char **av, int ac);
static void cs_set_signwatch(User * u, char **av, int ac);
static void cs_set_killwatch(User * u, char **av, int ac);
static void cs_set_modewatch(User * u, char **av, int ac);
static void cs_set_nickwatch(User * u, char **av, int ac);

int sign_watch;
int kill_watch;
int mode_watch;
int nick_watch;

void SaveSettings();
void Loadconfig();

char **Quit;
char **Local;
char **Kill;
int QuitCount = 0;
int LocalCount = 0;
int KillCount = 0;
int cs_online = 0;


Module_Info my_info[] = { {
			   "ConnectServ",
			   "Network Connection & Mode Monitoring Service",
			   "1.8"}
};

int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(351, origin,
		     "Module ConnectServ Loaded, Version: %s %s %s",
		     my_info[0].module_version, csversion_date,
		     csversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{MSG_VERSION, new_m_version, 1}
	,
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1}
	,
#endif
	{NULL, NULL, 0}
};

int __Bot_Message(char *origin, char **av, int ac)
{
	User *u;
	u = finduser(origin);

	if (!cs_online)
		return 1;

	if ((UserLevel(u) < 185)) {
		prefmsg(u->nick, s_ConnectServ, "Permission Denied!");
		return 1;
	}
	if (!strcasecmp(av[1], "HELP")) {
		if (ac <= 2) {
			privmsg_list(u->nick, s_ConnectServ, cs_help);
			return 1;
		} else if (!strcasecmp(av[2], "SET")) {
			privmsg_list(u->nick, s_ConnectServ,
				     cs_help_status);
			return 1;
		} else if (!strcasecmp(av[2], "ABOUT")) {
			privmsg_list(u->nick, s_ConnectServ,
				     cs_help_about);
			return 1;
		} else
			prefmsg(u->nick, s_ConnectServ,
				"Unknown Help Topic: \2%s\2", av[2]);
	} else if (!strcasecmp(av[1], "SET")) {
		do_set(u, av, ac);
	} else if (!strcasecmp(av[1], "VERSION")) {
		chanalert(s_Services,
			  "%s Wanted to know the current version information for %s",
			  u->nick, s_ConnectServ);
		cs_version(u);
	} else {
		prefmsg(u->nick, s_ConnectServ,
			"Unknown Command: \2%s\2, perhaps you need some HELP?",
			av[1]);
	}
	return 1;
}

void do_set(User * u, char **av, int ac)
{
	if (ac < 3) {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	} else if (!strcasecmp(av[2], "NICKWATCH")) {
		cs_set_nickwatch(u, av, ac);
	} else if (!strcasecmp(av[2], "SIGNWATCH")) {
		cs_set_signwatch(u, av, ac);
	} else if (!strcasecmp(av[2], "KILLWATCH")) {
		cs_set_killwatch(u, av, ac);
	} else if (!strcasecmp(av[2], "MODEWATCH")) {
		cs_set_modewatch(u, av, ac);
	} else if (!strcasecmp(av[2], "LIST")) {
		cs_set_list(u, av, ac);
	} else {
		prefmsg(u->nick, s_ConnectServ,
			"Unknown Set option %s. try /msg %s help set",
			av[2], s_ConnectServ);
		return;
	}
}


int Online(char **av, int ac)
{
	char *user;
	char *host;
	char *rname;

	if (GetConf((void *) &s_ConnectServ, CFGSTR, "Nick") < 0) {
		s_ConnectServ = "ConnectServ";
	}
	if (GetConf((void *) &user, CFGSTR, "User") < 0) {
		user = malloc(MAXUSER);
		snprintf(user, MAXUSER, "CS");
	}
	if (GetConf((void *) &host, CFGSTR, "Host") < 0) {
		host = malloc(MAXHOST);
		snprintf(host, MAXHOST, me.name);
	}
	if (GetConf((void *) &rname, CFGSTR, "RealName") < 0) {
		rname = malloc(MAXHOST);
		snprintf(rname, MAXHOST, "Connection Monitoring Service");
	}


	if (init_bot
	    (s_ConnectServ, user, host, rname, "+oS",
	     my_info[0].module_name) == -1) {
		/* Nick was in use */
		s_ConnectServ = strcat(s_ConnectServ, "_");
		init_bot(s_ConnectServ, user, host, rname, "+oS",
			 my_info[0].module_name);
	}
	cs_online = 1;
	free(user);
	free(host);
	free(rname);
	return 1;
};

EventFnList my_event_list[] = {
	{"ONLINE", Online}
	,
	{"SIGNON", cs_new_user}
	,
	{"UMODE", cs_user_modes}
	,
#ifdef ULTIMATE3
	{"SMODE", cs_user_smodes}
	,
#endif
	{"SIGNOFF", cs_del_user}
	,
	{"KILL", cs_user_kill}
	,
	{"NICK_CHANGE", cs_user_nick}
	,
	{NULL, NULL}
};

Module_Info *__module_get_info()
{
	return my_info;
};

Functions *__module_get_functions()
{
	return my_fn_list;
};

EventFnList *__module_get_events()
{
	return my_event_list;
};


void _init()
{
	Loadconfig();
}

void _fini()
{

};


/* Routine for VERSION */
static void cs_version(User * u)
{
	strcpy(segv_location, "cs_version");
	prefmsg(u->nick, s_ConnectServ,
		"\2ConnectServ Version Information\2");
	prefmsg(u->nick, s_ConnectServ,
		"-------------------------------------");
	prefmsg(u->nick, s_ConnectServ,
		"ConnectServ Version: %s Compiled %s at %s",
		my_info[0].module_version, csversion_date, csversion_time);
	prefmsg(u->nick, s_ConnectServ, "http://www.neostats.net");
	prefmsg(u->nick, s_ConnectServ,
		"-------------------------------------");
}


/* Routine for SIGNON message to be echoed */
int cs_new_user(char **av, int ac)
{
	User *u;
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_new_user");

	if (!cs_online)
		return 1;

	u = finduser(av[0]);
	if (!strcasecmp(u->server->name, me.name)) {
		/* its me, forget it */
		return 1;
	}

	/* Print Connection Notice */
	if (u && sign_watch) {
		chanalert(s_ConnectServ,
			  "\2\0034SIGNED ON\2 user: \2%s\2 (%s@%s) at: \2%s\2\003",
			  u->nick, u->username, u->hostname,
			  u->server->name);
	}
	return 1;
}


/* Routine for SIGNOFF message to be echoed */
int cs_del_user(char **av, int ac)
{
	char *cmd, *lcl, *QuitMsg, *KillMsg;
	User *u;
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_del_user");

	if (!cs_online)
		return 1;

	u = finduser(av[0]);

	if (!strcasecmp(u->server->name, me.name)) {
		/* its me, forget it */
		return 1;
	}

	cmd = sstrdup(recbuf);
	lcl = sstrdup(recbuf);

	QuitCount = split_buf(cmd, &Quit, 0);
	QuitMsg = joinbuf(Quit, QuitCount, 2);

	/* Local Kill Watch For Signoff */
	if (kill_watch) {
		if (strstr(cmd, "Local kill by") && strstr(cmd, "[")
		    && strstr(cmd, "]")) {

			LocalCount = split_buf(lcl, &Local, 0);
			KillMsg = joinbuf(Local, LocalCount, 7);
			chanalert(s_ConnectServ,
				  "\2LOCAL KILL\2 user: \2%s\2 (%s@%s) was Killed by: \2%s\2 - Reason sighted: \2%s\2",
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
	if (sign_watch) {
		chanalert(s_ConnectServ,
			  "\2\0033Signed Off\2 user: %s (%s@%s) at: %s - %s\003",
			  u->nick, u->username, u->hostname,
			  u->server->name, QuitMsg);
	}
	free(QuitMsg);
	free(cmd);
	free(lcl);
	free(Quit);
	QuitCount = 0;
	return 1;
}


/* Routine for MODES message to be echoed */
int cs_user_modes(char **av, int ac)
{
	int add = 1;
	char *modes;
	User *u;

	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_user_modes");

	if (!cs_online)
		return 1;

	if (mode_watch != 1)
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
	switch (*modes) {
	case '+':
		add = 1;
		break;
	case '-':
		add = 0;
		break;
	}

	while (*modes++) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
#ifndef ULTIMATE3
/* these modes in Ultimate3 are Smodes */
#ifdef NETADMIN_MODE
		case NETADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Network Administrator\2 (+%c)\003",
					  u->nick, NETADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\02\00313%s\2 is \2No Longer\2 a \2Network Administrator\2 (-%c)\003",
					  u->nick, NETADMIN_MODE);
			}
			break;
#endif
#ifdef CONETADMIN_MODE
		case CONETADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Co-Network Administrator\2 (+%c)\003",
					  u->nick, CONETADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Co-Network Administrator\2 (-%c)\003",
					  u->nick, CONETADMIN_MODE);
			}
			break;
#endif
#ifdef TECHADMIN_MODE
		case TECHADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Network Technical Administrator\2 (+%c)\003",
					  u->nick, TECHADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Network Technical Administrator\2 (-%c)\003",
					  u->nick, TECHADMIN_MODE);
			}
			break;
#endif
#ifdef SERVERADMIN_MODE
		case SERVERADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Server Administrator\2 (+%c) on \2%s\2\003",
					  u->nick, SERVERADMIN_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Server Administrator\2 (-%c) on \2%s\2\003",
					  u->nick, SERVERADMIN_MODE, u->server->name);
			}
			break;
#endif
#ifdef COSERVERADMIN_MODE
		case COSERVERADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Co-Server Administrator\2 (+%c) on \2%s\2\003",
					  u->nick, COSERVERADMIN_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Co-Server Administrator\2 (-%c) on \2%s\2\003",
					  u->nick, COSERVERADMIN_MODE, u->server->name);
			}
			break;
#endif
#ifdef GUESTADMIN_MODE
		case GUESTADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Guest Administrator\2 (+%c) on \2%s\2\003",
					  u->nick, GUESTADMIN_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Guest Administrator\2 (-%c) on \2%s\2\003",
					  u->nick, GUESTADMIN_MODE, u->server->name);
			}
			break;
/* these modes are not used in Ultimate3 */
#endif
#ifdef BOT_MODE
		case BOT_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Bot\2 (+%c)\003",
					  u->nick, BOT_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Bot\2 (-%c)\003",
					  u->nick, BOT_MODE);
			}
			break;
#endif
#ifdef INVISIBLE_MODE
		case INVISIBLE_MODE:
			if (add) {
				globops(s_ConnectServ,
					"\2%s\2 Is Using \2Invisible Mode\2 (+%c)",
					u->nick, INVISIBLE_MODE);
			} else {
				globops(s_ConnectServ,
					"\2%s\2 Is no longer using \2Invisible Mode\2 (-%c)",
					u->nick, INVISIBLE_MODE);
			}
			break;
#endif
#endif
#ifdef SERVICESADMIN_MODE
		case SERVICESADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Services Administrator\2 (+%c)\003",
					  u->nick, SERVICESADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Services Administrator\2 (-%c)\003",
					  u->nick, SERVICESADMIN_MODE);
			}
			break;
#endif
#ifdef OPER_MODE
		case OPER_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Global Operator\2 (+%c) on \2%s\2\003",
					  u->nick, OPER_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Global Operator\2 (-%c) on \2%s\2\003",
					  u->nick, OPER_MODE, u->server->name);
			}
			break;
#endif
#ifdef LOCOP_MODE
		case LOCOP_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Local Operator\2 (+%c) on \2%s\2\003",
					  u->nick, LOCOP_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Local Operator\2 (-%c) on \2%s\2\003",
					  u->nick, LOCOP_MODE, u->server->name);
			}
			break;
#endif
#ifdef NETSERVICE_MODE
		case NETSERVICE_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Network Service\2 (+%c)\003",
					  u->nick, NETSERVICE_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Network Service\2 (-%c)\003",
					  u->nick, NETSERVICE_MODE);
			}
			break;
#endif
		default:
			break;
		}
	}
	return 1;
}

#ifdef ULTIMATE3
/* smode support for Ultimate3 */
int cs_user_smodes(char **av, int ac)
{
	int add = 1;
	char *modes;
	User *u;

	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_user_modes");

	if (mode_watch != 1)
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
	switch (*modes) {
	case '+':
		add = 1;
		break;
	case '-':
		add = 0;
		break;
	}

	while (*modes++) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
#ifdef NETADMIN_MODE
		case NETADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Network Administrator\2 (+%c)\003",
					  u->nick, NETADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Network Administrator\2 (-%c)\003",
					  u->nick, NETADMIN_MODE);
			}
			break;
#endif
#ifdef CONETADMIN_MODE
		case CONETADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Co-Network Administrator\2 (+%c)\003",
					  u->nick, CONETADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Co-Network Administrator\2 (-%c)\003",
					  u->nick, CONETADMIN_MODE);
			}
			break;
#endif
#ifdef TECHADMIN_MODE
		case TECHADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Network Technical Administrator\2 (+%c)\003",
					  u->nick, TECHADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Network Technical Administrator\2 (-%c)\003",
					  u->nick, TECHADMIN_MODE);
			}
			break;
#endif
#ifdef COTECHADMIN_MODE
		case COTECHADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Network Co-Technical Administrator\2 (+%c)\003",
					  u->nick, COTECHADMIN_MODE);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Network Co-Technical Administrator\2 (-%c)\003",
					  u->nick, COTECHADMIN_MODE);
			}
			break;
#endif
#ifdef SERVERADMIN_MODE
		case SERVERADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Server Administrator\2 (+%c) on \2%s\2\003",
					  u->nick, SERVERADMIN_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Server Administrator\2 (-%c) on \2%s\2\003",
					  u->nick, SERVERADMIN_MODE, u->server->name);
			}
			break;
#endif
#ifdef COSERVERADMIN_MODE
		case COSERVERADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Co-Server Administrator\2 (+%c) on \2%s\2\003",
					  u->nick,COSERVERADMIN_MODE,  u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Co-Server Administrator\2 (-%c) on \2%s\2\003",
					  u->nick, COSERVERADMIN_MODE, u->server->name);
			}
			break;
#endif
#ifdef GUESTADMIN_MODE
		case GUESTADMIN_MODE:
			if (add) {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2Now\2 a \2Guest Administrator\2 (+%c) on \2%s\2\003",
					  u->nick, GUESTADMIN_MODE, u->server->name);
			} else {
				chanalert(s_ConnectServ,
					  "\2\00313%s\2 is \2No Longer\2 a \2Guest Administrator\2 (-%c) on \2%s\2\003",
					  u->nick, GUESTADMIN_MODE, u->server->name);
			}
			break;
#endif
		default:
			break;
		}
	}
	return 1;
}
#endif

/* Routine for KILL message to be echoed */
int cs_user_kill(char **av, int ac)
{
	char *cmd, *GlobalMsg;
	User *u;
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_user_kill");

	if (!cs_online)
		return 1;

	u = finduser(av[0]);
	
	if (!strcasecmp(u->server->name, me.name)) {
		/* its me, forget it */
		return 1;
	}

	cmd = sstrdup(recbuf);
	KillCount = split_buf(cmd, &Kill, 0);
	GlobalMsg = joinbuf(Kill, KillCount, 4);

	if (finduser(Kill[2])) {
		/* it was a User who was killed */
		if (kill_watch)
			chanalert(s_ConnectServ,
				  "\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) was Killed by \2%s\2 - Reason sighted: \2%s\2\003",
				  u->nick, u->username, u->hostname,
				  Kill[0], GlobalMsg);
	} else if (findserver(Kill[2])) {
		if (kill_watch)
			chanalert(s_ConnectServ,
				  "\2SERVER KILL\2 user: \2%s\2 was Killed by the Server \2%s\2 - Reason sighted: \2%s\2",
				  u->nick, Kill[0], GlobalMsg);
	}
	free(GlobalMsg);
	return 1;
}

/* If a user has changed their nick say so */
int cs_user_nick(char **av, int ac)
{
	User *u;
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_user_nick");

	if (!cs_online)
		return 1;

	if (nick_watch) {
		u = finduser(av[1]);
		if (!u)
			return -1
		if (!strcasecmp(u->server->name, me.name)) {
			/* its me, forget it */
			return 1;
		}
		chanalert(s_ConnectServ,
			  "\2\0037Nick Change\2 user: \2%s\2 Changed their nick to \2%s\2\003", av[0],
			  av[1]);
	}
	return 1;
}


static void cs_set_nickwatch(User * u, char **av, int ac)
{
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_nickwatch");

	if (ac < 4) {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}
	if ((!strcasecmp(av[3], "YES")) || (!strcasecmp(av[3], "ON"))) {
		nick_watch = 1;
		chanalert(s_ConnectServ, "\2NICK WATCH\2 Activated by \2%s\2",
			  u->nick);
		nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated NICK WATCH",
		     u->nick, u->username, u->hostname);
		SetConf((void *) 1, CFGBOOL, "NickWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2NICK WATCH\2 Activated");
	} else if ((!strcasecmp(av[3], "NO"))
		   || (!strcasecmp(av[3], "OFF"))) {
		nick_watch = 0;
		chanalert(s_ConnectServ,
			  "\2NICK WATCH\2 Deactivated by \2%s\2", u->nick);
		nlog(LOG_NORMAL, LOG_MOD,
		     "%s!%s@%s Deactivated NICK WATCH", u->nick,
		     u->username, u->hostname);
		SetConf(0, CFGBOOL, "NickWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2NICK WATCH\2 Deactivated");
	} else {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}

}



/* Routine for Signon/Signoff ENABLE or DISABLE */
static void cs_set_signwatch(User * u, char **av, int ac)
{
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_signwatch");

	if (ac < 4) {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}
	if ((!strcasecmp(av[3], "YES")) || (!strcasecmp(av[3], "ON"))) {
		sign_watch = 1;
		chanalert(s_ConnectServ,
			  "\2SIGNON/SIGNOFF WATCH\2 Activated by \2%s\2",
			  u->nick);
		nlog(LOG_NORMAL, LOG_MOD,
		     "%s!%s@%s Activated SIGNON/SIGNOFF WATCH", u->nick,
		     u->username, u->hostname);
		SetConf((void *) 1, CFGBOOL, "SignWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2SIGNON/SIGNOFF WATCH\2 Activated");
	} else if ((!strcasecmp(av[3], "NO"))
		   || (!strcasecmp(av[3], "OFF"))) {
		sign_watch = 0;
		chanalert(s_ConnectServ,
			  "\2SIGNON/SIGNOFF WATCH\2 Deactivated by \2%s\2",
			  u->nick);
		nlog(LOG_NORMAL, LOG_MOD,
		     "%s!%s@%s Deactivated SIGNON/SIGNOFF WATCH", u->nick,
		     u->username, u->hostname);
		SetConf(0, CFGBOOL, "SignWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2SIGNON/SIGNOFF WATCH\2 Deactivated");
	} else {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}

}


/* Routine for kill watch ENABLE or DISABLE */
static void cs_set_killwatch(User * u, char **av, int ac)
{
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_killwatch");

	if (ac < 4) {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}
	if ((!strcasecmp(av[3], "YES")) || (!strcasecmp(av[3], "ON"))) {
		kill_watch = 1;
		chanalert(s_ConnectServ, "\2KILL WATCH\2 Activated by \2%s\2",
			  u->nick);
		nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated KILL WATCH",
		     u->nick, u->username, u->hostname);
		SetConf((void *) 1, CFGBOOL, "KillWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2KILL WATCH\2 Activated");
	} else if ((!strcasecmp(av[3], "NO"))
		   || (!strcasecmp(av[3], "OFF"))) {
		kill_watch = 0;
		chanalert(s_ConnectServ,
			  "\2KILL WATCH\2 Deactivated by \2%s\2", u->nick);
		nlog(LOG_NORMAL, LOG_MOD,
		     "%s!%s@%s Deactivated KILL WATCH", u->nick,
		     u->username, u->hostname);
		SetConf(0, CFGBOOL, "KillWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2KILL WATCH\2 Deactivated");
	} else {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}

}


/* Routine for mode watch ENABLE or DISABLE */
static void cs_set_modewatch(User * u, char **av, int ac)
{
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_modewatch");

	if (ac < 4) {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}
	if ((!strcasecmp(av[3], "YES")) || (!strcasecmp(av[3], "ON"))) {
		mode_watch = 1;
		chanalert(s_ConnectServ, "\2MODE WATCH\2 Activated by \2%s\2",
			  u->nick);
		nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s Activated MODE WATCH",
		     u->nick, u->username, u->hostname);
		SetConf((void *) 1, CFGBOOL, "ModeWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2MODE WATCH\2 Activated");
	} else if ((!strcasecmp(av[3], "NO"))
		   || (!strcasecmp(av[3], "OFF"))) {
		mode_watch = 0;
		chanalert(s_ConnectServ,
			  "\2MODE WATCH\2 Deactivated by \2%s\2", u->nick);
		nlog(LOG_NORMAL, LOG_MOD,
		     "%s!%s@%s Deactivated MODE WATCH", u->nick,
		     u->username, u->hostname);
		SetConf(0, CFGBOOL, "ModeWatch");
		prefmsg(u->nick, s_ConnectServ,
			"\2MODE WATCH\2 Deactivated");
	} else {
		prefmsg(u->nick, s_ConnectServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_ConnectServ);
		return;
	}

}


/* Routine for STATUS echo */
static void cs_set_list(User * u, char **av, int ac)
{
	/* Approximate Segfault Location */
	strcpy(segv_location, "cs_set_list");
	prefmsg(u->nick, s_ConnectServ, "Current %s Settings:",
		s_ConnectServ);
	prefmsg(u->nick, s_ConnectServ, "SIGN WATCH: %s",
		sign_watch ? "Enabled" : "Disabled");
	prefmsg(u->nick, s_ConnectServ, "KILL WATCH: %s",
		kill_watch ? "Enabled" : "Disabled");
	prefmsg(u->nick, s_ConnectServ, "MODE WATCH: %s",
		mode_watch ? "Enabled" : "Disabled");
	prefmsg(u->nick, s_ConnectServ, "NICK WATCH: %s",
		nick_watch ? "Enabled" : "Disabled");
}

/* Load ConnectServ Config file and set defaults if does not exist */
void Loadconfig()
{
	strcpy(segv_location, "cs_Loadconfig");
	strcpy(segvinmodule, my_info[0].module_name);

	/* some defaults */
	sign_watch = 1;
	kill_watch = 1;
	mode_watch = 1;
	nick_watch = 1;

	GetConf((void *) &sign_watch, CFGBOOL, "SignWatch");
	GetConf((void *) &kill_watch, CFGBOOL, "KillWatch");
	GetConf((void *) &mode_watch, CFGBOOL, "ModeWatch");
	GetConf((void *) &nick_watch, CFGBOOL, "NickWatch");
}
