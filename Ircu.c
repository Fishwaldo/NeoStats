/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
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
#include "Ircu.h"
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
static void Srv_Kill (char *origin, char **argv, int argc);
static void Srv_Connect (char *origin, char **argv, int argc);
static void Srv_Svinfo (char *origin, char **argv, int argc);
static void Srv_Burst (char *origin, char **argv, int argc);
static void Srv_Sjoin (char *origin, char **argv, int argc);

static char ircd_buf[BUFSIZE];

const char ircd_version[] = "(IRCU)";
const char services_bot_modes[]= "+oS";

/* this is the command list and associated functions to run */
IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, Usr_Stats, 1, 0}
	,
	{MSG_VERSION, Usr_Version, 1, 0}
	,
	{MSG_MOTD, Usr_MOTD, 1, 0}
	,
	{MSG_ADMIN, Usr_Admin, 1, 0}
	,
	{MSG_CREDITS, Usr_Credits, 1, 0}
	,
	{MSG_SERVER, Usr_Server, 1, 0}
	,
	{MSG_SQUIT, Usr_Squit, 1, 0}
	,
	{MSG_QUIT, Usr_Quit, 1, 0}
	,
	{MSG_MODE, Usr_Mode, 1, 0}
	,
	{MSG_KILL, Usr_Kill, 1, 0}
	,
	{MSG_PONG, Usr_Pong, 1, 0}
	,
	{MSG_AWAY, Usr_Away, 1, 0}
	,
	{MSG_NICK, Usr_Nick, 1, 0}
	,
	{MSG_TOPIC, Usr_Topic, 1, 0}
	,
	{MSG_TOPIC, Usr_Topic, 0, 0}
	,
	{MSG_KICK, Usr_Kick, 1, 0}
	,
	{MSG_JOIN, Usr_Join, 1, 0}
	,
	{MSG_PART, Usr_Part, 1, 0}
	,
	{MSG_PING, Srv_Ping, 0, 0}
	,
	{MSG_SVINFO, Srv_Svinfo, 0, 0}
	,
	{MSG_PASS, Srv_Pass, 0, 0}
	,
	{MSG_SERVER, Srv_Server, 0, 0}
	,
	{MSG_SQUIT, Srv_Squit, 0, 0}
	,
	{MSG_NICK, Srv_Nick, 0, 0}
	,
	{MSG_KILL, Srv_Kill, 0, 0}
	,
	{MSG_EOB, Srv_Burst, 1, 0}
	,
	{MSG_SJOIN, Srv_Sjoin, 1, 0}
	,
	{NULL, NULL, 0, 0}
};

aCtab cFlagTab[] = {
	{MODE_HALFOP, 'h', 1, 0, '%'}
	,
	{MODE_CHANOP, 'o', 1, 0, '@'}
	,
	{MODE_VOICE, 'v', 1, 0, '+'}
	,
	{MODE_SECRET, 's', 0, 0, 0}
	,
	{MODE_PRIVATE, 'p', 0, 0, 0}
	,
	{MODE_MODERATED, 'm', 0, 0, 0}
	,
	{MODE_TOPICLIMIT, 't', 0, 0, 0}
	,
	{MODE_INVITEONLY, 'i', 0, 0, 0}
	,
	{MODE_NOPRIVMSGS, 'n', 0, 0, 0}
	,
	{MODE_HIDEOPS, 'a', 0, 0, 0}
	,
	{MODE_LIMIT, 'l', 0, 1, 0}
	,
	{MODE_KEY, 'k', 0, 1, 0}
	,
	{MODE_BAN, 'b', 0, 1, 0}
	,
	{MODE_EXCEPT, 'e', 0, 1, 0}
	,
	{MODE_INVEX, 'I', 0, 1, 0}
	,
	{0x0, 0x0, 0x0, 0x0, 0x0}
};


