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

#include "neostats.h"
#include "ircd.h"
#include "ultimate.h"

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
static void m_svsnick (char *, char **, int argc, int srv);
static void m_protoctl (char *, char **, int argc, int srv);
#ifdef ULTIMATE3
static void m_svinfo (char *, char **, int argc, int srv);
static void m_burst (char *origin, char **argv, int argc, int srv);
static void m_sjoin (char *origin, char **argv, int argc, int srv);
static void m_client (char *origin, char **argv, int argc, int srv);
static void m_smode (char *origin, char **argv, int argc, int srv);
#else
static void m_snetinfo (char *origin, char **argv, int argc, int srv);
#endif
static void m_vctrl (char *origin, char **argv, int argc, int srv);

#ifdef ULTIMATE3
const char ircd_version[] = "(UL3)";
const char services_bot_modes[]= "+oS";
#else
const char ircd_version[] = "(UL)";
const char services_bot_modes[]= "+oS";
#endif

/* Ultimate 2 does support these 5 tokens so may need to add them back 
 * in at some point
 * TOK_MODE, TOK_JOIN, TOK_NICK, TOK_AWAY, TOK_TOPIC
 */

ircd_cmd cmd_list[] = {
	/* Command      Token          Function       srvmsg */
	{MSG_PRIVATE,	/*TOK_PRIVATE,	 */m_private,	0},
	{MSG_NOTICE,	/*TOK_NOTICE,	 */m_notice,	0},
	{MSG_STATS,     /*TOK_STATS,     */m_stats,     0},
	{MSG_SETHOST,   /*TOK_SETHOST,   */m_vhost,     0},
	{MSG_VERSION,   /*TOK_VERSION,   */m_version,   0},
	{MSG_MOTD,      /*TOK_MOTD,      */m_motd,      0},
	{MSG_ADMIN,     /*TOK_ADMIN,     */m_admin,     0},
	{MSG_CREDITS,   /*TOK_CREDITS,   */m_credits,   0},
	{MSG_SERVER,    /*TOK_SERVER,    */m_server,	0},
	{MSG_SQUIT,     /*TOK_SQUIT,     */m_squit,		0},
	{MSG_QUIT,      /*TOK_QUIT,      */m_quit,		0},
	{MSG_MODE,      /*TOK_MODE,      */m_mode,		0},
	{MSG_SVSMODE,   /*TOK_SVSMODE,   */m_svsmode,   0},
	{MSG_KILL,      /*TOK_KILL,      */m_kill,      0},
	{MSG_PONG,      /*TOK_PONG,      */m_pong,      0},
	{MSG_AWAY,      /*TOK_AWAY,      */m_away,      0},
	{MSG_NICK,      /*TOK_NICK,      */m_nick,      0},
	{MSG_TOPIC,     /*TOK_TOPIC,     */m_topic,     0},
	{MSG_KICK,      /*TOK_KICK,      */m_kick,      0},
	{MSG_JOIN,      /*TOK_JOIN,      */m_join,      0},
	{MSG_PART,      /*TOK_PART,      */m_part,      0},
	{MSG_PING,      /*TOK_PING,      */m_ping,      0},
#ifdef ULTIMATE3
	{MSG_SVINFO,    /*NULL,          */m_svinfo,    0},
	{MSG_CAPAB,     /*NULL,          */m_protoctl,  0},
	{MSG_BURST,     /*NULL,          */m_burst,     0},
	{MSG_SJOIN,     /*NULL,          */m_sjoin,     0},
	{MSG_CLIENT,    /*NULL,          */m_client,    0},
	{MSG_SMODE,     /*NULL,          */m_smode,     0},
#else
	{MSG_SNETINFO,  /*TOK_SNETINFO,  */m_snetinfo,   0},
#endif
	{MSG_VCTRL,     /*TOK_VCTRL,     */m_vctrl,     0},
	{MSG_PASS,      /*TOK_PASS,      */m_pass,      0},
	{MSG_SVSNICK,   /*TOK_SVSNICK,   */m_svsnick,   0},
	{MSG_PROTOCTL,  /*TOK_PROTOCTL,  */m_protoctl,  0},
};

