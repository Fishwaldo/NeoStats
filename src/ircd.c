/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
#include "neostats.h"
#include "ircd.h"
#include "modules.h"
#include "dl.h"
#include "bots.h"
#include "commands.h"
#include "sock.h"
#include "users.h"
#include "channels.h"
#include "services.h"
#include "servers.h"
#include "bans.h"
#include "auth.h"

ircd_server ircd_srv;

static char ircd_buf[BUFSIZE];
static char UmodeStringBuf[64];
#ifdef GOTUSERSMODES
static char SmodeStringBuf[64];
#endif
long services_bot_umode= 0;

#ifdef IRCU
int scmode_op (const char *who, const char *chan, const char *mode, const char *bot);
#endif

/** @brief InitIrcd
 *
 *  ircd initialisation
 *
 * @return 
 */
void
InitIrcd ()
{
#ifdef IRCU
	/* Temp: force tokens for IRCU */
	ircd_srv.token = 1;
#endif
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
	char* tmpmode;

	/* Walk through mode string and convert to umode */
	tmpmode = (char*)UmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ircd_umodecount; i++) {
				if (user_umodes[i].mode == *tmpmode) {
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
		tmpmode++;
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
	char* tmpmode;

	/* Walk through mode string and convert to smode */
	tmpmode = (char*)SmodeString;
	while (*tmpmode) {
		switch (*tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ircd_smodecount; i++) {
				if (user_smodes[i].mode == *tmpmode) {
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
		tmpmode++;
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
	char savemod[SEGV_INMODULE_BUFSIZE];

	strlcpy(savemod, segv_inmodule, SEGV_INMODULE_BUFSIZE);
#ifdef GOTSJOIN
	ssjoin_cmd(who, chan, chflag);
#else
	sjoin_cmd(who, chan);
	if(chflag == CMODE_CHANOP || chflag == CMODE_CHANADMIN)
#if defined(IRCU)
		scmode_op(who, chan, "+o", who);
#else
		schmode_cmd(who, chan, "+o", who);
#endif

#endif
	SET_SEGV_INMODULE(savemod);
	return NS_SUCCESS;
}

/** @brief signon_newbot
 *
 *  work in progress to replace ircd specific routines
 *  needs sjoin fix to complete
 *
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
signon_newbot (const char *nick, const char *user, const char *host, const char *realname, long Umode)
{
	snewnick_cmd (nick, user, host, realname, Umode);
	if ((me.allbots > 0) || (Umode & services_bot_umode)) {
		join_bot_to_chan(nick, me.chan, CMODE_CHANADMIN);
	}
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
CloakHost (Bot *bot_ptr)
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
#ifndef IRCD_SPLITBUF
int
splitbuf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = calloc (sizeof (char *) * argvsize, 1);
	argc = 0;
	/*if (*buf == ':')
		buf++;*/
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = realloc (*argv, sizeof (char *) * argvsize);
		}
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
			if(colon_special) {
				(*argv)[argc++] = buf;
				break;
			}
		}
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
	return argc;
}
#endif

int
split_buf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;
	int colcount = 0;
	SET_SEGV_LOCATION();
	*argv = calloc (sizeof (char *) * argvsize, 1);
	argc = 0;
	if (*buf == ':')
		buf++;
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = realloc (*argv, sizeof (char *) * argvsize);
		}
		if ((*buf == ':') && (colcount < 1)) {
			buf++;
			colcount++;
		}
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
	return argc;
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
		nlog (LOG_DEBUG1, "joinbuf: from (%d) >= ac (%d)", from, ac);
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

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */
void 
m_notice (char* origin, char **av, int ac, int cmdptr)
{
	SET_SEGV_LOCATION();
	if( av[0] == NULL) {
		nlog (LOG_DEBUG1, "m_notice: dropping notice from %s to NULL: %s", origin, av[ac-1]);
		return;
	}
	nlog (LOG_DEBUG1, "m_notice: from %s, to %s : %s", origin, av[0], av[ac-1]);
	/* who to */
	if(av[0][0] == '#') {
		bot_chan_notice (origin, av, ac);
		return;
	}
#if 0
	if( ircstrcasecmp(av[0], "AUTH")) {
		nlog (LOG_DEBUG1, "m_notice: dropping server notice from %s, to %s : %s", origin, av[0], av[ac-1]);
		return;
	}
#endif
	bot_notice (origin, av, ac);
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */
void
m_private (char* origin, char **av, int ac, int cmdptr)
{
	char target[64];

	SET_SEGV_LOCATION();
	if( av[0] == NULL) {
		nlog (LOG_DEBUG1, "m_private: dropping privmsg from %s to NULL: %s", origin, av[ac-1]);
		return;
	}
	nlog (LOG_DEBUG1, "m_private: from %s, to %s : %s", origin, av[0], av[ac-1]);
	/* who to */
	if(av[0][0] == '#') {
		bot_chan_private (origin, av, ac);
		return;
	}
	if (strstr (av[0], "!")) {
		strlcpy (target, av[0], 64);
		av[0] = strtok (target, "!");
	} else if (strstr (av[0], "@")) {
		strlcpy (target, av[0], 64);
		av[0] = strtok (target, "@");
	}
	bot_private (origin, av, ac);
}

/** @brief process ircd commands
 *
 * 
 *
 * @return none
 */
#ifdef IRCU
void 
#else
static void 
#endif
process_ircd_cmd (int cmdptr, char *cmd, char* origin, char **av, int ac)
{
	int i;

	SET_SEGV_LOCATION();
	for (i = 0; i < ircd_cmdcount; i++) {
		if (!strcmp (cmd_list[i].name, cmd)
#ifdef GOTTOKENSUPPORT
			||(ircd_srv.token && cmd_list[i].token && !strcmp (cmd_list[i].token, cmd))
#endif
			) {
			if(cmd_list[i].function) {
				nlog (LOG_DEBUG3, "process_ircd_cmd: running command %s", cmd_list[i].name);
				cmd_list[i].function (origin, av, ac, cmdptr);
			} else {
				nlog (LOG_DEBUG3, "process_ircd_cmd: ignoring command %s", cmd);
			}
			cmd_list[i].usage++;
			return;
		}
	}
	nlog (LOG_INFO, "No support for %s", cmd);
}

/** @brief parse
 *
 * 
 *
 * @return none
 */
#ifndef IRCD_PARSE
void
parse (char *line)
{
	char origin[64], cmd[64], *coreLine;
	int cmdptr = 0;
	int ac;
	char **av;

	SET_SEGV_LOCATION();
	if (!(*line))
		return;
	nlog (LOG_DEBUG1, "--------------------------BEGIN PARSE---------------------------");
	nlog (LOG_DEBUG1, "R: %s", line);
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
	if (!*line)
		return;
	coreLine = strpbrk (line, " ");
	if (coreLine) {
		*coreLine = 0;
		while (isspace (*++coreLine));
	} else {
		coreLine = line + strlen (line);
	}
	strlcpy (cmd, line, sizeof (cmd)); 
	nlog (LOG_DEBUG1, "origin: %s", origin);
	nlog (LOG_DEBUG1, "cmd   : %s", cmd);
	nlog (LOG_DEBUG1, "args  : %s", coreLine);
	ac = splitbuf (coreLine, &av, 1);
	process_ircd_cmd (cmdptr, cmd, origin, av, ac);
	free (av);
	nlog (LOG_DEBUG1, "---------------------------END PARSE----------------------------");
}
#endif

/** @brief do_ping
 *
 * 
 *
 * @return none
 */
void
do_ping (const char* origin, const char* destination)
{
	send_pong (origin);
#ifdef MSG_BURST
	if (ircd_srv.burst) {
		send_ping (me.name, origin, origin);
	}
#endif
}

/** @brief do_pong
 *
 * 
 *
 * @return none
 */
void
do_pong (const char* origin, const char* destination)
{
	Server *s;
	CmdParams * cmdparams;

	s = findserver (origin);
	if (s) {
		s->ping = me.now - ping.last_sent;
		if (ping.ulag > 1)
			s->ping -= (float) ping.ulag;
		if (!strcmp (me.s->name, s->name))
			ping.ulag = me.s->ping;
		cmdparams = (CmdParams*)scalloc (sizeof(CmdParams));
		cmdparams->source.server = s;
		SendAllModuleEvent (EVENT_PONG, cmdparams);
		free (cmdparams);
		return;
	}
	nlog (LOG_NOTICE, "Received PONG from unknown server: %s", origin);
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
		nlog (LOG_WARNING, "flood: can't find user");
		return 0;
	}
	if (UserLevel (u) >= NS_ULEVEL_OPER)	/* locop or higher */
		return 0;
	if (current - u->tslastmsg > 10) {
		u->tslastmsg = me.now;
		u->flood = 0;
		return 0;
	}
	if (u->flood >= 5) {
		nlog (LOG_NORMAL, "FLOODING: %s!%s@%s", u->nick, u->username, u->hostname);
		ssvskill_cmd (u->nick, "%s!%s (Flooding Services.)", me.name, ns_botptr->nick);
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
do_version (const char* nick, const char *remoteserver)
{
	SET_SEGV_LOCATION();
	numeric (RPL_VERSION, nick, "%s :%s -> %s %s", me.versionfull, me.name, ns_module_info.build_date, ns_module_info.build_time);
	ModulesVersion (nick, remoteserver);
}

/** @brief Display our MOTD Message of the Day from the external neostats.motd file 
 *
 * 
 *
 * @return 
 */
void
do_motd (const char* nick, const char *remoteserver)
{
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	fp = fopen (MOTD_FILENAME, "r");
	if(!fp) {
		numeric (ERR_NOMOTD, nick, ":- MOTD file Missing");
	} else {
		numeric (RPL_MOTDSTART, nick, ":- %s Message of the Day -", me.name);
		numeric (RPL_MOTD, nick, ":- %s. Copyright (c) 1999 - 2004 The NeoStats Group", me.versionfull);
		numeric (RPL_MOTD, nick, ":-");

		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			numeric (RPL_MOTD, nick, ":- %s", buf);
		}
		fclose (fp);
		numeric (RPL_ENDOFMOTD, nick, ":End of MOTD command.");
	}
}

/** @brief Display the ADMIN Message from the external stats.admin file
 *
 * 
 *
 * @return 
 */
void
do_admin (const char* nick, const char *remoteserver)
{
	FILE *fp;
	char buf[BUFSIZE];
	SET_SEGV_LOCATION();

	fp = fopen (ADMIN_FILENAME, "r");
	if(!fp) {
		numeric (ERR_NOADMININFO, nick, "%s :No administrative info available", me.name);
	} else {
		numeric (RPL_ADMINME, nick, ":%s :Administrative info", me.name);
		numeric (RPL_ADMINME, nick, ":%s.  Copyright (c) 1999 - 2004 The NeoStats Group", me.versionfull);
		while (fgets (buf, sizeof (buf), fp)) {
			buf[strnlen (buf, BUFSIZE) - 1] = 0;
			numeric (RPL_ADMINLOC1, nick, ":- %s", buf);
		}
		fclose (fp);
		numeric (RPL_ADMINLOC2, nick, "End of /ADMIN command.");
	}
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
do_credits (const char* nick, const char *remoteserver)
{
	SET_SEGV_LOCATION();
	numeric (RPL_VERSION, nick, ":- NeoStats %s Credits ", me.versionfull);
	numeric (RPL_VERSION, nick, ":- Now Maintained by Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)");
	numeric (RPL_VERSION, nick, ":- For Support, you can find ^Enigma^ or Shmad at");
	numeric (RPL_VERSION, nick, ":- irc.irc-chat.net #NeoStats");
	numeric (RPL_VERSION, nick, ":- Thanks to:");
	numeric (RPL_VERSION, nick, ":- Enigma for being part of the dev team");
	numeric (RPL_VERSION, nick, ":- Stskeeps for writing the best IRCD ever!");
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
	numeric (RPL_VERSION, nick, ":- OvErRiTe - Giving us patches for Viagra IRCd support");
	numeric (RPL_VERSION, nick, ":- Reed Loden - Contributions to IRCu support");
}

/** @brief 
 *
 * 
 *
 * @return 
 */
void
do_stats (const char* nick, const char *what)
{
	time_t tmp;
	time_t tmp2;
	int i;
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "do_stats: message from unknown user %s", nick);
		return;
	}
	if (!ircstrcasecmp (what, "u")) {
		/* server uptime - Shmad */
		int uptime = me.now - me.t_start;
		numeric (RPL_STATSUPTIME, u->nick, "Statistical Server up %d days, %d:%02d:%02d", uptime / 86400, (uptime / 3600) % 24, (uptime / 60) % 60, uptime % 60);
	} else if (!ircstrcasecmp (what, "c")) {
		/* Connections */
		numeric (RPL_STATSNLINE, u->nick, "N *@%s * * %d 50", me.uplink, me.port);
		numeric (RPL_STATSCLINE, u->nick, "C *@%s * * %d 50", me.uplink, me.port);
	} else if (!ircstrcasecmp (what, "o")) {
		/* Operators */
		ListAuth(u);
	} else if (!ircstrcasecmp (what, "l")) {
		/* Port Lists */
		tmp = me.now - me.lastmsg;
		tmp2 = me.now - me.t_start;
		numeric (RPL_STATSLINKINFO, u->nick, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE");
		numeric (RPL_STATSLLINE, u->nick, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, (int)me.SendM, (int)me.SendBytes, (int)me.RcveM, (int)me.RcveBytes, (int)tmp2, (int)tmp);
        } else if (!ircstrcasecmp(what, "Z")) {
                if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
                        do_dns_stats_Z(u);
                }
	} else if (!ircstrcasecmp (what, "M")) {
		for (i = 0; i < ircd_cmdcount; i++) {
			if (cmd_list[i].usage > 0)
				numeric (RPL_STATSCOMMANDS, u->nick, "Command %s Usage %d", cmd_list[i].name, cmd_list[i].usage);
		}
	}
	numeric (RPL_ENDOFSTATS, u->nick, "%s :End of /STATS report", what);
	chanalert (ns_botptr->nick, "%s Requested Stats %s", u->nick, what);
};

