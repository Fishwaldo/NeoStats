/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
** $Id$
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "conf.h"
#include "log.h"

char *s_Spam;

ModuleInfo __module_info = {
   "Spam",
   "A User to Help Catch Spammers on the IRC network",
   "1.2",
	__DATE__,
	__TIME__
};

int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(RPL_VERSION, origin, "Module Spam Loaded, Version: %s %s %s",
		__module_info.module_version, __module_info.module_build_date,
		__module_info.module_build_time);
	return 0;
}

Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1}
	,
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1}
	,
#endif
	{NULL, NULL, 0}
};

	/* an easter egg for all the Neo users */

int __ChanMessage(char *origin, char **argv, int argc)
{
	FILE *fort;
	char *fortune;
	char *chan = argv[0];
	if (!strcasecmp(argv[1], s_Spam)) {
		fort = popen("/usr/games/fortune", "r");
		if (fort) {
			fortune = malloc(255);
			while ((fortune = fgets(fortune, 255, fort))) {
				privmsg(chan, s_Spam, fortune);
			}
			free(fortune);
			pclose(fort);
		}
	}
	return 1;
}



int __BotMessage(char *origin, char **argv, int argc)
{
	User *u;
	char *buf;
	u = finduser(origin);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Unable to find user %s (spam)", origin);
		return -1;
	}
/* 	if (u->is_oper)
		return -1;
*/
	buf = joinbuf(argv, argc, 1);
	globops(me.name, "Possible Mass Message -\2(%s!%s@%s)\2- %s",
		u->nick, u->username, u->hostname, buf);
	chanalert(s_Spam,
		  "WooHoo, A Spammer has Spammed! -\2(%s!%s@%s)\2- Sent me this: %s",
		  u->nick, u->username, u->hostname, buf);
	nlog(LOG_CRITICAL, LOG_MOD,
	     "Possible Mass Message -(%s!%s@%s)- %s", u->nick, u->username,
	     u->hostname, buf);
	free(buf);
	return 1;
}

int Online(char **av, int ac)
{
	char *user;
	char *host;
	char *rname;

	if (GetConf((void *) &s_Spam, CFGSTR, "Nick") < 0) {
		s_Spam = "sumyungguy";
	}
	if (GetConf((void *) &user, CFGSTR, "User") < 0) {
		user = malloc(MAXUSER);
		strlcpy(user, "please", MAXUSER);
	}
	if (GetConf((void *) &host, CFGSTR, "Host") < 0) {
		host = malloc(MAXHOST);
		strlcpy(host, me.name, MAXHOST);
	}
	if (GetConf((void *) &rname, CFGSTR, "RealName") < 0) {
		rname = malloc(MAXREALNAME);
		strlcpy(rname, "Chat to me", MAXREALNAME);
	}


	if (init_bot
	    (s_Spam, user, host, rname, "-x",
	     __module_info.module_name) == -1) {
		/* Nick was in use!!!! */
		s_Spam = strcat(s_Spam, "_");
		init_bot(s_Spam, user, host, rname, "-x",
			 __module_info.module_name);
	}
	free(user);
	free(host);
	free(rname);
	return 1;
};


EventFnList __module_events[] = {
	{EVENT_ONLINE, Online}
	,
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
	s_Spam = "sumyungguy";
	return 1;
}


void __ModFini()
{

};
