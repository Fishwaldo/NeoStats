/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: spam.c,v 1.6 2002/03/06 15:37:56 fishwaldo Exp $
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
	char temp[5];

	srand(time(NULL));

	bzero(temp, sizeof(temp));

	for (i = 0; i < sizeof(temp)-1; i++) {
		c = (((u_short)rand())%26)+97;
		temp[i] = c;

	}
/*	if (servsock > 0) {
		if (bot_nick_change(s_Spam, temp) == 1) {
			s_Spam = sstrdup(temp);
			notice(s_Spam, "Spam Users Nick is now: %s",temp);
		} else { */
/* ToDo: Add routine if nick is in use, to find another nick */
/*			return;
		}		
	} */
return;
}

int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
/*	u = finduser(origin); */
	if (!u) { 
		log("Unable to find user %s (spam)", origin); 
		return -1; 
	} 
/* 	if (u->is_oper)
		return -1;
*/	
	strtok(coreLine, " ");
	globops(me.name, "Possible Mass Message -\2(%s!%s@%s)\2- %s", u->nick,
		u->username, u->hostname, coreLine);
	notice(s_Spam,"WooHoo, A Spammer has Spammed! -\2(%s!%s@%s)\2- Sent me this: %s",u->nick,u->username,u->hostname,coreLine);
	log("Possible Mass Message -(%s!%s@%s)- %s", u->nick, u->username,
		u->hostname, coreLine);
	return 1;
}

int Online(Server *data) {

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
	s_Spam = "sumyungguy";
	sts(":%s GLOBOPS :Spam Module Loaded",me.name);
}


void _fini() {
	sts(":%s GLOBOPS :Spam Module Unloaded",me.name);

};