void
do_protocol (char *origin, char **argv, int argc)
{
	int i;

	ircd_srv.unkline = 0;
	for (i = 0; i < argc; i++) {
#ifdef GOTTOKENSUPPORT
		if (!ircstrcasecmp ("TOKEN", argv[i])) {
			ircd_srv.token = 1;
		}
#endif
#ifdef GOTCLIENTSUPPORT
		if (!ircstrcasecmp ("CLIENT", argv[i])) {
			me.client = 1;
		}
#endif
#if defined(HYBRID7)
		if (!ircstrcasecmp ("UNKLN", argv[i])) {
			ircd_srv.unkline = 1;
		}
#endif
	}
}

void
privmsg_list (char *to, char *from, const char **text)
{
	while (*text) {
		if (**text) {
			prefmsg (to, from, (char*)*text);
		} else {
			prefmsg (to, from, " ");
		}
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
	send_privmsg (from, me.chan, ircd_buf);
}

void
prefmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (ns_botptr->nick, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (me.want_privmsg) {
		send_privmsg (from, to, ircd_buf);
	} else {
		send_notice (from, to, ircd_buf);
	}
}

void
privmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (ns_botptr->nick, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	send_privmsg (from, to, ircd_buf);
}

void
notice (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (ns_botptr->nick, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	send_notice (from, to, ircd_buf);
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
		nlog (LOG_NORMAL, ircd_buf);
	}
}

void
wallops (const char *from, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	send_wallops ((char*)from, (char*)ircd_buf);
}

void
numeric (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;

	va_start (ap, data);
	ircvsnprintf (ircd_buf, BUFSIZE, data, ap);
	va_end (ap);
	send_numeric (me.name, numeric, target, ircd_buf);
}

void
unsupported_cmd(const char* cmd)
{
	chanalert (ns_botptr->nick, "Warning, %s tried to %s which is not supported", ((segv_inmodule[0] != 0)? segv_inmodule : ""), cmd);
	nlog (LOG_NOTICE, "Warning, %s tried to %s, which is not supported", ((segv_inmodule[0] != 0)? segv_inmodule : ""), cmd);
}

int
sumode_cmd (const char *who, const char *target, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	send_umode (who, target, newmode);
	UserMode (target, newmode);
	return NS_SUCCESS;
}

int
spart_cmd (const char *who, const char *chan)
{
	send_part(who, chan);
	part_chan (finduser (who), (char *) chan, NULL);
	return NS_SUCCESS;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	UserNick (oldnick, newnick, NULL);
	send_nickchange (oldnick, newnick, me.now);
	return NS_SUCCESS;
}

#ifdef IRCU
int 
scmode_op (const char *who, const char *chan, const char *mode, const char *bot)
{
	char **av;
	int ac;

	send_cmode (me.name, who, chan, mode, nicktobase64 (bot), me.now);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, bot);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);
	return NS_SUCCESS;
}
#endif

