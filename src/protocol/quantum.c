/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "quantum.h"
#include "neostats.h"
#include "ircd.h"

static void m_version (char *origin, char **argv, int argc, int srv);
static void m_motd (char *origin, char **argv, int argc, int srv);
static void m_admin (char *origin, char **argv, int argc, int srv);
static void m_credits (char *origin, char **argv, int argc, int srv);
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
static void m_vhost (char *origin, char **argv, int argc, int srv);
static void m_ping (char *origin, char **argv, int argc, int srv);
static void m_pass (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_protoctl (char *origin, char **argv, int argc, int srv);
static void m_svinfo (char *origin, char **argv, int argc, int srv);
static void m_burst (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);
static void m_vctrl (char *origin, char **argv, int argc, int srv);
static void m_client (char *origin, char **argv, int argc, int srv);
static void m_smode (char *origin, char **argv, int argc, int srv);

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
	PROTOCOL_TOKEN | PROTOCOL_CLIENT,
	/* Features supported by this IRCd */
	FEATURE_SVSHOST \
		| FEATURE_SMODES \
		| FEATURE_NICKIP \
		| FEATURE_SVSPART \
		| FEATURE_SVSNICK \
		| FEATURE_SVSKILL ,
	"+oS",
	"+a",
};

ircd_cmd cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_PRIVATE, TOK_PRIVATE, m_private, 0},
	{MSG_NOTICE, TOK_NOTICE, m_notice, 0},
	{MSG_STATS, TOK_STATS, m_stats, 0},
	{MSG_SETHOST, TOK_SETHOST, m_vhost, 0},
	{MSG_VERSION, TOK_VERSION, m_version, 0},
	{MSG_MOTD, TOK_MOTD, m_motd, 0},
	{MSG_ADMIN, TOK_ADMIN, m_admin, 0},
	{MSG_CREDITS, TOK_CREDITS, m_credits, 0},
	{MSG_SERVER, TOK_SERVER, m_server, 0},
	{MSG_SQUIT, TOK_SQUIT, m_squit, 0},
	{MSG_QUIT, TOK_QUIT, m_quit, 0},
	{MSG_MODE, TOK_MODE, m_mode, 0},
	{MSG_SVSMODE, TOK_SVSMODE, m_svsmode, 0},
	{MSG_KILL, TOK_KILL, m_kill, 0},
	{MSG_PONG, TOK_PONG, m_pong, 0},
	{MSG_AWAY, TOK_AWAY, m_away, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_TOPIC, TOK_TOPIC, m_topic, 0},
	{MSG_KICK, TOK_KICK, m_kick, 0},
	{MSG_JOIN, TOK_JOIN, m_join, 0},
	{MSG_PART, TOK_PART, m_part, 0},
	{MSG_PING, TOK_PING, m_ping, 0},
	{MSG_SVINFO, NULL, m_svinfo, 0},
	{MSG_CAPAB, NULL, m_protoctl, 0},
	{MSG_BURST, NULL, m_burst, 0},
	{MSG_SJOIN, NULL, m_sjoin, 0},
	{MSG_CLIENT, NULL, m_client, 0},
	{MSG_SMODE, NULL, m_smode, 0},
	{MSG_VCTRL, TOK_VCTRL, m_vctrl, 0},
	{MSG_PASS, TOK_PASS, m_pass, 0},
	{MSG_SVSNICK, TOK_SVSNICK, m_svsnick, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, m_protoctl, 0},
	{0, 0, 0, 0},
};

cumode_init chan_umodes[] = {
	{'o', CUMODE_CHANOP, '@'},
	{'h', CUMODE_HALFOP, '%'},
	{'a', CUMODE_CHANADMIN, '!'},
	{'v', CUMODE_VOICE, '+'},
	{'V', CMODE_VIP, '='},
	{'d', CMODE_SILENCE, '-'},
	{0, 0, 0},
};

cmode_init chan_modes[] = {
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
	{'M', CMODE_SEMIMODERATED, 0},
	{'X', CMODE_SUPERMODERATED, 0},
	{0, 0, 0},
};

umode_init user_umodes[] = {
	{'Z', UMODE_IRCADMIN},
	{'S', UMODE_SERVICES},
	{'P', UMODE_SADMIN},
	{'a', UMODE_SERVICESOPER},
	{'o', UMODE_OPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'i', UMODE_INVISIBLE},
	{'w', UMODE_WALLOP},
	{'s', UMODE_SERVNOTICE},
	{'c', UMODE_CLIENT},
	{'k', UMODE_KILLS},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'f', UMODE_FLOOD},
	{'y', UMODE_SPY},
	{'D', UMODE_DCC},
	{'g', UMODE_GLOBOPS},
	{'c', UMODE_CHATOP},
	{'j', UMODE_REJ},
	{'n', UMODE_ROUTE},
	{'m', UMODE_SPAM},
	{'x', UMODE_HIDE},
	{'p', UMODE_PROT},
	{'F', UMODE_GLOBCON},
	{'d', UMODE_DEBUG},
	{'d', UMODE_DCCWARN},
	{'W', UMODE_WHOIS},
	{0, 0},
};

