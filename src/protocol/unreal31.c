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

#include "unreal31.h"
#include "neostats.h"
#include "ircd.h"

static void m_server (char *origin, char **argv, int argc, int srv);
static void m_mode (char *origin, char **argv, int argc, int srv);
static void m_umode2 (char *origin, char **argv, int argc, int srv);
static void m_svsmode (char *origin, char **argv, int argc, int srv);
static void m_nick (char *origin, char **argv, int argc, int srv);
static void m_vhost (char *origin, char **argv, int argc, int srv);
static void m_netinfo (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_whois (char *origin, char **argv, int argc, int srv);
static void m_smo (char *origin, char **argv, int argc, int srv);
static void m_swhois (char *origin, char **argv, int argc, int srv);
static void m_tkl (char *origin, char **argv, int argc, int srv);

#define NICKV2	

/* buffer sizes */
const int proto_maxhost		= (128 + 1);
const int proto_maxpass		= (32 + 1);
const int proto_maxnick		= (30 + 1);
const int proto_maxuser		= (10 + 1);
const int proto_maxrealname	= (50 + 1);
const int proto_chanlen		= (32 + 1);
const int proto_topiclen	= (307 + 1);

ProtocolInfo protocol_info = {
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN,
	/* Features supported by this IRCd */
	FEATURE_UMODECLOAK,
	"+oSq",
	"+o",
};

ircd_cmd cmd_list[] = {
	/*Message	Token	Function	usage */
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
	{MSG_MODE, TOK_MODE, m_mode, 0},
	{MSG_UMODE2, TOK_UMODE2, m_umode2, 0},
	{MSG_SVSMODE, TOK_SVSMODE, m_svsmode, 0},
	{MSG_SVS2MODE, TOK_SVS2MODE, m_svsmode, 0},
	{MSG_KILL, TOK_KILL, _m_kill, 0},
	{MSG_PONG, TOK_PONG, _m_pong, 0},
	{MSG_AWAY, TOK_AWAY, _m_away, 0},
	{MSG_NICK, TOK_NICK, m_nick, 0},
	{MSG_TOPIC, TOK_TOPIC, _m_topic, 0},
	{MSG_KICK, TOK_KICK, _m_kick, 0},
	{MSG_JOIN, TOK_JOIN, _m_join, 0},
	{MSG_PART, TOK_PART, _m_part, 0},
	{MSG_PING, TOK_PING, _m_ping, 0},
	{MSG_NETINFO, TOK_NETINFO, m_netinfo, 0},
	{MSG_SJOIN, TOK_SJOIN, m_sjoin, 0},
	{MSG_PASS, TOK_PASS, _m_pass, 0},
	{MSG_SVSNICK, TOK_SVSNICK, m_svsnick, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, _m_protoctl, 0},
	{MSG_WHOIS, TOK_WHOIS, m_whois, 0},
	{MSG_SWHOIS, TOK_SWHOIS, m_swhois, 0},
	{MSG_SMO, TOK_SMO, m_smo, 0},
	{MSG_TKL, TOK_TKL, m_tkl, 0},
	{MSG_GLOBOPS, TOK_GLOBOPS, _m_globops, 0},
	{MSG_WALLOPS, TOK_WALLOPS, _m_wallops, 0},
	{MSG_CHATOPS, TOK_CHATOPS, _m_chatops, 0},
	{MSG_LOCOPS, TOK_LOCOPS, _m_locops, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = {
	{'v', CUMODE_VOICE, 0, '+'},
	{'h', CUMODE_HALFOP, 0, '%'},
	{'o', CUMODE_CHANOP, 0, '@'},
	{'a', CUMODE_CHANPROT, 0, '*'},
	{'q', CUMODE_CHANOWNER, 0, '~'},
	{0, 0, 0},
};

mode_init chan_modes[] = {
	{'l', CMODE_LIMIT, MODEPARAM},
	{'p', CMODE_PRIVATE, 0},
	{'s', CMODE_SECRET, 0},
	{'m', CMODE_MODERATED, 0},
	{'n', CMODE_NOPRIVMSGS, 0},
	{'t', CMODE_TOPICLIMIT, 0},
	{'i', CMODE_INVITEONLY, 0},
	{'k', CMODE_KEY, MODEPARAM},
	{'r', CMODE_RGSTR, 0},
	{'R', CMODE_RGSTRONLY, 0},
	{'c', CMODE_NOCOLOR, 0},
	{'O', CMODE_OPERONLY, 0},
	{'A', CMODE_ADMONLY, 0},
	{'L', CMODE_LINK, MODEPARAM},
	{'Q', CMODE_NOKICKS, 0},
	{'b', CMODE_BAN, MODEPARAM},
	{'S', CMODE_STRIP, 0},
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'K', CMODE_NOKNOCK, 0},
	{'V', CMODE_NOINVITE, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM},
	{'M', CMODE_MODREG, 0},
	{'G', CMODE_STRIPBADWORDS, 0},
	{'C', CMODE_NOCTCP, 0},
	{'u', CMODE_AUDITORIUM, 0},
	{'z', CMODE_ONLYSECURE, 0},
	{'N', CMODE_NONICKCHANGE, 0},
	{0, 0},
};

mode_init user_umodes[] = {
	{'S', UMODE_SERVICES},
	{'N', UMODE_NETADMIN},
	{'a', UMODE_SADMIN},
	{'A', UMODE_ADMIN},
	{'C', UMODE_COADMIN},
	{'o', UMODE_OPER},
	{'O', UMODE_LOCOP},
	{'r', UMODE_REGNICK},
	{'i', UMODE_INVISIBLE},
	{'w', UMODE_WALLOP},
	{'g', UMODE_FAILOP},
	{'h', UMODE_HELPOP},
	{'s', UMODE_SERVNOTICE},
	{'q', UMODE_KIX},
	{'B', UMODE_BOT},
 	{'d', UMODE_DEAF},
	{'k', UMODE_KILLS},
	{'e', UMODE_EYES},
	{'F', UMODE_FCLIENT},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FLOOD},
	{'j', UMODE_JUNK},
	{'G', UMODE_STRIPBADWORDS},
	{'t', UMODE_SETHOST},
	{'x', UMODE_HIDE},
	/*{'b', UMODE_CHATOP},*/
	{'W', UMODE_WHOIS},
	{'z', UMODE_SECURE},
	{'v', UMODE_VICTIM},	
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
/* PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NS SJB64 */
	send_cmd ("%s TOKEN NICKv2 VHP SJOIN SJOIN2 SJ3 UMODE2", MSGTOK(PROTOCTL));
	send_cmd ("%s %s", MSGTOK(PASS), pass);
	send_cmd ("%s %s %d :%s", MSGTOK(SERVER), name, numeric, infoline);
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
send_sjoin (const char *source, const char *target, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %lu %s + :%s", source, MSGTOK(SJOIN), ts, chan, target);
}

void 
send_cmode (const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", source, MSGTOK(MODE), chan, mode, args, ts);
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
	send_cmd ("%s %s 1 %lu %s %s %s 0 %s * :%s", MSGTOK(NICK), nick, ts, ident, host, server, newmode, realname);
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
send_netinfo (const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSGTOK(NETINFO), ts, prot, cloak, netname);
}

void 
send_kill (const char *source, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(KILL), target, reason);
}

void 
send_smo (const char *source, const char *umodetarget, const char *msg)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(SMO), umodetarget, msg);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, MSGTOK(NICK), newnick, ts);
}

