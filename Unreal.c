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
#include "sock.h"
#include "Unreal.h"
#include "dl.h"
#include "log.h"
#include "users.h"
#include "server.h"
#include "chans.h"

static void Usr_Version (char *origin, char **argv, int argc);
static void Usr_MOTD (char *origin, char **argv, int argc);
static void Usr_Admin (char *origin, char **argv, int argc);
static void Usr_Credits (char *origin, char **argv, int argc);
static void Usr_Server (char *origin, char **argv, int argc);
static void Usr_Squit (char *origin, char **argv, int argc);
static void Usr_Quit (char *origin, char **argv, int argc);
static void Usr_Mode (char *origin, char **argv, int argc);
static void Usr_Smode (char *origin, char **argv, int argc);
static void Usr_Kill (char *origin, char **argv, int argc);
static void Usr_Pong (char *origin, char **argv, int argc);
static void Usr_Away (char *origin, char **argv, int argc);
static void Usr_Nick (char *origin, char **argv, int argc);
static void Usr_Topic (char *origin, char **argv, int argc);
static void Usr_Kick (char *origin, char **argv, int argc);
static void Usr_Join (char *origin, char **argv, int argc);
static void Usr_Part (char *origin, char **argv, int argc);
static void Usr_Stats (char *origin, char **argv, int argc);
static void Usr_Vhost (char *origin, char **argv, int argc);
#ifdef UNREAL32
static void Usr_Eos (char *origin, char **argv, int argc);
#endif
static void Srv_Ping (char *origin, char **argv, int argc);
static void Srv_Netinfo (char *origin, char **argv, int argc);
static void Srv_Sjoin (char *origin, char **argv, int argc);
static void Srv_Pass (char *origin, char **argv, int argc);
static void Srv_Server (char *origin, char **argv, int argc);
static void Srv_Squit (char *origin, char **argv, int argc);
static void Srv_Nick (char *origin, char **argv, int argc);
static void Srv_Svsnick (char *origin, char **argv, int argc);
static void Srv_Kill (char *origin, char **argv, int argc);
static void Srv_Protocol (char *origin, char **argv, int argc);

static struct ircd_srv_ {
	int uprot;
	char cloak[10];
} ircd_srv;

#ifdef UNREAL32
const char ircd_version[] = "(U32)";
#else
const char ircd_version[] = "(U31)";
#endif
const char services_bot_modes[]= "+oSqd";
long services_bot_umode= 0;

IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, TOK_STATS, Usr_Stats, 1, 0},
	{MSG_SETHOST, TOK_SETHOST, Usr_Vhost, 1, 0},
	{MSG_VERSION, TOK_VERSION, Usr_Version, 1, 0},
	{MSG_MOTD, TOK_MOTD, Usr_MOTD, 1, 0},
	{MSG_ADMIN, TOK_ADMIN, Usr_Admin, 1, 0},
	{MSG_CREDITS, TOK_CREDITS, Usr_Credits, 1, 0},
	{MSG_SERVER, TOK_SERVER, Usr_Server, 1, 0},
	{MSG_SQUIT, TOK_SQUIT, Usr_Squit, 1, 0},
	{MSG_QUIT, TOK_QUIT, Usr_Quit, 1, 0},
	{MSG_MODE, TOK_MODE, Usr_Mode, 1, 0},
	{MSG_SVSMODE, TOK_SVSMODE, Usr_Smode, 1, 0},
	{MSG_SVS2MODE, TOK_SVS2MODE, Usr_Smode, 1, 0},
	{MSG_KILL, TOK_KILL, Usr_Kill, 1, 0},
	{MSG_PONG, TOK_PONG, Usr_Pong, 1, 0},
	{MSG_AWAY, TOK_AWAY, Usr_Away, 1, 0},
	{MSG_NICK, TOK_NICK, Usr_Nick, 1, 0},
	{MSG_TOPIC, TOK_TOPIC, Usr_Topic, 1, 0},
	{MSG_TOPIC, TOK_TOPIC, Usr_Topic, 0, 0},
	{MSG_KICK, TOK_KICK, Usr_Kick, 1, 0},
	{MSG_JOIN, TOK_JOIN, Usr_Join, 1, 0},
	{MSG_PART, TOK_PART, Usr_Part, 1, 0},
#ifdef UNREAL32
	{MSG_EOS, TOK_EOS, Usr_Eos, 0, 0},