ChanModes chan_modes[] = {
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_HALFOP, 'h', 1, 0, '%'},
	{CMODE_CHANADMIN, 'a', 1, 0, '!'},
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_BAN, 'b', 0, 1, 0},
	{CMODE_EXCEPT, 'e', 0, 1, 0},
	{CMODE_FLOODLIMIT, 'f', 0, 1, 0},			/* Flood limiter */
	{CMODE_INVITEONLY, 'i', 0, 0, 0},
	{CMODE_KEY, 'k', 0, 1, 0},
	{CMODE_LIMIT, 'l', 0, 1, 0},
	{CMODE_MODERATED, 'm', 0, 0, 0},
	{CMODE_NOPRIVMSGS, 'n', 0, 0, 0},
	{CMODE_PRIVATE, 'p', 0, 0, 0},
	{CMODE_RGSTR, 'r', 0, 0, 0},
	{CMODE_SECRET, 's', 0, 0, 0},
	{CMODE_TOPICLIMIT, 't', 0, 0, 0},
	{CMODE_NOCOLOR, 'x', 0, 0, 0},
	{CMODE_ADMONLY, 'A', 0, 0, 0},
	{CMODE_NOINVITE, 'I', 0, 0, 0},			/* no invites */
	{CMODE_NOKNOCK, 'K', 0, 0, 0},			/* knock knock (no way!) */
	{CMODE_LINK, 'L', 0, 1, 0},
	{CMODE_OPERONLY, 'O', 0, 0, 0},
	{CMODE_RGSTRONLY, 'R', 0, 0, 0},
	{CMODE_STRIP, 'S', 0, 0, 0},			/* works? */
};

#ifdef ULTIMATE3
UserModes user_umodes[] = {
	{UMODE_SRA, 'Z', NS_ULEVEL_ROOT},
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_SADMIN, 'P', NS_ULEVEL_ADMIN},
	{UMODE_SERVICESOPER, 'a', NS_ULEVEL_OPER},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_LOCOP, 'O', NS_ULEVEL_OPER},
	{UMODE_REGNICK, 'r', NS_ULEVEL_REG},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_WALLOP, 'w', 0},
	{UMODE_SERVNOTICE, 's', 0},
	{UMODE_CLIENT, 'c', 0},
	{UMODE_KILLS, 'k', 0},
	{UMODE_HELPOP, 'h', 0},
	{UMODE_FLOOD, 'f', 0},
	{UMODE_SPY, 'y', 0},
	{UMODE_DCC, 'D', 0},
	{UMODE_GLOBOPS, 'g', 0},
	{UMODE_CHATOPS, 'c', 0},
	{UMODE_REJ, 'j', 0},
	{UMODE_ROUTE, 'n', 0},
	{UMODE_SPAM, 'm', 0},
	{UMODE_HIDE, 'x', 0},
	{UMODE_KIX, 'p', 0},
	{UMODE_FCLIENT, 'F', 0},
#if 0
	/* useless modes, ignore them as services use these modes for services ID */
	{UMODE_DEBUG, 'd', 0},
#endif
	{UMODE_DCCWARN, 'e', 0},
	{UMODE_WHOIS, 'W', 0},
};

UserModes user_smodes[] = {
	{SMODE_NETADMIN, 'N', 190},
	{SMODE_CONET, 'n', 175},
	{SMODE_TECHADMIN, 'T', 150},
	{SMODE_COTECH, 't', 125},
	{SMODE_SERVADMIN, 'A', 100},
	{SMODE_GUEST, 'G', 100},
	{SMODE_COADMIN, 'a', 75},
	{SMODE_SSL, 's', 0},
};