int
schmode_cmd (const char *who, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;

	send_cmode (me.name, who, chan, mode, args, me.now);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);
	return NS_SUCCESS;
}

int
squit_cmd (const char *who, const char *quitmsg)
{
	send_quit (who, quitmsg);
	do_quit (who, quitmsg);
	return NS_SUCCESS;
}

int
skill_cmd (const char *from, const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	send_kill (from, target, ircd_buf);
	do_quit (target, ircd_buf);
	return NS_SUCCESS;
}

int
ssvskill_cmd (const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
#ifdef GOTSVSKILL
	send_svskill (me.name, target, ircd_buf);
#else
	send_kill (me.name, target, ircd_buf);
	do_quit (target, ircd_buf);
#endif
	return NS_SUCCESS;
}

#ifdef GOTSVSTIME
int 
ssvstime_cmd (const time_t ts)
{
	send_svstime(me.name, (unsigned long)ts);
	nlog (LOG_NOTICE, "ssvstime_cmd: synching server times to %lu", ts);
	return NS_SUCCESS;
}
#endif

int
skick_cmd (const char *who, const char *chan, const char *target, const char *reason)
{
	send_kick (who, chan, target, reason);
	part_chan (finduser (target), (char *) chan, reason[0] != 0 ? (char *)reason : NULL);
	return NS_SUCCESS;
}