Oper_Modes usr_mds[] = {
	{UMODE_OPER, 'o', 50}
	,
	{UMODE_ADMIN, 'A', 190}
	,
	{UMODE_BOTS, 'b', 0}
	,
	{UMODE_CCONN, 'c', 0}
	,
	{UMODE_DEBUG, 'd', NS_ULEVEL_ROOT}
	,
	{UMODE_FULL, 'f', 0}
	,
	{UMODE_CALLERID, 'g', 0}
	,
	{UMODE_INVISIBLE, 'i', 0}
	,
	{UMODE_SKILL, 'k', 0}
	,
	{UMODE_LOCOPS, 'l', 40}
	,
	{UMODE_NCHANGE, 'n', 0}
	,
	{UMODE_REJ, 'r', 0}
	,
	{UMODE_SERVNOTICE, 's', 0}
	,
	{UMODE_UNAUTH, 'u', 0}
	,
	{UMODE_WALLOP, 'w', 0}
	,
	{UMODE_EXTERNAL, 'x', 0}
	,
	{UMODE_SPY, 'y', 0}
	,
	{UMODE_OPERWALL, 'z', 0}
	,
	{UMODE_SERVICES, 'S', 0}
	,
	{0, 0, 0}
};

void
init_ircd ()
{
	/* count the number of commands */
	ircd_srv.cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])) - 1);
	ircd_srv.umodecount = ((sizeof (usr_mds) / sizeof (usr_mds[0])) - 1);
};

int
seob_cmd (const char *server)
{
	sts (":%s %s", server, MSG_EOB);
	return 1;
}

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
	sts ("CAPAB :TS EX CHW IE EOB KLN GLN KNOCK HOPS HUB AOPS MX");
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

int
squit_cmd (const char *who, const char *quitmsg)
{
	sts (":%s %s :%s", who, MSG_QUIT, quitmsg);
	DelUser (who);
	return 1;
}

int
spart_cmd (const char *who, const char *chan)
{
	sts (":%s %s %s", who, MSG_PART, chan);
	part_chan (finduser (who), (char *) chan);
	return 1;
}

int
sjoin_cmd (const char *who, const char *chan)
{
	sts (":%s %s 0 %s + :%s", me.name, MSG_SJOIN, chan, who);
	join_chan (finduser (who), (char *) chan);
	return 1;
}

