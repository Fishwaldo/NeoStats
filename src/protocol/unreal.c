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

#include "neostats.h"
#include "unreal.h"
#include "ircd.h"

static void m_version (char *origin, char **argv, int argc, int srv);
static void m_motd (char *origin, char **argv, int argc, int srv);
static void m_admin (char *origin, char **argv, int argc, int srv);
static void m_credits (char *origin, char **argv, int argc, int srv);
static void m_server (char *origin, char **argv, int argc, int srv);
static void m_squit (char *origin, char **argv, int argc, int srv);
static void m_quit (char *origin, char **argv, int argc, int srv);
static void m_mode (char *origin, char **argv, int argc, int srv);
static void m_umode2 (char *origin, char **argv, int argc, int srv);
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
#ifdef UNREAL32
static void m_eos (char *origin, char **argv, int argc, int srv);
#endif
static void m_ping (char *origin, char **argv, int argc, int srv);
static void m_netinfo (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);
static void m_pass (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_protocol (char *origin, char **argv, int argc, int srv);
static void m_whois (char *origin, char **argv, int argc, int srv);
static void m_smo (char *origin, char **argv, int argc, int srv);
static void m_swhois (char *origin, char **argv, int argc, int srv);
static void m_tkl (char *origin, char **argv, int argc, int srv);

#define NICKV2	

const int ircd_minprotocol = PROTOCOL_SJOIN;
const int ircd_optprotocol = PROTOCOL_TOKEN;
const int ircd_features = FEATURE_SVSTIME;
const char services_umode[]= "+oSq";
const char services_cmode[]= "+o";

ircd_cmd cmd_list[] = {
	/*Message	Token	Function	usage */
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
	{MSG_UMODE2, TOK_UMODE2, m_umode2, 0},
	{MSG_SVSMODE, TOK_SVSMODE, m_svsmode, 0},
	{MSG_SVS2MODE, TOK_SVS2MODE, m_svsmode, 0},
	{MSG_KILL, TOK_KILL, m_kill, 0},
	{MSG_PONG, TOK_PONG, m_pong, 0},
	{MSG_AWAY, TOK_AWAY, m_away, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_TOPIC, TOK_TOPIC, m_topic, 0},
	{MSG_KICK, TOK_KICK, m_kick, 0},
	{MSG_JOIN, TOK_JOIN, m_join, 0},
	{MSG_PART, TOK_PART, m_part, 0},
	{MSG_PING, TOK_PING, m_ping, 0},
	{MSG_NETINFO, TOK_NETINFO, m_netinfo, 0},
	{MSG_SJOIN, TOK_SJOIN, m_sjoin, 0},
	{MSG_PASS, TOK_PASS, m_pass, 0},
	{MSG_SVSNICK, TOK_SVSNICK, m_svsnick, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, m_protocol, 0},
	{MSG_WHOIS, TOK_WHOIS, m_whois, 0},
	{MSG_SWHOIS, TOK_SWHOIS, m_swhois, 0},
	{MSG_SMO, TOK_SMO, m_smo, 0},
#ifdef UNREAL32
	{MSG_EOS, TOK_EOS, m_eos, 0},
#endif
	{MSG_TKL, TOK_TKL, m_tkl, 0},
};

ChanModes chan_modes[] = {
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_HALFOP, 'h', 1, 0, '%'},
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_CHANPROT, 'a', 1, 0, '*'},
	{CMODE_CHANOWNER, 'q', 1, 0, '~'},
	{CMODE_LIMIT, 'l', 0, 1},
	{CMODE_PRIVATE, 'p', 0, 0},
	{CMODE_SECRET, 's', 0, 0},
	{CMODE_MODERATED, 'm', 0, 0},
	{CMODE_NOPRIVMSGS, 'n', 0, 0},
	{CMODE_TOPICLIMIT, 't', 0, 0},
	{CMODE_INVITEONLY, 'i', 0, 0},
	{CMODE_KEY, 'k', 0, 1},
	{CMODE_RGSTR, 'r', 0, 0},
	{CMODE_RGSTRONLY, 'R', 0, 0},
	{CMODE_NOCOLOR, 'c', 0, 0},
	{CMODE_OPERONLY, 'O', 0, 0},
	{CMODE_ADMONLY, 'A', 0, 0},
	{CMODE_LINK, 'L', 0, 1},
	{CMODE_NOKICKS, 'Q', 0, 0},
	{CMODE_BAN, 'b', 0, 1},
	{CMODE_STRIP, 'S', 0, 0},			/* works? */
	{CMODE_EXCEPT, 'e', 0, 1},			/* exception ban */
	{CMODE_NOKNOCK, 'K', 0, 0},			/* knock knock (no way!) */
	{CMODE_NOINVITE, 'V', 0, 0},			/* no invites */
	{CMODE_FLOODLIMIT, 'f', 0, 1},			/* flood limiter */
	{CMODE_MODREG, 'M', 0, 0},			/* need umode +r to talk */
	{CMODE_STRIPBADWORDS, 'G', 0, 0},			/* no badwords */
	{CMODE_NOCTCP, 'C', 0, 0},			/* no CTCPs */
	{CMODE_AUDITORIUM, 'u', 0, 0},
	{CMODE_ONLYSECURE, 'z', 0, 0},
	{CMODE_NONICKCHANGE, 'N', 0, 0},
};

