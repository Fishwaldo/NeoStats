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
** $Id: ircd.c,v 1.126 2003/06/20 12:24:27 fishwaldo Exp $
*/
#include <setjmp.h>
#include "stats.h"
#include "ircd.h"
#include "dl.h"
#include "log.h"

extern const char protocol_version[];

extern IntCommands cmd_list[];






/* The Following Array is a list of Commands that are Handled Internally by NeoStats 
they have Nothing to do with the Array that the Modules use. These Functions are called before any
Module Functions to make sure the Internal Database is kept uptodate, as Some modules may utilize the Users or Servers
or Channel array.
if "srvmsg" is defined, it means that the command must be prefixed by ":" indicating a message from a Server (I think)

*/

#ifdef UNREAL
#endif
#if defined(HYBRID7) || defined(NEOIRCD)
#endif
#ifdef ULTIMATE
IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, Usr_Stats, 1, 0},
	{TOK_STATS, Usr_Stats, 1, 0},
	{MSG_SETHOST, Usr_Vhost, 1, 0},
	{TOK_SETHOST, Usr_Vhost, 1, 0},
	{MSG_VERSION, Usr_Version, 1, 0},
	{TOK_VERSION, Usr_Version, 1, 0},
	{MSG_MOTD, Usr_ShowMOTD, 1, 0},
	{TOK_MOTD, Usr_ShowMOTD, 1, 0},
	{MSG_CREDITS, Usr_Showcredits, 1, 0},
	{TOK_CREDITS, Usr_Showcredits, 1, 0},
	{MSG_SERVER, Usr_AddServer, 1, 0},
	{TOK_SERVER, Usr_AddServer, 1, 0},
	{MSG_SQUIT, Usr_DelServer, 1, 0},
	{TOK_SQUIT, Usr_DelServer, 1, 0},
	{MSG_QUIT, Usr_DelUser, 1, 0},
	{TOK_QUIT, Usr_DelUser, 1, 0},
	{MSG_MODE, Usr_Mode, 1, 0},
	{TOK_MODE, Usr_Mode, 1, 0},
	{MSG_SVSMODE, Usr_Smode, 1, 0},
	{TOK_SVSMODE, Usr_Smode, 1, 0},
	{MSG_KILL, Usr_Kill, 1, 0},
	{TOK_KILL, Usr_Kill, 1, 0},
	{MSG_PONG, Usr_Pong, 1, 0},
	{TOK_PONG, Usr_Pong, 1, 0},
	{MSG_AWAY, Usr_Away, 1, 0},
	{TOK_AWAY, Usr_Away, 1, 0},
	{MSG_NICK, Usr_Nick, 1, 0},
	{TOK_NICK, Usr_Nick, 1, 0},
	{MSG_TOPIC, Usr_Topic, 1, 0},
	{TOK_TOPIC, Usr_Topic, 1, 0},
	{MSG_KICK, Usr_Kick, 1, 0},
	{TOK_KICK, Usr_Kick, 1, 0},
	{MSG_JOIN, Usr_Join, 1, 0},
	{TOK_JOIN, Usr_Join, 1, 0},
	{MSG_PART, Usr_Part, 1, 0},
	{TOK_PART, Usr_Part, 1, 0},
	{MSG_PING, Srv_Ping, 0, 0},
	{TOK_PING, Srv_Ping, 0, 0},
#ifndef ULTIMATE3
	{MSG_SNETINFO, Srv_Netinfo, 0, 0},
	{TOK_SNETINFO, Srv_Netinfo, 0, 0},

#endif
#ifdef ULTIMATE3
	{MSG_SVINFO, Srv_Svinfo, 0, 0},
	{MSG_CAPAB, Srv_Connect, 0, 0},
	{MSG_BURST, Srv_Burst, 0, 0},
	{MSG_SJOIN, Srv_Sjoin, 1, 0},
	{MSG_CLIENT, Srv_Client, 0, 0},
	{MSG_SMODE, Srv_Smode, 1, 0},
