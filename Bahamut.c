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
#include "Bahamut.h"
#include "dl.h"
#include "log.h"
#include "server.h"
#include "chans.h"
#include "users.h"

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
static void Srv_Ping (char *origin, char **argv, int argc);
static void Srv_Pass (char *origin, char **argv, int argc);
static void Srv_Server (char *origin, char **argv, int argc);
static void Srv_Squit (char *origin, char **argv, int argc);
static void Srv_Nick (char *origin, char **argv, int argc);
static void Srv_Svsnick (char *origin, char **argv, int argc);
static void Srv_Kill (char *origin, char **argv, int argc);
static void Srv_Connect (char *origin, char **argv, int argc);
static void Srv_Svinfo (char *origin, char **argv, int argc);
static void Srv_Burst (char *origin, char **argv, int argc);
static void Srv_Sjoin (char *origin, char **argv, int argc);

static char ircd_buf[BUFSIZE];

const char ircd_version[] = "(B)";
const char services_bot_modes[]= "+oS";
long services_bot_umode= 0;

IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, Usr_Stats, 1, 0},
	{MSG_VERSION, Usr_Version, 1, 0},
	{MSG_MOTD, Usr_MOTD, 1, 0},
	{MSG_ADMIN, Usr_Admin, 1, 0},
	{MSG_CREDITS, Usr_Credits, 1, 0},
	{MSG_SERVER, Usr_Server, 1, 0},
	{MSG_SQUIT, Usr_Squit, 1, 0},
	{MSG_QUIT, Usr_Quit, 1, 0},
	{MSG_MODE, Usr_Mode, 1, 0},
	{MSG_SVSMODE, Usr_Smode, 1, 0},
	{MSG_KILL, Usr_Kill, 1, 0},
	{MSG_PONG, Usr_Pong, 1, 0},
	{MSG_AWAY, Usr_Away, 1, 0},
	{MSG_NICK, Usr_Nick, 1, 0},
	{MSG_TOPIC, Usr_Topic, 1, 0},
	{MSG_KICK, Usr_Kick, 1, 0},
	{MSG_JOIN, Usr_Join, 1, 0},
	{MSG_PART, Usr_Part, 1, 0},
	{MSG_PING, Srv_Ping, 0, 0},
	{MSG_SVINFO, Srv_Svinfo, 0, 0},
	{MSG_CAPAB, Srv_Connect, 0, 0},
	{MSG_BURST, Srv_Burst, 0, 0},
	{MSG_SJOIN, Srv_Sjoin, 1, 0},
	{MSG_PASS, Srv_Pass, 0, 0},
	{MSG_SERVER, Srv_Server, 0, 0},
	{MSG_SQUIT, Srv_Squit, 0, 0},
	{MSG_NICK, Srv_Nick, 0, 0},
	{MSG_SVSNICK, Srv_Svsnick, 0, 0},
	{MSG_KILL, Srv_Kill, 0, 0},
};

ChanModes chan_modes[] = {
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_BAN, 'b', 0, 1, 0},
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
	{CMODE_OPERONLY, 'O', 0, 0, 0},
	{CMODE_RGSTRONLY, 'R', 0, 0, 0},
};

UserModes user_umodes[] = {
	{UMODE_SERVICESADMIN, 'a', NS_ULEVEL_ROOT},
	{UMODE_SERVADMIN, 'A', NS_ULEVEL_ADMIN},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_REGNICK, 'r', 0},
	{UMODE_REGONLY, 'R', 0},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

int
sserver_cmd (const char *name, const int numeric, const char *infoline)
{
	sts (":%s %s %s %d :%s", me.name, MSG_SERVER, name, numeric, infoline);
	return 1;
}

int
slogin_cmd (const char *name, const int numeric, const char *infoline, const char *pass)
{
	sts ("%s %s :TS", MSG_PASS, pass);
	sts ("CAPAB TS3 SSJOIN BURST NICKIP");
	sts ("%s %s %d :%s", MSG_SERVER, name, numeric, infoline);
	return 1;
}

int
ssquit_cmd (const char *server)
{
	sts ("%s %s", MSG_SQUIT, server);
	return 1;
}

int
sprotocol_cmd (const char *option)
{
	return 1;
}

void 
send_quit (const char *who, const char *quitmsg)
{
	sts (":%s %s :%s", who, MSG_QUIT, quitmsg);
}

void 
send_part (const char *who, const char *chan)
{
	sts (":%s %s %s", who, MSG_PART, chan);
}