int 
sinvite_cmd (const char *from, const char *to, const char *chan) 
{
	send_invite(from, to, chan);
	return NS_SUCCESS;
}

int
ssvsmode_cmd (const char *target, const char *modes)
{
#ifdef GOTSVSMODE
	User *u;

	u = finduser (target);
	if (!u) {
		nlog (LOG_WARNING, "ssvsmode_cmd: can't find user %s", target);
		return 0;
	}
	send_svsmode(me.name, target, modes);
	UserMode (target, modes);
#else
	unsupported_cmd("SVSMODE");
#endif
	return NS_SUCCESS;
}

int
ssvshost_cmd (const char *target, const char *vhost)
{
#ifdef GOTSVSHOST 
	User *u;

	u = finduser (target);
	if (!u) {
		nlog (LOG_WARNING, "ssvshost_cmd: can't find user %s", target);
		return 0;
	}

	strlcpy (u->vhost, vhost, MAXHOST);
	send_svshost(me.name, target, vhost);
#else
	unsupported_cmd("SVSHOST");
#endif
	return NS_SUCCESS;
}

int
ssvsjoin_cmd (const char *target, const char *chan)
{
#ifdef GOTSVSJOIN 
	send_svsjoin (me.name, target, chan);
#else
	unsupported_cmd("SVSJOIN");
#endif
	return NS_SUCCESS;
}