#endif
	{MSG_VCTRL, Srv_Vctrl, 0, 0},
	{TOK_VCTRL, Srv_Vctrl, 0, 0},
	{MSG_PASS, Srv_Pass, 0, 0},
	{TOK_PASS, Srv_Pass, 0, 0},
	{MSG_SERVER, Srv_Server, 0, 0},
	{TOK_SERVER, Srv_Server, 0, 0},
	{MSG_SQUIT, Srv_Squit, 0, 0},
	{TOK_SQUIT, Srv_Squit, 0, 0},
	{MSG_NICK, Srv_Nick, 0, 0},
	{TOK_NICK, Srv_Nick, 0, 0},
	{MSG_SVSNICK, Srv_Svsnick, 0, 0},
	{TOK_SVSNICK, Srv_Svsnick, 0, 0},
	{MSG_KILL, Srv_Kill, 0, 0},
	{TOK_KILL, Srv_Kill, 0, 0},
	{MSG_PROTOCTL, Srv_Connect, 0, 0},
	{TOK_PROTOCTL, Srv_Connect, 0, 0},
	{NULL, NULL, 0, 0}
};
#endif


int init_bot(char *nick, char *user, char *host, char *rname, char *modes,
	     char *mod_name)
{
	User *u;
#if 0
#ifndef ULTIMATE3
#ifndef HYBRID7
	char cmd[63];
#endif
#endif
#endif
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
/* split code */
#if 0

#ifdef ULTIMATE3
	snewnick_cmd(nick, user, host, rname, Umode);
#elif defined(HYBRID7)
	snewnick_cmd(nick, user, host, rname, Umode);
#elif defined(NEOIRCD)
	snewnick_cmd(nick, user, host, rname, Umode);
#else
	snewnick_cmd(nick, user, host, rname);
//      sumode_cmd(nick, nick, Umode);
#endif
#ifdef UNREAL
	sumode_cmd(nick, nick, Umode);
#elif !ULTIMATE
#if !defined(HYBRID7) && !defined(NEOIRCD)
	sumode_cmd(nick, nick, Umode);
#endif
#endif
#ifndef HYBRID7
	if ((me.allbots > 0) || (Umode & UMODE_SERVICES)) {
#endif
#ifdef ULTIMATE3
		sjoin_cmd(nick, me.chan, MODE_CHANADMIN);
#else				/* ulitmate3 */
		sjoin_cmd(nick, me.chan);
#ifndef HYBRID7
		snprintf(cmd, 63, "%s %s", nick, nick);
#else
		schmode_cmd(me.name, me.chan, "+o", nick);
#endif
#ifndef HYBRID7
#ifdef NEOIRCD
		schmode_cmd(me.name, me.chan, "+a", cmd);
#else				/* neoircd */
		schmode_cmd(nick, me.chan, "+oa", cmd);
#endif				/* neoircd */
#endif				/* hybrid7 */
#endif				/* ultimate3 */
#ifndef HYBRID7
		/* all bots join */
	}
#endif
/* split code */
#endif
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

/* split code */

	SignOn_NewBot(s_Services, Servbot.user, Servbot.host, rname,
		      UMODE_SERVICES);

#if 0
#ifdef ULTIMATE3
	sburst_cmd(1);
	snewnick_cmd(s_Services, Servbot.user, Servbot.host, rname,
		     UMODE_SERVICES);
#elif defined(HYBRID7) || defined(NEOIRCD)
	snewnick_cmd(s_Services, Servbot.user, Servbot.host, rname,
		     UMODE_ADMIN | UMODE_SERVICES);
#else
	snewnick_cmd(s_Services, Servbot.user, Servbot.host, rname);
#endif
#ifdef UNREAL
	sumode_cmd(s_Services, s_Services,
		   UMODE_SERVICES | UMODE_DEAF | UMODE_KIX);
#elif !ULTIMATE
#if !defined(HYBRID7) && !defined(NEOIRCD)
	sumode_cmd(s_Services, s_Services,
		   UMODE_SERVICES | UMODE_DEAF | UMODE_SBOT);
#endif
#endif

#ifdef ULTIMATE3
	sjoin_cmd(s_Services, me.chan, MODE_CHANADMIN);
#else
	sjoin_cmd(s_Services, me.chan);
	snprintf(rname, 63, "%s %s", s_Services, s_Services);
#if !defined(HYBRID7)
#ifdef NEOIRCD

	schmode_cmd(me.name, me.chan, "+a", rname);
#else
	schmode_cmd(s_Services, me.chan, "+oa", rname);
#endif
#else
	schmode_cmd(s_Services, me.chan, "+o", rname);
#endif
#endif				/* ultimate3 */

#endif				/* SPLIT CODE */
	me.onchan = 1;
	AddStringToList(&av, me.uplink, &ac);
	Module_Event("ONLINE", av, ac);
	free(av);
	ac = 0;
	AddStringToList(&av, s_Services, &ac);
	Module_Event("SIGNON", av, ac);
	free(av);

}