int
sjoin_cmd (const char *who, const char *chan, unsigned long chflag)
{
	char flag;
	char mode;
	char **av;
	int ac;
	time_t tstime;
	Chans *c;

	c = findchan ((char *) chan);
	if (!c) {
		tstime = me.now;
	} else {
		tstime = c->tstime;
	}
	switch (chflag) {
	case CMODE_CHANOP:
		flag = '@';
		mode= 'o';
		break;
	case CMODE_VOICE:
		flag = '+';
		mode= 'v';
		break;
	default:
		flag = ' ';
		mode= '\0';
	}
	sts (":%s %s %d %s + :%c%s", me.name, MSG_SJOIN, (int)tstime, chan, flag, who);
	join_chan (who, chan);
	ircsnprintf (ircd_buf, BUFSIZE, "%s +%c %s", chan, mode, who);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);
	return 1;
}

void 
send_cmode (const char *who, const char *chan, const char *mode, const char *args)
{
	sts (":%s %s %s %s %s %lu", me.name, MSG_MODE, chan, mode, args, me.now);
}

int
snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	sts ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, me.now, newmode, ident, host, me.name, me.now, realname);
	AddUser (nick, ident, host, realname, me.name, 0, me.now);
	UserMode (nick, newmode);
	return 1;
}

int
sping_cmd (const char *from, const char *reply, const char *to)
{
	sts (":%s %s %s :%s", from, MSG_PING, reply, to);
	return 1;
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	sts (":%s %s %s :%s", who, MSG_MODE, target, mode);
}

void 
send_numeric (const int numeric, const char *target, const char *buf)
{
	sts (":%s %d %s :%s", me.name, numeric, target, ircd_buf);
}

int
spong_cmd (const char *reply)
{
	sts ("%s %s", MSG_PONG, reply);
	return 1;
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	sts (":%s %s %s :%s", from, MSG_KILL, target, reason);	
}

void 
send_svskill (const char *target, const char *reason)
{
	sts (":%s %s %s :%s", me.name, MSG_SVSKILL, who, reason);
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	chanalert (s_Services, "Warning, Module %s tried to SMO, which is not supported in Bahamut", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SMO, which is not supported in Bahamut", segvinmodule);
	return 1;
}

void 
send_nick (const char *oldnick, const char *newnick)
{
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, (int)me.now);
}

