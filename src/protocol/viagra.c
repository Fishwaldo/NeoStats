/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "viagra.h"
#include "neostats.h"
#include "ircd.h"

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

/* buffer sizes */
const int proto_maxhost		= (128 + 1);
const int proto_maxpass		= (63 + 1);
const int proto_maxnick		= (30 + 1);
const int proto_maxuser		= (10 + 1);
const int proto_maxrealname	= (50 + 1);
const int proto_chanlen		= (32 + 1);
const int proto_topiclen	= (307 + 1);

ProtocolInfo protocol_info = {
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	FEATURE_UMODECLOAK,
	"+oS",
	"+a",
};

ircd_cmd cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_PRIVATE, 0, m_private, 0},
	{MSG_NOTICE, 0, m_notice, 0},
	{MSG_STATS, 0, m_stats, 0},
	{MSG_VERSION, 0, m_version, 0},
	{MSG_MOTD, 0, m_motd, 0},
	{MSG_ADMIN, 0, m_admin, 0},
	{MSG_SERVER, 0, m_server, 0},
	{MSG_SQUIT, 0, m_squit, 0},
	{MSG_QUIT, 0, m_quit, 0},
	{MSG_MODE, 0, m_mode, 0},
	{MSG_SVSMODE, 0, m_svsmode, 0},
	{MSG_KILL, 0, m_kill, 0},
	{MSG_PONG, 0, m_pong, 0},
	{MSG_AWAY, 0, m_away, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, m_topic, 0},
	{MSG_KICK, 0, m_kick, 0},
	{MSG_JOIN, 0, m_join, 0},
	{MSG_PART, 0, m_part, 0},
	{MSG_PING, 0, m_ping, 0},
	{MSG_SVINFO, 0, m_svinfo, 0},
	{MSG_CAPAB, 0, m_protoctl, 0},
	{MSG_BURST, 0, m_burst, 0},
	{MSG_SJOIN, 0, m_sjoin, 0},
	{MSG_PASS, 0, m_pass, 0},
	{MSG_SVSNICK, 0, m_svsnick, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'o', CUMODE_CHANOP, 0, '@'},
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANADMIN, 0, '*'},
	{'q', CUMODE_CHANOWNER, 0, '!'},
	{'v', CUMODE_VOICE, 0, '+'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'b', CMODE_BAN, MODEPARAM},
	{'i', CMODE_INVITEONLY, 0},
	{'k', CMODE_KEY, MODEPARAM},
	{'l', CMODE_LIMIT, MODEPARAM},
	{'m', CMODE_MODERATED, 0},
	{'n', CMODE_NOPRIVMSGS, 0},
	{'p', CMODE_PRIVATE, 0},
	{'r', CMODE_RGSTR, 0},
	{'s', CMODE_SECRET, 0},
	{'t', CMODE_TOPICLIMIT, 0},
	{'x', CMODE_NOCOLOR, 0},
	{'O', CMODE_OPERONLY, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = {
	{'a', UMODE_SADMIN},
	{'A', UMODE_ADMIN},
	{'i', UMODE_ALTADMIN},
	{'o', UMODE_OPER},
	{'r', UMODE_REGNICK},
	{'i', UMODE_INVISIBLE},
	{'R', UMODE_RGSTRONLY},
	{'x', UMODE_HIDE},
	{'w', UMODE_WALLOP},
	{'c', UMODE_CLIENT},
	{'h', UMODE_HELPOP},
	{'e', UMODE_FCLIENT},
	{'W', UMODE_WHOIS},
	{0, 0},
};

void
send_server (const char *source, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", source, MSG_SERVER, name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink)
{
	send_cmd ("%s %s :TS", MSG_PASS, pass);
	send_cmd ("CAPAB TS3 SSJOIN BURST NICKIP");
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
send_sjoin (const char *source, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, who);
}

void 
send_join (const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %s", who, MSG_JOIN, chan);
}

void 
send_cmode (const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", source, MSG_MODE, chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, ts, newmode, ident, host, server, ts, realname);
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
send_svskill (const char *source, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSG_SVSKILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long  ts)
{
	send_cmd (":%s %s %s %lu", oldnick, MSG_NICK, newnick, ts);
}

void 
send_svsnick (const char *source, const char *target, const char *newnick, const unsigned long  ts)
{
	send_cmd ("%s %s %s :%lu", MSG_SVSNICK, target, newnick, ts);
}

void
send_svsjoin (const char *source, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", MSG_SVSJOIN, target, chan);
}

void
send_svspart (const char *source, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", MSG_SVSPART, target, chan);
}
void
send_svshost (const char *source, const char *who, const char *vhost)
{
	send_cmd (":%s %s %s %s", source, MSG_CHGHOST, who, vhost);
}
void
send_svsmode (const char *source, const char *target, const char *modes)
{
	send_cmd (":%s %s %s %s", source, MSG_SVSMODE, target, modes);
}
void
send_kick (const char *source, const char *target, const char *chan, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", source, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *who, const char *buf)
{
	send_cmd (":%s %s :%s", who, MSG_WALLOPS, buf);
}
void
send_akill (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %lu %s %lu :%s", source, MSG_AKILL, host, ident, length, setby, ts, reason);
}
void
send_invite (const char *from, const char *to, const char *chan) 
{
	send_cmd (":%s %s %s %s", from, MSG_INVITE, to, chan);
}
void
send_rakill (const char *source, const char *host, const char *ident)
{
	send_cmd (":%s %s %s %s", source, MSG_RAKILL, host, ident);
}


void
send_svinfo (const int tscurrent, const int tsmin, const unsigned long tsnow)
{
	send_cmd ("%s %d %d 0 :%lu", MSG_SVINFO, tscurrent, tsmin, tsnow);
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
	send_cmd (":%s %s :%s", from, MSG_GLOBOPS, buf);
}

static void
m_sjoin (char *origin, char **argv, int argc, int srv)
{
	do_sjoin (argv[0], argv[1], ((argc <= 2) ? argv[1] : argv[2]), origin, argv, argc);
}

static void
m_burst (char *origin, char **argv, int argc, int srv)
{
	do_burst (origin, argv, argc);
}

static void
m_protoctl (char *origin, char **argv, int argc, int srv)
{
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
	if(argc > 2) {
		do_server (argv[0], origin, argv[1], argv[2], NULL, srv);
	} else {
		do_server (argv[0], origin, NULL, argv[1], NULL, srv);
	}
}

static void
m_squit (char *origin, char **argv, int argc, int srv)
{
	do_squit (argv[0], argv[1]);
}

static void
m_quit (char *origin, char **argv, int argc, int srv)
{
	do_quit (origin, argv[0]);
}

static void
m_svsmode (char *origin, char **argv, int argc, int srv)
{
	if (argv[0][0] == '#') {
		do_svsmode_channel (origin, argv, argc);
	} else {
		do_svsmode_user (argv[0], argv[2], NULL);
	}
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
	do_kill (origin, [0], argv[1]);
}
static void
m_pong (char *origin, char **argv, int argc, int srv)
{
	do_pong (argv[0], argv[1]);
}
static void
m_away (char *origin, char **argv, int argc, int srv)
{
	do_away (origin, (argc > 0) ? argv[0] : NULL);
}
static void
m_nick (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
		do_nick (argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			argv[8], NULL, argv[3], NULL, argv[9], NULL, NULL);
	} else {
		do_nickchange (origin, argv[0], argv[1]);
	}
}

static void
m_topic (char *origin, char **argv, int argc, int srv)
{
	do_topic (argv[0], argv[1], argv[2], argv[3]);
}
static void
m_kick (char *origin, char **argv, int argc, int srv)
{
	do_kick (origin, argv[0], argv[1], argv[2]);
}
static void
m_join (char *origin, char **argv, int argc, int srv)
{
	do_join (origin, argv[0], NULL);
}
static void
m_part (char *origin, char **argv, int argc, int srv)
{
	do_part (origin, argv[0], argv[1]);
}

static void
m_ping (char *origin, char **argv, int argc, int srv)
{
	do_ping (argv[0], argv[1]);
}

static void
m_svinfo (char *origin, char **argv, int argc, int srv)
{
	do_svinfo ();
}

static void
m_pass (char *origin, char **argv, int argc, int srv)
{
}

static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (argv[0], argv[1], NULL);
}
