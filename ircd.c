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
** $Id: ircd.c,v 1.129 2003/06/26 06:00:43 fishwaldo Exp $
*/
#include <setjmp.h>
#include "stats.h"
#include "ircd.h"
#include "dl.h"
#include "log.h"

extern const char protocol_version[];

extern IntCommands cmd_list[];

int init_bot(char *nick, char *user, char *host, char *rname, char *modes,
	     char *mod_name)
{
	User *u;
	char **av;
	int ac = 0;
	int add = 0;
	int i;
	long Umode;
	char tmpmode;


	strcpy(segv_location, "init_bot");
	u = finduser(nick);
	if (u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Attempting to Login with a Nickname that already Exists: %s",
		     nick);
		return -1;
	}
	if (strlen(user) > 8) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Warning, %s bot %s has a username longer than 8 chars. Some IRCd's don't like that",
		     mod_name, nick);
	}
	add_mod_user(nick, mod_name);
	Umode = 0;
	tmpmode = *(modes);
	while (tmpmode) {
		switch (tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0;
			     i <
			     ((sizeof(usr_mds) / sizeof(usr_mds[0])) - 1);
			     i++) {
				if (usr_mds[i].mode == tmpmode) {
					if (add) {
						Umode |= usr_mds[i].umodes;
						break;
					} else {
						Umode &=
						    ~usr_mds[i].umodes;
						break;
					}
				}
			}
		}
		tmpmode = *modes++;
	}
	SignOn_NewBot(nick, user, host, rname, Umode);
	AddStringToList(&av, nick, &ac);
	Module_Event("SIGNON", av, ac);
	free(av);
	/* restore segvinmodule from SIGNON */
	strcpy(segvinmodule, mod_name);
	return 1;
}

int del_bot(char *nick, char *reason)
{
	User *u;
	char **av;
	int ac = 0;
	strcpy(segv_location, "del_bot");
	u = finduser(nick);
	nlog(LOG_DEBUG1, LOG_CORE, "Killing %s for %s", nick, reason);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Attempting to Logoff with a Nickname that does not Exists: %s",
		     nick);
		return -1;
	}
	AddStringToList(&av, nick, &ac);
	Module_Event("SIGNOFF", av, ac);
	free(av);
	squit_cmd(nick, reason);
	del_mod_user(nick);
	return 1;
}




void Module_Event(char *event, char **av, int ac)
{
	Module *module_ptr;
	EventFnList *ev_list;
	hscan_t ms;
	hnode_t *mn;

	strcpy(segv_location, "Module_Event");
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		module_ptr = hnode_get(mn);
		ev_list = module_ptr->other_funcs;
		if (ev_list) {
			while (ev_list->cmd_name != NULL) {
				/* This goes through each Command */
				if (!strcasecmp(ev_list->cmd_name, event)) {
					nlog(LOG_DEBUG1, LOG_CORE,
					     "Running Module %s for Comamnd %s -> %s",
					     module_ptr->info->module_name,
					     event, ev_list->cmd_name);
					strcpy(segv_location,
					       module_ptr->info->
					       module_name);
					strcpy(segvinmodule,
					       module_ptr->info->
					       module_name);
					if (setjmp(sigvbuf) == 0) {
						ev_list->function(av, ac);
					} else {
						nlog(LOG_CRITICAL,
						     LOG_CORE,
						     "setjmp() Failed, Can't call Module %s\n",
						     module_ptr->info->
						     module_name);
					}
					strcpy(segvinmodule, "");
					strcpy(segv_location,
					       "Module_Event_Return");
					break;
				}
				ev_list++;
			}
		}
	}

}


/* Taken from Epona - Thanks! */

/*************************************************************************/

/* split_buf:  Split a buffer into arguments and store the arguments in an
 *             argument vector pointed to by argv (which will be malloc'd
 *             as necessary); return the argument count.  If colon_special
 *             is non-zero, then treat a parameter with a leading ':' as
 *             the last parameter of the line, per the IRC RFC.  Destroys
 *             the buffer by side effect.
 */

extern int split_buf(char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int flag = 0;
	int colcount = 0;

	*argv = calloc(sizeof(char *) * argvsize, 1);
	argc = 0;
	if (*buf == ':')
		buf++;
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = realloc(*argv, sizeof(char *) * argvsize);
		}
#if 0
		if ((colon_special == 1) && (*buf == ':')) {
			(*argv)[argc++] = buf + 1;
			buf = "";
			flag = 1;
		}