int
sswhois_cmd (const char *target, const char *swhois)
{
	chanalert (s_Services, "Warning Module %s tried to SWHOIS, which is not supported in Bahamut", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SWHOIS, which is not supported in Bahamut", segvinmodule);
	return 1;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
	sts ("%s %s %s :%d", MSG_SVSNICK, target, newnick, (int)me.now);
	return 1;
}

int
ssvsjoin_cmd (const char *target, const char *chan)
{
	chanalert (s_Services, "Warning Module %s tried to SVSJOIN, which is not supported in Bahamut", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSJOIN, which is not supported in Bahamut", segvinmodule);
	return 1;
}

int
ssvspart_cmd (const char *target, const char *chan)
{
	chanalert (s_Services, "Warning Module %s tried to SVSPART, which is not supported in Bahamut", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSPART, which is not supported in Bahamut", segvinmodule);
	return 1;
}

int
skick_cmd (const char *who, const char *target, const char *chan, const char *reason)
{
	sts (":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
	part_chan (finduser (target), (char *) chan);
	return 1;
}

void send_wallops (char *who, char *buf)
{
	sts (":%s %s :%s", who, MSG_WALLOPS, buf);
}

int
ssvshost_cmd (const char *who, const char *vhost)
{
	chanalert (s_Services, "Warning Module %s tried to SVSHOST, which is not supported in Bahamut", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSHOST, which is not supported in Bahamut", segvinmodule);
	return 1;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s %s %d %s %d :%s", me.name, MSG_AKILL, host, ident, length, setby, (int)me.now, ircd_buf);
	return 1;
}

int 
sinvite_cmd (const char *from, const char *to, const char *chan) {
	sts (":%s %s %s %s", from, MSG_INVITE, to, chan);
	return 1;
}


int
srakill_cmd (const char *host, const char *ident)
{
	sts (":%s %s %s %s", me.name, MSG_RAKILL, host, ident);
	return 1;
}


int
ssvinfo_cmd ()
{
	sts ("SVINFO 3 3 0 :%d", (int)me.now);
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
	sts (":%s %s %s :%s", who, MSG_PRIVATE, me.chan, buf);
}

void
send_privmsg (char *to, const char *from, char *buf)
{
	sts (":%s %s %s :%s", from, MSG_PRIVATE, to, buf);
}

void
send_notice (char *to, const char *from, char *buf)
{
	sts (":%s %s %s :%s", from, MSG_NOTICE, to, buf);
}

void
send_globops (char *from, char *buf)
{
	sts (":%s %s :%s", from, MSG_GLOBOPS, buf);
}

static void
Srv_Sjoin (char *origin, char **argv, int argc)
{
	char nick[MAXNICK];
	long mode = 0;
	long mode1 = 0;
	char *modes;
	int ok = 1, i, j = 3;
	ModesParm *m;
	Chans *c;
	lnode_t *mn = NULL;
	list_t *tl;
	if (argc > 4) {
		modes = argv[2];
	} else {
		modes = argv[1];
	}
	if (*modes == '#') {
		join_chan (origin, modes);
		return;
	}
	tl = list_create (10);
	if (*modes != '+') {
		goto nomodes;
	}
	while (*modes) {
		for (i = 0; i < ircd_cmodecount; i++) {
			if (*modes == chan_modes[i].flag) {
				if (chan_modes[i].parameters) {
					m = smalloc (sizeof (ModesParm));
					m->mode = chan_modes[i].mode;
					strlcpy (m->param, argv[j], PARAMSIZE);
					mn = lnode_create (m);
					if (!list_isfull (tl)) {
						list_append (tl, mn);
					} else {
						nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, tl list is full in Svr_Sjoin(ircd.c)");
						do_exit (NS_EXIT_ERROR, "List full - see log file");
					}
					j++;
				} else {
					mode1 |= chan_modes[i].mode;
				}
			}
		}
		modes++;
	}
      nomodes:
	while (argc > j) {
		modes = argv[j];
		mode = 0;
		while (ok == 1) {
			for (i = 0; i < ircd_cmodecount; i++) {
				if (chan_modes[i].sjoin != 0) {
					if (*modes == chan_modes[i].sjoin) {
						mode |= chan_modes[i].mode;
						modes++;
						i = -1;
					}
				} else {
					/* sjoin's should be at the top of the list */
					ok = 0;
					strlcpy (nick, modes, MAXNICK);
					break;
				}
			}
		}
		join_chan (nick, argv[1]);
		ChangeChanUserMode (findchan (argv[1]), finduser (nick), 1, mode);
		j++;
		ok = 1;
	}
	c = findchan (argv[1]);
	/* update the TS time */
	ChangeChanTS (c, atoi (argv[0]));
	c->modes |= mode1;
	if (!list_isempty (tl)) {
		if (!list_isfull (c->modeparms)) {
			list_transfer (c->modeparms, tl, list_first (tl));
		} else {
			/* eeeeeeek, list is full! */
			nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, c->modeparms list is full in Svr_Sjoin(ircd.c)");
			do_exit (NS_EXIT_ERROR, "List full - see log file");
		}
	}
	list_destroy (tl);
}

static void
Srv_Burst (char *origin, char **argv, int argc)
{
	if (argc > 0) {
		if (ircd_srv.burst == 1) {
			sburst_cmd (0);
			ircd_srv.burst = 0;
			me.synced = 1;
			init_services_bot ();
		}
	} else {
		ircd_srv.burst = 1;
	}
}

static void
Srv_Connect (char *origin, char **argv, int argc)
{
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
		UserMode (argv[0], argv[2]);
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

	buf = joinbuf (argv, argc, 3);
	ChanTopic (argv[1], argv[0], atoi (argv[2]), buf);
	free (buf);
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
		join_chan (origin, s);
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
	if (ircd_srv.burst) {
		sping_cmd (me.name, argv[0], argv[0]);
	}
}

static void
Srv_Svinfo (char *origin, char **argv, int argc)
{
	ssvinfo_cmd ();
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

	realname = joinbuf (argv, argc, 9);
	AddUser (argv[0], argv[4], argv[5], realname, argv[6], strtoul (argv[8], NULL, 10), strtoul (argv[2], NULL, 10));
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3]);
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
SignOn_NewBot (const char *nick, const char *user, const char *host, const char *rname, long Umode)
{

	snewnick_cmd (nick, user, host, rname, Umode);
	if ((me.allbots > 0) || (Umode & services_bot_umode)) {
		sjoin_cmd (nick, me.chan, CMODE_CHANOP);
		/* all bots join */
	}
	return 1;
}
