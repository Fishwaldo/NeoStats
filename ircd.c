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
#include "users.h"
#include "chans.h"
#include "services.h"

static char ircd_buf[BUFSIZE];
static char UmodeStringBuf[64];
#ifdef GOTUSERSMODES
static char SmodeStringBuf[64];
#endif

/** @brief init_ircd
 *
 *  ircd initialisation
 *
 * @return 
 */
void
init_ircd ()
{
	services_bot_umode = UmodeStringToMask(services_bot_modes, 0);
};

/** @brief UmodeMaskToString
 *
 *  Translate a mode mask to the string equivalent
 *
 * @return 
 */
char* 
UmodeMaskToString(const long Umode) 
{
	int i, j;

	UmodeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < ircd_umodecount; i++) {
		if (Umode & user_umodes[i].umode) {
			UmodeStringBuf[j] = user_umodes[i].mode;
			j++;
		}
	}
	UmodeStringBuf[j] = '\0';
	return(UmodeStringBuf);
}

/** @brief UmodeStringToMask
 *
 *  Translate a mode string to the mask equivalent
 *
 * @return 
 */
long
UmodeStringToMask(const char* UmodeString, long Umode)
{
	int i;
	int add = 0;
	char tmpmode;

	/* Walk through mode string and convert to umode */
	tmpmode = *(UmodeString);
	while (tmpmode) {
		switch (tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ircd_umodecount; i++) {
				if (user_umodes[i].mode == tmpmode) {
					if (add) {
						Umode |= user_umodes[i].umode;
						break;
					} else {
						Umode &= ~user_umodes[i].umode;
						break;
					}
				}
			}
		}
		tmpmode = *UmodeString++;
	}
	return(Umode);
}

#ifdef GOTUSERSMODES
/** @brief SmodeMaskToString
 *
 *  Translate a smode mask to the string equivalent
 *
 * @return 
 */
char* 
SmodeMaskToString(const long Smode) 
{
	int i, j;

	SmodeStringBuf[0] = '+';
	j = 1;
	for (i = 0; i < ircd_smodecount; i++) {
		if (Smode & user_smodes[i].umode) {
			SmodeStringBuf[j] = user_smodes[i].mode;
			j++;
		}
	}
	SmodeStringBuf[j] = '\0';
	return(SmodeStringBuf);
}

/** @brief SmodeStringToMask
 *
 *  Translate a smode string to the mask equivalent
 *
 * @return 
 */
long
SmodeStringToMask(const char* SmodeString, long Smode)
{
	int i;
	int add = 0;
	char tmpmode;

	/* Walk through mode string and convert to smode */
	tmpmode = *(SmodeString);
	while (tmpmode) {
		switch (tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ircd_smodecount; i++) {
				if (user_smodes[i].mode == tmpmode) {
					if (add) {
						Smode |= user_smodes[i].umode;
						break;
					} else {
						Smode &= ~user_smodes[i].umode;
						break;
					}
				}
			}
		}
		tmpmode = *SmodeString++;
	}
	return(Smode);
}
#endif

