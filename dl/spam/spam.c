/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: spam.c,v 1.11 2002/07/13 06:30:36 fishwaldo Exp $
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"

const char spamversion_date[] = __DATE__;
const char spamversion_time[] = __TIME__;
char *s_Spam;

Module_Info my_info[] = { {
	"Spam",
	"A User to Help Catch Spammers on the IRC network",
	"1.0"
} };


int new_m_version(char *origin, char **av, int ac) {
	snumeric_cmd(351,origin, "Module Spam Loaded, Version: %s %s %s",my_info[0].module_version,spamversion_date,spamversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ MSG_VERSION,	new_m_version,	1 },
	{ TOK_VERSION,	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};
	/* a easter egg for all the Neo users */

int __Chan_Message(char *chan, char **argv, int argc)
{
	FILE *fort;
	char *fortune;
	fortune = malloc(255);
	if (!strcasecmp(argv[1], s_Spam)) {
		fort = popen("/usr/games/fortune", "r");
		if (fort) {
			while ((fortune = fgets(fortune, 255, fort))) {
				privmsg(chan, s_Spam, "%s", fortune);
			}
			pclose(fort);
		}
 	}
	return 1;	
}



int __Bot_Message(char *origin, char **argv, int argc)
{
	User *u;
	u = finduser(origin); 
	if (!u) { 
		log("Unable to find user %s (spam)", origin); 
		return -1; 
	} 
/* 	if (u->is_oper)
		return -1;
*/	
	globops(me.name, "Possible Mass Message -\2(%s!%s@%s)\2- %s", u->nick,
		u->username, u->hostname, argv[1]);
	notice(s_Spam,"WooHoo, A Spammer has Spammed! -\2(%s!%s@%s)\2- Sent me this: %s",u->nick,u->username,u->hostname,argv[1]);
	log("Possible Mass Message -(%s!%s@%s)- %s", u->nick, u->username,
		u->hostname, argv[1]);
	return 1;
}

int Online(char **av, int ac) {

	if (init_bot(s_Spam,"please",me.name,"Chat to me", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_Spam = strcat(s_Spam, "_");
		init_bot(s_Spam,"Please",me.name,"Chat to me", "+xd", my_info[0].module_name);
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
	s_Spam = "sumyungguy";
	globops(me.name, "Spam Module Loaded");
}


void _fini() {
	globops(me.name, "Spam Module Unloaded");

};