UserModes user_umodes[] = {
	{UMODE_SERVICES, 'S'},
	{UMODE_NETADMIN, 'N'},
	{UMODE_SADMIN, 'a'},
	{UMODE_ADMIN, 'A'},
	{UMODE_COADMIN, 'C'},
	{UMODE_OPER, 'o'},
	{UMODE_LOCOP, 'O'},
	{UMODE_REGNICK, 'r'},
	{UMODE_INVISIBLE, 'i'},
	{UMODE_WALLOP, 'w'},
	{UMODE_FAILOP, 'g'},
	{UMODE_HELPOP, 'h'},
	{UMODE_SERVNOTICE, 's'},
	{UMODE_KIX, 'q'},
	{UMODE_BOT, 'B'},
/* temp removal of deaf for SVSMODE Services Stamp */
/* 	{UMODE_DEAF, 'd'},*/
#ifdef UNREAL32
	{UMODE_RGSTRONLY, 'R'},
 	{UMODE_NOCTCP, 'T'},
	{UMODE_WEBTV, 'V'},
	{UMODE_HIDEWHOIS, 'p'},
	{UMODE_HIDEOPER, 'H'},
#else 
	{UMODE_KILLS, 'k'},
	{UMODE_EYES, 'e'},
	{UMODE_FCLIENT, 'F'},
	{UMODE_CLIENT, 'c'},
	{UMODE_FLOOD, 'f'},
	{UMODE_JUNK, 'j'},
#endif
	{UMODE_STRIPBADWORDS, 'G'},
	{UMODE_SETHOST, 't'},
	{UMODE_HIDE, 'x'},
	/*{UMODE_CHATOP, 'b'},*/
	{UMODE_WHOIS, 'W'},
	{UMODE_SECURE, 'z'},
	{UMODE_VICTIM, 'v'},	
};

UserModes user_smodes[] = {
	{0, '0'},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_smodecount = 0;
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_server (const char *sender, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink)
{
/* PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NS SJB64 */
	send_cmd ("%s TOKEN NICKv2 VHP SJOIN SJOIN2 SJ3 UMODE2", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_PROTOCTL : MSG_PROTOCTL));
	send_cmd ("%s %s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_PASS : MSG_PASS), pass);
	send_cmd ("%s %s %d :%s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

void
send_squit (const char *server, const char *quitmsg)
{
	send_cmd ("%s %s :%s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SQUIT : MSG_SQUIT), server, quitmsg);
}

void 
send_quit (const char *who, const char *quitmsg)
{
	send_cmd (":%s %s :%s", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_QUIT : MSG_QUIT), quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	send_cmd (":%s %s %s", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_PART : MSG_PART), chan);
}

void 
send_join (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %s", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_JOIN : MSG_JOIN), chan);
}

void 
send_sjoin (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %lu %s + :%s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SJOIN : MSG_SJOIN), ts, chan, who);
}

void 
send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_MODE : MSG_MODE), chan, mode, args, ts);
}