int
ssvspart_cmd (const char *target, const char *chan)
{
#ifdef GOTSVSPART
	send_svspart (me.name, target, chan);
#else
	unsupported_cmd("SVSPART");
#endif
	return NS_SUCCESS;
}

int
sswhois_cmd (const char *target, const char *swhois)
{
#ifdef GOTSWHOIS
	send_swhois (me.name, target, swhois);
#else
	unsupported_cmd("SWHOIS");
#endif
	return NS_SUCCESS;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
#ifdef GOTSVSNICK
	send_svsnick (me.name, target, newnick, me.now);
#else
	unsupported_cmd("SVSNICK");
#endif
	return NS_SUCCESS;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
#ifdef GOTSMO
	send_smo (from, umodetarget, msg);
#else
	unsupported_cmd("SMO");
#endif
	return NS_SUCCESS;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	send_akill(me.name, host, ident, setby, length, ircd_buf, me.now);
	return NS_SUCCESS;
}

int
srakill_cmd (const char *host, const char *ident)
{
	send_rakill (me.name, host, ident);
	return NS_SUCCESS;
}

int
ssjoin_cmd (const char *who, const char *chan, unsigned long chflag)
{
	char flag;
	char mode;
	char **av;
	int ac;
	time_t ts;
	Channel *c;

	c = findchan ((char *) chan);
	if (!c) {
		ts = me.now;
	} else {
		ts = c->creationtime;
	}
	switch (chflag) {
#ifdef CMODE_FL_CHANOP
	case CMODE_CHANOP:
		flag = CMODE_FL_CHANOP;
		mode= CMODE_CH_CHANOP;
		break;
#endif
#ifdef CMODE_FL_VOICE
	case CMODE_VOICE:
		flag = CMODE_FL_VOICE;
		mode= CMODE_CH_VOICE;
		break;
#endif
#ifdef CMODE_FL_CHANOWNER
    case CMODE_CHANOWNER:
        flag = CMODE_FL_CHANOWNER;
        mode= CMODE_CH_CHANOWNER;
        break;
#endif
#ifdef CMODE_FL_CHANPROT
    case CMODE_CHANPROT:
        flag = CMODE_FL_CHANPROT;
        mode= CMODE_CH_CHANPROT;
        break;
#endif
#ifdef CMODE_FL_VIP
    case CMODE_VIP:
		flag = CMODE_FL_VIP;
		mode= CMODE_CH_VIP;
		break;
#endif
#ifdef CMODE_FL_HALFOP
    case CMODE_HALFOP:
        flag = CMODE_FL_HALFOP;
        mode= CMODE_CH_HALFOP;
        break;
#endif
#ifdef CMODE_FL_UOP
    case CMODE_UOP:
        flag = CMODE_FL_UOP;
        mode= CMODE_CH_UOP;
        break;
#endif
#ifndef FAKE_CMODE_CHANADMIN
#ifdef CMODE_FL_CHANADMIN
	case CMODE_CHANADMIN:
		flag = CMODE_FL_CHANADMIN;
		mode= CMODE_CH_CHANADMIN;
		break;
#endif
#endif
#ifdef CMODE_SILENCE
	case CMODE_FL_SILENCE:
		flag = CMODE_FL_SILENCE;
		mode= CMODE_CH_SILENCE;
		break;
#endif
	default:
		flag = ' ';
		mode= '\0';
	}
	if (mode == 0) {
		ircsnprintf (ircd_buf, BUFSIZE, "%s", who);
	} else {
		ircsnprintf (ircd_buf, BUFSIZE, "%c%s", flag, who);
	}
	send_sjoin (me.name, ircd_buf, chan, (unsigned long)ts);
	join_chan (who, chan);
	ircsnprintf (ircd_buf, BUFSIZE, "%s +%c %s", chan, mode, who);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);
	return NS_SUCCESS;
}

int
sjoin_cmd (const char *who, const char *chan)
{
	send_join (me.name, who, chan, me.now);
	join_chan (who, chan);
	return NS_SUCCESS;
}

int
sping_cmd (const char *from, const char *reply, const char *to)
{
	send_ping (from, reply, to);
	return NS_SUCCESS;
}

int
spong_cmd (const char *reply)
{
	send_pong (reply);
	return NS_SUCCESS;
}

