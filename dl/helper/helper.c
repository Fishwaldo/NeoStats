/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: helper.c,v 1.1 2000/04/08 12:44:15 fishwaldo Exp $
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "dotconf.h"

const char helpversion_date[] = __DATE__;
const char helpversion_time[] = __TIME__;

char *s_Helper;

void ss_cb_Config(char *, int);

static config_option options[] = {
{ "HELPER_NICK", ARG_STR, ss_cb_Config, 0},
{ "HELPER_USER", ARG_STR, ss_cb_Config, 1},
{ "HELPER_HOST", ARG_STR, ss_cb_Config, 2},
{ "HELPER_VCHAN", ARG_STR, ss_cb_Config, 3}
};

void ss_cb_Config(char *arg, int configtype) {
	if (configtype == 0) {
		/* Nick */
		memcpy(IcqServ.nick, arg, MAXNICK);
		memcpy(s_Icq, IcqServ.nick, MAXNICK);
#ifdef DEBUG
		log("IcqServ nick :%s ", arg);
#endif
	} else if (configtype == 1) {
		/* User */
		memcpy(IcqServ.user, arg, 8);
	} else if (configtype == 2) {
		/* host */
		memcpy(IcqServ.host, arg, MAXHOST);
	} else if (configtype == 3) {
		/* uin */
		IcqServ.uin = atoi(arg);
	} else if (configtype == 4) {
		/* Server */
		memcpy(IcqServ.server, arg, MAXHOST);
	}  else if (configtype == 5) {
		/* Server */
		memcpy(IcqServ.passwd, arg, MAXPASS);
	}
}

Module_Info my_info[] = { {
	"Helper",
	"A Helper (+h) Module to give Helpers extra commands",
	"1.0"
} };

int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module Helper Loaded, Version: %s %s %s",me.name,av,my_info[0].module_version,helpversion_date,helpversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
	char *cmd, *nick, *chan ;
	char *tmpcoreLine;
	
	u = finduser(origin);
	if (findbot(u->nick)) return 1;
	if (!u) {
		log("Unable to find user %s (NetInfo)", origin);
		return -1;
	}
	tmpcoreLine = sstrdup(coreLine);
	cmd = strtok(tmpcoreLine, " ");
	if (!strcasecmp(cmd, "VJOIN")) {
		if (u->Umode & UMODE_HELPOP) {
			nick = strtok(NULL, " ");
			if (!nick) {
				privmsg(u->nick, s_Helper, "Incorrect Syntax: /msg %s vjoin <nick> <channel> <reason>", s_Helper);
				return -1;
			}
			chan = strtok(NULL, " ");
			if (!chan) {
				privmsg(u->nick, s_Helper, "Incorrect Syntax: /msg %s vjoin <nick> <channel> <reason>", s_Helper);
				return -1;
			}

		} else {
			privmsg(u->nick, s_Helper, "Access Denied");
		}
	}
	return 1;
}

int Online(Server *data) {

	if (init_bot(s_Helper,"Service",me.name,"Helper Service", "+Sd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_Helper = strcat(s_Helper, "_");
		init_bot(s_Helper,"Service",me.name,"Helper Service", "+Sd", my_info[0].module_name);
	}
	return 1;
};


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
	{ NULL, 	NULL}
};



Module_Info *__module_get_info() {
	return my_info;
};

Functions *__module_get_functions() {
	return my_fn_list;
};

EventFnList *__module_get_events() {
	return my_event_list;
};

void _init() {
	s_Helper = "Helper";
	sts(":%s GLOBOPS :Helper Module Loaded",me.name);
}


void _fini() {
	sts(":%s GLOBOPS :Helper Module Unloaded",me.name);
};
