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
#include <setjmp.h>
#include "stats.h"
#include "ircd.h"
#include "dl.h"
#include "log.h"
#include "services.h"

/** @brief init_bot_modes
 *
 *  Translate a mode string to the bitwise mode
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int
init_bot_modes (const char *modes)
{
	int add = 0;
	long Umode = 0;
	char tmpmode;
	int i;

	/* Walk through mode string and convert to umode */
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
			for (i = 0; i < ((sizeof (usr_mds) / sizeof (usr_mds[0])) - 1); i++) {
				if (usr_mds[i].mode == tmpmode) {
					if (add) {
						Umode |= usr_mds[i].umodes;
						break;
					} else {
						Umode &= ~usr_mds[i].umodes;
						break;
					}
				}
			}
		}
		tmpmode = *modes++;
	}
	return(Umode);
}


/** @brief init_bot
 *
 * 
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
init_bot (char *nick, char *user, char *host, char *rname, const char *modes, char *mod_name)
{
	User *u;
	char **av;
	int ac = 0;
	long Umode;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (u) {
		nlog (LOG_WARNING, LOG_CORE, "Attempting to Login with a Nickname that already Exists: %s", nick);
		return NS_FAILURE;
	}
	if (strnlen (user, MAXUSER) > MAXUSERWARN) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, %s bot %s has an username longer than 8 chars. Some IRCd's don't like that", mod_name, nick);
	}
	if(!add_mod_user (nick, mod_name)) {
		nlog (LOG_WARNING, LOG_CORE, "add_mod_user failed for module %s bot %s", mod_name, nick);
		return NS_FAILURE;
	}
	Umode = init_bot_modes(modes);
	SignOn_NewBot (nick, user, host, rname, Umode);
	AddStringToList (&av, nick, &ac);
	ModuleEvent (EVENT_SIGNON, av, ac);
	free (av);
	/* restore segv_inmodule from SIGNON */
	SET_SEGV_INMODULE(mod_name);
	return NS_SUCCESS;
}

/** @brief init_mod_bot
 *
 *  replacement for init_bot - work in progress
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
ModUser * init_mod_bot (char * nick, char * user, char * host, char * rname, 
						const char *modes, unsigned int flags, bot_cmd *bot_cmd_list, 
						bot_setting *bot_setting_list, char * mod_name)
{
	ModUser * bot_ptr; 
	User *u;
	char **av;
	int ac = 0;
	long Umode;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (u) {
		nlog (LOG_WARNING, LOG_CORE, "Attempting to Login with a Nickname that already Exists: %s", nick);
		return NULL;
	}
	if (strnlen (user, MAXUSER) > MAXUSERWARN) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, %s bot %s has an username longer than 8 chars. Some IRCd's don't like that", mod_name, nick);
	}
	bot_ptr = add_mod_user (nick, mod_name);
	if(!bot_ptr) {
		nlog (LOG_WARNING, LOG_CORE, "add_mod_user failed for module %s bot %s", mod_name, nick);
		return NULL;
	}
	Umode = init_bot_modes(modes);
	SignOn_NewBot (nick, user, host, rname, Umode);
	AddStringToList (&av, nick, &ac);
	ModuleEvent (EVENT_SIGNON, av, ac);
	free (av);
	/* restore segv_inmodule from SIGNON */
	SET_SEGV_INMODULE(mod_name);
	bot_ptr->flags = flags;
	add_bot_cmd_list(bot_ptr, bot_cmd_list);
	bot_ptr->bot_settings = bot_setting_list;	
	return bot_ptr;
}

/** @brief del_bot
 *
 * 
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
del_bot (char *nick, char *reason)
{
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Attempting to Logoff with a Nickname that does not Exists: %s", nick);
		return NS_FAILURE;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Killing %s for %s", nick, reason);
	//XXXX TODO: need to free the channel list hash. We dont according to valgrind
	squit_cmd (nick, reason);
	del_mod_user (nick);
	return NS_SUCCESS;
}

/** @brief split_buf
 * Taken from Epona - Thanks! 
 * Split a buffer into arguments and store the arguments in an
 * argument vector pointed to by argv (which will be malloc'd
 * as necessary); return the argument count.  If colon_special
 * is non-zero, then treat a parameter with a leading ':' as
 * the last parameter of the line, per the IRC RFC.  Destroys
 * the buffer by side effect. 
 *
 * @return 
 */
int
split_buf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int flag = 0;
#ifndef IRCU
	int colcount = 0;
#endif
	SET_SEGV_LOCATION();
	*argv = calloc (sizeof (char *) * argvsize, 1);
	argc = 0;
#ifndef IRCU
	if (*buf == ':')
		buf++;
#endif
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = realloc (*argv, sizeof (char *) * argvsize);
		}
