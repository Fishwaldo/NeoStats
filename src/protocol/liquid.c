/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "liquid.h"
#include "neostats.h"
#include "ircd.h"

static void m_server (char *origin, char **argv, int argc, int srv);
static void m_mode (char *origin, char **argv, int argc, int srv);
static void m_svsmode (char *origin, char **argv, int argc, int srv);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_vhost (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_svinfo (char *origin, char **argv, int argc, int srv);
static void m_burst (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);

/* buffer sizes */
const int proto_maxhost		= (128 + 1);
const int proto_maxpass		= (32 + 1);
const int proto_maxnick		= (32 + 1);
const int proto_maxuser		= (10 + 1);
const int proto_maxrealname	= (50 + 1);
const int proto_chanlen		= (32 + 1);
const int proto_topiclen	= (307 + 1);

ProtocolInfo protocol_info = {
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN | PROTOCOL_NICKIP,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	"+oS",
	"+o",
};

ircd_cmd cmd_list[] = {
	/* Command Token Function usage */
	{MSG_PRIVATE, 0, _m_private, 0},
	{MSG_NOTICE, 0, _m_notice, 0},
	{MSG_STATS, 0, _m_stats, 0},
	{MSG_SETHOST, 0, m_vhost, 0},
	{MSG_VERSION, 0, _m_version, 0},
	{MSG_MOTD, 0, _m_motd, 0},
	{MSG_ADMIN, 0, _m_admin, 0},
	{MSG_SERVER, 0, m_server, 0},
	{MSG_SQUIT, 0, _m_squit, 0},
	{MSG_QUIT, 0, _m_quit, 0},
	{MSG_MODE, 0, m_mode, 0},
	{MSG_SVSMODE, 0, m_svsmode, 0},
	{MSG_KILL, 0, _m_kill, 0},
	{MSG_PONG, 0, _m_pong, 0},
	{MSG_AWAY, 0, _m_away, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, _m_topic, 0},
	{MSG_KICK, 0, _m_kick, 0},
	{MSG_JOIN, 0, _m_join, 0},
	{MSG_PART, 0, _m_part, 0},
	{MSG_PING, 0, _m_ping, 0},
	{MSG_SVINFO, 0, m_svinfo, 0},
	{MSG_CAPAB, 0, _m_capab, 0},
	{MSG_BURST, 0, m_burst, 0},
	{MSG_SJOIN, 0, m_sjoin, 0},
	{MSG_PASS, 0, _m_pass, 0},
	{MSG_SVSNICK, 0, m_svsnick, 0},
	{MSG_GLOBOPS,	0, _m_globops, 0},
	{MSG_WALLOPS,	0, _m_wallops, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'q', CUMODE_CHANOWNER, 0, '!'},
	{'a', CUMODE_CHANPROT, 0, '*'},
	{'o', CUMODE_CHANOP, 0, '@'},
	{'h', CUMODE_HALFOP, 0, '%'},
	{'v', CUMODE_VOICE, 0, '+'},
	{'u', CMODE_UOP, 0, '-'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'b', CMODE_BAN, MODEPARAM},
	{'c', CMODE_NOCOLOR, 0},
	{'i', CMODE_INVITEONLY, 0},
	{'k', CMODE_KEY, MODEPARAM},
	{'l', CMODE_LIMIT, MODEPARAM},
	{'L', CMODE_LINK, MODEPARAM},
	{'m', CMODE_MODERATED, 0},
	{'n', CMODE_NOPRIVMSGS, 0},
	{'N', CMODE_NONICKCHANGE, 0},
	{'p', CMODE_PRIVATE, 0},
	{'r', CMODE_RGSTR, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'s', CMODE_SECRET, 0},
	{'S', CMODE_STRIP, 0},
	{'t', CMODE_TOPICLIMIT, 0},
	{'O', CMODE_OPERONLY, 0},
	{'M', CMODE_MODREG, 0},
	{'N', CMODE_AUDITORIUM, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = {
	{'a', UMODE_SADMIN},
	{'T', UMODE_TECHADMIN},
	{'N', UMODE_NETADMIN},
	{'A', UMODE_ADMIN},
	{'o', UMODE_OPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'i', UMODE_INVISIBLE},
	{'w', UMODE_WALLOP},
	{'g', UMODE_GLOBOPS},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'k', UMODE_KILLS},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FLOOD},
	{'R', UMODE_RGSTRONLY},
	{'q', UMODE_KIX},
	{'B', UMODE_BOT},
	{'z', UMODE_HIDE},
	{'W', UMODE_WHOIS},
	{'y', UMODE_SPY},
	{'d', UMODE_DEBUG},
	{'b', UMODE_CHATOPS},
	{'n', UMODE_ROUTE},
	{'m', UMODE_SPAM},
	{'e', UMODE_OPERNOTICE},
	{'x', UMODE_SQUELCH},
	{'D', UMODE_HIDDENDCC},
	{'F', UMODE_THROTTLE},
	{'j', UMODE_REJ},
	{'K', UMODE_ULINEKILL},
	{'C', UMODE_CODER},
	{0, 0},
};

mode_init user_smodes[] = {
	{'s', SMODE_SSL},
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
send_quit (const char *source, const char *quitmsg)
{
	send_cmd (":%s %s :%s", source, MSG_QUIT, quitmsg);
}

void 
send_part (const char *source, const char *chan, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSG_PART, chan, reason);
}

void 
send_sjoin (const char *source, const char *target, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, target);
}

