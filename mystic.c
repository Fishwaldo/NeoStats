/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
#include "mystic.h"
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
static void Srv_Ping (char *origin, char **argv, int argc);
static void Srv_Netinfo (char *origin, char **argv, int argc);
static void Srv_Pass (char *origin, char **argv, int argc);
static void Srv_Server (char *origin, char **argv, int argc);
static void Srv_Squit (char *origin, char **argv, int argc);
static void Srv_Nick (char *origin, char **argv, int argc);
static void Srv_Svsnick (char *origin, char **argv, int argc);
static void Srv_Kill (char *origin, char **argv, int argc);
static void Srv_Connect (char *origin, char **argv, int argc);
static void Srv_Vctrl (char *origin, char **argv, int argc);

static int vctrl_cmd ();

static char ircd_buf[BUFSIZE];

const char ircd_version[] = "(M)";
const char services_bot_modes[]= "+oS";
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
	{MSG_KILL, TOK_KILL, Usr_Kill, 1, 0},
	{MSG_PONG, TOK_PONG, Usr_Pong, 1, 0},
	{MSG_AWAY, TOK_AWAY, Usr_Away, 1, 0},
	{MSG_NICK, TOK_NICK, Usr_Nick, 1, 0},
	{MSG_TOPIC, TOK_TOPIC, Usr_Topic, 1, 0},
	{MSG_KICK, TOK_KICK, Usr_Kick, 1, 0},
	{MSG_JOIN, TOK_JOIN, Usr_Join, 1, 0},
	{MSG_PART, TOK_PART, Usr_Part, 1, 0},
	{MSG_PING, TOK_PING, Srv_Ping, 0, 0},
	{MSG_SNETINFO, TOK_SNETINFO, Srv_Netinfo, 0, 0},
	{MSG_VCTRL, TOK_VCTRL, Srv_Vctrl, 0, 0},
	{MSG_PASS, TOK_PASS, Srv_Pass, 0, 0},
	{MSG_SERVER, TOK_SERVER, Srv_Server, 0, 0},
	{MSG_SQUIT, TOK_SQUIT, Srv_Squit, 0, 0},
	{MSG_NICK, TOK_NICK, Srv_Nick, 0, 0},
	{MSG_SVSNICK, TOK_SVSNICK, Srv_Svsnick, 0, 0},
	{MSG_KILL, TOK_KILL, Srv_Kill, 0, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, Srv_Connect, 0, 0},
};

ChanModes chan_modes[] = {
	{MODE_CHANOP, 'o', 1, 0, '@'},
	{MODE_HALFOP, 'h', 1, 0, '%'},
	{MODE_VOICE, 'v', 1, 0, '+'},
	{MODE_BAN, 'b', 0, 1, 0},
	{MODE_EXCEPT, 'e', 0, 1, 0},
	{MODE_FLOODLIMIT, 'f', 0, 1, 0},			/* Flood limiter */
	{MODE_INVITEONLY, 'i', 0, 0, 0},
	{MODE_KEY, 'k', 0, 1, 0},
	{MODE_LIMIT, 'l', 0, 1, 0},
	{MODE_MODERATED, 'm', 0, 0, 0},
	{MODE_NOPRIVMSGS, 'n', 0, 0, 0},
	{MODE_PRIVATE, 'p', 0, 0, 0},
	{MODE_RGSTR, 'r', 0, 0, 0},
	{MODE_SECRET, 's', 0, 0, 0},
	{MODE_TOPICLIMIT, 't', 0, 0, 0},
	{MODE_NOCOLOR, 'x', 0, 0, 0},
	{MODE_ADMONLY, 'A', 0, 0, 0},
	{MODE_NOINVITE, 'I', 0, 0, 0},			/* no invites */
	{MODE_NOKNOCK, 'K', 0, 0, 0},			/* knock knock (no way!) */
	{MODE_LINK, 'L', 0, 1, 0},
	{MODE_OPERONLY, 'O', 0, 0, 0},
	{MODE_RGSTRONLY, 'R', 0, 0, 0},
	{MODE_STRIP, 'S', 0, 0, 0},			/* works? */
	{MODE_STRIPBADWORDS, 'U', 0, 0, 0},
};

UserModes user_umodes[] = {
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_SERVICESADMIN, 'P', NS_ULEVEL_ROOT},
	{UMODE_TECHADMIN, 'T', NS_ULEVEL_ADMIN},
	{UMODE_NETADMIN, 'N', NS_ULEVEL_ADMIN},
	{UMODE_SERVICESOPER, 'a', NS_ULEVEL_OPER},
	{UMODE_IRCADMIN, 'Z', NS_ULEVEL_OPER},
	{UMODE_COADMIN, 'z', NS_ULEVEL_OPER},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_SUPER, 'p', NS_ULEVEL_OPER},
	{UMODE_LOCOP, 'O', NS_ULEVEL_OPER},
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
	{UMODE_REGNICK, 'r', 0},
	{UMODE_HIDE, 'x', 0},
	{UMODE_WATCHER, 'W', 0},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