#endif
	{MSG_PING, TOK_PING, Srv_Ping, 0, 0},
	{MSG_NETINFO, TOK_NETINFO, Srv_Netinfo, 0, 0},
	{MSG_SJOIN, TOK_SJOIN, Srv_Sjoin, 1, 0},
	{MSG_SJOIN, TOK_SJOIN, Srv_Sjoin, 0, 0},
	{MSG_PASS, TOK_PASS, Srv_Pass, 0, 0},
	{MSG_SERVER, TOK_SERVER, Srv_Server, 0, 0},
	{MSG_SQUIT, TOK_SQUIT, Srv_Squit, 0, 0},
	{MSG_NICK, TOK_NICK, Srv_Nick, 0, 0},
	{MSG_SVSNICK, TOK_SVSNICK, Srv_Svsnick, 0, 0},
	{MSG_KILL, TOK_KILL, Srv_Kill, 0, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, Srv_Protocol, 0, 0},
};

ChanModes chan_modes[] = {
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_HALFOP, 'h', 1, 0, '%'},
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_CHANPROT, 'a', 1, 0, '~'},
	{CMODE_CHANOWNER, 'q', 1, 0, '*'},
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
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_NETADMIN, 'N', NS_ULEVEL_ADMIN},
	{UMODE_SADMIN, 'a', NS_ULEVEL_ADMIN},
	{UMODE_ADMIN, 'A', NS_ULEVEL_OPER},
	{UMODE_COADMIN, 'C', NS_ULEVEL_OPER},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_LOCOP, 'O', NS_ULEVEL_LOCOPER},
	{UMODE_REGNICK, 'r', NS_ULEVEL_REG},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_WALLOP, 'w', 0},
	{UMODE_FAILOP, 'g', 0},
	{UMODE_HELPOP, 'h', 0},
	{UMODE_SERVNOTICE, 's', 0},
	{UMODE_KIX, 'q', 0},
	{UMODE_BOT, 'B', 0},
	{UMODE_DEAF, 'd', 0},
#ifdef UNREAL32
	{UMODE_RGSTRONLY, 'R', 0},
 	{UMODE_NOCTCP, 'T', 0},
	{UMODE_WEBTV, 'V', 0},
	{UMODE_HIDEWHOIS, 'p', 0},
#else 
	{UMODE_KILLS, 'k', 0},
	{UMODE_EYES, 'e', 0},
	{UMODE_FCLIENT, 'F', 0},
	{UMODE_CLIENT, 'c', 0},
	{UMODE_FLOOD, 'f', 0},
	{UMODE_JUNK, 'j', 0},
