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

#include "mystic.h"
#include "neostats.h"
#include "ircd.h"
#include "services.h"

static void m_server (char *origin, char **argv, int argc, int srv);
static void m_svsmode (char *origin, char **argv, int argc, int srv);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_vhost (char *origin, char **argv, int argc, int srv);
static void m_snetinfo (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_vctrl (char *origin, char **argv, int argc, int srv);

/* buffer sizes */
const int proto_maxhost		= (75 + 1);
const int proto_maxpass		= (32 + 1);
const int proto_maxnick		= (32 + 1);
const int proto_maxuser		= (10 + 1);
const int proto_maxrealname	= (50 + 1);
const int proto_chanlen		= (32 + 1);
const int proto_topiclen	= (512 + 1);

ProtocolInfo protocol_info = {
	/* Protocol options required by this IRCd */
	0,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN,
	/* Features supported by this IRCd */
	0,
	"+oS",
	"+o",
};

ircd_cmd cmd_list[] = {
	/* Command Token Function usage */
	{MSG_PRIVATE, TOK_PRIVATE, _m_private, 0},
	{MSG_NOTICE, TOK_NOTICE, _m_notice, 0},
	{MSG_STATS, TOK_STATS, _m_stats, 0},
	{MSG_SETHOST, TOK_SETHOST, m_vhost, 0},
	{MSG_VERSION, TOK_VERSION, _m_version, 0},
	{MSG_MOTD, TOK_MOTD, _m_motd, 0},
	{MSG_ADMIN, TOK_ADMIN, _m_admin, 0},
	{MSG_CREDITS, TOK_CREDITS, _m_credits, 0},
	{MSG_SERVER, TOK_SERVER, m_server, 0},
	{MSG_SQUIT, TOK_SQUIT, _m_squit, 0},
	{MSG_QUIT, TOK_QUIT, _m_quit, 0},
	{MSG_MODE, TOK_MODE, _m_mode, 0},
	{MSG_SVSMODE, TOK_SVSMODE, m_svsmode, 0},
	{MSG_KILL, TOK_KILL, _m_kill, 0},
	{MSG_PONG, TOK_PONG, _m_pong, 0},
	{MSG_AWAY, TOK_AWAY, _m_away, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_TOPIC, TOK_TOPIC, _m_topic, 0},
	{MSG_KICK, TOK_KICK, _m_kick, 0},
	{MSG_JOIN, TOK_JOIN, _m_join, 0},
	{MSG_PART, TOK_PART, _m_part, 0},
	{MSG_PING, TOK_PING, _m_ping, 0},
	{MSG_SNETINFO, TOK_SNETINFO, m_snetinfo, 0},
	{MSG_VCTRL, TOK_VCTRL, m_vctrl, 0},
	{MSG_PASS, TOK_PASS, _m_pass, 0},
	{MSG_SVSNICK, TOK_SVSNICK, m_svsnick, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, _m_capab, 0},
	{MSG_GLOBOPS, TOK_GLOBOPS, _m_globops, 0},
	{MSG_WALLOPS, TOK_WALLOPS, _m_wallops, 0},
	{MSG_CHATOPS, TOK_CHATOPS, _m_chatops, 0},
	{MSG_ERROR, TOK_ERROR, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'o', CUMODE_CHANOP, 0, '@'},
	{'h', CUMODE_HALFOP, 0, '%'},
	{'v', CUMODE_VOICE, 0, '+'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'b', CMODE_BAN, MODEPARAM},
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
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
	{'A', CMODE_ADMONLY, 0},
	{'I', CMODE_NOINVITE, 0},
	{'K', CMODE_NOKNOCK, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'O', CMODE_OPERONLY, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'S', CMODE_STRIP, 0},	
	{'U', CMODE_STRIPBADWORDS, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = {
	{'S', UMODE_SERVICES},
	{'P', UMODE_SADMIN},
	{'T', UMODE_TECHADMIN},
	{'N', UMODE_NETADMIN},
	{'A', UMODE_ADMIN}, 
	{'a', UMODE_SERVICESOPER},
	{'Z', UMODE_IRCADMIN},
	{'z', UMODE_COADMIN},
	{'o', UMODE_OPER},
	{'p', UMODE_SUPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'i', UMODE_INVISIBLE},
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
	{'C', UMODE_CHATOP},
	{'G', UMODE_NGLOBAL},
	{'m', UMODE_WHOIS},
	{'n', UMODE_NETINFO},
	{'M', UMODE_MAGICK},
	{'X', UMODE_NETMON},
	{0, 0},
};

void
send_server (const char *source, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", source, MSGTOK(SERVER), name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink)
{
	send_cmd ("%s %s", MSGTOK(PASS), pass);
	send_cmd ("%s %s %d :%s", MSGTOK(SERVER), name, numeric, infoline);
	send_cmd ("%s TOKEN CLIENT", MSGTOK(PROTOCTL));
}

void
send_squit (const char *server, const char *quitmsg)
{
	send_cmd ("%s %s :%s", MSGTOK(SQUIT), server, quitmsg);
}

void 
send_quit (const char *source, const char *quitmsg)
{
	send_cmd (":%s %s :%s", source, MSGTOK(QUIT), quitmsg);
}

void 
send_part (const char *source, const char *chan, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(PART), chan, reason);
}

void 
send_join (const char *source, const char *chan, const char *key, const unsigned long ts)
{
	send_cmd (":%s %s %s", source, MSGTOK(JOIN), chan);
}

void 
send_cmode (const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", source, MSGTOK(MODE), chan, mode, args, ts);
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
	send_cmd (":%s %s %s :%s", source, MSGTOK(PING), reply, target);
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
	send_cmd ("%s %s", MSGTOK(PONG), reply);
}

void
send_snetinfo (const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSGTOK(SNETINFO), ts, prot, cloak, netname);
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
	send_cmd (":%s %s %s :%s", source, MSGTOK(KILL), target, reason);
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
	send_cmd ("%s %s %s :%lu", MSGTOK(SVSNICK), target, newnick, ts);
}

void
send_svsjoin (const char *source, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", MSGTOK(SVSJOIN), target, chan);
}

void
send_svspart (const char *source, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", MSGTOK(SVSPART), target, chan);
}

void 
send_kick (const char *source, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", source, MSGTOK(KICK), chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *source, const char *buf)
{
	send_cmd (":%s %s :%s", source, MSGTOK(WALLOPS), buf);
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
	send_cmd (":%s %s :%s@%s", ns_botptr->name, MSG_REMGLINE, host, ident);
}

void
send_privmsg (const char *source, const char *target, const char *buf)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(PRIVATE), target, buf);
}

void
send_notice (const char *source, const char *target, const char *buf)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(NOTICE), target, buf);
}

void
send_globops (const char *source, const char *buf)
{
	send_cmd (":%s %s :%s", source, MSGTOK(GLOBOPS), buf);
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