#endif
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
		}
		s = strpbrk(buf, " ");
		if (s) {
			*s++ = 0;
			while (isspace(*s))
				s++;
		} else {
			s = buf + strlen(buf);
		}
		if (*buf == 0) {
			buf++;
		}
		(*argv)[argc++] = buf;
		buf = s;
	}
	return argc - flag;
}

extern char *joinbuf(char **av, int ac, int from)
{
	int i;
	char *buf;
	char buf1[512];

	buf = malloc(512);
	snprintf(buf, 512, "%s", av[from]);
	for (i = from + 1; i < ac; i++) {
		snprintf(buf1, 512, "%s %s", buf, av[i]);
		strncpy(buf, buf1, 512);
	}
	return (char *) buf;
}


void parse(char *line)
{
	char origin[64], cmd[64], *coreLine;
	char *nick;
	int cmdptr = 0;
	int I = 0;
	int ac;
	char **av;
	Module *module_ptr;
	Functions *fn_list;
	Mod_User *list;
	hscan_t ms;
	hnode_t *mn;

	strcpy(segv_location, "parse");

	strip(line);
	strncpy(recbuf, line, BUFSIZE);
	if (!(*line))
		return;
	nlog(LOG_DEBUG1, LOG_CORE, "R: %s", line);

	if (*line == ':') {
		coreLine = strpbrk(line, " ");
		if (!coreLine)
			return;
		*coreLine = 0;
		while (isspace(*++coreLine));
		strncpy(origin, line + 1, sizeof(origin));
		memmove(line, coreLine, strlen(coreLine) + 1);
		cmdptr = 1;
	} else {
		cmdptr = 0;
		*origin = 0;
	}
	if (!*line)
		return;
	coreLine = strpbrk(line, " ");
	if (coreLine) {
		*coreLine = 0;
		while (isspace(*++coreLine));
	} else
		coreLine = line + strlen(line);
	strncpy(cmd, line, sizeof(cmd));

	ac = split_buf(coreLine, &av, 1);



	/* First, check if its a privmsg, and if so, handle it in the correct Function */
	if (!strcasecmp("PRIVMSG", cmd) || (!strcasecmp("!", cmd))) {



		/* its a privmsg, now lets see who too... */
		if (strstr(av[0], "!")) {
			strncpy(cmd, av[0], 64);
			nick = strtok(cmd, "!");
		} else if (strstr(av[0], "@")) {
			strncpy(cmd, av[0], 64);
			nick = strtok(cmd, "@");
		} else {
			nick = malloc(64);
			strncpy(nick, av[0], 64);
			I = 1;
		}

		if (!strcasecmp(s_Services, nick)) {
			if (flood(finduser(origin))) {
				free(av);
				return;
			}
			/* its to the Internal Services Bot */
			strcpy(segv_location, "servicesbot");
			servicesbot(origin, av, ac);
			strcpy(segv_location, "ServicesBot_return");
			if (I == 1)
				free(nick);

			free(av);
			return;
		} else {
			list = findbot(nick);
			/* Check to see if any of the Modules have this nick Registered */
			if (list) {
				nlog(LOG_DEBUG1, LOG_CORE, "nicks: %s",
				     list->nick);
				if (flood(finduser(origin))) {
					free(av);
					return;
				}

				/* Check to make sure there are no blank spaces so we dont crash */
				if (strlen(av[1]) >= 350) {
					prefmsg(origin, s_Services,
						"command line too long!");
					notice(s_Services,
					       "%s tried to send a very LARGE command, we told them to shove it!",
					       origin);
					free(av);
					return;
				}

				strcpy(segv_location, list->modname);
				strcpy(segvinmodule, list->modname);
				if (setjmp(sigvbuf) == 0) {
					list->function(origin, av, ac);
				}
				strcpy(segvinmodule, "");
				strcpy(segv_location,
				       "Return from Module Message");
				free(av);
				if (I == 1)
					free(nick);
				return;
			} else {
				bot_chan_message(origin, av[0], av, ac);
				if (I == 1)
					free(nick);
				free(av);
				return;
			}
		}
	}

	/* now, Parse the Command to the Internal Functions... */
	strcpy(segv_location, "Parse - Internal Functions");
	for (I = 0; I < ircd_srv.cmdcount; I++) {
		if (!strcmp(cmd_list[I].name, cmd)) {
			if (cmd_list[I].srvmsg == cmdptr) {
				strcpy(segv_location, cmd_list[I].name);
				cmd_list[I].function(origin, av, ac);
				cmd_list[I].usage++;
				break;
			}
		}
	}
	/* K, now Parse it to the Module functions */
	strcpy(segv_location, "Parse - Module Functions");
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		module_ptr = hnode_get(mn);
		fn_list = module_ptr->function_list;
		while (fn_list->cmd_name != NULL) {
			/* This goes through each Command */
			if (!strcasecmp(fn_list->cmd_name, cmd)) {
				if (fn_list->srvmsg == cmdptr) {
					nlog(LOG_DEBUG1, LOG_CORE,
					     "Running Module %s for Function %s",
					     module_ptr->info->module_name,
					     fn_list->cmd_name);
					strcpy(segv_location,
					       module_ptr->info->
					       module_name);
					strcpy(segvinmodule,
					       module_ptr->info->
					       module_name);
					if (setjmp(sigvbuf) == 0) {
						fn_list->function(origin,
								  av, ac);
					}
					strcpy(segvinmodule, "");
					strcpy(segv_location,
					       "Parse_Return_Module");
					break;
				}
			}
			fn_list++;
		}
	}
	free(av);
}









