/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
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

#include "stats.h"
#include "ircd.h"
#include "sock.h"
#include "Ircu.h"
#include "dl.h"
#include "log.h"

static void m_version (char *origin, char **argv, int argc, int srv);
static void m_motd (char *origin, char **argv, int argc, int srv);
static void m_admin (char *origin, char **argv, int argc, int srv);
static void m_server (char *origin, char **argv, int argc, int srv);
static void m_squit (char *origin, char **argv, int argc, int srv);
static void m_quit (char *origin, char **argv, int argc, int srv);
static void m_mode (char *origin, char **argv, int argc, int srv);
static void m_kill (char *origin, char **argv, int argc, int srv);
static void m_pong (char *origin, char **argv, int argc, int srv);
static void m_away (char *origin, char **argv, int argc, int srv);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_topic (char *origin, char **argv, int argc, int srv);
static void m_kick (char *origin, char **argv, int argc, int srv);
static void m_join (char *origin, char **argv, int argc, int srv);
static void m_part (char *origin, char **argv, int argc, int srv);
static void m_stats (char *origin, char **argv, int argc, int srv);
static void m_ping (char *origin, char **argv, int argc, int srv);
static void m_pass (char *origin, char **argv, int argc, int srv);
static void m_burst (char *origin, char **argv, int argc, int srv);

const char ircd_version[] = "(IRCU)";
const char services_bot_modes[]= "+oS";

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_PRIVATE, m_privmsg, 0},
	{MSG_NOTICE, m_notice, 0},
	{MSG_STATS, m_stats, 0},
	{MSG_VERSION, m_version, 0},
	{MSG_MOTD, m_motd, 0},
	{MSG_ADMIN, m_admin, 0},
	{MSG_SERVER, m_server, 0},
	{MSG_SQUIT, m_squit, 0},
	{MSG_QUIT, m_quit, 0},
	{MSG_MODE, m_mode, 0},
	{MSG_KILL, m_kill, 0},
	{MSG_PONG, m_pong, 0},
	{MSG_AWAY, m_away, 0},
	{MSG_NICK, m_nick, 0},
	{MSG_TOPIC, m_topic, 0},
	{MSG_KICK, m_kick, 0},
	{MSG_JOIN, m_join, 0},
	{MSG_PART, m_part, 0},
	{MSG_PING, m_ping, 0},
	{MSG_PASS, m_pass, 0},
	{MSG_END_OF_BURST, m_burst, 0},
};

ChanModes chan_modes[] = {
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_SECRET, 's', 0, 0, 0},
	{CMODE_PRIVATE, 'p', 0, 0, 0},
	{CMODE_MODERATED, 'm', 0, 0, 0},
	{CMODE_TOPICLIMIT, 't', 0, 0, 0},
	{CMODE_INVITEONLY, 'i', 0, 0, 0},
	{CMODE_NOPRIVMSGS, 'n', 0, 0, 0},
	{CMODE_LIMIT, 'l', 0, 1, 0},
	{CMODE_KEY, 'k', 0, 1, 0},
	{CMODE_BAN, 'b', 0, 1, 0},
	/*{CMODE_SENDTS, 'b', 0, 1, 0},*/
	{CMODE_DELAYJOINS, 'D', 0, 1, 0},
	/*{CMODE_LISTED, 'b', 0, 1, 0},*/
};

