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
#include "hybrid7.h"
#include "dl.h"
#include "log.h"
#include "users.h"
#include "server.h"
#include "chans.h"

static void Usr_Version (char *origin, char **argv, int argc);
static void Usr_ShowMOTD (char *origin, char **argv, int argc);
static void Usr_ShowADMIN (char *origin, char **argv, int argc);
static void Usr_Showcredits (char *origin, char **argv, int argc);
static void Usr_AddServer (char *origin, char **argv, int argc);
static void Usr_DelServer (char *origin, char **argv, int argc);
static void Usr_DelUser (char *origin, char **argv, int argc);
static void Usr_Mode (char *origin, char **argv, int argc);
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
static void Srv_Kill (char *origin, char **argv, int argc);
static void Srv_Svinfo (char *origin, char **argv, int argc);
static void Srv_Burst (char *origin, char **argv, int argc);
static void Srv_Sjoin (char *origin, char **argv, int argc);

static char ircd_buf[BUFSIZE];

const char ircd_version[] = "(H)";
const char services_bot_modes[]= "+o";

/* this is the command list and associated functions to run */
IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, Usr_Stats, 1, 0}
	,
	{MSG_VERSION, Usr_Version, 1, 0}
	,
	{MSG_MOTD, Usr_ShowMOTD, 1, 0}
	,
	{MSG_ADMIN, Usr_ShowADMIN, 1, 0}
	,
	{MSG_CREDITS, Usr_Showcredits, 1, 0}
	,
	{MSG_SERVER, Usr_AddServer, 1, 0}
	,
	{MSG_SQUIT, Usr_DelServer, 1, 0}
	,
	{MSG_QUIT, Usr_DelUser, 1, 0}
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
	sts (":%s %s %d %s + :%s", me.name, MSG_SJOIN, me.now, chan, who);
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
	int i, j;
	char newmode[20];
	newmode[0] = '+';
	j = 1;
	for (i = 0; i < ((sizeof (usr_mds) / sizeof (usr_mds[0])) - 1); i++) {
		if (mode & usr_mds[i].umodes) {
			newmode[j] = usr_mds[i].mode;
			j++;
		}

	}
	newmode[j] = '\0';
	sts ("%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, me.now, newmode, ident, host, me.name, realname);
	AddUser (nick, ident, host, me.name, 0, me.now);
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
	int i, j;
	char newmode[20];
	newmode[0] = '+';
	j = 1;
	for (i = 0; i < ((sizeof (usr_mds) / sizeof (usr_mds[0])) - 1); i++) {
		if (mode & usr_mds[i].umodes) {
			newmode[j] = usr_mds[i].mode;
			j++;
		}

	}
	newmode[j] = '\0';
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
	Change_User (finduser (oldnick), newnick);
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, (int)me.now);
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
sinvite_cmd (const char *from, const char *to, const char *chan) {
	sts (":%s INVITE %s %s", from, to, chan);
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
	return 1;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	/* there isn't an akill on Hybrid, so we send a kline to all servers! */

	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s * %lu %s %s :%s", setby, MSG_KLINE, (unsigned long)length, ident, host, ircd_buf);
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
	if (ircd_srv.unkline) {
		sts(":%s %s %s %s", me.name, MSG_UNKLINE, ident, host);
	} else {
		chanalert (s_Services, "Please Manually remove KLINES using /unkline on each server");
	}
	return 1;
}


void
chanalert (char *who, char *fmt, ...)
{
	va_list ap;

	if (!me.onchan)
		return;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	sts (":%s PRIVMSG %s :%s", who, me.chan, ircd_buf);
}

void
prefmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (me.want_privmsg) {
		sts (":%s PRIVMSG %s :%s", from, to, ircd_buf);
	} else {
		sts (":%s NOTICE %s :%s", from, to, ircd_buf);
	}
}

void
privmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	sts (":%s PRIVMSG %s :%s", from, to, ircd_buf);
}

void
notice (char *to, const char *from, char *fmt, ...)
{
	va_list ap;

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);
	sts (":%s NOTICE %s :%s", from, to, ircd_buf);
}


void
privmsg_list (char *to, char *from, const char **text)
{
	while (*text) {
		if (**text)
			prefmsg (to, from, (char*)*text);
		else
			prefmsg (to, from, " ");
		text++;
	}
}


void
globops (char *from, char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (ircd_buf, BUFSIZE, fmt, ap);
	va_end (ap);

	if (me.onchan) {
		sts (":%s WALLOPS :%s", from, ircd_buf);
	} else {
		nlog (LOG_NORMAL, LOG_CORE, ircd_buf);
	}
}

