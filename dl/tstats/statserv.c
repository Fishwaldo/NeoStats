/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      statserv.c 
** Version: 3.0+Alpha 0.0.0.1
** Date:    12/30/2001
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "statserv.h"
#include "ss_help.c"

extern const char version_date[], version_time[];
static int new_m_version(char *av, char *tmp);

static void ss_JOIN(User *u, char *chan);
static void ss_version(User *u);


const char meversion_date[] = __DATE__;
const char meversion_time[] = __TIME__;
char *s_StatServ;

Module_Info StatServ_Info[] = { {
	"StatServ",
	"An IRC Statistical Bot",
	"3.0+Alpha-0.0.0.1"
} };


int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module StatServ Test Loaded, Version: %s %s  %s",me.name,av,StatServ_Info[0].module_version,meversion_date,meversion_time);
	return 0;
}

Functions StatServ_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
        char *cmd;
	u = finduser(origin);
	if (!u) {
		log("Unable to find user %s (statserv)", origin);
		return -1;
	}
/*
	if (u->is_oper)
		return -1;
*/
    if (coreLine == NULL) return -1;
    cmd = strtok(coreLine, " ");


/*    if (!strcasecmp(cmd, "HELP")) {
        coreLine = strtok(NULL, " ");
        if(!coreLine) {
            notice(s_StatServ, "%s requested %s Help", u->nick, s_StatServ); */

    if (!strcasecmp(cmd, "HELP")) {
        coreLine = strtok(NULL, " ");
        if(!coreLine) {
            privmsg(u->nick, s_StatServ, "\2STATSERV IS BEING COMPLETELY REWROTE ATM -- ONLY VERSION WORKS ATM)");

            privmsg_list(u->nick, s_StatServ, ss_help);
            if (UserLevel(u) >= 150)
                privmsg_list(u->nick, s_StatServ, ss_myuser_help);
        } else if (!strcasecmp(coreLine, "VERSION")) 
            privmsg_list(u->nick, s_StatServ, ss_version_help); 
        else if (!strcasecmp(coreLine, "JOIN") && UserLevel(u) >= 190)
            privmsg_list(u->nick, s_StatServ, ss_join_help);

        else
            privmsg(u->nick, s_StatServ, "Unknown Help Topic: \2%s\2 (VERSION ONLY WORKS ATM -- RECODE)", coreLine);

    } else if (!strcasecmp(cmd, "VERSION")) {
        ss_version(u);
    } else if (!strcasecmp(cmd, "JOIN") && (UserLevel(u) >= 185)) {
        cmd = strtok(NULL, " ");
        ss_JOIN(u, cmd);
    } else {
        privmsg(u->nick, s_StatServ, "Unknown Command: \2%s\2 (VERSION ONLY WORKS ATM -- RECODE)", cmd);
    }
    return 1;

/*


	notice(s_StatServ,"We got a 
message! It 
says \2%s", coreLine);
	privmsg(u->nick, s_StatServ, "I'm not the real StatServ, IM AN IMPOSTER!");
	return 1;
*/

}

int Online(Server *data) {

	if (init_bot(s_StatServ,"please",me.name,"Chat to me", "+xd", StatServ_Info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_StatServ = strcat(s_StatServ, "_");
		init_bot(s_StatServ,"Please",me.name,"Chat to me", "+xd", StatServ_Info[0].module_name);
	}
	return 1;
};


EventFnList StatServ_event_list[] = {
	{ "ONLINE", 	Online},
	{ NULL, 	NULL}
};



Module_Info *__module_get_info() {
	return StatServ_Info;
};

Functions *__module_get_functions() {
	return StatServ_fn_list;
};

EventFnList *__module_get_events() {
	return StatServ_event_list;
};

void _init() {
	s_StatServ = "StatServ";
	sts(":%s GLOBOPS :StatServ Test Module Loaded",me.name);
}


void _fini() {
	sts(":%s GLOBOPS :StatServ Test Module Unloaded",me.name);

};


static void ss_JOIN(User *u, char *chan)
{
    if (UserLevel(u) < 190) {
        log("Access Denied (JOIN) to %s", u->nick);
        privmsg(u->nick, s_StatServ, "Access Denied.");
        notice(s_StatServ,"%s Requested JOIN, but is not a god!",u->nick);
        return;
    }
    if (!chan) {
        privmsg(u->nick, s_StatServ, "Syntax: /msg %s JOIN <chan>",s_StatServ);
        return;
    }
    globops(s_StatServ, "JOINING CHANNEL -\2(%s)\2- Thanks to %s!%s@%s)", chan, u->nick, u->username, u->hostname);
    privmsg(me.chan, s_StatServ, "%s Asked me to Join %s, So, I'm Leaving %s", u->nick, chan, me.chan);
    sts(":%s part %s", s_StatServ, me.chan);
    log("%s!%s@%s Asked me to Join %s, I was on %s", u->nick, u->username, u->hostname, chan, me.chan);
    sts(":%s JOIN %s",s_StatServ,chan);
    sts(":%s MODE %s +o %s",me.name,chan,s_StatServ);
}


static void ss_version(User *u)
{
        privmsg(u->nick, s_StatServ, "\2StatServ Version Information\2");
        privmsg(u->nick, s_StatServ, "StatServ Version: %s Compiled %s at %s", StatServ_Info[0].module_version,  meversion_date, meversion_time);
        privmsg(u->nick, s_StatServ, "http://www.neostats.net"); 
	privmsg(u->nick, s_StatServ, "-------------------------------------");
	privmsg(u->nick, s_StatServ, "This StatServ is being completely re-wrote!");
}