UserModes user_umodes[] = {
	{UMODE_DEBUG,       'g', NS_ULEVEL_ROOT},
	{UMODE_OPER,		'o', NS_ULEVEL_ADMIN},
	{UMODE_LOCOP,		'O', NS_ULEVEL_OPER},
	{UMODE_INVISIBLE,	'i', 0},
	{UMODE_WALLOP,	'w', 0},
	{UMODE_SERVNOTICE,	's', 0},
	{UMODE_DEAF,		'd', 0},
	{UMODE_CHSERV,	'k', 0},
	{UMODE_HELPER,	'h', 0},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_server (const char *name, const int numeric, const char *infoline)
{
	sts (":%s %s %s %d :%s", me.name, MSG_SERVER, name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass)
{
	sts ("%s %s :TS", MSG_PASS, pass);
	sts ("CAPAB :TS EX CHW IE KLN GLN KNOCK HOPS HUB AOPS MX");
	sts ("%s %s %d :%s", MSG_SERVER, name, numeric, infoline);
}

void
send_squit (const char *server, const char *quitmsg)
{
	sts ("%s %s :%s", MSG_SQUIT, server, quitmsg);
}

void 
send_quit (const char *who, const char *quitmsg)
{
	sts (":%s %s :%s", who, MSG_QUIT, quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	sts (":%s %s %s", who, MSG_PART, chan);
}

void
send_join (const char *who, const char *chan)
{
	sts (":%s %s 0 %s + :%s", me.name, MSG_JOIN, chan, who);
}

void 
send_cmode (const char *who, const char *chan, const char *mode, const char *args)
{
	sts (":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, me.now);
}

void
send_nick (const char *nick, const char *ident, const char *host, const char *realname, const char* newmode, time_t tstime)
{
	sts ("%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, tstime, newmode, ident, host, me.name, realname);
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	sts (":%s %s %s :%s", from, MSG_PING, reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	sts (":%s %s %s :%s", who, MSG_MODE, target, mode);
}

void 
send_numeric (const char *from, const int numeric, const char *target, const char *buf)
{
	sts (":%s %d %s :%s", from, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	sts ("%s %s", MSG_PONG, reply);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	sts (":%s %s %s :%s", from, MSG_KILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const time_t ts)
{
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, (int) me.now);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
}

void 
send_sjoin (const char *who, const char *chan, const char flag, time_t tstime)
{
}

void 
send_kick (const char *who, const char *chan, const char *target, const char *reason)
{
	sts (":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *who, const char *buf)
{
	sts (":%s %s :%s", who, MSG_WALLOPS, buf);
}

void
send_burst (int b)
{
	if (b == 0) {
		sts ("BURST 0");
	} else {
		sts ("BURST");
	}
}

void 
send_akill (const char *host, const char *ident, const char *setby, const int length, const char *reason)
{
	/* there isn't an akill on Hybrid, so we send a kline to all servers! */
	hscan_t ss;
	hnode_t *sn;
	Server *s;

	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		//sts (":%s %s %s %lu %s %s :%s", setby, MSG_KLINE, s->name, (unsigned long)length, ident, host, reason);
	}
}

void 
send_rakill (const char *host, const char *ident)
{
	chanalert (s_Services, "Please Manually remove KLINES using /unkline on each server");
}

void
send_privmsg (const char *from, const char *to, const char *buf)
{
	sts (":%s %s %s :%s", from, MSG_PRIVATE, to, buf);
}

void
send_notice (const char *from, const char *to, const char *buf)
{
	sts (":%s %s %s :%s", from, MSG_NOTICE, to, buf);
}

void
send_globops (const char *from, const char *buf)
{
//	sts (":%s %s :%s", from, MSG_GLOBOPS, buf);
}


static void
m_stats (char *origin, char **argv, int argc, int srv)
{
	do_stats (origin, argv[0]);
}

static void
m_version (char *origin, char **argv, int argc, int srv)
{
	do_version (origin, argv[0]);
}

static void
m_motd (char *origin, char **argv, int argc, int srv)
{
	do_motd (origin, argv[0]);
}

static void
m_admin (char *origin, char **argv, int argc, int srv)
{
	do_admin (origin, argv[0]);
}

static void
m_server (char *origin, char **argv, int argc, int srv)
{
	do_server (argv[0], origin, argv[1], NULL, NULL, srv);
}

static void
m_squit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	do_squit (argv[0], tmpbuf);
	free(tmpbuf);
}

static void
m_quit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 0);
	do_quit (origin, tmpbuf);
	free(tmpbuf);
}

static void
m_mode (char *origin, char **argv, int argc, int srv)
{
	if (argv[0][0] == '#') {
		do_mode_channel (origin, argv, argc);
	} else {
		do_mode_user (argv[0], argv[1]);
	}
}
static void
m_kill (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	do_kill (argv[0], tmpbuf);
	free(tmpbuf);
}
static void
m_pong (char *origin, char **argv, int argc, int srv)
{
	do_pong (argv[0], argv[1]);
}
static void
m_away (char *origin, char **argv, int argc, int srv)
{
	char *buf;

	if (argc > 0) {
		buf = joinbuf (argv, argc, 0);
		do_away (origin, buf);
		free (buf);
	} else {
		do_away (origin, NULL);
	}
}
static void
m_nick (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
		char *realname;
		realname = joinbuf (argv, argc, 7);
		do_nick (argv[0], argv[1], argv[2], argv[4], argv[5], 
			argv[6], NULL, NULL, argv[3], NULL, realname);
		free (realname);
	} else {
		do_nickchange (origin, argv[0], NULL);
	}
}
static void
m_topic (char *origin, char **argv, int argc, int srv)
{
	char *buf;

	buf = joinbuf (argv, argc, 2);
	do_topic (argv[0], origin, NULL, buf);
	free (buf);
}

static void
m_kick (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 2);
	do_kick (origin, argv[0], argv[1], tmpbuf);
	free(tmpbuf);
}
static void
m_join (char *origin, char **argv, int argc, int srv)
{
	do_join (origin, argv[0], NULL);
}
static void
m_part (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	do_part (origin, argv[0], tmpbuf);
	free(tmpbuf);
}

static void
m_ping (char *origin, char **argv, int argc, int srv)
{
	do_ping (argv[0], argv[1]);
}

static void
m_pass (char *origin, char **argv, int argc, int srv)
{
}

static void
m_burst (char *origin, char **argv, int argc, int srv)
{
}

/* Override the core splitbuf and parse functions until 
 * IRCU support is complete
 */

int
splitbuf (char *buf, char ***argv, int colon_special)
{
	int argvsize = 8;
	int argc;
	char *s;

	SET_SEGV_LOCATION();
	*argv = calloc (sizeof (char *) * argvsize, 1);
	argc = 0;
	while (*buf) {
		if (argc == argvsize) {
			argvsize += 8;
			*argv = realloc (*argv, sizeof (char *) * argvsize);
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

void
parse (char *line)
{
	char origin[64], cmd[64], *coreLine;
	int cmdptr = 0;
	int ac;
	char **av;

	SET_SEGV_LOCATION();
	strip (line);
	strlcpy (recbuf, line, BUFSIZE);
	if (!(*line))
		return;
	nlog (LOG_DEBUG1, LOG_CORE, "R: %s", line);
	if (!*line)
		return;
	coreLine = strpbrk (line, " ");
	if (coreLine) {
		*coreLine = 0;
		while (isspace (*++coreLine));
	} else
		coreLine = line + strlen (line);
	if ((!ircstrcasecmp(line, "SERVER")) || (!ircstrcasecmp(line, "PASS"))) {
		strlcpy(cmd, line, sizeof(cmd));
		ac = splitbuf(coreLine, &av, 1);
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
		ac = splitbuf(line, &av, 0);
	}
	process_ircd_cmd (cmdptr, cmd, origin, av, ac);
	free (av);
}