#ifndef IRCU
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
		}
#endif
		s = strpbrk (buf, " ");
		if (s) {
			*s++ = 0;
			while (isspace (*s))
				s++;
		} else {
			s = buf + strnlen (buf, BUFSIZE);
		}
		if (*buf == 0) {
			buf++;
		}
		(*argv)[argc++] = buf;
		buf = s;
	}
	return argc - flag;
}

/** @brief joinbuf 
 *
 * 
 *
 * @return 
 */
char *
joinbuf (char **av, int ac, int from)
{
	int i;
	char *buf;

	buf = malloc (BUFSIZE);
	if(from > ac) {
		strlcpy (buf, "(null)", BUFSIZE);
	}
	else {
		strlcpy (buf, av[from], BUFSIZE);
		for (i = from + 1; i < ac; i++) {
			strlcat (buf, " ", BUFSIZE);
			strlcat (buf, av[i], BUFSIZE);
		}
	}
	return (char *) buf;
}

/** @brief parse
 *
 * 
 *
 * @return none
 */
void
parse (char *line)
{
	User *u;
	char origin[64], cmd[64], *coreLine;
	int cmdptr = 0;
	int I = 0;
	int ac;
	char **av;
	ModUser *mod_usr;

	SET_SEGV_LOCATION();
	strip (line);
	strlcpy (recbuf, line, BUFSIZE);
	if (!(*line))
		return;
	nlog (LOG_DEBUG1, LOG_CORE, "R: %s", line);
#ifndef IRCU
	if (*line == ':') {
		coreLine = strpbrk (line, " ");
		if (!coreLine)
			return;
		*coreLine = 0;
		while (isspace (*++coreLine));
		strlcpy (origin, line + 1, sizeof (origin));
		memmove (line, coreLine, strnlen (coreLine, BUFSIZE) + 1);
		cmdptr = 1;
	} else {
		cmdptr = 0;
		*origin = 0;
	}
#endif
	if (!*line)
		return;
	coreLine = strpbrk (line, " ");
	if (coreLine) {
		*coreLine = 0;
		while (isspace (*++coreLine));
	} else
		coreLine = line + strlen (line);
#ifdef IRCU
	if ((!strcasecmp(line, "SERVER")) || (!strcasecmp(line, "PASS"))) {
		strlcpy(cmd, line, sizeof(cmd));
		ac = split_buf(coreLine, &av, 1);
		cmdptr = 0;
	} else {
		strlcpy(origin, line, sizeof(origin));	
		cmdptr = 1;
		line = strpbrk (coreLine, " ");
		if (line) {
			*line = 0;
			while (isspace (*++line));
		} else
			coreLine = line + strlen (line);
		ac = split_buf(line, &av, 0);
	}

#else 
	strlcpy (cmd, line, sizeof (cmd));

	ac = split_buf (coreLine, &av, 1);
#endif

	/* First, check if its a privmsg, and if so, handle it in the correct Function */
#ifndef IRCU
	if (!strcmp ("PRIVMSG", cmd) || (!strcmp ("!", cmd))) {
#else 
	if (!strcmp("PRIVMSG", cmd) || !strcmp("P", cmd) || !strcmp("CPRIVMSG", cmd) || !strcmp("CP", cmd)) {
#endif

		/* its a privmsg, now lets see who too... */
		if (strstr (av[0], "!")) {
			strlcpy (cmd, av[0], 64);
			av[0] = strtok (cmd, "!");
		} else if (strstr (av[0], "@")) {
			strlcpy (cmd, av[0], 64);
			av[0] = strtok (cmd, "@");
		}
		if (!strcasecmp (s_Services, av[0])) {
			if (flood (finduser (origin))) {
				free (av);
				return;
			}
			/* its to the Internal Services Bot */
			SET_SEGV_LOCATION();
			servicesbot (origin, av, ac);
			SET_SEGV_LOCATION();
			free (av);
			return;
		} else {
			mod_usr = findbot (av[0]);
			/* Check to see if any of the Modules have this nick Registered */
			if (mod_usr) {
				nlog (LOG_DEBUG1, LOG_CORE, "nicks: %s", mod_usr->nick);
				if (flood (finduser (origin))) {
					free (av);
					return;
				}

				/* Check to make sure there are no blank spaces so we dont crash */
				/* Use strnlen so long lines do not just DOS the server by tying us up
				   in strlen */
				if (strnlen (av[1], MAX_CMD_LINE_LENGTH) >= MAX_CMD_LINE_LENGTH) {
					prefmsg (origin, s_Services, "command line too long!");
					notice (s_Services, "%s tried to send a very LARGE command, we told them to shove it!", origin);
					free (av);
					return;
				}

				SET_SEGV_LOCATION();
				SET_SEGV_INMODULE(mod_usr->modname);
				if (setjmp (sigvbuf) == 0) {
					if(mod_usr->function) {
						mod_usr->function (origin, av, ac);
					}
					else {
						u = finduser (origin);
						if (!u) {
							nlog (LOG_WARNING, LOG_CORE, "Unable to finduser %s (%s)", origin, mod_usr->nick);
							return;
						}
						run_bot_cmd(mod_usr, u, av, ac);
					}
				}
				CLEAR_SEGV_INMODULE();
				SET_SEGV_LOCATION();
				free (av);
				return;
			} else {
				bot_chan_message (origin, av, ac);
				free (av);
				return;
			}
		}
	}

	/* now, Parse the Command to the Internal Functions... */
	SET_SEGV_LOCATION();
	for (I = 0; I < ircd_srv.cmdcount; I++) {
		if (!strcmp (cmd_list[I].name, cmd)) {
			if (cmd_list[I].srvmsg == cmdptr) {
				cmd_list[I].function (origin, av, ac);
				cmd_list[I].usage++;
				break;
			}
		}
	}
	/* K, now Parse it to the Module functions */
	SET_SEGV_LOCATION();
	ModuleFunction (cmdptr, cmd, origin, av, ac);
	free (av);
}

/** @brief init_ServBot
 *
 * 
 *
 * @return none
 */
void
init_ServBot (void)
{
	char **av;
	int ac = 0;

	SET_SEGV_LOCATION();
	if (finduser (s_Services)) {
		/* nick already exists on the network */
		strlcat (s_Services, "1", MAXNICK);
	}
	ircsnprintf (me.rname, MAXREALNAME, "/msg %s \2HELP\2", s_Services);
	SignOn_NewBot (s_Services, me.user, me.host, me.rname, UMODE_SERVICES);
	me.onchan = 1;
	AddStringToList (&av, me.uplink, &ac);
	ModuleEvent (EVENT_ONLINE, av, ac);
	free (av);
	ac = 0;
	AddStringToList (&av, s_Services, &ac);
	ModuleEvent (EVENT_SIGNON, av, ac);
	free (av);
}

/** @brief dopong
 *
 * 
 *
 * @return none
 */
void
dopong (Server * s)
{
	char **av;
	int ac = 0;
	
	if (s) {
		s->ping = me.now - ping.last_sent;
		if (ping.ulag > 1)
			s->ping -= (float) ping.ulag;
		if (!strcmp (me.s->name, s->name))
			ping.ulag = me.s->ping;
		AddStringToList (&av, s->name, &ac);
		ModuleEvent (EVENT_PONG, av, ac);
		free (av);
	} else {
		nlog (LOG_NOTICE, LOG_CORE, "Received PONG from unknown server: %s", recbuf);
	}
}

/** @brief flood
 *
 * 
 *
 * @return 
 */
int
flood (User * u)
{
	time_t current = me.now;

	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Can't find user for FLOODcheck");
		return 0;
	}
	if (UserLevel (u) >= NS_ULEVEL_OPER)	/* locop or higher */
		return 0;
	if (current - u->t_flood > 10) {
		u->t_flood = me.now;
		u->flood = 0;
		return 0;
	}
	if (u->flood >= 5) {
		nlog (LOG_NORMAL, LOG_CORE, "FLOODING: %s!%s@%s", u->nick, u->username, u->hostname);
		ssvskill_cmd (u->nick, "%s!%s (Flooding Services.)", me.host, s_Services);
		return 1;
	} else {
		u->flood++;
	}
	return 0;
}