#endif
	{UMODE_STRIPBADWORDS, 'G', 0},
	{UMODE_SETHOST, 't', 0},
	{UMODE_HIDE, 'x', 0},
	/*{UMODE_CHATOP, 'b', 0},*/
	{UMODE_WHOIS, 'W', 0},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_server (const char *name, const int numeric, const char *infoline)
{
	sts (":%s %s %s %d :%s", me.name, (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass)
{
#if 0
	sts ("%s TOKEN SJOIN", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL));
#else
	sts ("%s TOKEN", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL));
#endif
	sts ("%s %s", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts ("%s %s %d :%s", (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

void
send_squit (const char *server, const char *quitmsg)
{
	sts ("%s %s :%s", (me.token ? TOK_SQUIT : MSG_SQUIT), server, quitmsg);
}

void 
send_quit (const char *who, const char *quitmsg)
{
	sts (":%s %s :%s", who, (me.token ? TOK_QUIT : MSG_QUIT), quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	sts (":%s %s %s", who, (me.token ? TOK_PART : MSG_PART), chan);
}

void 
send_join (const char *who, const char *chan)
{
	sts (":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
}

void 
send_sjoin (const char *who, const char *chan, const char flag, time_t tstime)
{
	sts (":%s %s %ld %s + :%c%s", me.name, (me.token ? TOK_SJOIN : MSG_SJOIN), (int)tstime, chan, flag, who);
}

void 
send_cmode (const char *who, const char *chan, const char *mode, const char *args)
{
	sts (":%s %s %s %s %s %lu", who, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, me.now);
}

void
send_nick (const char *nick, const char *ident, const char *host, const char *realname, const char* newmode, time_t tstime)
{
	sts ("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, tstime, ident, host, me.name, realname);
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_PING : MSG_PING), reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	sts (":%s %s %s :%s", who, (me.token ? TOK_MODE : MSG_MODE), target, mode);
}

void 
send_numeric (const int numeric, const char *target, const char *buf)
{
	sts (":%s %d %s :%s", me.name, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	sts ("%s %s", (me.token ? TOK_PONG : MSG_PONG), reply);
}

void
send_netinfo (void)
{
	sts (":%s %s 0 %d %d %s 0 0 0 :%s", me.name, (me.token ? TOK_NETINFO : MSG_NETINFO), (int)me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname);
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, reason);
}

void 
send_smo (const char *from, const char *umodetarget, const char *msg)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_SMO : MSG_SMO), umodetarget, msg);
}

void 
send_nickchange (const char *oldnick, const char *newnick)
{
	sts (":%s %s %s %d", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, (int)me.now);
}

void
send_swhois (const char *target, const char *swhois)
{
	sts ("%s %s :%s", (me.token ? TOK_SWHOIS : MSG_SWHOIS), target, swhois);
}

void 
send_svsnick (const char *target, const char *newnick)
{
	sts ("%s %s %s :%d", (me.token ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, (int)me.now);
}

void
send_svsjoin (const char *target, const char *chan)
{
	sts ("%s %s %s", (me.token ? TOK_SVSJOIN : MSG_SVSJOIN), target, chan);
}

void
send_svspart (const char *target, const char *chan)
{
	sts ("%s %s %s", (me.token ? TOK_SVSPART : MSG_SVSPART), target, chan);
}

void 
send_kick (const char *who, const char *target, const char *chan, const char *reason)
{
	sts (":%s %s %s %s :%s", who, (me.token ? TOK_KICK : MSG_KICK), chan, target, (reason ? reason : "No Reason Given"));
}

void 
send_wallops (char *who, char *buf)
{
	sts (":%s %s :%s", who, (me.token ? TOK_WALLOPS : MSG_WALLOPS), buf);
}

void
send_svshost (const char *who, const char *vhost)
{
	sts (":%s %s %s %s", me.name, (me.token ? TOK_CHGHOST : MSG_CHGHOST), who, vhost);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	sts (":%s %s %s %s", from, (me.token ? TOK_INVITE : MSG_INVITE), to, chan);
}

void
send_svsmode (const char *target, const char *modes)
{
	sts (":%s %s %s %s", me.name, (me.token ? TOK_SVSMODE : MSG_SVSMODE), target, modes);
}

void 
send_svskill (const char *target, const char *reason)
{
	sts (":%s %s %s :%s", me.name, (me.token ? TOK_SVSKILL : MSG_SVSKILL), target, reason);
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void 
send_akill (const char *host, const char *ident, const char *setby, const int length, const char *reason)
{
	sts (":%s %s + G %s %s %s %d %d :%s", me.name, (me.token ? TOK_TKL : MSG_TKL), ident, host, setby, (int)(me.now + length), (int)me.now, reason);
}

void 
send_rakill (const char *host, const char *ident)
{
	sts (":%s %s - G %s %s %s", me.name, (me.token ? TOK_TKL : MSG_TKL), ident, host, me.name);
}

void
send_privmsg (char *to, const char *from, char *buf)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_PRIVATE : MSG_PRIVATE), to, buf);
}

void
send_notice (char *to, const char *from, char *buf)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_NOTICE : MSG_NOTICE), to, buf);
}

void
send_globops (char *from, char *buf)
{
	sts (":%s %s :%s", from, (me.token ? TOK_GLOBOPS : MSG_GLOBOPS), buf);
}


static void
Srv_Protocol (char *origin, char **argv, int argc)
{
	int i;

    /*  Dodgy code to determine if PREFIX_AQ is enabled on the server 
	    we are connecting to. We should really parse CHANMODES properly
		but this is just an early test. Clients get sent CHPFIX which maps
		the mode to the sjoin char but this is enabled per server and 
		should be enabled on all servers if someone uses it <sigh>
		If not enabled we get something like:
	    R: PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 
	    VL SJ3 NS SJB64 CHANMODES=beqa,kfL,l,psmntirRcOAQKVGCuzNSM 
		When enabled, this changes to have CHANMODES=be,... so let's 
		look for that.
		*/
	for (i = 0; i < argc; i++) {
		if (strstr ("CHANMODES=be,", argv[i])) {
			chan_modes[3].sjoin = '&';
			chan_modes[4].sjoin = '~';
		}
	}
	ns_srv_protocol(origin, argv, argc);
}

static void
Usr_Stats (char *origin, char **argv, int argc)
{
	ns_usr_stats (origin, argv, argc);
}

static void
Usr_Version (char *origin, char **argv, int argc)
{
	ns_usr_version (origin, argv, argc);
}

static void
Usr_MOTD (char *origin, char **argv, int argc)
{
	ns_usr_motd (origin, argv, argc);
}

static void
Usr_Admin (char *origin, char **argv, int argc)
{
	ns_usr_admin (origin, argv, argc);
}

static void
Usr_Credits (char *origin, char **argv, int argc)
{
	ns_usr_credits (origin, argv, argc);
}

static void
Usr_Server (char *origin, char **argv, int argc)
{
	AddServer (argv[0], origin, atoi (argv[1]));
}

static void
Usr_Squit (char *origin, char **argv, int argc)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	SquitServer (argv[0], tmpbuf);
	free(tmpbuf);
}