/* Here are the Following Internal Functions.
they should update the internal Structures */

void init_ServBot()
{
	char rname[63];
	char **av;
	int ac = 0;
	strcpy(segv_location, "init_ServBot");
	if (finduser(s_Services))
		/* nick already exists on the network */
		snprintf(s_Services, MAXNICK, "NeoStats1");
	snprintf(rname, 63, "/msg %s \2HELP\2", s_Services);
	SignOn_NewBot(s_Services, Servbot.user, Servbot.host, rname,
		      UMODE_SERVICES);
	me.onchan = 1;
	AddStringToList(&av, me.uplink, &ac);
	Module_Event("ONLINE", av, ac);
	free(av);
	ac = 0;
	AddStringToList(&av, s_Services, &ac);
	Module_Event("SIGNON", av, ac);
	free(av);

}

void dopong(Server * s)
{
	char **av;
	int ac = 0;
	if (s) {
		s->ping = time(NULL) - ping.last_sent;
		if (ping.ulag > 1)
			s->ping -= (float) ping.ulag;
		if (!strcmp(me.s->name, s->name))
			ping.ulag = me.s->ping;
		AddStringToList(&av, s->name, &ac);
		Module_Event("PONG", av, ac);
		free(av);
	} else {
		nlog(LOG_NOTICE, LOG_CORE,
		     "Received PONG from unknown server: %s", recbuf);
	}
}



int flood(User * u)
{
	time_t current = time(NULL);

	if (UserLevel(u) >= 40)	/* locop or higher */
		return 0;
	if (current - u->t_flood > 10) {
		u->t_flood = time(NULL);
		u->flood = 0;
		return 0;
	}
	if (u->flood >= 5) {
		nlog(LOG_NORMAL, LOG_CORE, "FLOODING: %s!%s@%s", u->nick,
		     u->username, u->hostname);
		skill_cmd(s_Services, u->nick,
			  "%s!%s (Flooding Services.)", Servbot.host,
			  s_Services);
#if 0
		DelUser(u->nick);
#endif
		return 1;
	} else {
		u->flood++;
	}
	return 0;
}

/* Display our MOTD Message of the Day from the external neostats.motd file */
void ShowMOTD(char *nick)
{
	FILE *fp;
	char buf[BUFSIZE];

	snumeric_cmd(375, nick, ":- %s Message of the Day -", me.name);
	snumeric_cmd(372, nick,
		     ":- %d.%d.%d%s. Copyright (c) 1999 - 2002 The NeoStats Group",
		     MAJOR, MINOR, REV, version);
	snumeric_cmd(372, nick, ":-");

	fp = fopen("neostats.motd", "r");

	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			buf[strlen(buf) - 1] = 0;
			snumeric_cmd(372, nick, ":- %s", buf);
		}
		fclose(fp);
	} else {
		snumeric_cmd(372, nick, ":- MOTD file Missing");
	}
	snumeric_cmd(376, nick, ":End of /MOTD command.");
}