/** @brief join_bot_to_chan
 *
 * 
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
join_bot_to_chan (const char *who, const char *chan, unsigned long chflag)
{
#if defined(ULTIMATE3) || defined(BAHAMUT) || defined(QUANTUM) || defined(LIQUID)
	sjoin_cmd(who, chan, chflag);
#else
	sjoin_cmd(who, chan);
	if(chflag == CMODE_CHANOP || chflag == CMODE_CHANADMIN)
		schmode_cmd(who, chan, "+o", who);
#endif
	return NS_SUCCESS;
}

/** @brief SignOn_NewBot
 *
 *  work in progress to replace ircd specific routines
 *  needs sjoin fix to complete
 *
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
#if 0
int
SignOn_NewBot (const char *nick, const char *user, const char *host, const char *rname, long Umode)
{
	snewnick_cmd (nick, user, host, rname, Umode);
	sumode_cmd (nick, nick, Umode);
	if ((me.allbots > 0) || (Umode & services_bot_umode)) {
		sjoin_cmd (nick, me.chan);
		schmode_cmd (me.name, me.chan, "+o", nick);
	}
	return NS_SUCCESS;
}
#endif

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
	Umode = UmodeStringToMask(modes, 0);
	SignOn_NewBot (nick, user, host, rname, Umode);
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
	Umode = UmodeStringToMask(modes, 0);
	SignOn_NewBot (nick, user, host, rname, Umode);
	/* restore segv_inmodule from SIGNON */
	SET_SEGV_INMODULE(mod_name);
	bot_ptr->flags = flags;
	add_bot_cmd_list(bot_ptr, bot_cmd_list);
	if (bot_setting_list != NULL) {
		bot_ptr->bot_settings = bot_setting_list;	
		/* Default SET to ROOT only */
		bot_ptr->set_ulevel = NS_ULEVEL_ROOT;
		/* Now calculate minimum defined user level */
		while(bot_setting_list->option != NULL) {
			if(bot_setting_list->ulevel < bot_ptr->set_ulevel)
				bot_ptr->set_ulevel = bot_setting_list->ulevel;
			bot_setting_list++;
		}
	} else {
		bot_ptr->bot_settings = NULL;	
		bot_ptr->set_ulevel = NS_ULEVEL_ROOT;
	}
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