/** @brief Display our MOTD Message of the Day from the external neostats.motd file 
 *
 * 
 *
 * @return 
 */
void
ShowMOTD (char *nick)
{
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	snumeric_cmd (RPL_MOTDSTART, nick, ":- %s Message of the Day -", me.name);
	snumeric_cmd (RPL_MOTD, nick, ":- %d.%d.%d%s. Copyright (c) 1999 - 2003 The NeoStats Group", MAJOR, MINOR, REV, ircd_version);
	snumeric_cmd (RPL_MOTD, nick, ":-");

	fp = fopen (MOTD_FILENAME, "r");

	if (fp) {
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			snumeric_cmd (RPL_MOTD, nick, ":- %s", buf);
		}
		fclose (fp);
	} else {
		snumeric_cmd (RPL_MOTD, nick, ":- MOTD file Missing");
	}
	snumeric_cmd (RPL_ENDOFMOTD, nick, ":End of /MOTD command.");
}

/** @brief Display the ADMIN Message from the external stats.admin file
 *
 * 
 *
 * @return 
 */
void
ShowADMIN (char *nick)
{
	FILE *fp;
	char buf[BUFSIZE];
	SET_SEGV_LOCATION();

	snumeric_cmd (RPL_ADMINME, nick, ":- %s NeoStats Admins -", me.name);
	snumeric_cmd (RPL_ADMINME, nick, ":- %d.%d.%d%s.  Copyright (c) 1999 - 2003 The NeoStats Group", MAJOR, MINOR, REV, ircd_version);

	fp = fopen (ADMIN_FILENAME, "r");

	if (fp) {
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			snumeric_cmd (RPL_ADMINLOC1, nick, ":- %s", buf);
		}
		fclose (fp);
	}
	snumeric_cmd (RPL_ADMINLOC2, nick, ":End of /ADMIN command.");
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
Showcredits (char *nick)
{
	SET_SEGV_LOCATION();
	snumeric_cmd (RPL_VERSION, nick, ":- NeoStats %d.%d.%d%s Credits ", MAJOR, MINOR, REV, ircd_version);
	snumeric_cmd (RPL_VERSION, nick, ":- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	snumeric_cmd (RPL_VERSION, nick, ":- For Support, you can find ^Enigma^ or Shmad at");
	snumeric_cmd (RPL_VERSION, nick, ":- irc.irc-chat.net #NeoStats");
	snumeric_cmd (RPL_VERSION, nick, ":- Thanks to:");
	snumeric_cmd (RPL_VERSION, nick, ":- Enigma for being part of the dev team");
	snumeric_cmd (RPL_VERSION, nick, ":- Stskeeps for Writting the best IRCD ever!");
	snumeric_cmd (RPL_VERSION, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)");
	snumeric_cmd (RPL_VERSION, nick, ":- monkeyIRCD for the Module Segv Catching code");
	snumeric_cmd (RPL_VERSION, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!");
	snumeric_cmd (RPL_VERSION, nick, ":- Andy For Ideas");
	snumeric_cmd (RPL_VERSION, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies");
	snumeric_cmd (RPL_VERSION, nick, ":- sre and Jacob for development systems and access");
	snumeric_cmd (RPL_VERSION, nick, ":- Error51 for Translating our FAQ and README files");
	snumeric_cmd (RPL_VERSION, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!");
	snumeric_cmd (RPL_VERSION, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't (and all the bug reports!)");
	snumeric_cmd (RPL_VERSION, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips :)");
	snumeric_cmd (RPL_VERSION, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback");
	snumeric_cmd (RPL_VERSION, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd");
	snumeric_cmd (RPL_VERSION, nick, ":- Blud - Giving us patches for Mystic IRCd");
	snumeric_cmd (RPL_VERSION, nick, ":- herrohr - Giving us patches for Liquid IRCd support");
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
ShowStats (char *what, User * u)
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
	SET_SEGV_LOCATION();
	if (!strcasecmp (what, "u")) {
		/* server uptime - Shmad */
		int uptime = me.now - me.t_start;
		snumeric_cmd (RPL_STATSUPTIME, u->nick, "Statistical Server up %d days, %d:%02d:%02d", uptime / 86400, (uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
	} else if (!strcasecmp (what, "c")) {
		/* Connections */
		snumeric_cmd (RPL_STATSNLINE, u->nick, "N *@%s * * %d 50", me.uplink, me.port);
		snumeric_cmd (RPL_STATSCLINE, u->nick, "C *@%s * * %d 50", me.uplink, me.port);
	} else if (!strcasecmp (what, "o")) {
		/* Operators */
#ifdef EXTAUTH
		dl = get_dl_handle ("extauth");
		if (dl > 0) {
			listauth = dlsym ((int *) dl, "__list_auth");
			if (listauth)
				(*listauth) (u);
		} else
#endif
			snumeric_cmd (RPL_STATSOLINE, u->nick, "Operators think they are God, but you and I know they are not!");
	} else if (!strcasecmp (what, "l")) {
		/* Port Lists */
		tmp = me.now - me.lastmsg;
		tmp2 = me.now - me.t_start;
		snumeric_cmd (RPL_STATSLINKINFO, u->nick, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		snumeric_cmd (RPL_STATSLLINE, u->nick, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, me.SendM, me.SendBytes, me.RcveM, me.RcveBytes, tmp2, tmp);
	} else if (!strcasecmp (what, "M")) {
		for (I = 0; I < ircd_srv.cmdcount; I++) {
			if (cmd_list[I].usage > 0)
				snumeric_cmd (RPL_STATSCOMMANDS, u->nick, "Command %s Usage %d", cmd_list[I].name, cmd_list[I].usage);
		}
	}
	snumeric_cmd (RPL_ENDOFSTATS, u->nick, "%s :End of /STATS report", what);
	chanalert (s_Services, "%s Requested Stats %s", u->nick, what);
};
