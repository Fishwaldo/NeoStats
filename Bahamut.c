/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "Bahamut.h"
#include "dl.h"
#include "log.h"
#include "server.h"
#include "chans.h"
#include "users.h"

static void m_version (char *origin, char **argv, int argc, int srv);
static void m_motd (char *origin, char **argv, int argc, int srv);
static void m_admin (char *origin, char **argv, int argc, int srv);
static void m_server (char *origin, char **argv, int argc, int srv);
static void m_squit (char *origin, char **argv, int argc, int srv);
static void m_quit (char *origin, char **argv, int argc, int srv);
static void m_mode (char *origin, char **argv, int argc, int srv);
static void m_svsmode (char *origin, char **argv, int argc, int srv);
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
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_protoctl (char *origin, char **argv, int argc, int srv);
static void m_svinfo (char *origin, char **argv, int argc, int srv);
static void m_burst (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);

const char ircd_version[] = "(B)";
const char services_bot_modes[]= "+oS";

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
	{MSG_SVSMODE, m_svsmode, 0},
	{MSG_KILL, m_kill, 0},
	{MSG_PONG, m_pong, 0},
	{MSG_AWAY, m_away, 0},
	{MSG_NICK, m_nick, 0},
	{MSG_TOPIC, m_topic, 0},
	{MSG_KICK, m_kick, 0},
	{MSG_JOIN, m_join, 0},
	{MSG_PART, m_part, 0},
	{MSG_PING, m_ping, 0},
	{MSG_SVINFO, m_svinfo, 0},
	{MSG_CAPAB, m_protoctl, 0},
	{MSG_BURST, m_burst, 0},
	{MSG_SJOIN, m_sjoin, 0},
	{MSG_PASS, m_pass, 0},
	{MSG_SVSNICK, m_svsnick, 0},
};

ChanModes chan_modes[] = {
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_VOICE, 'v', 1, 0, '+'},
 /* CMODE_DEOPPED */
	{CMODE_PRIVATE, 'p', 0, 0, 0},
	{CMODE_SECRET, 's', 0, 0, 0},
	{CMODE_MODERATED, 'm', 0, 0, 0},
	{CMODE_TOPICLIMIT, 't', 0, 0, 0},
	{CMODE_INVITEONLY, 'i', 0, 0, 0},
	{CMODE_NOPRIVMSGS, 'n', 0, 0, 0},
	{CMODE_KEY, 'k', 0, 1, 0},
	{CMODE_BAN, 'b', 0, 1, 0},
	{CMODE_LIMIT, 'l', 0, 1, 0},
	{CMODE_RGSTR, 'M', 0, 0, 0},
	{CMODE_RGSTRONLY, 'R', 0, 0, 0},
	{CMODE_NOCOLOR, 'x', 0, 0, 0},
	{CMODE_OPERONLY, 'O', 0, 0, 0},
/* CMODE_MODREG */
	{CMODE_LISTED, 'L', 0, 0, 0},
};

UserModes user_umodes[] = {
	{UMODE_SADMIN, 'a', NS_ULEVEL_ROOT},
	{UMODE_ADMIN, 'A', NS_ULEVEL_ADMIN},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_LOCOP, 'o', NS_ULEVEL_OPER},
	{UMODE_REGNICK, 'r', NS_ULEVEL_REG},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_WALLOP, 'w', 0},
	{UMODE_SERVNOTICE, 's', 0},
	{UMODE_CLIENT, 'c', 0},
	{UMODE_KILLS, 'k', 0},
	{UMODE_FLOOD, 'f', 0},
	{UMODE_SPY, 'y', 0},
	{UMODE_DEBUG, 'd', 0},
	{UMODE_GLOBOPS, 'g', 0},
	{UMODE_CHATOPS, 'b', 0},
	{UMODE_ROUTE, 'n', 0},
	{UMODE_HELPOP, 'h', 0},
	{UMODE_SPAM, 'm', 0},
	{UMODE_REGONLY, 'R', 0},
	{UMODE_OPERNOTICE, 'e', 0},
	{UMODE_SQUELCH, 'x', 0},
	{UMODE_SQUELCHN, 'X', 0},
	{UMODE_HIDDENDCC, 'D', 0},
	{UMODE_THROTTLE, 'F', 0},
	{UMODE_REJ, 'j', 0},
	{UMODE_ULINEKILL, 'K', 0},
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
	sts ("CAPAB TS3 SSJOIN BURST NICKIP");
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
send_sjoin (const char *who, const char *chan, const char flag, time_t tstime)
{
	sts (":%s %s %d %s + :%c%s", me.name, MSG_SJOIN, (int)tstime, chan, flag, who);
}

void 
send_cmode (const char *who, const char *chan, const char *mode, const char *args)
{
	sts (":%s %s %s %s %s %lu", me.name, MSG_MODE, chan, mode, args, me.now);
}