void 
send_join (const char *source, const char *chan, const char *key, const unsigned long ts)
{
	send_cmd (":%s %s %s", source, MSG_JOIN, chan);
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
send_ping (const char *source, const char *reply, const char *target)
{
	send_cmd (":%s %s %s :%s", source, MSG_PING, reply, target);
}

void 
send_umode (const char *source, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", source, MSG_MODE, target, mode);
}

void 
send_numeric (const char *source, const int numeric, const char *target, const char *buf)
{
	send_cmd (":%s %d %s :%s", source, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	send_cmd ("%s %s", MSG_PONG, reply);
}

void 
send_kill (const char *source, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSG_KILL, target, reason);
}

void 
send_svskill (const char *source, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSG_SVSKILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, MSG_NICK, newnick, ts);
}

void 
send_svsnick (const char *source, const char *target, const char *newnick, const unsigned long ts)
{
	send_cmd ("%s %s %s :%lu", MSG_SVSNICK, target, newnick, ts);
}

void 
send_kick (const char *source, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", source, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *source, const char *buf)
{
	send_cmd (":%s %s :%s", source, MSG_WALLOPS, buf);
}

void
send_svshost (const char *source, const char *target, const char *vhost)
{
	send_cmd (":%s SVSCHGHOST %s %s", source, target, vhost);
}

void
send_invite (const char *source, const char *target, const char *chan) 
{
	send_cmd (":%s %s %s %s", source, MSG_INVITE, target, chan);
}

void 
send_akill (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts)
{
#if 0
	send_cmd (":%s %s %s %s %lu %s %lu :%s", source, MSG_AKILL, host, ident, length, setby, ts, reason);
#else
	send_cmd (":%s %s add %s@%s %lu %d %s :%s", me.name, MSG_GLINE, ident, host, length, (int)me.now, setby, reason); 
#endif
}

void 
send_rakill (const char *source, const char *host, const char *ident)
{
#if 0
	send_cmd (":%s %s %s %s", source, MSG_RAKILL, host, ident);
#else
	send_cmd (":%s %s del %s@%s", me.name, MSG_GLINE, ident, host); 
#endif
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
send_privmsg (const char *source, const char *target, const char *buf)
{
	send_cmd (":%s %s %s :%s", source, MSG_PRIVATE, target, buf);
}

void
send_notice (const char *source, const char *target, const char *buf)
{
	send_cmd (":%s %s %s :%s", source, MSG_NOTICE, target, buf);
}

void
send_globops (const char *source, const char *buf)
{
	send_cmd (":%s %s :%s", source, MSG_GLOBOPS, buf);
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
m_server (char *origin, char **argv, int argc, int srv)
{
	if(argc > 2) {
		do_server (argv[0], origin, argv[1], argv[2], NULL, srv);
	} else {
		do_server (argv[0], origin, NULL, argv[1], NULL, srv);
	}
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
m_vhost (char *origin, char **argv, int argc, int srv)
{
	do_vhost (origin, argv[0]);
}

static void
m_nick (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
		do_nick (argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			argv[8], NULL, argv[3], NULL, argv[9], NULL, NULL);
	} else {
		do_nickchange (origin, argv[0], NULL);
	}
}

static void
m_svinfo (char *origin, char **argv, int argc, int srv)
{
	do_svinfo ();
}

static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (argv[0], argv[1], NULL);
}