/* Display the ADMIN Message from the external stats.admin file */
void ShowADMIN(char *nick)
{
	FILE *fp;
	char buf[BUFSIZE];

	snumeric_cmd(256, nick, ":- %s NeoStats Admins -", me.name);
	snumeric_cmd(256, nick,
		     ":- %d.%d.%d%s.  Copyright (c) 1999 - 2002 The NeoStats Group",
		     MAJOR, MINOR, REV, version);

	fp = fopen("stats.admin", "r");

	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			buf[strlen(buf) - 1] = 0;
			snumeric_cmd(257, nick, ":- %s", buf);
		}
		fclose(fp);
	}
	snumeric_cmd(258, nick, ":End of /ADMIN command.");
}


void Showcredits(char *nick)
{
	snumeric_cmd(351, nick, ":- NeoStats %d.%d.%d%s Credits ", MAJOR,
		     MINOR, REV, version);
	snumeric_cmd(351, nick,
		     ":- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	snumeric_cmd(351, nick,
		     ":- For Support, you can find ^Enigma^ or Shmad at");
	snumeric_cmd(351, nick, ":- irc.irc-chat.net #NeoStats");
	snumeric_cmd(351, nick, ":- Thanks to:");
	snumeric_cmd(351, nick,
		     ":- \2Fish\2 still part of the team with patch submissions.");
	snumeric_cmd(351, nick,
		     ":- Stskeeps for Writting the best IRCD ever!");
	snumeric_cmd(351, nick,
		     ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)");
	snumeric_cmd(351, nick,
		     ":- monkeyIRCD for the Module Segv Catching code");
	snumeric_cmd(351, nick,
		     ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!");
	snumeric_cmd(351, nick, ":- Andy For Ideas");
	snumeric_cmd(351, nick,
		     ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies");
	snumeric_cmd(351, nick,
		     ":- sre and Jacob for development systems and access");
	snumeric_cmd(351, nick,
		     ":- Error51 for Translating our FAQ and README files");
	snumeric_cmd(351, nick,
		     ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!");
	snumeric_cmd(351, nick,
		     ":- Eggy for proving to use our code still had bugs when we thought it didn't (and all the bug reports!)");
	snumeric_cmd(351, nick,
		     ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips :)");
	snumeric_cmd(351, nick,
		     ":- M - Updating lots of Doco and code and providing lots of great feedback");
}

void ShowStats(char *what, User * u)
{
	time_t tmp;
	time_t tmp2;
	int I;
#ifdef EXTAUTH
	int dl;
	int (*listauth) (User * u);
#endif

	if (!u) {
		return;
	}
	if (!strcasecmp(what, "u")) {
		/* server uptime - Shmad */
		int uptime = time(NULL) - me.t_start;
		snumeric_cmd(242, u->nick,
			     "Statistical Server up %d days, %d:%02d:%02d",
			     uptime / 86400, (uptime / 3600) % 24,
			     (uptime / 60) % 60, uptime % 60);
	} else if (!strcasecmp(what, "c")) {
		/* Connections */
		snumeric_cmd(214, u->nick, "N *@%s * * %d 50", me.uplink,
			     me.port);
		snumeric_cmd(213, u->nick, "C *@%s * * %d 50", me.uplink,
			     me.port);
	} else if (!strcasecmp(what, "o")) {
		/* Operators */
#ifdef EXTAUTH
		dl = get_dl_handle("extauth");
		if (dl > 0) {
			listauth = dlsym((int *) dl, "__list_auth");
			if (listauth)
				(*listauth) (u);
		} else
#endif
			snumeric_cmd(243, u->nick,
				     "Operators think they are God, but you and I know they are not!");
	} else if (!strcasecmp(what, "l")) {
		/* Port Lists */
		tmp = time(NULL) - me.lastmsg;
		tmp2 = time(NULL) - me.t_start;
		snumeric_cmd(211, u->nick,
			     "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		snumeric_cmd(241, u->nick, "%s 0 %d %d %d %d %d 0 :%d",
			     me.uplink, me.SendM, me.SendBytes, me.RcveM,
			     me.RcveBytes, tmp2, tmp);
	} else if (!strcasecmp(what, "M")) {
		for (I = 0; I < ircd_srv.cmdcount; I++) {
			if (cmd_list[I].usage > 0)
				snumeric_cmd(212, u->nick,
					     "Command %s Usage %d",
					     cmd_list[I].name,
					     cmd_list[I].usage);
		}
	}
	snumeric_cmd(219, u->nick, "%s :End of /STATS report", what);
	chanalert(s_Services, "%s Requested Stats %s", u->nick, what);
};