int
schmode_cmd (const char *who, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;

	sts (":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, me.now);
	ircsnprintf (ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode ("", av, ac);
	free (av);
	return 1;
}


int
snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	sts ("%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, me.now, newmode, ident, host, me.name, realname);
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

int
sumode_cmd (const char *who, const char *target, long mode)
{
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	sts (":%s %s %s :%s", who, MSG_MODE, target, newmode);
	UserMode (target, newmode);
	return 1;
}

int
snumeric_cmd (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;

	va_start (ap, data);
	ircvsnprintf (ircd_buf, BUFSIZE, data, ap);
	va_end (ap);
	sts (":%s %d %s :%s", me.name, numeric, target, ircd_buf);
	return 1;
}

int
spong_cmd (const char *reply)
{
	sts ("%s %s", MSG_PONG, reply);
	return 1;
}

int
skill_cmd (const char *from, const char *target, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s :%s", from, MSG_KILL, target, ircd_buf);
	DelUser (target);
	return 1;
}

int
ssvskill_cmd (const char *who, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s :%s", me.name, MSG_KILL, who, ircd_buf);
/* hybrid doesn't have svskill, so this is handled just like a normal kill */
	DelUser (who);
	return 1;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	notice (s_Services, "Warning, Module %s tried to SMO, which is not supported in Hybrid", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SMO, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	UserNick (oldnick, newnick);
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, (int) me.now);
	return 1;
}

int
sswhois_cmd (const char *target, const char *swhois)
{
	notice (s_Services, "Warning Module %s tried to SWHOIS, which is not supported in Hybrid", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SWHOIS, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
	notice (s_Services, "Warning Module %s tried to SVSNICK, which is not supported in Hybrid", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSNICK, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int
ssvsjoin_cmd (const char *target, const char *chan)
{
	notice (s_Services, "Warning Module %s tried to SJOIN, which is not supported in Hybrid", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SJOIN, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int
ssvspart_cmd (const char *target, const char *chan)
{
	notice (s_Services, "Warning Module %s tried to SVSPART, which is not supported in Hybrid", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSPART, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int
skick_cmd (const char *who, const char *target, const char *chan, const char *reason)
{
	sts (":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
	part_chan (finduser (target), (char *) chan);
	return 1;
}

int
swallops_cmd (const char *who, const char *msg, ...)
{
	va_list ap;

	va_start (ap, msg);
	ircvsnprintf (ircd_buf, BUFSIZE, msg, ap);
	va_end (ap);
	sts (":%s %s :%s", who, MSG_WALLOPS, ircd_buf);
	return 1;
}

int
ssvshost_cmd (const char *who, const char *vhost)
{
	notice (s_Services, "Warning Module %s tried to SVSHOST, which is not supported in Hybrid", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SVSHOST, which is not supported in Hybrid", segvinmodule);
	return 1;
}

int
ssvinfo_cmd ()
{
	sts ("SVINFO 5 3 0 :%d", (int) me.now);
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

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	/* there isn't an akill on Hybrid, so we send a kline to all servers! */
	hscan_t ss;
	hnode_t *sn;
	Server *s;
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);

	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		sts (":%s %s %s %lu %s %s :%s", setby, MSG_KLINE, s->name, (unsigned long)length, ident, host, ircd_buf);
	}
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
	chanalert (s_Services, "Please Manually remove KLINES using /unkline on each server");
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
	if (argc <= 2) {
		modes = argv[1];
	} else {
		modes = argv[2];
	}

	if (*modes == '#') {
		join_chan (finduser (argv[4]), modes);
		return;
	}
	tl = list_create (10);

	if (*modes != '+') {
		goto nomodes;
	}
	while (*modes) {
		for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
			if (*modes == cFlagTab[i].flag) {
				if (cFlagTab[i].parameters) {
					m = smalloc (sizeof (ModesParm));
					m->mode = cFlagTab[i].mode;
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
					mode1 |= cFlagTab[i].mode;
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
			for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
				if (cFlagTab[i].sjoin != 0) {
					if (*modes == cFlagTab[i].sjoin) {
						mode |= cFlagTab[i].mode;
						modes++;
					}
				}
			}
			strlcpy (nick, modes, MAXNICK);
			ok = 0;
			break;
		}
		join_chan (finduser (nick), argv[1]);
		ChangeChanUserMode (findchan (argv[1]), finduser (nick), 1, mode);
		j++;
		ok = 1;
	}
	c = findchan (argv[1]);
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
	seob_cmd (me.name);
	init_services_bot ();
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
	DelUser (origin);
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
	User *u;
	u = finduser (origin);
	if (u) {
		strlcpy (u->vhost, argv[0], MAXHOST);
	}
}
static void
Usr_Pong (char *origin, char **argv, int argc)
{
	ns_usr_pong (origin, argv, argc);
}
static void
Usr_Away (char *origin, char **argv, int argc)
{
	User *u = finduser (origin);
	char *buf;
	if (u) {
		if (argc > 0) {
			buf = joinbuf (argv, argc, 0);
		} else {
			buf = NULL;
		}
		UserAway (u, buf);
		if (argc > 0) {
			free (buf);
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
		buf = joinbuf (argv, argc, 2);
		ChangeTopic (origin, c, me.now, buf);
		free (buf);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Ehhh, Can't find Channel %s", argv[0]);
	}

}

static void
Usr_Kick (char *origin, char **argv, int argc)
{
	User *u, *k;
	u = finduser (argv[1]);
	k = finduser (origin);
	if (u) {
		kick_chan (u, argv[0], k);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Can't find user %s for kick %s", argv[1], argv[0]);
	}
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
Srv_Svinfo (char *origin, char **argv, int argc)
{
	ssvinfo_cmd ();
}

static void
Srv_Netinfo (char *origin, char **argv, int argc)
{
	me.onchan = 1;
	ircd_srv.uprot = atoi (argv[2]);
	strlcpy (ircd_srv.cloak, argv[3], 10);
	strlcpy (me.netname, argv[7], MAXPASS);
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
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
	Server *s;
	s = findserver (argv[0]);
	if (s) {
		DelServer (argv[0]);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Squit from Unknown Server %s", argv[0]);
	}

}

static void
Srv_Nick (char *origin, char **argv, int argc)
{
	char *realname;

	realname = joinbuf (argv, argc, 7);
	AddUser (argv[0], argv[4], argv[5], realname, argv[6], 0, strtoul (argv[2], NULL, 10));
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3]);
}

static void
Srv_Kill (char *origin, char **argv, int argc)
{
}

int
SignOn_NewBot (const char *nick, const char *user, const char *host, const char *rname, long Umode)
{
	snewnick_cmd (nick, user, host, rname, Umode);

	if ((me.allbots > 0) || (Umode & UMODE_SERVICES)) {
		sjoin_cmd (nick, me.chan);
		schmode_cmd (me.name, me.chan, "+o", nick);
	}
	return 1;
}