int
sserver_cmd (const char *name, const int numeric, const char *infoline)
{
	sts (":%s %s %s %d :%s", me.name, (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	return 1;
}

int
slogin_cmd (const char *name, const int numeric, const char *infoline, const char *pass)
{
	sts ("%s %s", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts ("%s %s %d :%s", (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	return 1;
}

int
ssquit_cmd (const char *server)
{
	sts ("%s %s", (me.token ? TOK_SQUIT : MSG_SQUIT), server);
	return 1;
}

int
sprotocol_cmd (const char *option)
{
	sts ("%s %s", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL), option);
	return 1;
}

int
squit_cmd (const char *who, const char *quitmsg)
{
	sts (":%s %s :%s", who, (me.token ? TOK_QUIT : MSG_QUIT), quitmsg);
	UserQuit (who, quitmsg);
	return 1;
}

int
spart_cmd (const char *who, const char *chan)
{
	sts (":%s %s %s", who, (me.token ? TOK_PART : MSG_PART), chan);
	part_chan (finduser (who), (char *) chan);
	return 1;
}

int
sjoin_cmd (const char *who, const char *chan)
{
	sts (":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
	join_chan (finduser (who), (char *) chan);
	return 1;
}

int
schmode_cmd (const char *who, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;

	sts (":%s %s %s %s %s %lu", me.name, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, me.now);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode ("", av, ac);
	free (av);
	return 1;
}

int
snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname, long mode)
{
	sts ("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, me.now, ident, host, me.name, realname);
	AddUser (nick, ident, host, realname, me.name, 0, me.now);
	return 1;
}

int
sping_cmd (const char *from, const char *reply, const char *to)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_PING : MSG_PING), reply, to);
	return 1;
}

int
sumode_cmd (const char *who, const char *target, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	sts (":%s %s %s :%s", who, (me.token ? TOK_MODE : MSG_MODE), target, newmode);
	UserMode (target, newmode);
	return 1;
}

void 
send_numeric (const int numeric, const char *target, const char *buf)
{
	sts (":%s %d %s :%s", me.name, numeric, target, buf);
}

int
spong_cmd (const char *reply)
{
	sts ("%s %s", (me.token ? TOK_PONG : MSG_PONG), reply);
	return 1;
}

int
snetinfo_cmd ()
{
	sts (":%s %s 0 %d %d %s 0 0 0 :%s", me.name, MSG_SNETINFO, (int)me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname);
	return 1;
}

int
vctrl_cmd ()
{
	sts ("%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, ircd_srv.uprot, ircd_srv.nicklg, ircd_srv.modex, ircd_srv.gc, me.netname);
	return 1;
}

int
skill_cmd (const char *from, const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, ircd_buf);
	UserQuit (target, ircd_buf);
	return 1;
}

int
ssvskill_cmd (const char *who, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s :%s", me.name, MSG_SVSKILL, who, ircd_buf);
	return 1;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	notice (s_Services, "Warning, Module %s tried to SMO, which is not supported in Mystic", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SMO, which is not supported in Mystic", segvinmodule);
	return 1;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	UserNick (oldnick, newnick);
	sts (":%s %s %s %d", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, (int)me.now);
	return 1;
}

int
sswhois_cmd (const char *target, const char *swhois)
{
	notice (s_Services, "Warning Module %s tried to SWHOIS, which is not supported in Mystic", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SWHOIS, which is not supported in Mystic", segvinmodule);
	return 1;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
	sts ("%s %s %s :%d", (me.token ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, (int)me.now);
	return 1;
}

int
ssvsjoin_cmd (const char *target, const char *chan)
{
	sts ("%s %s %s", (me.token ? TOK_SVSJOIN : MSG_SVSJOIN), target, chan);
	return 1;
}

int
ssvspart_cmd (const char *target, const char *chan)
{
	sts ("%s %s %s", (me.token ? TOK_SVSPART : MSG_SVSPART), target, chan);
	return 1;
}

int
skick_cmd (const char *who, const char *target, const char *chan, const char *reason)
{
	sts (":%s %s %s %s :%s", who, (me.token ? TOK_KICK : MSG_KICK), chan, target, (reason ? reason : "No Reason Given"));
	part_chan (finduser (target), (char *) chan);
	return 1;
}

void send_wallops (char *who, char *buf)
{
	sts (":%s %s :%s", who, (me.token ? TOK_WALLOPS : MSG_WALLOPS), ircd_buf);
}

int
ssvshost_cmd (const char *who, const char *vhost)
{
	User *u;

	u = finduser (who);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Can't Find user %s for ssvshost_cmd", who);
		return 0;
	}
	strlcpy (u->vhost, vhost, MAXHOST);
	sts (":%s %s %s %s", me.name, MSG_CHGHOST, who, vhost);
	return 1;
}
int 
sinvite_cmd (const char *from, const char *to, const char *chan) {
	sts (":%s %s %s %s", from, MSG_INVITE, to, chan);
	return 1;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s@%s %d %d %s :%s", me.name, MSG_GLINE, ident, host, (int)(me.now + length), (int)me.now, setby, ircd_buf);
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
	/* ultimate2 needs an oper to remove */
	sts (":%s %s :%s@%s", s_Services, MSG_REMGLINE, host, ident);
	return 1;
}

int
ssvinfo_cmd ()
{
	sts ("SVINFO 5 3 0 :%d", (int)me.now);
	return 1;
}

int
sburst_cmd (int b)
{
	if (b == 0) {
		sts ("BURST 0");
	} else {
		sts ("BURST");
	}
	return 1;
}

void
chan_privmsg (char *who, char *buf)
{
	sts (":%s %s %s :%s", who, (me.token ? TOK_PRIVATE : MSG_PRIVATE), me.chan, buf);
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
Srv_Connect (char *origin, char **argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcasecmp ("TOKEN", argv[i])) {
			me.token = 1;
		}
	}
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
	DelServer (argv[0]);
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
static void
Usr_Mode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
		nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[0]);
		UserMode (argv[0], argv[1]);
	} else {
		ChanMode (origin, argv, argc);
	}
}
static void
Usr_Kill (char *origin, char **argv, int argc)
{
	KillUser (argv[0]);
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
	char *Buf;
	User *u = finduser (origin);
	if (u) {
		if (argc > 0) {
			Buf = joinbuf (argv, argc, 0);
		} else {
			Buf = NULL;
		}
		UserAway (u, Buf);
		if (argc > 0) {
			free (Buf);
		}
	} else {
		nlog (LOG_NOTICE, LOG_CORE, "Warning, Unable to find User %s for Away", origin);
	}
}
static void
Usr_Nick (char *origin, char **argv, int argc)
{
	UserNick (origin, argv[0]);
}
static void
Usr_Topic (char *origin, char **argv, int argc)
{
	char *buf;
	Chans *c;
	c = findchan (argv[0]);
	if (c) {
		buf = joinbuf (argv, argc, 3);
		ChangeTopic (argv[1], c, atoi (argv[2]), buf);
		free (buf);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Ehhh, Can't find Channel %s", argv[0]);
	}

}

static void
Usr_Kick (char *origin, char **argv, int argc)
{
	kick_chan(argv[0], argv[1], origin);
}
static void
Usr_Join (char *origin, char **argv, int argc)
{
	char *s, *t;
	t = argv[0];
	while (*(s = t)) {
		t = s + strcspn (s, ",");
		if (*t)
			*t++ = 0;
		join_chan (finduser (origin), s);
	}
}
static void
Usr_Part (char *origin, char **argv, int argc)
{
	part_chan (finduser (origin), argv[0]);
}

static void
Srv_Ping (char *origin, char **argv, int argc)
{
	spong_cmd (argv[0]);
}

static void
Srv_Vctrl (char *origin, char **argv, int argc)
{
	ircd_srv.uprot = atoi (argv[0]);
	ircd_srv.nicklg = atoi (argv[1]);
	ircd_srv.modex = atoi (argv[2]);
	ircd_srv.gc = atoi (argv[3]);
	strlcpy (me.netname, argv[14], MAXPASS);
	vctrl_cmd ();
}

static void
Srv_Netinfo (char *origin, char **argv, int argc)
{
	me.onchan = 1;
	ircd_srv.uprot = atoi (argv[2]);
	strlcpy (ircd_srv.cloak, argv[3], 10);
	strlcpy (me.netname, argv[7], MAXPASS);

	snetinfo_cmd ();
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
#ifdef DEBUG
	me.debug_mode = 1;
#endif
	ModuleEvent (EVENT_NETINFO, NULL, 0);
	me.synced = 1;
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
	SquitServer(argv[0]);
}

static void
Srv_Nick (char *origin, char **argv, int argc)
{
	char *realname;

	realname = joinbuf (argv, argc, 7);
	AddUser (argv[0], argv[3], argv[4], realname, argv[5], 0, strtoul (argv[2], NULL, 10));
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
}



int
SignOn_NewBot (const char *nick, const char *ident, const char *host, const char *rname, long Umode)
{
	snewnick_cmd (nick, ident, host, rname, Umode);
	sumode_cmd (nick, nick, Umode);
	if ((me.allbots > 0) || (Umode & services_bot_umode)) {
		sjoin_cmd (nick, me.chan);
		schmode_cmd (nick, me.chan, "+o", nick);
		/* all bots join */
	}
	return 1;
}
