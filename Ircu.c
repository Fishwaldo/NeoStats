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

void process_ircd_cmd (int cmdptr, char *cmd, char* origin, char **av, int ac);

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
	{MSG_PRIVATE, TOK_PRIVATE, m_private, 0},
	{MSG_CPRIVMSG, TOK_CPRIVMSG, m_private, 0},
	{MSG_NOTICE, TOK_NOTICE, m_notice, 0},
	{MSG_CNOTICE, TOK_CNOTICE, m_notice, 0},
	{MSG_STATS, TOK_STATS, m_stats, 0},
	{MSG_VERSION, TOK_VERSION, m_version, 0},
	{MSG_MOTD, TOK_MOTD, m_motd, 0},
	{MSG_ADMIN, TOK_ADMIN, m_admin, 0},
	{MSG_SERVER, TOK_SERVER, m_server, 0},
	{MSG_SQUIT, TOK_SQUIT, m_squit, 0},
	{MSG_QUIT, TOK_QUIT, m_quit, 0},
	{MSG_MODE, TOK_MODE, m_mode, 0},
	{MSG_KILL, TOK_KILL, m_kill, 0},
	{MSG_PONG, TOK_PONG, m_pong, 0},
	{MSG_AWAY, TOK_AWAY, m_away, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_TOPIC, TOK_TOPIC, m_topic, 0},
	{MSG_KICK, TOK_KICK, m_kick, 0},
	{MSG_JOIN, TOK_JOIN, m_join, 0},
	{MSG_PART, TOK_PART, m_part, 0},
	{MSG_PING, TOK_PING, m_ping, 0},
	{MSG_PASS, TOK_PASS, m_pass, 0},
	{MSG_END_OF_BURST, TOK_END_OF_BURST, m_burst, 0},
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

/*
 * Numeric nicks are new as of version ircu2.10.00beta1.
 *
 * The idea is as follows:
 * In most messages (for protocol 10+) the original nick will be
 * replaced by a 3 character string: YXX
 * Where 'Y' represents the server, and 'XX' the nick on that server.
 *
 * 'YXX' should not interfer with the input parser, and therefore is
 * not allowed to contain spaces or a ':'.
 * Also, 'Y' can't start with a '+' because of m_server().
 *
 * We keep the characters printable for debugging reasons too.
 *
 * The 'XX' value can be larger then the maximum number of clients
 * per server, we use a mask (struct Server::nn_mask) to get the real
 * client numeric. The overhead is used to have some redundancy so
 * just-disconnected-client aren't confused with just-connected ones.
 */

/* These must be the same on ALL servers ! Do not change ! */

#define NUMNICKLOG 6
#define NUMNICKMAXCHAR 'z'      /* See convert2n[] */
#define NUMNICKBASE 64          /* (2 << NUMNICKLOG) */
#define NUMNICKMASK 63          /* (NUMNICKBASE-1) */
#define NN_MAX_SERVER 4096      /* (NUMNICKBASE * NUMNICKBASE) */
#define NN_MAX_CLIENT 262144    /* NUMNICKBASE ^ 3 */

/* *INDENT-OFF* */

/*
 * convert2y[] converts a numeric to the corresponding character.
 * The following characters are currently known to be forbidden:
 *
 * '\0' : Because we use '\0' as end of line.
 *
 * ' '  : Because parse_*() uses this as parameter seperator.
 * ':'  : Because parse_server() uses this to detect if a prefix is a
 *        numeric or a name.
 * '+'  : Because m_nick() uses this to determine if parv[6] is a
 *        umode or not.
 * '&', '#', '+', '$', '@' and '%' :
 *        Because m_message() matches these characters to detect special cases.
 */
static const char convert2y[] = {
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
  'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
  'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
  'w','x','y','z','0','1','2','3','4','5','6','7','8','9','[',']'
};

static const unsigned int convert2n[] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  52,53,54,55,56,57,58,59,60,61, 0, 0, 0, 0, 0, 0, 
   0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,62, 0,63, 0, 0,
   0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
  41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0,

   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* *INDENT-ON* */