int
sserver_cmd (const char *name, const int numeric, const char *infoline)
{
	send_server (me.name, name, numeric, infoline);
	return NS_SUCCESS;
}

int
ssquit_cmd (const char *server, const char *quitmsg)
{
	send_squit (server, quitmsg);
	return NS_SUCCESS;
}

int
snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	AddUser (nick, ident, host, realname, me.name, NULL, NULL, NULL);
	send_nick (nick, (unsigned long)me.now, newmode, ident, host, me.name, realname);
#if defined(ULTIMATE3) || defined(BAHAMUT) || defined(HYBRID7) || defined(IRCU) || defined(NEOIRCD) || defined(QUANTUM) || defined(LIQUID)
	UserMode (nick, newmode);
#else
	sumode_cmd (nick, nick, mode);
#endif
	return NS_SUCCESS;
}

/* SJOIN <TS> #<channel> <modes> :[@][+]<nick_1> ...  [@][+]<nick_n> */
void 
do_sjoin (char* tstime, char* channame, char *modes, char *sjoinnick, char **argv, int argc)
{
	char nick[MAXNICK];
	char* nicklist;
	int modeexists;
	long mode = 0;
	int ok = 1, i, j = 3;
	ModesParm *m;
	Channel *c;
	lnode_t *mn = NULL;
	char **param;
	int paramcnt = 0;
	int paramidx = 0;

	if (*modes == '#') {
		join_chan (sjoinnick, modes);
		return;
	}

	paramcnt = split_buf(argv[argc-1], &param, 0);
		   
	while (paramcnt > paramidx) {
		nicklist = param[paramidx];
#ifdef UNREAL
		/* Unreal passes +b(&) and +e(") via SJ3 so skip them for now */	
		if(*nicklist == '&' || *nicklist == '"') {
			nlog (LOG_DEBUG1, "Skipping %s", nicklist);
			paramidx++;
			continue;
		}
#endif
		mode = 0;
		while (ok == 1) {
			for (i = 0; i < ircd_cmodecount; i++) {
				if (chan_modes[i].sjoin != 0) {
					if (*nicklist == chan_modes[i].sjoin) {
						mode |= chan_modes[i].mode;
						nicklist++;
						i = -1;
					}
				} else {
					/* sjoin's should be at the top of the list */
					ok = 0;
					strlcpy (nick, nicklist, MAXNICK);
					break;
				}
			}
		}
		join_chan (nick, channame); 
		ChanUserMode (channame, nick, 1, mode);
		paramidx++;
		ok = 1;
	}
	c = findchan (channame);
	if(c) {
		/* update the TS time */
		SetChanTS (c, atoi (tstime)); 
		if (*modes == '+') {
			while (*modes) {
				for (i = 0; i < ircd_cmodecount; i++) {
					if (*modes == chan_modes[i].flag) {
						if (chan_modes[i].parameters) {
							mn = list_first (c->modeparms);
							modeexists = 0;
							while (mn) {
								m = lnode_get (mn);
								/* mode limit and mode key replace current values */
								if ((m->mode == CMODE_LIMIT) && (chan_modes[i].mode == CMODE_LIMIT)) {
									strlcpy (m->param, argv[j], PARAMSIZE);
									j++;
									modeexists = 1;
									break;
								} else if ((m->mode == CMODE_KEY) && (chan_modes[i].mode == CMODE_KEY)) {
									strlcpy (m->param, argv[j], PARAMSIZE);
									j++;
									modeexists = 1;
									break;
								} else if (((int *) m->mode == (int *) chan_modes[i].mode) && !ircstrcasecmp (m->param, argv[j])) {
									nlog (LOG_INFO, "ChanMode: Mode %c (%s) already exists, not adding again", chan_modes[i].flag, argv[j]);
									j++;
									modeexists = 1;
									break;
								}
								mn = list_next (c->modeparms, mn);
							}
							if (modeexists != 1) {
								m = smalloc (sizeof (ModesParm));
								m->mode = chan_modes[i].mode;
								strlcpy (m->param, argv[j], PARAMSIZE);
								mn = lnode_create (m);
								if (list_isfull (c->modeparms)) {
									nlog (LOG_CRITICAL, "ChanMode: modelist is full adding to channel %s", c->name);
									do_exit (NS_EXIT_ERROR, "List full - see log file");
								} else {
									list_append (c->modeparms, mn);
								}
								j++;
							}
						} else {
							c->modes |= chan_modes[i].mode;
						}
					}
				}
				modes++;
			}
		}
	}
	free(param);
}

#ifdef MSG_NETINFO
void 
do_netinfo(const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname)
{
	ircd_srv.maxglobalcnt = atoi (maxglobalcnt);
	ircd_srv.tsendsync = atoi (tsendsync);
	ircd_srv.uprot = atoi (prot);
	strlcpy (ircd_srv.cloak, cloak, 10);
	strlcpy (me.netname, netname, MAXPASS);
	send_netinfo (me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, me.now);
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
	SendAllModuleEvent (EVENT_NETINFO, NULL);
	me.synced = 1;
}
#endif