#else
UserModes user_umodes[] = {
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_SADMIN, 'P', NS_ULEVEL_ROOT},
	{UMODE_TECHADMIN, 'T', 190},
	{UMODE_NETADMIN, 'N', NS_ULEVEL_ADMIN},
	{UMODE_SERVICESOPER, 'a', 100},
	{UMODE_IRCADMIN, 'Z', 100},
	{UMODE_ADMIN, 'z', 70},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_SUPER, 'p', NS_ULEVEL_LOCOPER},
	{UMODE_LOCOP, 'O', NS_ULEVEL_LOCOPER},
	{UMODE_REGNICK, 'r', NS_ULEVEL_REG},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_WALLOP, 'w', 0},
	{UMODE_FAILOP, 'g', 0},
	{UMODE_HELPOP, 'h', 0},
	{UMODE_SERVNOTICE, 's', 0},
	{UMODE_KILLS, 'k', 0},
	{UMODE_RBOT, 'B', 0},
	{UMODE_SBOT, 'b', 0},
	{UMODE_CLIENT, 'c', 0},
	{UMODE_FLOOD, 'f', 0},
	{UMODE_HIDE, 'x', 0},
	{UMODE_WATCHER, 'W', 0},
};
#endif

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
#ifdef GOTUSERSMODES
const int ircd_smodecount = ((sizeof (user_smodes) / sizeof (user_smodes[0])));
#endif
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_server (const char *sender, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", sender, (ircd_srv.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink)
{
#ifdef ULTIMATE3
	send_cmd ("%s %s :TS", (ircd_srv.token ? TOK_PASS : MSG_PASS), pass);
	send_cmd ("CAPAB TS5 BURST SSJ5 NICKIP CLIENT");
	send_cmd ("%s %s %d :%s", (ircd_srv.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
#else
	send_cmd ("%s %s", (ircd_srv.token ? TOK_PASS : MSG_PASS), pass);
	send_cmd ("%s %s %d :%s", (ircd_srv.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
/*	send_cmd ("%s TOKEN CLIENT", (ircd_srv.token ? TOK_PROTOCTL : MSG_PROTOCTL));*/
	send_cmd ("%s CLIENT", (ircd_srv.token ? TOK_PROTOCTL : MSG_PROTOCTL));
#endif
}

void
send_squit (const char *server, const char *quitmsg)
{
	send_cmd ("%s %s :%s", (ircd_srv.token ? TOK_SQUIT : MSG_SQUIT), server, quitmsg);
}

void 
send_quit (const char *who, const char *quitmsg)
{
	send_cmd (":%s %s :%s", who, (ircd_srv.token ? TOK_QUIT : MSG_QUIT), quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	send_cmd (":%s %s %s", who, (ircd_srv.token ? TOK_PART : MSG_PART), chan);
}

void 
send_sjoin (const char *sender, const char *who, const char *chan, const char flag, const unsigned long ts)
{
#ifdef ULTIMATE3
	send_cmd (":%s %s %lu %s + :%c%s", sender, MSG_SJOIN, ts, chan, flag, who);
#endif
}

void 
send_join (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %s", who, MSG_JOIN, chan);
}

void 
send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", who, (ircd_srv.token ? TOK_MODE : MSG_MODE), chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
#ifdef ULTIMATE3
	send_cmd ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", (ircd_srv.token ? TOK_NICK : MSG_NICK), nick, ts, newmode, ident, host, server, ts, realname);
#else
	send_cmd ("%s %s 1 %lu %s %s %s 0 :%s", (ircd_srv.token ? TOK_NICK : MSG_NICK), nick, ts, ident, host, server, realname);
#endif
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	send_cmd (":%s %s %s :%s", from, (ircd_srv.token ? TOK_PING : MSG_PING), reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	send_cmd (":%s %s %s :%s", who, (ircd_srv.token ? TOK_MODE : MSG_MODE), target, mode);
}

void 
send_numeric (const char *from, const int numeric, const char *target, const char *buf)
{
	send_cmd (":%s %d %s :%s", from, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	send_cmd ("%s %s", (ircd_srv.token ? TOK_PONG : MSG_PONG), reply);
}

void
send_snetinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", from, MSG_SNETINFO, ts, prot, cloak, netname);
}

void
send_netinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts)
{
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", from, MSG_NETINFO, ts, prot, cloak, netname);
}

void
send_vctrl (const int uprot, const int nicklen, const int modex, const int gc, const char* netname)
{
	send_cmd ("%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, uprot, nicklen, modex, gc, netname);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", from, (ircd_srv.token ? TOK_KILL : MSG_KILL), target, reason);
}

void 
send_svskill (const char *sender, const char *target, const char *reason)
{
	send_cmd (":%s %s %s :%s", sender, MSG_SVSKILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts)
{
	send_cmd (":%s %s %s %lu", oldnick, (ircd_srv.token ? TOK_NICK : MSG_NICK), newnick, ts);
}

void 
send_svsnick (const char *sender, const char *target, const char *newnick, const unsigned long ts)
{
	send_cmd ("%s %s %s :%lu", (ircd_srv.token ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, ts);
}

void
send_svsjoin (const char *sender, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", (ircd_srv.token ? TOK_SVSJOIN : MSG_SVSJOIN), target, chan);
}

void
send_svspart (const char *sender, const char *target, const char *chan)
{
	send_cmd ("%s %s %s", (ircd_srv.token ? TOK_SVSPART : MSG_SVSPART), target, chan);
}

void 
send_kick (const char *who, const char *chan, const char *target, const char *reason)
{
	send_cmd (":%s %s %s %s :%s", who, (ircd_srv.token ? TOK_KICK : MSG_KICK), chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (const char *who, const char *buf)
{
	send_cmd (":%s %s :%s", who, (ircd_srv.token ? TOK_WALLOPS : MSG_WALLOPS), buf);
}

void
send_svshost (const char *sender, const char *who, const char *vhost)
{
#ifdef ULTIMATE3
	send_cmd (":%s %s %s %s", sender, (ircd_srv.token ? TOK_SETHOST : MSG_SETHOST), who, vhost);
#elif ULTIMATE
	send_cmd (":%s %s %s %s", sender, (ircd_srv.token ? TOK_CHGHOST : MSG_CHGHOST), who, vhost);
#endif
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	send_cmd (":%s %s %s %s", from, (ircd_srv.token ? TOK_INVITE : MSG_INVITE), to, chan);
}

void 
send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, const unsigned long ts)
{
#ifdef ULTIMATE3
	send_cmd (":%s %s %s %s %d %s %lu :%s", sender, (ircd_srv.token ? TOK_AKILL : MSG_AKILL), host, ident, length, setby, ts, reason);
#elif ULTIMATE
	send_cmd (":%s %s %s@%s %lu %lu %s :%s", sender, MSG_GLINE, ident, host, (ts + length), ts, setby, reason);
#endif
}

void 
send_rakill (const char *sender, const char *host, const char *ident)
{
#ifdef ULTIMATE3
	send_cmd (":%s %s %s %s", sender, (ircd_srv.token ? TOK_RAKILL : MSG_RAKILL), host, ident);
#elif ULTIMATE
	/* ultimate2 needs an oper to remove */
	send_cmd (":%s %s :%s@%s", s_Services, MSG_REMGLINE, host, ident);
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
send_privmsg (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, (ircd_srv.token ? TOK_PRIVATE : MSG_PRIVATE), to, buf);
}

void
send_notice (const char *from, const char *to, const char *buf)
{
	send_cmd (":%s %s %s :%s", from, (ircd_srv.token ? TOK_NOTICE : MSG_NOTICE), to, buf);
}

void
send_globops (const char *from, const char *buf)
{
	send_cmd (":%s %s :%s", from, (ircd_srv.token ? TOK_GLOBOPS : MSG_GLOBOPS), buf);
}

#ifdef ULTIMATE3
/* :from SJOIN TS #chan modebuf  :nickbuf */
/* :from SJOIN TS #chan modebuf parabuf :nickbuf */
/* :from SJOIN TS #chan */
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
#endif

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
#ifdef ULTIMATE3
		do_svsmode_user (argv[0], argv[2], NULL);
#else
		do_svsmode_user (argv[0], argv[1], NULL);
#endif
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
#ifdef ULTIMATE3
	do_vhost (argv[0], argv[1]);
#else
	do_vhost (origin, argv[0]);
#endif
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
#ifdef ULTIMATE3
		do_nick (argv[0], argv[1], argv[2], argv[4], argv[5], 
			argv[6], argv[8], NULL, argv[3], NULL, argv[9], NULL, NULL);
#elif ULTIMATE
		do_nick (argv[0], argv[1], argv[2], argv[3], argv[4],
			argv[5], NULL, NULL, NULL, NULL, argv[7], NULL);
#endif
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

#ifdef ULTIMATE3
static void
m_svinfo (char *origin, char **argv, int argc, int srv)
{
	do_svinfo ();
}
#endif

#ifndef ULTIMATE3
static void
m_snetinfo (char *origin, char **argv, int argc, int srv)
{
	do_snetinfo(argv[0], argv[1], argv[2], argv[3], argv[7]);
}
#endif

static void
m_pass (char *origin, char **argv, int argc, int srv)
{
}

/* Ultimate3 Client Support */
#ifdef ULTIMATE3
static void
m_client (char *origin, char **argv, int argc, int srv)
{
	do_client (argv[0], NULL, argv[2], argv[3], argv[4], argv[5], argv[6], 
		 argv[7], argv[8], NULL, argv[10], argv[11]);
}

static void
m_smode (char *origin, char **argv, int argc, int srv)
{
	do_smode (argv[0], argv[1]);
};

/* ultimate 3 */
#endif

static void
m_svsnick (char *origin, char **argv, int argc, int srv)
{
	do_nickchange (argv[0], argv[1], NULL);
}
