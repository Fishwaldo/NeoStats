/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
** $Id: spam.c,v 1.18 2002/09/04 08:40:29 fishwaldo Exp $
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
	"1.1"
} };


int new_m_version(char *origin, char **av, int ac) {
	snumeric_cmd(351,origin, "Module Spam Loaded, Version: %s %s %s",my_info[0].module_version,spamversion_date,spamversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ MSG_VERSION,	new_m_version,	1 },
#ifdef HAVE_TOKEN_SUP
	{ TOK_VERSION,	new_m_version,	1 },
#endif
	{ NULL,		NULL,		0 }
};
	/* a easter egg for all the Neo users */

int __Chan_Message(char *origin, char *chan, char **argv, int argc)
{
	FILE *fort;
	char *fortune;
	fortune = malloc(255);
	if (!strcasecmp(argv[1], s_Spam)) {
		fort = popen("/usr/local/games/fortune", "r");
		if (!fort) fort = popen("/usr/games/fortune", "r");
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
	char *buf;
	u = finduser(origin); 
	if (!u) { 
		log("Unable to find user %s (spam)", origin); 
		return -1; 
	} 
/* 	if (u->is_oper)
		return -1;
*/	
	buf = joinbuf(argv, argc, 1);
	globops(me.name, "Possible Mass Message -\2(%s!%s@%s)\2- %s", u->nick,
		u->username, u->hostname, buf);
	chanalert(s_Spam,"WooHoo, A Spammer has Spammed! -\2(%s!%s@%s)\2- Sent me this: %s",u->nick,u->username,u->hostname,buf);
	log("Possible Mass Message -(%s!%s@%s)- %s", u->nick, u->username,
		u->hostname, buf);
	free(buf);
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