// XXXXX Start of SplitCode
#if 0




void Srv_Sjoin(char *origin, char **argv, int argc)
{
	char nick[MAXNICK];
	long mode = 0;
	long mode1 = 0;
	char *modes;
	int ok = 1, i, j = 3;
	ModesParm *m;
	Chans *c;
	lnode_t *mn = NULL;
	list_t *tl;
	if (argc <= 2) {
		modes = argv[1];
	} else {
		modes = argv[2];
	}

	if (*modes == '#') {
#if defined(HYBRID7) || defined(NEOIRCD)
		join_chan(finduser(argv[4]), modes);
#else
		join_chan(finduser(origin), modes);
#endif
		return;
	}
	tl = list_create(10);

	if (*modes != '+') {
		goto nomodes;
	}
	while (*modes) {
		for (i = 0;
		     i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1);
		     i++) {
			if (*modes == cFlagTab[i].flag) {
				if (cFlagTab[i].parameters) {
					m = smalloc(sizeof(ModesParm));
					m->mode = cFlagTab[i].mode;
					strncpy(m->param, argv[j],
						PARAMSIZE);
					mn = lnode_create(m);
					if (!list_isfull(tl)) {
						list_append(tl, mn);
					} else {
						nlog(LOG_CRITICAL,
						     LOG_CORE,
						     "Eeeek, tl list is full in Svr_Sjoin(ircd.c)");
						do_exit(0);
					}
					j++;
				} else {
					mode1 |= cFlagTab[i].mode;
				}
			}
		}
		modes++;
	}
      nomodes:
	while (argc > j) {
		modes = argv[j];
		mode = 0;
		while (ok == 1) {
			if (*modes == '@') {
				mode |= MODE_CHANOP;
				modes++;
#if defined(ULTIMATE3) || defined(NEOIRCD)
				/* Ultimate3 has had 3 different flags for Chan Admins! */
			} else if (*modes == '!') {
				mode |= MODE_CHANADMIN;
				modes++;
#endif
#ifdef ULTIMATE3
			} else if (*modes == '¤') {
				mode |= MODE_CHANADMIN;
				modes++;
#endif
#if defined(ULTIMATE) || defined(ULTIMATE3)
			} else if (*modes == '*') {
				mode |= MODE_CHANADMIN;
				modes++;
#endif
			} else if (*modes == '%') {
				mode |= MODE_HALFOP;
				modes++;
			} else if (*modes == '+') {
				mode |= MODE_VOICE;
				modes++;
			} else {
				strncpy(nick, modes, MAXNICK);
				ok = 0;
				break;
			}
		}
		join_chan(finduser(nick), argv[1]);
		ChangeChanUserMode(findchan(argv[1]), finduser(nick), 1,
				   mode);
		j++;
		ok = 1;
	}
	c = findchan(argv[1]);
	c->modes |= mode1;
	if (!list_isempty(tl)) {
		if (!list_isfull(c->modeparms)) {
			list_transfer(c->modeparms, tl, list_first(tl));
		} else {
			/* eeeeeeek, list is full! */
			nlog(LOG_CRITICAL, LOG_CORE,
			     "Eeeek, c->modeparms list is full in Svr_Sjoin(ircd.c)");
			do_exit(0);
		}
	}
	list_destroy(tl);
}

