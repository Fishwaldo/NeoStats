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
#include "mystic.h"
#include "log.h"

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
static void m_snetinfo (char *origin, char **argv, int argc, int srv);
static void m_pass (char *origin, char **argv, int argc, int srv);
static void m_svsnick (char *origin, char **argv, int argc, int srv);
static void m_protoctl (char *origin, char **argv, int argc, int srv);
static void m_vctrl (char *origin, char **argv, int argc, int srv);

const char ircd_version[] = "(M)";
const char services_bot_modes[]= "+oS";

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
	{MSG_SNETINFO, TOK_SNETINFO, m_snetinfo, 0},
	{MSG_VCTRL, TOK_VCTRL, m_vctrl, 0},
	{MSG_PASS, TOK_PASS, m_pass, 0},
	{MSG_SVSNICK, TOK_SVSNICK, m_svsnick, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, m_protoctl, 0},
};

ChanModes chan_modes[] = {
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_HALFOP, 'h', 1, 0, '%'},
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
	{CMODE_STRIPBADWORDS, 'U', 0, 0, 0},
};

UserModes user_umodes[] = {
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_SADMIN, 'P', NS_ULEVEL_ROOT},
	{UMODE_TECHADMIN, 'T', NS_ULEVEL_ADMIN},
	{UMODE_NETADMIN, 'N', NS_ULEVEL_ADMIN},
	{UMODE_ADMIN, 'A', NS_ULEVEL_ADMIN}, 
	{UMODE_SERVICESOPER, 'a', NS_ULEVEL_OPER},
	{UMODE_IRCADMIN, 'Z', NS_ULEVEL_OPER},
	{UMODE_COADMIN, 'z', NS_ULEVEL_OPER},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_SUPER, 'p', NS_ULEVEL_OPER},
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
	{UMODE_CHATOP, 'C', 0},
	{UMODE_NGLOBAL, 'G', 0},
	{UMODE_WHOIS, 'm', 0},
	{UMODE_NETINFO, 'n', 0},
	{UMODE_MAGICK, 'M', 0},
	{UMODE_NETMON, 'X', 0},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_server (const char *sender, const char *name, const int numeric, const char *infoline)
{
	send_cmd (":%s %s %s %d :%s", sender, (ircd_srv.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink)
{
	send_cmd ("%s %s", (ircd_srv.token ? TOK_PASS : MSG_PASS), pass);
	send_cmd ("%s %s %d :%s", (ircd_srv.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	send_cmd ("%s TOKEN CLIENT", (ircd_srv.token ? TOK_PROTOCTL : MSG_PROTOCTL));
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
send_join (const char *sender, const char *who, const char *chan, const unsigned long ts)
{
	send_cmd (":%s %s %s", who, (ircd_srv.token ? TOK_JOIN : MSG_JOIN), chan);
}

void 
send_sjoin (const char *sender, const char *who, const char *chan, const char flag, const unsigned long ts)
{
}

void 
send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts)
{
	send_cmd (":%s %s %s %s %s %lu", sender, (ircd_srv.token ? TOK_MODE : MSG_MODE), chan, mode, args, ts);
}

void
send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname)
{
	send_cmd ("%s %s 1 %lu %s %s %s 0 :%s", (ircd_srv.token ? TOK_NICK : MSG_NICK), nick, ts, ident, host, server, realname);
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
	send_cmd (":%s %s 0 %lu %d %s 0 0 0 :%s", from, (ircd_srv.token ? TOK_SNETINFO : MSG_SNETINFO), ts, prot, cloak, netname);
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
	send_cmd (":%s %s %s %s", sender, MSG_CHGHOST, who, vhost);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	send_cmd (":%s %s %s %s", from, MSG_INVITE, to, chan);
}

void 
send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, const unsigned long ts)
{
	send_cmd (":%s %s %s@%s %lu %lu %s :%s", sender, MSG_GLINE, ident, host, (ts + length), ts, setby, reason);
}

void 
send_rakill (const char *sender, const char *host, const char *ident)
{
	send_cmd (":%s %s :%s@%s", s_Services, MSG_REMGLINE, host, ident);
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
		do_svsmode_user (argv[0], argv[1], NULL);
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
	do_vhost (origin, argv[0]);
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
		do_nick (argv[0], argv[1], argv[2], argv[3], argv[4], 
			argv[5], NULL, NULL, NULL, NULL, argv[7], NULL);
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
m_snetinfo (char *origin, char **argv, int argc, int srv)
{
	do_snetinfo(argv[0], argv[1], argv[2], argv[3], argv[7]);
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
