/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: spam.c,v 1.1 2000/02/03 23:45:59 fishwaldo Exp $
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


int new_m_version(char *av, char *tmp) {
	sts(":%s 351 %s :Module Spam Loaded, Version: %s %s %s",me.name,av,my_info[0].module_version,spamversion_date,spamversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ "VERSION",	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};

void TimerSpam()
{
	register char c;
	register int i;
	char *t;

	srand(time(NULL));

	t = sstrdup(s_Spam);
	bzero(s_Spam, sizeof(s_Spam));
	for (i = 0; i < sizeof(s_Spam-1); i++) {
		c = (((u_short)rand())%26)+97;
		s_Spam[i] = c;
	}

	if (servsock > 0) {
		if (bot_nick_change(t,s_Spam) == 1) {
			notice(s_Spam, "Spam Users Nick is now: %s",s_Spam);
		} else {
			/* The newnick already existed on the network */
			log("ehh, couldn't change my nick!");
			notice(t, "Attempted to change nick to %s but its already in use",s_Spam);
			for (i = 0; i < sizeof(t-1); i++) {
				s_Spam[i] = t[i];
			}			
		}		
	}

	free(t);
}

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
	u = finduser(origin);
	if (!u) {
		log("Unable to find user %s (spam)", origin);
		return -1;
	}

/*	if (u->is_oper)
		return -1;
*/
	globops(me.name, "Possible Mass Message -\2(%s!%s@%s)\2- %s", u->nick,
		u->username, u->hostname, coreLine);
	notice(s_Spam,"WooHoo, A Spammer has Spammed! -\2(%s!%s@%s)\2- Sent me this: %s",u->nick,u->username,u->hostname,coreLine);

	log("Possible Mass Message -(%s!%s@%s)- %s", u->nick, u->username,
		u->hostname, coreLine);
	return 1;
}

int Online() {
	if (init_bot(s_Spam,"please",me.name,"Chat to me", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_Spam = strcat(s_Spam, "_");
		init_bot(s_Spam,"Please",me.name,"Chat to me", "+xd", my_info[0].module_name);
	}
	add_mod_timer("TimerSpam","Spam_Nick",my_info[0].module_name, 300);
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
	s_Spam = "Spam";
	sts(":%s GLOBOPS :Spam Module Loaded",me.name);
}


void _fini() {
	log("unloading Spam");
	sts(":%s GLOBOPS :Spam Module Unloaded",me.name);

};