void
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
void
Srv_Burst (char *origin, char **argv, int argc)
{
	seob_cmd (me.name);
	init_services_bot ();
}
void
Srv_Connect (char *origin, char **argv, int argc)
{
	int i;
	ircd_srv.unkline = 0;
	for (i = 0; i < argc; i++) {
		if (!strcasecmp ("UNKLN", argv[i])) {
			ircd_srv.unkline = 1;
		}
	}
}


void
Usr_Stats (char *origin, char **argv, int argc)
{
	User *u;

	u = finduser (origin);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Received a Message from an Unknown User! (%s)", origin);
	}
	ns_stats (argv[0], u);
}

void
Usr_Version (char *origin, char **argv, int argc)
{
	snumeric_cmd (RPL_VERSION, origin, "%d.%d.%d%s :%s -> %s %s", MAJOR, MINOR, REV, ircd_version, me.name, version_date, version_time);
}

void
Usr_ShowMOTD (char *origin, char **argv, int argc)
{
	ns_motd (origin);
}

void
Usr_ShowADMIN (char *origin, char **argv, int argc)
{
	ns_admin (origin);
}

void
Usr_Showcredits (char *origin, char **argv, int argc)
{
	ns_credits (origin);
}

void
Usr_AddServer (char *origin, char **argv, int argc)
{
	AddServer (argv[0], origin, atoi (argv[1]));
}

void
Usr_DelServer (char *origin, char **argv, int argc)
{
	DelServer (argv[0]);
}

void
Usr_DelUser (char *origin, char **argv, int argc)
{
	DelUser (origin);
}

void
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
void
Usr_Mode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
		nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[0]);
		UserMode (argv[0], argv[1]);
	} else {
		ChanMode (origin, argv, argc);
	}
}
void
Usr_Kill (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (argv[0]);
	if (u) {
		KillUser (argv[0]);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Can't find user %s for Kill", argv[0]);
	}
}
void
Usr_Vhost (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (origin);
	if (u) {
		strlcpy (u->vhost, argv[0], MAXHOST);
	}
}
void
Usr_Pong (char *origin, char **argv, int argc)
{
	Server *s;
	s = findserver (argv[0]);
	if (s) {
		dopong (s);
	} else {
		nlog (LOG_NOTICE, LOG_CORE, "Received PONG from unknown server: %s", argv[0]);
	}
}
void
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
void
Usr_Nick (char *origin, char **argv, int argc)
{
	User *u = finduser (origin);
	if (u) {
		Change_User (u, argv[0]);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Ehh, Can't find the user for %s", origin);
	}

}
void
Usr_Topic (char *origin, char **argv, int argc)
{
	char *buf;
	Chans *c;
	/*
	** Hybrid uses two different formats for the topic change protocol... 
	** :user TOPIC channel :topic 
	** and 
	** :server TOPIC channel author topicts :topic 
	** Both forms must be accepted.
	** - Hwy
	*/	
	c = findchan (argv[0]);
	if (c) {
		if (finduser(origin)) {
			buf = joinbuf (argv, argc, 1);
			ChangeTopic (origin, c, me.now, buf);
		} else if (findserver(origin)) {
			buf = joinbuf (argv, argc, 3);
			ChangeTopic (argv[1], c, atoi(argv[2]), buf);
		} else {
			nlog(LOG_WARNING, LOG_CORE, "Ehhh, Can't find Topic Setter %s/%s", origin, argv[1]); 
		free (buf);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Ehhh, Can't find Channel %s", argv[0]);
	}

}

void
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
void
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
void
Usr_Part (char *origin, char **argv, int argc)
{
	part_chan (finduser (origin), argv[0]);
}

void
Srv_Ping (char *origin, char **argv, int argc)
{
	spong_cmd (argv[0]);
}

void
Srv_Svinfo (char *origin, char **argv, int argc)
{
	ssvinfo_cmd ();
}

void
Srv_Pass (char *origin, char **argv, int argc)
{
}
void
Srv_Server (char *origin, char **argv, int argc)
{
	Server *s;
	if (*origin == 0) {
		AddServer (argv[0], me.name, atoi (argv[1]));
	} else {
		AddServer (argv[0], origin, atoi (argv[1]));
	}
	s = findserver (argv[0]);
	me.s = s;
}

void
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

void
Srv_Nick (char *origin, char **argv, int argc)
{
	char *realname;

	AddUser (argv[0], argv[4], argv[5], argv[6], 0, strtoul (argv[2], NULL, 10));
	realname = joinbuf (argv, argc, 7);
	AddRealName (argv[0], realname);
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3]);
}

void
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