#ifndef UNREAL
void Srv_Burst(char *origin, char **argv, int argc)
{
	if (argc > 0) {
		if (ircd_srv.burst == 1) {
			sburst_cmd(0);
			ircd_srv.burst = 0;
			me.synced = 1;
			init_ServBot();
		}
	} else {
		ircd_srv.burst = 1;
	}
#if defined(HYBRID7) || defined(NEOIRCD)
	seob_cmd(me.name);
	init_ServBot();
#endif

}
#endif
void Srv_Connect(char *origin, char **argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcasecmp("TOKEN", argv[i])) {
			me.token = 1;
		}
#ifdef ULTIMATE3
		if (!strcasecmp("CLIENT", argv[i])) {
			me.client = 1;
		}
#endif
	}
}


void Usr_Stats(char *origin, char **argv, int argc)
{
	User *u;
	time_t tmp;
	time_t tmp2;
	int I;
#ifdef EXTAUTH
	int dl;
	int (*listauth) (User * u);
#endif

	u = finduser(origin);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Recieved a Message from a Unknown User! (%s)",
		     origin);
	}
	if (!strcasecmp(argv[0], "u")) {
		/* server uptime - Shmad */
		int uptime = time(NULL) - me.t_start;
		snumeric_cmd(242, u->nick,
			     "Statistical Server up %d days, %d:%02d:%02d",
			     uptime / 86400, (uptime / 3600) % 24,
			     (uptime / 60) % 60, uptime % 60);
	} else if (!strcasecmp(argv[0], "c")) {
		/* Connections */
		snumeric_cmd(214, u->nick, "N *@%s * * %d 50", me.uplink,
			     me.port);
		snumeric_cmd(213, u->nick, "C *@%s * * %d 50", me.uplink,
			     me.port);
	} else if (!strcasecmp(argv[0], "o")) {
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
	} else if (!strcasecmp(argv[0], "l")) {
		/* Port Lists */
		tmp = time(NULL) - me.lastmsg;
		tmp2 = time(NULL) - me.t_start;
		snumeric_cmd(211, u->nick,
			     "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		snumeric_cmd(241, u->nick, "%s 0 %d %d %d %d %d 0 :%d",
			     me.uplink, me.SendM, me.SendBytes, me.RcveM,
			     me.RcveBytes, tmp2, tmp);
	} else if (!strcasecmp(argv[0], "M")) {
		if (UserLevel(u) > 40) {
			for (I = 0; I < ircd_srv.cmdcount; I++) {
				if (cmd_list[I].usage > 0)
					snumeric_cmd(212, u->nick,
						     "Command %s Usage %d",
						     cmd_list[I].name,
						     cmd_list[I].usage);
			}
		}
	}
	snumeric_cmd(219, u->nick, "%s :End of /STATS report", argv[0]);
	chanalert(s_Services, "%s Requested Stats %s", u->nick, argv[0]);
}