#ifdef MSG_SNETINFO
void 
do_snetinfo(const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname)
{
	ircd_srv.uprot = atoi (prot);
	strlcpy (ircd_srv.cloak, cloak, 10);
	strlcpy (me.netname, netname, MAXPASS);
	send_snetinfo (me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, me.now);
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
	SendAllModuleEvent (EVENT_NETINFO, NULL);
	me.synced = 1;
}
#endif

void
do_join (const char* nick, const char* chanlist, const char* keys)
{
	char *s, *t;
	t = (char*)chanlist;
	while (*(s = t)) {
		t = s + strcspn (s, ",");
		if (*t)
			*t++ = 0;
		join_chan (nick, s);
	}
}

void 
do_part (const char* nick, const char* chan, const char* reason)
{
	part_chan (finduser (nick), chan, reason);
}

void 
do_nick (const char *nick, const char *hopcount, const char* TS, 
		 const char *user, const char *host, const char *server, 
		 const char *ip, const char *servicestamp, const char *modes, 
		 const char *vhost, const char *realname, const char *numeric
#ifdef GOTUSERSMODES
		 , const char *smodes
#endif
		 )
{
	AddUser (nick, user, host, realname, server, ip, TS, numeric);
	if(modes) {
		UserMode (nick, modes);
	}
	if(vhost) {
		SetUserVhost(nick, vhost);
	}
#ifdef GOTUSERSMODES
	if(smodes) {
		UserSMode (nick, smodes);
	}
#endif		
}

void 
do_client (const char *nick, const char *arg1, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *arg9, 
		const char *ip, const char *realname)
{
	AddUser (nick, user, host, realname, server, ip, TS, NULL);
	if(modes) {
		UserMode (nick, modes);
	}
	if(vhost) {
		SetUserVhost(nick, vhost);
	}
#ifdef GOTUSERSMODES
	if(smodes) {
		UserSMode (nick, smodes);
	}
#endif		
}

void
do_kill (const char *nick, const char *reason)
{
	KillUser (nick, reason);
}

void
do_quit (const char *nick, const char *quitmsg)
{
	QuitUser (nick, quitmsg);
}

void 
do_squit(const char *name, const char* reason)
{
	DelServer (name, reason);
}

void
do_kick (const char *kickby, const char *chan, const char *kicked, const char *kickreason)
{
	kick_chan (kickby, chan, kicked, kickreason);
}

#ifdef MSG_SVINFO
void 
do_svinfo (void)
{
	send_svinfo (TS_CURRENT, TS_MIN, (unsigned long)me.now);
}
#endif

#ifdef MSG_VCTRL
void 
do_vctrl (const char* uprot, const char* nicklen, const char* modex, const char* gc, const char* netname)
{
	ircd_srv.uprot = atoi(uprot);
	ircd_srv.nicklen = atoi(nicklen);
	ircd_srv.modex = atoi(modex);
	ircd_srv.gc = atoi(gc);
	strlcpy (me.netname, netname, MAXPASS);
	send_vctrl (ircd_srv.uprot, ircd_srv.nicklen, ircd_srv.modex, ircd_srv.gc, me.netname);
}
#endif

#ifdef GOTUSERSMODES
void 
do_smode (const char* nick, const char* modes)
{
	UserSMode (nick, modes);
}
#endif

void 
do_mode_user (const char* nick, const char* modes)
{
	UserMode (nick, modes);
}

void 
do_svsmode_user (const char* nick, const char* modes, const char* ts)
{
	char modebuf[MODESIZE];
	
	if (ts && isdigit(*ts)) {
		const char* pModes;	
		char* pNewModes;	

		SetUserServicesTS (nick, ts);
		/* If only setting TS, we do not need further mode processing */
		if(strcasecmp(modes, "+d") == 0) {
			nlog (LOG_DEBUG3, "dropping modes since this is a services TS %s", modes);
			return;
		}
		/* We need to strip the d from the mode string */
		pNewModes = modebuf;
		pModes = modes;
		while(*pModes) {
			if(*pModes != 'd') {
				*pNewModes = *pModes;
			}
			pModes++;
			pNewModes++;			
		}
		/* NULL terminate */
		*pNewModes = 0;
		UserMode (nick, modebuf);
	} else {
		UserMode (nick, modes);
	}
}

void 
do_mode_channel (char *origin, char **argv, int argc)
{
	ChanMode (origin, argv, argc);
}

void 
do_away (const char* nick, const char *reason)
{
	UserAway (nick, reason);
}

void 
do_vhost (const char* nick, const char *vhost)
{
	SetUserVhost(nick, vhost);
}

void
do_nickchange (const char * oldnick, const char *newnick, const char * ts)
{
	UserNick (oldnick, newnick, ts);
}