unsigned int base64toint(const char* s)
{
	unsigned int i = convert2n[(unsigned char) *s++];
	while (*s) {
		i <<= NUMNICKLOG;
		i += convert2n[(unsigned char) *s++];
	}
	return i;
}

const char* inttobase64(char* buf, unsigned int v, unsigned int count)
{
	buf[count] = '\0';  
	while (count > 0) {
		buf[--count] = convert2y[(v & NUMNICKMASK)];
		v >>= NUMNICKLOG;
	}
	return buf;
}

void
send_server (const char *sender, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", sender, MSG_SERVER, name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass)
{
	send_cmd ("%s %s :TS", MSG_PASS, pass);
	send_cmd ("CAPAB :TS EX CHW IE KLN GLN KNOCK HOPS HUB AOPS MX");
	send_cmd ("%s %s %d :%s", MSG_SERVER, name, numeric, infoline);
}

void
send_squit (const char *server, const char *quitmsg)
{
	send_cmd ("%s %s :%s", MSG_SQUIT, server, quitmsg);
}

void 
send_quit (const char *who, const char *quitmsg)
{
	send_cmd (":%s %s :%s", who, MSG_QUIT, quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	send_cmd (":%s %s %s", who, MSG_PART, chan);
}

void
send_join (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s 0 %s + :%s", sender, MSG_JOIN, chan, who);
}

void 
send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, ts, newmode, ident, host, server, realname);
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	send_cmd (":%s %s %s :%s", from, MSG_PING, reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", who, MSG_MODE, target, mode);
}

void 
send_numeric (const char *from, const int numeric, const char *target, const char *buf)
{
	send_cmd (":%s %d %s :%s", from, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	send_cmd ("%s %s", MSG_PONG, reply);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", from, MSG_KILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, MSG_NICK, newnick, ts);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
}

void 
send_sjoin (const char *sender, const char *who, const char *chan, const char flag, const unsigned long ts)
{
}

void 
send_kick (const char *who, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *who, const char *buf)
{
	send_cmd (":%s %s :%s", who, MSG_WALLOPS, buf);
}

void
send_burst (int b)
{
	if (b == 0) {
		send_cmd ("BURST 0");
	} else {
		send_cmd ("BURST");
	}
}

void 
send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, const unsigned long ts)
{
	/* there isn't an akill on Hybrid, so we send a kline to all servers! */
	hscan_t ss;
	hnode_t *sn;
	Server *s;

	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		//send_cmd (":%s %s %s %lu %s %s :%s", setby, MSG_KLINE, s->name, (unsigned long)length, ident, host, reason);
	}
}

void 
send_rakill (const char *sender, const char *host, const char *ident)
{
	chanalert (s_Services, "Please Manually remove KLINES using /unkline on each server");
}

void
send_privmsg (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, MSG_PRIVATE, to, buf);
}

void
send_notice (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, MSG_NOTICE, to, buf);
}

void
send_globops (const char *from, const char *buf)
{
//	send_cmd (":%s %s :%s", from, MSG_GLOBOPS, buf);
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

/* m_server
 *
 * argv[0] = sender prefix
 * argv[1] = servername
 * argv[2] = hopcount
 * argv[3] = start timestamp
 * argv[4] = link timestamp
 * argv[5] = major protocol version: P10/P11
 * argv[6] = YMM, YMMM or YYMMM; where 'YY' is the server numeric and
 *      "MMM" is the numeric nick mask of this server.
 * argv[7] = 0 (not used yet, mandatory unsigned int after u2.10.06)
 * argv[argc-1] = serverinfo
 * NumServ(sptr) SERVER name hop 0 TSL PROT YxxCap 0 :info
 */

static void
m_server (char *origin, char **argv, int argc, int srv)
{
	do_server (argv[0], origin, argv[1], NULL, argv[argc-1], srv);
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