void Usr_Version(char *origin, char **argv, int argc)
{
	snumeric_cmd(351, origin, "%d.%d.%d%s :%s -> %s %s", MAJOR, MINOR,
		     REV, version, me.name, version_date, version_time);
}
void Usr_ShowMOTD(char *origin, char **argv, int argc)
{
	ShowMOTD(origin);
}
void Usr_ShowADMIN(char *origin, char **argv, int argc)
{
	ShowADMIN(origin);
}
void Usr_Showcredits(char *origin, char **argv, int argc)
{
	Showcredits(origin);
}
void Usr_AddServer(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	AddServer(argv[0], origin, atoi(argv[1]));
	AddStringToList(&av, argv[0], &ac);
	Module_Event("NEWSERVER", av, ac);
	free(av);
//      FreeList(av, ac);
}
void Usr_DelServer(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	AddStringToList(&av, argv[0], &ac);
	Module_Event("DELSERVER", av, ac);
	free(av);
//      FreeList(av, ac);
	DelServer(argv[0]);
}
void Usr_DelUser(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	AddStringToList(&av, origin, &ac);
	Module_Event("SIGNOFF", av, ac);
	free(av);
//      FreeList(av, ac);
	DelUser(origin);
}
void Usr_Smode(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	AddStringToList(&av, argv[0], &ac);

	if (!strchr(argv[0], '#')) {
		/* its user svsmode change */
#ifdef ULTIMATE3
		AddStringToList(&av, argv[2], &ac);
		UserMode(argv[0], argv[2], 0);
#else
		AddStringToList(&av, argv[1], &ac);
		UserMode(argv[0], argv[1]);
#endif
		Module_Event("UMODE", av, ac);
	} else {
		/* its a channel svsmode change */
		ChanMode(origin, argv, argc);
	}
	free(av);
//      FreeList(av, ac);
}
void Usr_Mode(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	if (!strchr(argv[0], '#')) {
		nlog(LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[0]);
#ifdef ULTIMATE3
		UserMode(argv[0], argv[1], 0);
#else
		UserMode(argv[0], argv[1]);
#endif
		AddStringToList(&av, argv[0], &ac);
		AddStringToList(&av, argv[1], &ac);
		Module_Event("UMODE", av, ac);
		free(av);
//              FreeList(av, ac);
	} else {
		ChanMode(origin, argv, argc);
	}
}
void Usr_Kill(char *origin, char **argv, int argc)
{
	User *u;
	Mod_User *mod_ptr;
	char **av;
	int ac = 0;

	mod_ptr = findbot(argv[0]);
	if (mod_ptr) {		/* Oh Oh, one of our Bots has been Killed off! */
		AddStringToList(&av, argv[0], &ac);
		Module_Event("BOTKILL", av, ac);
		free(av);
//              FreeList(av, ac);
		DelUser(argv[0]);
		return;
	}
	u = finduser(argv[0]);
	if (u) {
		AddStringToList(&av, u->nick, &ac);
		Module_Event("KILL", av, ac);
		free(av);
//              FreeList(av, ac);
		DelUser(argv[0]);
	}
}
void Usr_Vhost(char *origin, char **argv, int argc)
{
	User *u;
#ifndef ULTIMATE3
	u = finduser(origin);
#else
	u = finduser(argv[0]);
#endif
	if (u) {
#ifndef ULTIMATE3
		strncpy(u->vhost, argv[0], MAXHOST);
#else
		strncpy(u->vhost, argv[1], MAXHOST);
#endif
	}
}
void Usr_Pong(char *origin, char **argv, int argc)
{
	Server *s;
	char **av;
	int ac = 0;
	s = findserver(argv[0]);
	if (s) {
		s->ping = time(NULL) - ping.last_sent;
		if (ping.ulag > 1)
			s->ping -= (float) ping.ulag;
		if (!strcmp(me.s->name, s->name))
			ping.ulag = me.s->ping;
		AddStringToList(&av, argv[0], &ac);
		Module_Event("PONG", av, ac);
		free(av);
//              FreeList(av, ac);
	} else {
		nlog(LOG_NOTICE, LOG_CORE,
		     "Received PONG from unknown server: %s", argv[0]);
	}
}
void Usr_Away(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	User *u = finduser(origin);
	if (u) {
		AddStringToList(&av, origin, &ac);
		if ((u->is_away == 1) && (argc == 0)) {
			u->is_away = 0;
			Module_Event("AWAY", av, ac);
		} else if ((u->is_away == 0) && (argc > 0)) {
			u->is_away = 1;
			Module_Event("AWAY", av, ac);
		}
		free(av);
//              FreeList(av, ac);
	} else {
		nlog(LOG_NOTICE, LOG_CORE,
		     "Warning, Unable to find User %s for Away", origin);
	}
}
void Usr_Nick(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	User *u = finduser(origin);
	if (u) {
		Change_User(u, argv[0]);
		AddStringToList(&av, origin, &ac);
		AddStringToList(&av, argv[0], &ac);
		Module_Event("NICK_CHANGE", av, ac);
		free(av);
//              FreeList(av, ac);
	}
}
void Usr_Topic(char *origin, char **argv, int argc)
{
	char *buf;
	Chans *c;
	c = findchan(argv[0]);
	if (c) {
#if !defined(HYBRID7) && !defined(NEOIRCD)
		buf = joinbuf(argv, argc, 3);
		Change_Topic(argv[1], c, atoi(argv[2]), buf);
#else
		buf = joinbuf(argv, argc, 2);
		Change_Topic(origin, c, time(NULL), buf);
#endif
		free(buf);
	} else {
		nlog(LOG_WARNING, LOG_CORE, "Ehhh, Can't find Channel %s",
		     argv[0]);
	}

}