/* m_nick
 *  argv[0] = nickname
 * if from new client
 *  argv[1] = nick password
 * if from server:
 *  argv[1] = hopcount
 *  argv[2] = timestamp
 *  argv[3] = username
 *  argv[4] = hostname
 *  argv[5] = servername
 * if NICK version 1:
 *  argv[6] = servicestamp
 *  argv[7] = info
 * if NICK version 2:
 *  argv[6] = servicestamp
 *  argv[7] = umodes
 *  argv[8] = virthost, * if none
 *  argv[9] = info
 */
void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s 0 %s * :%s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_NICK : MSG_NICK), nick, ts, ident, host, server, newmode, realname);
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	send_cmd (":%s %s %s :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_PING : MSG_PING), reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_MODE : MSG_MODE), target, mode);
}

void 
send_numeric (const char *from, const int numeric, const char *target, const char *buf)
{
	send_cmd (":%s %d %s :%s", from, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	send_cmd ("%s %s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_PONG : MSG_PONG), reply);
}

void
send_netinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_NETINFO : MSG_NETINFO), ts, prot, cloak, netname);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_KILL : MSG_KILL), target, reason);
}

void 
send_smo (const char *from, const char *umodetarget, const char *msg)
{
	send_cmd (":%s %s %s :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SMO : MSG_SMO), umodetarget, msg);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_NICK : MSG_NICK), newnick, ts);
}

void
send_swhois (const char *sender, const char *target, const char *swhois)
{
	send_cmd ("%s %s :%s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SWHOIS : MSG_SWHOIS), target, swhois);
}

void 
send_svsnick (const char *sender, const char *target, const char *newnick, const unsigned long ts)
{
	send_cmd ("%s %s %s :%lu", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, ts);
}

void
send_svsjoin (const char *sender, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SVSJOIN : MSG_SVSJOIN), target, chan);
}

void
send_svspart (const char *sender, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SVSPART : MSG_SVSPART), target, chan);
}

void 
send_kick (const char *who, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_KICK : MSG_KICK), chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *who, const char *buf)
{
	send_cmd (":%s %s :%s", who, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_WALLOPS : MSG_WALLOPS), buf);
}

void
send_svshost (const char *sender, const char *who, const char *vhost)
{
	send_cmd (":%s %s %s %s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_CHGHOST : MSG_CHGHOST), who, vhost);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	send_cmd (":%s %s %s %s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_INVITE : MSG_INVITE), to, chan);
}

void
send_svsmode (const char *sender, const char *target, const char *modes)
{
	send_cmd (":%s %s %s %s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SVSMODE : MSG_SVSMODE), target, modes);
}

void 
send_svskill (const char *sender, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_SVSKILL : MSG_SVSKILL), target, reason);
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void 
send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, unsigned long ts)
{
	send_cmd (":%s %s + G %s %s %s %lu %lu :%s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_TKL : MSG_TKL), ident, host, setby, (ts + length), ts, reason);
}

void 
send_rakill (const char *sender, const char *host, const char *ident)
{
	send_cmd (":%s %s - G %s %s %s", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_TKL : MSG_TKL), ident, host, sender);
}

void
send_privmsg (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_PRIVATE : MSG_PRIVATE), to, buf);
}

void
send_notice (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_NOTICE : MSG_NOTICE), to, buf);
}

void
send_globops (const char *from, const char *buf)
{
	send_cmd (":%s %s :%s", from, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_GLOBOPS : MSG_GLOBOPS), buf);
}

void 
send_svstime (const char *sender, const unsigned long ts)
{
	send_cmd (":%s %s SVSTIME %lu", sender, ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_TSCTL : MSG_TSCTL), ts);
}

static void
m_protocol (char *origin, char **argv, int argc, int srv)
{
	do_protocol (origin, argv, argc);
}

static void
m_stats (char *origin, char **argv, int argc, int srv)
{
	do_stats (origin, argv[0]);
}

/*
 * m_version
 *	argv[0] = remote server
 */
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

/* m_admin
 *	argv[0] = servername
 */
static void
m_admin (char *origin, char **argv, int argc, int srv)
{
	do_admin (origin, argv[0]);
}

/*m_credits
 *   argv[0] = servername
 */
static void
m_credits (char *origin, char **argv, int argc, int srv)
{
	do_credits (origin, argv[0]);
}

/* m_server
 *	argv[0] = servername
 *  argv[1] = hopcount
 *  argv[2] = numeric
 *  argv[3] = serverinfo
 * on old protocols, serverinfo is argv[2], and numeric is left out
 */
