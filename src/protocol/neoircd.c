/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neoircd.h"
#include "neostats.h"
#include "ircd.h"

static void m_server (char *origin, char **argv, int argc, int srv);
static void m_mode (char *origin, char **argv, int argc, int srv);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_svinfo (char *origin, char **argv, int argc, int srv);
static void m_burst (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);
static void m_tburst (char *origin, char **argv, int argc, int srv);

/* buffer sizes */
const int proto_maxhost		= (128 + 1);
const int proto_maxpass		= (32 + 1);
const int proto_maxnick		= (32 + 1);
const int proto_maxuser		= (15 + 1);
const int proto_maxrealname	= (50 + 1);
const int proto_chanlen		= (50 + 1);
const int proto_topiclen	= (512 + 1);

ProtocolInfo protocol_info = {
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	"+oS",
	"+a",
};

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = {
	/* Command Token Function usage */
	{MSG_PRIVATE, 0, _m_private, 0},
	{MSG_NOTICE, 0, _m_notice, 0},
	{MSG_STATS, 0, _m_stats, 0},
	{MSG_VERSION, 0, _m_version, 0},
	{MSG_MOTD, 0, _m_motd, 0},
	{MSG_ADMIN, 0, _m_admin, 0},
	{MSG_CREDITS, 0, _m_credits, 0},
	{MSG_SERVER, 0, m_server, 0},
	{MSG_SQUIT, 0, _m_squit, 0},
	{MSG_QUIT, 0, _m_quit, 0},
	{MSG_MODE, 0, m_mode, 0},
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
	{MSG_PASS, 0, _m_pass, 0},
	{MSG_EOB, 0, m_burst, 0},
	{MSG_SJOIN, 0, m_sjoin, 0},
	{MSG_TBURST, 0, m_tburst, 0},
	{MSG_WALLOPS,	0, _m_wallops, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'h', CUMODE_HALFOP, 0, '%'},
	{'o', CUMODE_CHANOP, 0, '@'},
	{'v', CUMODE_VOICE, 0, '+'},
	{'a', CUMODE_CHANADMIN, 0, '!'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'s', CMODE_SECRET, 0},
	{'p', CMODE_PRIVATE, 0},
	{'m', CMODE_MODERATED, 0},
	{'t', CMODE_TOPICLIMIT, 0},
	{'i', CMODE_INVITEONLY, 0},
	{'n', CMODE_NOPRIVMSGS, 0},
	{'A', CMODE_HIDEOPS, 0},
	{'l', CMODE_LIMIT, MODEPARAM},
	{'k', CMODE_KEY, MODEPARAM},
	{'b', CMODE_BAN, MODEPARAM},
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'I', CMODE_INVEX, MODEPARAM},
	{'r', CMODE_RGSTR, 0},
	{'O', CMODE_OPERONLY, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = {
	{'S', UMODE_SERVICES},
	{'d', UMODE_DEBUG},
	{'A', UMODE_ADMIN},
	{'o', UMODE_OPER},
	{'l', UMODE_LOCOP},
	{'b', UMODE_BOTS},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FULL},
	{'g', UMODE_CALLERID},
	{'i', UMODE_INVISIBLE},
	{'k', UMODE_SKILL},
	{'n', UMODE_NCHANGE},
	{'R', UMODE_REJ},
	{'s', UMODE_SERVNOTICE},
	{'u', UMODE_UNAUTH},
	{'w', UMODE_WALLOP},
	{'x', UMODE_EXTERNAL},
	{'y', UMODE_SPY},
	{'z', UMODE_OPERWALL},
	{0, 0},
};

void
send_eob (const char *server)
{
	send_cmd (":%s %s", server, MSG_EOB);
}

void
send_server (const char *source, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s 2 0 :%s", source, MSG_SERVER, name, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink)
{
	send_cmd ("%s %s :TS", MSG_PASS, pass);
	send_cmd ("CAPAB :TS EOB HUB PARA");
	send_cmd ("%s %s 0 0 :%s", MSG_SERVER, name, infoline);
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
send_cmode (const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s * %s 0 :%s", MSG_NICK, nick, ts, newmode, ident, host, server, realname);
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
send_snetinfo (const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSG_SNETINFO, ts, prot, cloak, netname);
}

void
send_netinfo (const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSG_NETINFO, ts, prot, cloak, netname);
}

void 
send_kill (const char *source, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSG_KILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, MSG_NICK, newnick, ts);
}

void
send_swhois (const char *source, const char *target, const char *swhois)
{
	send_cmd (":%s SWHOIS %s :%s", source, target, swhois);
}

void 
send_svsnick (const char *source, const char *target, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %s :%lu", source, MSG_SVSNICK, target, newnick, ts);
}

void
send_svsjoin (const char *source, const char *target, const char *chan)
{
	send_cmd (":%s SVSJOIN %s %s", source, target, chan);
}

void
send_svspart (const char *source, const char *target, const char *chan)
{
	send_cmd (":%s SVSPART %s %s", source, target, chan);
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
	send_cmd (":%s SVSHOST %s :%s", source, target, vhost);
}

void
send_invite (const char *source, const char *target, const char *chan) 
{
	send_cmd (":%s %s %s %s", source, MSG_INVITE, target, chan);
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
send_akill (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s GLINE %s %s %lu :%s", source, ident, host, (ts + length), reason);
}

void 
send_rakill (const char *source, const char *host, const char *ident)
{
	send_cmd (":%s UNGLINE %s@%s", source, ident, host);
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
	send_cmd (":%s %s :%s", source, MSG_WALLOPS, buf);
}

static void
m_sjoin (char *origin, char **argv, int argc, int srv)
{
	do_sjoin (argv[0], argv[1], ((argc <= 2) ? argv[1] : argv[2]), argv[4], argv, argc);
}

static void
m_burst (char *origin, char **argv, int argc, int srv)
{
	do_burst (origin, argv, argc);
	send_eob (me.name);
}

static void
m_server (char *origin, char **argv, int argc, int srv)
{
	do_server (argv[0], origin, argv[1], NULL, argv[2], srv);
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
m_nick (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
		do_nick (argv[0], argv[1], argv[2], argv[4], argv[5], argv[7], 
			NULL, NULL, argv[3], argv[6], argv[9], NULL, NULL);
	} else {
		do_nickchange (origin, argv[0], NULL);
	}
}

static void
m_svinfo (char *origin, char **argv, int argc, int srv)
{
	do_svinfo ();
}

/* Topic Bursting for NeoIRCD */
/* R: :fish.dynam.ac TBURST 1034639893 #ircop 1034652780 ChanServ!services@neostats.net :NeoIRCd Test Oper Channel */

static void
m_tburst (char *origin, char **argv, int argc, int srv)
{
	do_topic (argv[1], argv[3], argv[2], argv[4]);
}

