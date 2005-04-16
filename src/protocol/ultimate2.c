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

#include "neostats.h"
#include "ultimate2.h"
#include "ircd.h"
#include "services.h"

static void m_server (char *origin, char **argv, int argc, int srv);
static void m_svsmode (char *origin, char **argv, int argc, int srv);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_vhost (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_snetinfo (char *origin, char **argv, int argc, int srv);
static void m_vctrl (char *origin, char **argv, int argc, int srv);

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
	PROTOCOL_TOKEN,
	/* Features supported by this IRCd */
	0,
	"+oS",
	"+a",
};

ircd_cmd cmd_list[] = {
	/* Command Token Function usage */
	{MSG_PRIVATE,	0,		_m_private,	0},
	{MSG_NOTICE,	0,		_m_notice,	0},
	{MSG_STATS,     0,		_m_stats,     0},
	{MSG_SETHOST,   0,		m_vhost,     0},
	{MSG_VERSION,   0,		_m_version,   0},
	{MSG_MOTD,      0,		_m_motd,      0},
	{MSG_ADMIN,     0,		_m_admin,     0},
	{MSG_CREDITS,   0,		_m_credits,   0},
	{MSG_SERVER,    0,		m_server,	0},
	{MSG_SQUIT,     0,		_m_squit,		0},
	{MSG_QUIT,      0,		_m_quit,		0},
	{MSG_MODE,      TOK_MODE,      _m_mode,		0},
	{MSG_SVSMODE,   0,   m_svsmode,   0},
	{MSG_KILL,      0,      _m_kill,      0},
	{MSG_PONG,      0,      _m_pong,      0},
	{MSG_AWAY,      TOK_AWAY,      _m_away,      0},
	{MSG_NICK,      TOK_NICK,      m_nick,      0},
	{MSG_TOPIC,     TOK_TOPIC,     _m_topic,     0},
	{MSG_KICK,      0,      _m_kick,      0},
	{MSG_JOIN,      TOK_JOIN,      _m_join,      0},
	{MSG_PART,      0,      _m_part,      0},
	{MSG_PING,      0,      _m_ping,      0},
	{MSG_SNETINFO,  0,  m_snetinfo,   0},
	{MSG_VCTRL,     0,     m_vctrl,     0},
	{MSG_PASS,      0,      _m_pass,      0},
	{MSG_SVSNICK,   0,   m_svsnick,   0},
	{MSG_SVSJOIN,   0,   _m_svsjoin,   0},
	{MSG_SVSPART,   0,   _m_svspart,   0},
	{MSG_PROTOCTL,  0,  _m_protoctl,  0},
	{MSG_GLOBOPS,	0, _m_globops, 0},
	{MSG_WALLOPS,	0, _m_wallops, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{MSG_ERROR,		0, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANADMIN, 0, '!'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
	{'r', CMODE_RGSTR, 0},
	{'x', CMODE_NOCOLOR, 0},
	{'A', CMODE_ADMONLY, 0},
	{'I', CMODE_NOINVITE, 0},
	{'K', CMODE_NOKNOCK, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'O', CMODE_OPERONLY, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'S', CMODE_STRIP, 0},	
	{0, 0, 0},
};

mode_init user_umodes[] = {
	{'S', UMODE_SERVICES},
	{'P', UMODE_SADMIN},
	{'T', UMODE_TECHADMIN},
	{'N', UMODE_NETADMIN},
	{'a', UMODE_SERVICESOPER},
	{'Z', UMODE_IRCADMIN},
	{'z', UMODE_ADMIN},
	{'i', UMODE_ALTADMIN},
	{'p', UMODE_SUPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'w', UMODE_WALLOP},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'k', UMODE_KILLS},
	{'B', UMODE_RBOT},
	{'b', UMODE_SBOT},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FLOOD},
	{'x', UMODE_HIDE},
	{'W', UMODE_WATCHER},
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
	send_cmd ("%s %s", MSG_PASS, pass);
	send_cmd ("%s %s %d :%s", MSG_SERVER, name, numeric, infoline);
	send_cmd ("%s TOKEN CLIENT", MSG_PROTOCTL);
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
send_join (const char *source, const char *chan, const char *key, const unsigned long ts)
{
	send_cmd (":%s %s %s", source, MSG_JOIN, chan);
}

void 
send_cmode (const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", who, MSGTOK(MODE), chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s 0 :%s", MSGTOK(NICK), nick, ts, ident, host, server, realname);
	send_umode (nick, nick, newmode);
}

void
send_ping (const char *source, const char *reply, const char *target)
{
	send_cmd (":%s %s %s :%s", source, MSG_PING, reply, target);
}

void 
send_umode (const char *source, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(MODE), target, mode);
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
send_vctrl (const int uprot, const int nicklen, const int modex, const int gc, const char* netname)
{
	send_cmd ("%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, uprot, nicklen, modex, gc, netname);
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
	send_cmd (":%s %s %s %lu", oldnick, MSGTOK(NICK), newnick, ts);
}

void 
send_svsnick (const char *source, const char *target, const char *newnick, const unsigned long ts)
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
	send_cmd (":%s %s %s %s", source, MSG_CHGHOST, target, vhost);
}

void
send_invite (const char *source, const char *target, const char *chan) 
{
	send_cmd (":%s %s %s %s", source, MSG_INVITE, target, chan);
}

void 
send_akill (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s %s %s@%s %lu %lu %s :%s", source, MSG_GLINE, ident, host, (ts + length), ts, setby, reason);
}

void 
send_rakill (const char *source, const char *host, const char *ident)
{
	/* ultimate2 needs an oper to remove */
	send_cmd (":%s %s :%s@%s", ns_botptr->name, MSG_REMGLINE, host, ident);
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
		do_svsmode_user (argv[0], argv[1], NULL);
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
		do_nick (argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, NULL, NULL, NULL, argv[7], NULL, NULL);
	} else {
		do_nickchange (origin, argv[0], NULL);
	}
}

static void
m_vctrl (char *origin, char **argv, int argc, int srv)
{
	do_vctrl (argv[0], argv[1], argv[2], argv[3], argv[14]);
}

static void
m_snetinfo (char *origin, char **argv, int argc, int srv)
{
	do_snetinfo(argv[0], argv[1], argv[2], argv[3], argv[7]);
}

static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (argv[0], argv[1], NULL);
}