/*SERVER servername hopcount :U<protocol>-flags-numeric serverdesc*/
static void
m_server (char *origin, char **argv, int argc, int srv)
{
	char* s = argv[argc-1];
	if (*origin== 0) {
		/* server desc from uplink includes extra info so we need to 
		   strip protocol, flags and numeric. We can use the first
		   space to do this*/
		while(*s != ' ')
			s++;
		/* Strip the now leading space */
		s++;
	}
	if(argc > 3) {
		do_server (argv[0], origin, argv[1], argv[2], s, srv);
	} else {
		do_server (argv[0], origin, argv[1], NULL, s, srv);
	}
	
}

/* m_squit
 *	argv[0] = server name
 *	argv[argc-1] = comment
 */
static void
m_squit (char *origin, char **argv, int argc, int srv)
{
	do_squit (argv[0], argv[1]);
}

/* m_quit
 *	argv[0] = comment
 */
static void
m_quit (char *origin, char **argv, int argc, int srv)
{
	do_quit (origin, argv[0]);
}

/* m_svsmode
 *  argv[0] - username to change mode for
 *  argv[1] - modes to change
 *  argv[2] - Service Stamp (if mode == d)
 */
static void
m_svsmode (char *origin, char **argv, int argc, int srv)
{
	if (argv[0][0] == '#') {
		do_svsmode_channel (origin, argv, argc);
	} else {
		do_svsmode_user (argv[0], argv[1], argv[2]);
	}
}

/* m_mode 
 *  argv[0] - channel

 * m_umode
 * argv[0] - username to change mode for
 * argv[1] - modes to change
 */
/*  MODE
 *  :nick MODE nick :+modestring 
 *  :servername MODE #channel +modes parameter list TS 
 */
static void
m_mode (char *origin, char **argv, int argc, int srv)
{
	if (argv[0][0] == '#') {
		do_mode_channel (origin, argv, argc);
	} else {
		do_mode_user (argv[0], argv[1]);
	}
}

/* m_umode2
 * argv[0] - modes to change
 */
static void
m_umode2 (char *origin, char **argv, int argc, int srv)
{
	do_mode_user (origin, argv[0]);
}

/* m_kill
 *	argv[0] = kill victim(s) - comma separated list
 *	argv[1] = kill path
 */
static void
m_kill (char *origin, char **argv, int argc, int srv)
{
	do_kill (argv[0], argv[1]);
}
static void
m_vhost (char *origin, char **argv, int argc, int srv)
{
	do_vhost (origin, argv[0]);
}

/* m_pong
 *  argv[0] = origin
 *  argv[1] = destination
 */
static void
m_pong (char *origin, char **argv, int argc, int srv)
{
	do_pong (argv[0], argv[1]);
}

/* m_away
 *  argv[0] = away message
 */
static void
m_away (char *origin, char **argv, int argc, int srv)
{
	do_away (origin, (argc > 0) ? argv[0] : NULL);
}

/* m_nick
 *  argv[0] = nickname
 * if from new client
 *  argv[1] = nick password
 * if from server:
 *  argv[1] = hopcount
 *  argv[2] = timestamp
 *  argv[3] = username
 *  argv[4] = hostname
 *  argv[5] = servername
 * if NICK version 1:
 *  argv[6] = servicestamp
 *  argv[7] = info
 * if NICK version 2:
 *  argv[6] = servicestamp
 *  argv[7] = umodes
 *  argv[8] = virthost, * if none
 *  argv[9] = info
 */
static void
m_nick (char *origin, char **argv, int argc, int srv)
{
	if(!srv) {
#ifdef NICKV2	
		do_nick (argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, argv[6], argv[7], argv[8], argv[9], NULL, NULL);
#else
		do_nick (argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, argv[6], NULL, NULL, argv[9], NULL, NULL);
#endif
	} else {
		do_nickchange (origin, argv[0], NULL);
	}
}

/* m_topic
 *  argv[0] = topic text
 * For servers using TS:
 *  argv[0] = channel name
 *  argv[1] = topic nickname
 *  argv[2] = topic time
 *  argv[3] = topic text
 */
/* TOPIC #channel owner TS :topic */
static void
m_topic (char *origin, char **argv, int argc, int srv)
{
	do_topic (argv[0], argv[1], argv[2], argv[3]);
}