void 
do_topic (const char* chan, const char *owner, const char* ts, const char *topic)
{
	ChanTopic (chan, owner, ts, topic);
}

void 
do_server (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline, int srv)
{
	if(!srv) {
		if (uplink == NULL) {
			me.s = AddServer (name, me.name, hops, numeric, infoline);
		} else {
			me.s = AddServer (name, uplink, hops, numeric, infoline);
		}
	} else {
		AddServer (name, uplink, hops, numeric, infoline);
	}
	
}

#ifndef IRCU
#ifdef MSG_BURST
void 
do_burst (char *origin, char **argv, int argc)
{
	if (argc > 0) {
		if (ircd_srv.burst == 1) {
			send_burst (0);
			ircd_srv.burst = 0;
			me.synced = 1;
			init_services_bot ();
		}
	} else {
		ircd_srv.burst = 1;
	}
}
#endif
#endif

#ifdef MSG_SWHOIS
void 
do_swhois (char *who, char *swhois)
{
	User* u;
	u = finduser(who);
	if(u) {
		strlcpy(u->swhois, swhois, MAXHOST);
	}
}
#endif

#ifdef MSG_TKL
void 
do_tkl(const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason)
{
	char mask[MAXHOST];
	ircsnprintf(mask, MAXHOST, "%s@%s", user, host);
	if(add[0] == '+') {
		AddBan(type, user, host, mask, reason, setby, tsset, tsexpire);
	} else {
		DelBan(type, user, host, mask, reason, setby, tsset, tsexpire);
	}
}
#endif

#ifdef MSG_EOS
void 
do_eos (const char *name)
{
	Server *s;

	s = findserver (name);
	if(s) {
		SynchServer(s);
		nlog (LOG_DEBUG1, "do_eos: server %s is now synched", name);
	} else {
		nlog (LOG_WARNING, "do_eos: server %s not found", name);
	}
}
#endif

void
send_cmd (char *fmt, ...)
{
	va_list ap;
	char buf[BUFSIZE];
	int buflen;
	
	va_start (ap, fmt);
	ircvsnprintf (buf, BUFSIZE, fmt, ap);
	va_end (ap);

	nlog (LOG_DEBUG2, "SENT: %s", buf);
	if(strnlen (buf, BUFSIZE) < BUFSIZE - 2) {
		strlcat (buf, "\n", BUFSIZE);
	} else {
		buf[BUFSIZE - 1] = 0;
		buf[BUFSIZE - 2] = '\n';
	}
	buflen = strnlen (buf, BUFSIZE);
	sts (buf, buflen);
}

#ifdef BASE64SERVERNAME

void
setserverbase64 (const char *name, const char* num)
{
	Server *s;

	s = findserver(name);
	if(s) {
		nlog (LOG_DEBUG1, "setserverbase64: setting %s to %s", name, num);
		strlcpy(s->name64, num, 6);
	} else {
		nlog (LOG_DEBUG1, "setserverbase64: cannot find %s for %s", name, num);
	}
}

char* 
servertobase64 (const char* name)
{
	Server *s;

	nlog (LOG_DEBUG1, "servertobase64: scanning for %s", name);
	s = findserver(name);
	if(s) {
		return s->name64;
	} else {
		nlog (LOG_DEBUG1, "servertobase64: cannot find %s", name);
	}
	return NULL;
}

char* 
base64toserver (const char* num)
{
	Server *s;

	nlog (LOG_DEBUG1, "base64toserver: scanning for %s", num);
	s = findserverbase64(num);
	if(s) {
		return s->name;
	} else {
		nlog (LOG_DEBUG1, "base64toserver: cannot find %s", num);
	}
	return NULL;
}

#endif

#ifdef BASE64NICKNAME
void
setnickbase64 (const char *nick, const char* num)
{
	User *u;

	u = finduser(nick);
	if(u) {
		nlog (LOG_DEBUG1, "setnickbase64: setting %s to %s", nick, num);
		strlcpy(u->nick64, num, B64SIZE);
	} else {
		nlog (LOG_DEBUG1, "setnickbase64: cannot find %s for %s", nick, num);
	}
}

char* 
nicktobase64 (const char* nick)
{
	User *u;

	nlog (LOG_DEBUG1, "nicktobase64: scanning for %s", nick);
	u = finduser(nick);
	if(u) {
		return u->nick64;
	} else {
		nlog (LOG_DEBUG1, "nicktobase64: cannot find %s", nick);
	}
	return NULL;
}

char* 
base64tonick (const char* num)
{
	User *u;

	nlog (LOG_DEBUG1, "base64tonick: scanning for %s", num);
	u = finduserbase64(num);
	if(u) {
		return u->nick;
	} else {
		nlog (LOG_DEBUG1, "base64tonick: cannot find %s", num);
	}
	return NULL;
}

#endif