void Usr_Kick(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	AddStringToList(&av, argv[0], &ac);
	AddStringToList(&av, argv[1], &ac);
	Module_Event("KICK", av, ac);
	free(av);
//      FreeList(av, ac);
	part_chan(finduser(argv[1]), argv[0]);

}
void Usr_Join(char *origin, char **argv, int argc)
{
	char *s, *t;
	t = argv[0];
	while (*(s = t)) {
		t = s + strcspn(s, ",");
		if (*t)
			*t++ = 0;
		join_chan(finduser(origin), s);
	}
}
void Usr_Part(char *origin, char **argv, int argc)
{
	part_chan(finduser(origin), argv[0]);
}
void Srv_Ping(char *origin, char **argv, int argc)
{
	spong_cmd(argv[0]);
#ifdef ULTIMATE3
	if (ircd_srv.burst) {
		sping_cmd(me.name, argv[0], argv[0]);
	}
#endif
}

#ifdef ULTIMATE
void Srv_Vctrl(char *origin, char **argv, int argc)
{
	ircd_srv.uprot = atoi(argv[0]);
	ircd_srv.nicklg = atoi(argv[1]);
	ircd_srv.modex = atoi(argv[2]);
	ircd_srv.gc = atoi(argv[3]);
	strncpy(me.netname, argv[14], MAXPASS);
	vctrl_cmd();
}
#endif
#ifndef UNREAL
void Srv_Svinfo(char *origin, char **argv, int argc)
{
	ssvinfo_cmd();
}
#endif
#ifndef ULTIMATE3
void Srv_Netinfo(char *origin, char **argv, int argc)
{
	me.onchan = 1;
	ircd_srv.uprot = atoi(argv[2]);
	strncpy(ircd_srv.cloak, argv[3], 10);
	strncpy(me.netname, argv[7], MAXPASS);

#if !defined(HYBRID7) && !defined(NEOIRCD)
	snetinfo_cmd();
#endif
	init_ServBot();
	globops(me.name, "Link with Network \2Complete!\2");
#ifdef DEBUG
	ns_debug_to_coders(me.chan);
#endif
	if (ircd_srv.uprot == 2109) {
		me.usesmo = 1;
	}
	Module_Event("NETINFO", NULL, 0);
	me.synced = 1;
}
#endif

void Srv_Pass(char *origin, char **argv, int argc)
{
}
void Srv_Server(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	Server *s;
	if (*origin == 0) {
		AddServer(argv[0], me.name, atoi(argv[1]));
	} else {
		AddServer(argv[0], origin, atoi(argv[1]));
	}
	s = findserver(argv[0]);
	me.s = s;
	AddStringToList(&av, argv[0], &ac);
	Module_Event("NEWSERVER", av, ac);
	free(av);
//                      FreeList(av, ac);
}
void Srv_Squit(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	Server *s;
	s = findserver(argv[0]);
	if (s) {
		AddStringToList(&av, argv[0], &ac);
		Module_Event("SQUIT", av, ac);
		free(av);
//                              FreeList(av, ac);
		DelServer(argv[0]);
	} else {
		nlog(LOG_WARNING, LOG_CORE,
		     "Waring, Squit from Unknown Server %s", argv[0]);
	}

}

/* BE REALLY CAREFULL ABOUT THE ORDER OF THESE ifdef's */