/* m_kick
 *	argv[0] = channel
 *	argv[1] = client to kick
 *	argv[2] = kick comment
 */
static void
m_kick (char *origin, char **argv, int argc, int srv)
{
	do_kick (origin, argv[0], argv[1], argv[2]);
}

/* m_join
 *	argv[0] = channel
 *	argv[1] = channel password (key)
 */
static void
m_join (char *origin, char **argv, int argc, int srv)
{
	do_join (origin, argv[0], argv[1]);
}

/* m_part
 *	argv[0] = channel
 *	argv[1] = comment
 */
static void
m_part (char *origin, char **argv, int argc, int srv)
{
	do_part (origin, argv[0], argv[1]);
}

/* m_ping
 *	argv[0] = origin
 *	argv[1] = destination
 */
static void
m_ping (char *origin, char **argv, int argc, int srv)
{
	do_ping (argv[0], argv[1]);
}

/* m_netinfo
 *  argv[0] = max global count
 *  argv[1] = time of end sync
 *  argv[2] = unreal protocol using (numeric)
 *  argv[3] = cloak-crc (> u2302)
 *  argv[4] = free(**)
 *  argv[5] = free(**)
 *  argv[6] = free(**)
 *  argv[7] = ircnet
 */
static void
m_netinfo (char *origin, char **argv, int argc, int srv)
{
	do_netinfo(argv[0], argv[1], argv[2], argv[3], argv[7]);
}

#ifdef UNREAL32
/*  EOS
 *  :servername EOS
 */
static void 
m_eos (char *origin, char **argv, int argc, int srv)
{
	do_eos (origin);
}
#endif
    
/* m_sjoin  
 *  argv[0] = channel timestamp
 *    char *argv[], pvar[MAXMODEPARAMS][MODEBUFLEN + 3];
 *  argv[1] = channel name
 *  "ts chname :"
 * if (argc == 3) 
 *  argv[2] = nick names + modes - all in one parameter
 *  "ts chname modebuf :"
 *  "ts chname :"@/"""name"	OPT_SJ3
 * if (argc == 4)
 *  argv[2] = channel modes
 *  argv[3] = nick names + modes - all in one parameter
 *  "ts chname modebuf parabuf :"
 * if (argc > 4)
 *  argv[2] = channel modes
 *  argv[3 to argc - 2] = mode parameters
 *  argv[argc - 1] = nick names + modes
 *  "ts parabuf :parv[parc - 1]"	OPT_SJOIN | OPT_SJ3 
 */
/*    MSG_SJOIN creationtime chname    modebuf parabuf :member list */
/* R: ~         1073861298   #services +       <none>  :Mark */
static void
m_sjoin (char *origin, char **argv, int argc, int srv)
{
	do_sjoin (argv[0], argv[1], ((argc >= 4) ? argv[2] : ""), origin, argv, argc);
}

/* m_pass
 *	argv[0] = password
 */
static void
m_pass (char *origin, char **argv, int argc, int srv)
{
}

/* m_svsnick
 *  argv[0] = old nickname
 *  argv[1] = new nickname
 *  argv[2] = timestamp
 */
static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (argv[0], argv[1], argv[2]);
}

/* m_whois
 *	argv[0] = nickname masklist
 */
static void
m_whois (char *origin, char **argv, int argc, int srv)
{
	/* TODO */
}

/* m_swhois
 *  argv[0] = nickname
 *  argv[1] = new swhois
 */
static void
m_swhois (char *origin, char **argv, int argc, int srv)
{
	do_swhois (argv[0], argv[1]);
}

static void
m_smo (char *origin, char **argv, int argc, int srv)
{
	/* TODO */
}

/*
 *  argv[0]  +|- 
 *  argv[1]  G   
 *  argv[2]  user 
 *  argv[3]  host 
 *  argv[4]  setby 
 *  argv[5]  expire_at 
 *  argv[6]  set_at 
 *  argv[7]  reason 

R: :server BD + G * mask setter 1074811259 1074206459 :reason
R: :server BD + Z * mask setter 0 1070062390 :reason
R: :server c dos_bot* :Reserved nickname: Dosbot
*/

static void m_tkl (char *origin, char **argv, int argc, int srv)
{
	do_tkl(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
}