void
send_nick (const char *nick, const char *ident, const char *host, const char *realname, const char* newmode, time_t tstime)
{
	sts ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, tstime, newmode, ident, host, me.name, me.now, realname);
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
send_numeric (const int numeric, const char *target, const char *buf)
{
	sts (":%s %d %s :%s", me.name, numeric, target, buf);
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
send_svskill (const char *target, const char *reason)
{
	sts (":%s %s %s :%s", me.name, MSG_SVSKILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick)
{
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, (int)me.now);
}

void 
send_svsnick (const char *target, const char *newnick)
{
	sts ("%s %s %s :%d", MSG_SVSNICK, target, newnick, (int)me.now);
}

void 
send_kick (const char *who, const char *target, const char *chan, const char *reason)
{
	sts (":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void send_wallops (char *who, char *buf)
{
	sts (":%s %s :%s", who, MSG_WALLOPS, buf);
}

void 
send_akill (const char *host, const char *ident, const char *setby, const int length, const char *reason)
{
	sts (":%s %s %s %s %d %s %d :%s", me.name, MSG_AKILL, host, ident, length, setby, (int)me.now, reason);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	sts (":%s %s %s %s", from, MSG_INVITE, to, chan);
}

void 
send_rakill (const char *host, const char *ident)
{
	sts (":%s %s %s %s", me.name, MSG_RAKILL, host, ident);
}

void
send_svinfo (void)
{
	sts ("SVINFO 3 3 0 :%d", (int)me.now);
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
send_privmsg (char *to, const char *from, char *buf)
{
	sts (":%s %s %s :%s", from, MSG_PRIVATE, to, buf);
}

void
send_notice (char *to, const char *from, char *buf)
{
	sts (":%s %s %s :%s", from, MSG_NOTICE, to, buf);
}

void
send_globops (char *from, char *buf)
{
	sts (":%s %s :%s", from, MSG_GLOBOPS, buf);
}

static void
m_sjoin (char *origin, char **argv, int argc, int srv)
{
	handle_sjoin (argv[0], argv[1], ((argc > 4) ? argv[2] : argv[1]), 3, origin, argv, argc);
}

static void
m_burst (char *origin, char **argv, int argc, int srv)
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

static void
m_protoctl (char *origin, char **argv, int argc, int srv)
{
}

static void
m_stats (char *origin, char **argv, int argc, int srv)
{
	ns_usr_stats (origin, argv, argc);
}

static void
m_version (char *origin, char **argv, int argc, int srv)
{
	ns_usr_version (origin, argv, argc);
}

static void
m_motd (char *origin, char **argv, int argc, int srv)
{
	ns_usr_motd (origin, argv, argc);
}

static void
m_admin (char *origin, char **argv, int argc, int srv)
{
	ns_usr_admin (origin, argv, argc);
}

static void
m_server (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
		if (*origin == 0) {
			me.s = AddServer (argv[0], me.name, argv[1], NULL);
		} else {
			me.s = AddServer (argv[0], origin, argv[1], NULL);
		}
	} else {
		AddServer (argv[0], origin, argv[1], NULL);
	}
}

static void
m_squit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	SquitServer (argv[0], tmpbuf);
	free(tmpbuf);
}

static void
m_quit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 0);
	UserQuit (origin, tmpbuf);
	free(tmpbuf);
}

static void
m_svsmode (char *origin, char **argv, int argc, int srv)
{
	if (argv[0][0] == '#') {
		ChanMode (origin, argv, argc);
	} else {
		UserMode (argv[0], argv[2]);
	}
}
static void
m_mode (char *origin, char **argv, int argc, int srv)
{
	if (argv[0][0] == '#') {
		ChanMode (origin, argv, argc);
	} else {
		UserMode (argv[0], argv[1]);
	}
}
static void
m_kill (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	KillUser (argv[0], tmpbuf);
	free(tmpbuf);
}
static void
m_pong (char *origin, char **argv, int argc, int srv)
{
	ns_usr_pong (origin, argv, argc);
}
static void
m_away (char *origin, char **argv, int argc, int srv)
{
	char *buf;

	if (argc > 0) {
		buf = joinbuf (argv, argc, 0);
		UserAway (origin, buf);
		free (buf);
	} else {
		UserAway (origin, NULL);
	}
}
static void
m_nick (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
		char *realname;

		realname = joinbuf (argv, argc, 9);
		AddUser (argv[0], argv[4], argv[5], realname, argv[6], argv[8], argv[2]);
		free (realname);
		UserMode (argv[0], argv[3]);
	} else {
		UserNick (origin, argv[0], NULL);
	}
}

static void
m_topic (char *origin, char **argv, int argc, int srv)
{
	char *buf;

	buf = joinbuf (argv, argc, 3);
	ChanTopic (argv[1], argv[0], argv[2], buf);
	free (buf);
}

static void
m_kick (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 2);
	kick_chan(argv[0], argv[1], origin, tmpbuf);
	free(tmpbuf);
}
static void
m_join (char *origin, char **argv, int argc, int srv)
{
	UserJoin (origin, argv[0]);
}
static void
m_part (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	part_chan (finduser (origin), argv[0], tmpbuf);
	free(tmpbuf);
}

static void
m_ping (char *origin, char **argv, int argc, int srv)
{
	send_pong (argv[0]);
	if (ircd_srv.burst) {
		send_ping (me.name, argv[0], argv[0]);
	}
}

static void
m_svinfo (char *origin, char **argv, int argc, int srv)
{
	send_svinfo ();
}


static void
m_pass (char *origin, char **argv, int argc, int srv)
{
}

static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	UserNick (argv[0], argv[1], NULL);
}