umode_init user_smodes[] = {
	{'N', SMODE_NETADMIN},
	{'n', SMODE_CONET},
	{'T', SMODE_TECHADMIN},
	{'t', SMODE_COTECH},
	{'A', SMODE_SERVADMIN},
	{'G', SMODE_GUEST},
	{'a', SMODE_COADMIN},
	{'s', SMODE_SSL},
	{0, 0},
};

void
send_server (const char *sender, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", sender, MSGTOK(SERVER), name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink)
{
	send_cmd ("%s %s :TS", MSGTOK(PASS), pass);
	send_cmd ("CAPAB TS5 BURST SSJ5 NICKIP CLIENT");
	send_cmd ("%s %s %d :%s", MSGTOK(SERVER), name, numeric, infoline);
}

void
send_squit (const char *server, const char *quitmsg)
{
	send_cmd ("%s %s :%s", MSGTOK(SQUIT), server, quitmsg);
}

void 
send_quit (const char *who, const char *quitmsg)
{
	send_cmd (":%s %s :%s", who, MSGTOK(QUIT), quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	send_cmd (":%s %s %s", who, MSGTOK(PART), chan);
}

void 
send_sjoin (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %lu %s + :%s", sender, MSG_SJOIN, ts, chan, who);
}

void 
send_join (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %s", who, MSG_JOIN, chan);
}

void 
send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", sender, MSGTOK(MODE), chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSGTOK(NICK), nick, ts, newmode, ident, host, server, ts, realname);
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	send_cmd (":%s %s %s :%s", from, MSGTOK(PING), reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", who, MSGTOK(MODE), target, mode);
}

void 
send_numeric (const char *from, const int numeric, const char *target, const char *buf)
{
	send_cmd (":%s %d %s :%s", from, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	send_cmd ("%s %s", MSGTOK(PONG), reply);
}

void
send_snetinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", from, MSGTOK(SNETINFO), ts, prot, cloak, netname);
}

void
send_netinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", from, MSGTOK(NETINFO), ts, prot, cloak, netname);
}

void
send_vctrl (const int uprot, const int nicklen, const int modex, const int gc, const char* netname)
{
	send_cmd ("%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, uprot, nicklen, modex, gc, netname);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", from, MSGTOK(KILL), target, reason);
}

void 
send_svskill (const char *sender, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", sender, MSG_SVSKILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, MSGTOK(NICK), newnick, ts);
}

void 
send_svsnick (const char *sender, const char *target, const char *newnick, const unsigned long ts)
{
	send_cmd ("%s %s %s :%lu", MSGTOK(SVSNICK), target, newnick, ts);
}

void
send_svsjoin (const char *sender, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", MSGTOK(SVSJOIN), target, chan);
}

void
send_svspart (const char *sender, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", MSGTOK(SVSPART), target, chan);
}

void 
send_kick (const char *who, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", who, MSGTOK(KICK), chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *who, const char *buf)
{
	send_cmd (":%s %s :%s", who, MSGTOK(WALLOPS), buf);
}

void
send_svshost (const char *sender, const char *who, const char *vhost)
{
	send_cmd (":%s %s %s %s", sender, MSGTOK(SETHOST), who, vhost);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	send_cmd (":%s %s %s %s", from, MSG_INVITE, to, chan);
}

void 
send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %d %s %lu :%s", sender, MSGTOK(AKILL), host, ident, length, setby, ts, reason);
}

void 
send_rakill (const char *sender, const char *host, const char *ident)
{
	send_cmd (":%s %s %s %s", sender, MSGTOK(RAKILL), host, ident);
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
	send_cmd (":%s %s %s :%s", from, MSGTOK(PRIVATE), to, buf);
}

void
send_notice (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, MSGTOK(NOTICE), to, buf);
}

void
send_globops (const char *from, const char *buf)
{
	send_cmd (":%s %s :%s", from, MSGTOK(GLOBOPS), buf);
}

void 
send_svstime (const char *sender, const unsigned long ts)
{
}
void 
send_swhois (const char *sender, const char *target, const char *swhois)
{
}
void 
send_smo (const char *from, const char *umodetarget, const char *msg)
{
}
void 
send_svsmode (const char *sender, const char *target, const char *modes)
{
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
	do_protocol (origin, argv, argc);
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
m_credits (char *origin, char **argv, int argc, int srv)
{
	do_credits (origin, argv[0]);
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
	do_kill (argv[0], argv[1]);
}

static void
m_vhost (char *origin, char **argv, int argc, int srv)
{
	do_vhost (argv[0], argv[1]);
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
		do_nickchange (origin, argv[0], NULL);
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
m_vctrl (char *origin, char **argv, int argc, int srv)
{
	do_vctrl (argv[0], argv[1], argv[2], argv[3], argv[14]);
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

/* Ultimate3 Client Support */
static void
m_client (char *origin, char **argv, int argc, int srv)
{
	do_client (argv[0], NULL, argv[2], argv[3], argv[4], argv[5], argv[6], 
		 NULL, argv[8], NULL, argv[10], argv[11]);
}

static void
m_smode (char *origin, char **argv, int argc, int srv)
{
	do_smode (argv[0], argv[1]);
};

/* ultimate 3 */

static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (argv[0], argv[1], NULL);
}