/** @brief CloakBotHost
 *
 *  Create a hidden hostmask for the bot 
 *  Currently only Unreal support via UMODE auto cloaking
 *  but function created for future use and propogation to
 *  external modules to avoid a future joint release.
 *
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
CloakHost (ModUser *bot_ptr)
{
#ifdef GOTUMODECLOAKING
	sumode_cmd (bot_ptr->nick, bot_ptr->nick, UMODE_HIDE);
	return NS_SUCCESS;	
#else
	return NS_FAILURE;	
#endif
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
	/* from is zero based while ac has base of 1. 
	 * Therefore we need to check >= before trying to perform
	 * the join.
	 * The current (null) string we return may not be needed
	 * so should be removed once all joinbuf calls are checked.
	 * Maybe we should just return NULL if we fail and let
	 * the caller handle that case. 
	 */
	if(from >= ac) {
		nlog (LOG_DEBUG1, LOG_CORE, "joinbuf: from (%d) >= ac (%d)", from, ac);
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

/** @brief process ircd commands
 *
 * 
 *
 * @return none
 */
static void 
process_ircd_cmd(int cmdptr, char *cmd, char* origin, char **av, int ac)
{
	int i;

	SET_SEGV_LOCATION();
	for (i = 0; i < ircd_cmdcount; i++) {
		if (cmd_list[i].srvmsg == cmdptr) {
			if (!strcmp (cmd_list[i].name, cmd)
#ifdef GOTTOKENSUPPORT
				||(me.token && cmd_list[i].token && !strcmp (cmd_list[i].token, cmd))
#endif
				) {
				cmd_list[i].function (origin, av, ac);
				cmd_list[i].usage++;
				break;
			}
		}
	}
#if 0
	if(i >= ircd_cmdcount) {
		nlog (LOG_INFO, LOG_CORE, "No support for %s", cmd);
	}
#endif
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
	if (!strcmp (MSG_PRIVATE, cmd) || (!strcmp ("!", cmd))) {
#else 
	if (!strcmp(MSG_PRIVATE, cmd) || !strcmp("P", cmd) || !strcmp("CPRIVMSG", cmd) || !strcmp("CP", cmd)) {
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
	process_ircd_cmd(cmdptr, cmd, origin, av, ac);
	/* K, now Parse it to the Module functions */
	ModuleFunction (cmdptr, cmd, origin, av, ac);
	free (av);
}

/** @brief init_services_bot
 *
 * 
 *
 * @return none
 */
int 
init_services_bot (void)
{
	char **av;
	int ac = 0;
	long Umode;

	SET_SEGV_LOCATION();
	if (finduser (s_Services)) {
		/* nick already exists on the network */
		strlcat (s_Services, "1", MAXNICK);
	}
	ircsnprintf (me.rname, MAXREALNAME, "/msg %s \2HELP\2", s_Services);
	Umode = UmodeStringToMask(services_bot_modes, 0);
	SignOn_NewBot (s_Services, me.user, me.host, me.rname, Umode);
	me.onchan = 1;
	AddStringToList (&av, me.uplink, &ac);
	ModuleEvent (EVENT_ONLINE, av, ac);
	free (av);
	return NS_SUCCESS;
}

/** @brief ns_usr_pong
 *
 * 
 *
 * @return none
 */
void
ns_usr_pong (char *origin, char **argv, int argc)
{
	Server *s;
	char **av;
	int ac = 0;

	s = findserver (argv[0]);
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
		nlog (LOG_NOTICE, LOG_CORE, "Received PONG from unknown server: %s", argv[0]);
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

/** @brief Display NeoStats version info
 *
 * 
 *
 * @return 
 */
void
ns_usr_version (char *origin, char **argv, int argc)
{
	SET_SEGV_LOCATION();
	numeric (RPL_VERSION, origin, "%d.%d.%d%s :%s -> %s %s", MAJOR, MINOR, REV, ircd_version, me.name, version_date, version_time);
	ModulesVersion (origin, argv, argc);
}

/** @brief Display our MOTD Message of the Day from the external neostats.motd file 
 *
 * 
 *
 * @return 
 */
void
ns_usr_motd (char *nick, char **argv, int argc)
{
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	numeric (RPL_MOTDSTART, nick, ":- %s Message of the Day -", me.name);
	numeric (RPL_MOTD, nick, ":- %d.%d.%d%s. Copyright (c) 1999 - 2003 The NeoStats Group", MAJOR, MINOR, REV, ircd_version);
	numeric (RPL_MOTD, nick, ":-");

	fp = fopen (MOTD_FILENAME, "r");

	if (fp) {
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			numeric (RPL_MOTD, nick, ":- %s", buf);
		}
		fclose (fp);
	} else {
		numeric (RPL_MOTD, nick, ":- MOTD file Missing");
	}
	numeric (RPL_ENDOFMOTD, nick, ":End of /MOTD command.");
}

/** @brief Display the ADMIN Message from the external stats.admin file
 *
 * 
 *
 * @return 
 */
void
ns_usr_admin (char *nick, char **argv, int argc)
{
	FILE *fp;
	char buf[BUFSIZE];
	SET_SEGV_LOCATION();

	numeric (RPL_ADMINME, nick, ":- %s NeoStats Admins -", me.name);
	numeric (RPL_ADMINME, nick, ":- %d.%d.%d%s.  Copyright (c) 1999 - 2003 The NeoStats Group", MAJOR, MINOR, REV, ircd_version);

	fp = fopen (ADMIN_FILENAME, "r");

	if (fp) {
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			numeric (RPL_ADMINLOC1, nick, ":- %s", buf);
		}
		fclose (fp);
	}
	numeric (RPL_ADMINLOC2, nick, ":End of /ADMIN command.");
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
ns_usr_credits (char *nick, char **argv, int argc)
{
	SET_SEGV_LOCATION();
	numeric (RPL_VERSION, nick, ":- NeoStats %d.%d.%d%s Credits ", MAJOR, MINOR, REV, ircd_version);
	numeric (RPL_VERSION, nick, ":- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	numeric (RPL_VERSION, nick, ":- For Support, you can find ^Enigma^ or Shmad at");
	numeric (RPL_VERSION, nick, ":- irc.irc-chat.net #NeoStats");
	numeric (RPL_VERSION, nick, ":- Thanks to:");
	numeric (RPL_VERSION, nick, ":- Enigma for being part of the dev team");
	numeric (RPL_VERSION, nick, ":- Stskeeps for Writting the best IRCD ever!");
	numeric (RPL_VERSION, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)");
	numeric (RPL_VERSION, nick, ":- monkeyIRCD for the Module Segv Catching code");
	numeric (RPL_VERSION, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!");
	numeric (RPL_VERSION, nick, ":- Andy For Ideas");
	numeric (RPL_VERSION, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies");
	numeric (RPL_VERSION, nick, ":- sre and Jacob for development systems and access");
	numeric (RPL_VERSION, nick, ":- Error51 for Translating our FAQ and README files");
	numeric (RPL_VERSION, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!");
	numeric (RPL_VERSION, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't (and all the bug reports!)");
	numeric (RPL_VERSION, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips :)");
	numeric (RPL_VERSION, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback");
	numeric (RPL_VERSION, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd");
	numeric (RPL_VERSION, nick, ":- Blud - Giving us patches for Mystic IRCd");
	numeric (RPL_VERSION, nick, ":- herrohr - Giving us patches for Liquid IRCd support");
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
ns_usr_stats (char *origin, char **argv, int argc)
{
	time_t tmp;
	time_t tmp2;
	int i;
#ifdef EXTAUTH
	int dl;
	int (*listauth) (User * u);
#endif
	User *u;
	char *what;

	SET_SEGV_LOCATION();
	u = finduser (origin);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Received a Message from an Unknown User! (%s)", origin);
		return;
	}
	what = argv[0];
	if (!strcasecmp (what, "u")) {
		/* server uptime - Shmad */
		int uptime = me.now - me.t_start;
		numeric (RPL_STATSUPTIME, u->nick, "Statistical Server up %d days, %d:%02d:%02d", uptime / 86400, (uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
	} else if (!strcasecmp (what, "c")) {
		/* Connections */
		numeric (RPL_STATSNLINE, u->nick, "N *@%s * * %d 50", me.uplink, me.port);
		numeric (RPL_STATSCLINE, u->nick, "C *@%s * * %d 50", me.uplink, me.port);
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
			numeric (RPL_STATSOLINE, u->nick, "Operators think they are God, but you and I know they are not!");
	} else if (!strcasecmp (what, "l")) {
		/* Port Lists */
		tmp = me.now - me.lastmsg;
		tmp2 = me.now - me.t_start;
		numeric (RPL_STATSLINKINFO, u->nick, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		numeric (RPL_STATSLLINE, u->nick, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, (int)me.SendM, (int)me.SendBytes, (int)me.RcveM, (int)me.RcveBytes, (int)tmp2, (int)tmp);
	} else if (!strcasecmp (what, "M")) {
		for (i = 0; i < ircd_cmdcount; i++) {
			if (cmd_list[i].usage > 0)
				numeric (RPL_STATSCOMMANDS, u->nick, "Command %s Usage %d", cmd_list[i].name, cmd_list[i].usage);
		}
	}
	numeric (RPL_ENDOFSTATS, u->nick, "%s :End of /STATS report", what);
	chanalert (s_Services, "%s Requested Stats %s", u->nick, what);
};

void
ns_srv_protocol(char *origin, char **argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++) {
#ifdef GOTTOKENSUPPORT
		if (!strcasecmp ("TOKEN", argv[i])) {
			me.token = 1;
		}
#endif
#ifdef GOTCLIENTSUPPORT
		if (!strcasecmp ("CLIENT", argv[i])) {
			me.client = 1;
		}
#endif
	}
}

void
privmsg_list (char *to, char *from, const char **text)
{
	while (*text) {
		if (**text)
			prefmsg (to, from, (char*)*text);
		else
			prefmsg (to, from, " ");
		text++;
	}
}

void
chanalert (char *from, char *fmt, ...)
{
	va_list ap;

	if (!me.onchan)
		return;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	send_privmsg (me.chan, from, ircd_buf);
}

void
prefmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (me.want_privmsg) {
		send_privmsg (to, from, ircd_buf);
	} else {
		send_notice (to, from, ircd_buf);
	}
}

void
privmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	send_privmsg (to, from, ircd_buf);
}

void
notice (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	send_notice (to, from, ircd_buf);
}

void
globops (char *from, char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);

	if (me.onchan) {
		send_globops(from, ircd_buf);
	} else {
		nlog (LOG_NORMAL, LOG_CORE, ircd_buf);
	}
}

int
wallops (const char *from, const char *msg, ...)
{
	va_list ap;

	va_start (ap, msg);
	ircvsnprintf (ircd_buf, BUFSIZE, msg, ap);
	va_end (ap);
	send_wallops ((char*)from, (char*)ircd_buf);
	return 1;
}

int
numeric (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;

	va_start (ap, data);
	ircvsnprintf (ircd_buf, BUFSIZE, data, ap);
	va_end (ap);
	send_numeric (numeric, target, ircd_buf);
	return 1;
}

int
sumode_cmd (const char *who, const char *target, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	send_umode (who, target, newmode);
	UserMode (target, newmode);
	return 1;
}

int
spart_cmd (const char *who, const char *chan)
{
	send_part(who, chan);
	part_chan (finduser (who), (char *) chan);
	return 1;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	UserNick (oldnick, newnick);
	send_nick (oldnick, newnick);
	return 1;
}

int
schmode_cmd (const char *who, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;

	send_cmode (who, chan, mode, args);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode ("", av, ac);
	free (av);
	return 1;
}

int
squit_cmd (const char *who, const char *quitmsg)
{
	send_quit (who, quitmsg);
	UserQuit (who, quitmsg);
	return 1;
}

int
skill_cmd (const char *from, const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	send_kill (from, target, ircd_buf);
	UserQuit (target, ircd_buf);
	return 1;
}

int
ssvskill_cmd (const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
#ifdef GOTSVSKILL
	send_svskill (target, ircd_buf);
#else
	send_kill (me.name, target, ircd_buf);
	UserQuit (target, ircd_buf);
#endif
	return 1;
}

int
skick_cmd (const char *who, const char *target, const char *chan, const char *reason)
{
	send_kick (who, target, chan, reason);
	part_chan (finduser (target), (char *) chan);
	return 1;
}

int 
sinvite_cmd (const char *from, const char *to, const char *chan) 
{
	send_invite(from, to, chan);
	return 1;
}

int
ssvsmode_cmd (const char *target, const char *modes)
{
#ifdef GOTSVSMODE
	User *u;

	u = finduser (target);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Can't find user %s for ssvsmode_cmd", target);
		return 0;
	}
	send_svsmode(target, modes);
	UserMode (target, modes);
#else
	chanalert (s_Services, "Warning, Module %s tried to SVSMODE which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SVSMODE, which is not supported", segvinmodule);
#endif
	return 1;
}

int
ssvshost_cmd (const char *who, const char *vhost)
{
#ifdef GOTSVSHOST 
	User *u;

	u = finduser (who);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Can't Find user %s for ssvshost_cmd", who);
		return 0;
	}

	strlcpy (u->vhost, vhost, MAXHOST);
	send_svshost(who, vhost);
	return 1;
#else
	chanalert (s_Services, "Warning Module %s tried to SVSHOST, which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSHOST, which is not supported", segvinmodule);
#endif
	return 1;
}

int
ssvsjoin_cmd (const char *target, const char *chan)
{
#ifdef GOTSVSJOIN 
	send_svsjoin (target, chan);
#else
	chanalert (s_Services, "Warning Module %s tried to SVSJOIN, which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSJOIN, which is not supported", segvinmodule);
#endif
	return 1;
}

int
ssvspart_cmd (const char *target, const char *chan)
{
#ifdef GOTSVSPART
	send_svspart (target, chan);
#else
	chanalert (s_Services, "Warning Module %s tried to SVSPART, which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSPART, which is not supported", segvinmodule);
#endif
	return 1;
}

int
sswhois_cmd (const char *target, const char *swhois)
{
#ifdef GOTSWHOIS
	send_swhois (target, swhois);
#else
	chanalert (s_Services, "Warning Module %s tried to SWHOIS, which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SWHOIS, which is not supported", segvinmodule);
#endif
	return 1;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
#ifdef GOTSVSNICK
	send_svsnick (target, newnick);
#else
	notice (s_Services, "Warning Module %s tried to SVSNICK, which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSNICK, which is not supported", segvinmodule);
#endif
	return 1;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
#ifdef GOTSMO
	send_smo (from, umodetarget, msg);
#else
	chanalert (s_Services, "Warning, Module %s tried to SMO, which is not supported", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SMO, which is not supported", segvinmodule);
#endif
	return 1;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	send_akill(host, ident, setby, length, ircd_buf);
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
	send_rakill(host, ident);
	return 1;
}