void
send_swhois (const char *source, const char *target, const char *swhois)
{
	send_cmd ("%s %s :%s", MSGTOK(SWHOIS), target, swhois);
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
	send_cmd (":%s %s %s %s", source, MSGTOK(CHGHOST), target, vhost);
}

void
send_invite (const char *source, const char *target, const char *chan) 
{
	send_cmd (":%s %s %s %s", source, MSGTOK(INVITE), target, chan);
}

void
send_svsmode (const char *source, const char *target, const char *modes)
{
	send_cmd (":%s %s %s %s", source, MSGTOK(SVSMODE), target, modes);
}

void 
send_svskill (const char *source, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", source, MSGTOK(SVSKILL), target, reason);
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void 
send_akill (const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s %s + G %s %s %s %lu %lu :%s", source, MSGTOK(TKL), ident, host, setby, (ts + length), ts, reason);
}

void 
send_rakill (const char *source, const char *host, const char *ident)
{
	send_cmd (":%s %s - G %s %s %s", source, MSGTOK(TKL), ident, host, source);
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

void 
send_svstime (const char *source, const unsigned long ts)
{
	send_cmd (":%s %s SVSTIME %lu", source, MSGTOK(TSCTL), ts);
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

static void
m_vhost (char *origin, char **argv, int argc, int srv)
{
	do_vhost (origin, argv[0]);
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