void Srv_Nick(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	char *realname;
#ifdef NEOIRCD
	User *u;
#endif
	AddStringToList(&av, argv[0], &ac);
#ifdef UNREAL
	AddUser(argv[0], argv[3], argv[4], argv[5], 0,
		strtol(argv[2], NULL, 10));
	realname = joinbuf(argv, argc, 7);
	AddRealName(argv[0], realname);
	free(realname);
	Module_Event("SIGNON", av, ac);
#elif ULTIMATE3
	AddUser(argv[0], argv[4], argv[5], argv[6],
		strtoul(argv[8], NULL, 10), strtoul(argv[2], NULL, 10));
	realname = joinbuf(argv, argc, 9);
	AddRealName(argv[0], realname);
	free(realname);
	Module_Event("SIGNON", av, ac);
	nlog(LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode(argv[0], argv[3], 0);
	AddStringToList(&av, argv[3], &ac);
	Module_Event("UMODE", av, ac);
#elif ULTIMATE
	AddUser(argv[0], argv[3], argv[4], argv[5], 0,
		strtoul(argv[2], NULL, 10));
	realname = joinbuf(argv, argc, 7);
	AddRealName(argv[0], realname);
	free(realname);
	Module_Event("SIGNON", av, ac);
#elif HYBRID7
	AddUser(argv[0], argv[4], argv[5], argv[6], 0,
		strtoul(argv[2], NULL, 10));
	realname = joinbuf(argv, argc, 7);
	AddRealName(argv[0], realname);
	free(realname);
	Module_Event("SIGNON", av, ac);
	nlog(LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode(argv[0], argv[3]);
	AddStringToList(&av, argv[3], &ac);
	Module_Event("UMODE", av, ac);
#elif NEOIRCD
	AddUser(argv[0], argv[4], argv[5], argv[7], 0,
		strtoul(argv[2], NULL, 10));
	realname = joinbuf(argv, argc, 9);
	AddRealName(argv[0], realname);
	free(realname);
	Module_Event("SIGNON", av, ac);
	u = finduser(argv[0]);
	if (u) {
		strncpy(u->vhost, argv[6], MAXHOST);
	}
	nlog(LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode(argv[0], argv[3]);
	AddStringToList(&av, argv[3], &ac);
	Module_Event("UMODE", av, ac);
#endif
	free(av);
//                      FreeList(av, ac);
}

/* Ultimate3 Client Support */
#ifdef ULTIMATE3
void Srv_Client(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	char *realname;

	AddStringToList(&av, argv[0], &ac);
	AddUser(argv[0], argv[5], argv[6], argv[8],
		strtoul(argv[10], NULL, 10), strtoul(argv[2], NULL, 10));
	realname = joinbuf(argv, argc, 11);
	AddRealName(argv[0], realname);
	free(realname);
	Module_Event("SIGNON", av, ac);
	nlog(LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode(argv[0], argv[3], 0);
	AddStringToList(&av, argv[3], &ac);
	Module_Event("UMODE", av, ac);
	free(av);
//                      FreeList(av, ac);
	ac = 0;
#ifdef ULTIMATE3
	AddStringToList(&av, argv[0], &ac);
	nlog(LOG_DEBUG1, LOG_CORE, "Smode: SMode: %s", argv[4]);
	UserMode(argv[0], argv[4], 1);
	AddStringToList(&av, argv[4], &ac);
	Module_Event("SMODE", av, ac);
	free(av);
//                      FreeList(av, ac);
#endif

}

void Srv_Smode(char *origin, char **argv, int argc)
{
	char **av;
	int ac = 0;
	AddStringToList(&av, argv[0], &ac);
	AddStringToList(&av, argv[1], &ac);
	UserMode(argv[0], argv[1], 1);
	Module_Event("SMODE", av, ac);
	free(av);
};


#endif
void Srv_Svsnick(char *origin, char **argv, int argc)
{
	User *u;
	char **av;
	int ac = 0;
	u = finduser(argv[0]);
	Change_User(u, argv[1]);
	AddStringToList(&av, argv[0], &ac);
	AddStringToList(&av, argv[1], &ac);
	Module_Event("NICK_CHANGE", av, ac);
	free(av);
//                      FreeList(av, ac);

}
void Srv_Kill(char *origin, char **argv, int argc)
{
}


#ifdef NEOIRCD
/* Topic Bursting for NeoIRCD */
/* R: :fish.dynam.ac TBURST 1034639893 #ircop 1034652780 ChanServ!services@neostats.net :NeoIRCd Test Oper Channel */

void Srv_Tburst(char *origin, char **argv, int argc)
{
	char *buf;
	Chans *c;
	c = findchan(argv[1]);
	if (c) {
		buf = joinbuf(argv, argc, 4);
		Change_Topic(argv[3], c, atoi(argv[2]), buf);
		free(buf);
	} else {
		nlog(LOG_WARNING, LOG_CORE,
		     "TopicBurst: Ehhh, Can't find Channel %s", argv[1]);
	}


}
#endif


// XXXX end of split code
#endif

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