static void
Usr_Quit (char *origin, char **argv, int argc)
{
	UserQuit (origin, NULL);
}

static void
Usr_Smode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
		/* its user svsmode change */
		UserMode (argv[0], argv[1]);
	} else {
		/* its a channel svsmode change */
		ChanMode (origin, argv, argc);
	}
}

/*  MODE
 *  :nick MODE nick :+modestring 
 *  :servername MODE #channel +modes parameter list TS 
 */
static void
Usr_Mode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
		UserMode (argv[0], argv[1]);
	} else {
		ChanMode (origin, argv, argc);
	}
}
static void
Usr_Kill (char *origin, char **argv, int argc)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	KillUser (argv[0], tmpbuf);
	free(tmpbuf);
}
static void
Usr_Vhost (char *origin, char **argv, int argc)
{
	SetUserVhost(origin, argv[0]);
}
static void
Usr_Pong (char *origin, char **argv, int argc)
{
	ns_usr_pong (origin, argv, argc);
}
static void
Usr_Away (char *origin, char **argv, int argc)
{
	char *buf;

	if (argc > 0) {
		buf = joinbuf (argv, argc, 0);
		UserAway (origin, buf);
		free (buf);
	} else {
		UserAway (origin, NULL);
	}
}
static void
Usr_Nick (char *origin, char **argv, int argc)
{
	UserNick (origin, argv[0]);
}

/* TOPIC #channel ownder TS :topic */
static void
Usr_Topic (char *origin, char **argv, int argc)
{
	char *buf;

	buf = joinbuf (argv, argc, 3);
	ChanTopic (argv[1], argv[0], atoi (argv[2]), buf);
	free (buf);
}

static void
Usr_Kick (char *origin, char **argv, int argc)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 2);
	kick_chan(argv[0], argv[1], origin, tmpbuf);
	free(tmpbuf);
}
static void
Usr_Join (char *origin, char **argv, int argc)
{
	UserJoin (origin, argv[0]);
}
static void
Usr_Part (char *origin, char **argv, int argc)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	part_chan (finduser (origin), argv[0], tmpbuf);
	free(tmpbuf);
}

static void
Srv_Ping (char *origin, char **argv, int argc)
{
	send_pong (argv[0]);
}

/*  NETINFO 
 *  argv[0] global_max 
 *  argv[1] TStime 
 *  argv[2] UnrealProtocol 
 *  argv[3] CLOAK_KEYCRC 
 *  argv[4] 0 
 *  argv[5] 0 
 *  argv[6] 0 
 *  argv[7] :netname
 */
static void
Srv_Netinfo (char *origin, char **argv, int argc)
{
	char *buf;
	ircd_srv.uprot = atoi (argv[2]);
	strlcpy (ircd_srv.cloak, argv[3], 10);
	buf = joinbuf (argv, argc, 7);
	strlcpy (me.netname, buf, MAXPASS);
	free (buf);
	send_netinfo ();
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
	ModuleEvent (EVENT_NETINFO, NULL, 0);
	me.synced = 1;
}

#ifdef UNREAL32
/*  EOS
 *  :servername EOS
 */
static void 
Usr_Eos (char *origin, char **argv, int argc)
{
    
}
#endif
    
/*    MSG_SJOIN creationtime chname    modebuf parabuf :member list */
/* R: ~         1073861298   #services +       <none>  :Mark */
static void
Srv_Sjoin (char *origin, char **argv, int argc)
{
	handle_sjoin (argv[1], argv[0], argv[2], 4, origin, argv, argc);
}

static void
Srv_Pass (char *origin, char **argv, int argc)
{
}
static void
Srv_Server (char *origin, char **argv, int argc)
{
	if (*origin == 0) {
		me.s = AddServer (argv[0], me.name, atoi (argv[1]));
	} else {
		me.s = AddServer (argv[0], origin, atoi (argv[1]));
	}
}

static void
Srv_Squit (char *origin, char **argv, int argc)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	SquitServer (argv[0], tmpbuf);
	free(tmpbuf);
}

static void
Srv_Nick (char *origin, char **argv, int argc)
{
	char *realname;

	realname = joinbuf (argv, argc, 7);
	AddUser (argv[0], argv[3], argv[4], realname, argv[5], 0, strtol (argv[2], NULL, 10));
	free (realname);
}

static void
Srv_Svsnick (char *origin, char **argv, int argc)
{
	if(UserNick (argv[0], argv[1]) == NS_FAILURE) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, SVSNICK for %s failed", argv[0]);
	}
}

static void
Srv_Kill (char *origin, char **argv, int argc)
{
	nlog (LOG_WARNING, LOG_CORE, "Got Srv_Kill, but its un-handled (%s)", recbuf);
}

