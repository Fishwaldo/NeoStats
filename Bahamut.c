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


IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, Usr_Stats, 1, 0}
	,
	{MSG_VERSION, Usr_Version, 1, 0}
	,
	{MSG_MOTD, Usr_ShowMOTD, 1, 0}
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
	{MSG_SVSMODE, Usr_Smode, 1, 0}
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
	{MSG_CAPAB, Srv_Connect, 0, 0}
	,
	{MSG_BURST, Srv_Burst, 0, 0}
	,
	{MSG_SJOIN, Srv_Sjoin, 1, 0}
	,
	{MSG_PASS, Srv_Pass, 0, 0}
	,
	{MSG_SERVER, Srv_Server, 0, 0}
	,
	{MSG_SQUIT, Srv_Squit, 0, 0}
	,
	{MSG_NICK, Srv_Nick, 0, 0}
	,
	{MSG_SVSNICK, Srv_Svsnick, 0, 0}
	,
	{MSG_KILL, Srv_Kill, 0, 0}
	,
	{NULL, NULL, 0, 0}
};




aCtab cFlagTab[] = {
	{MODE_CHANOP, 'o', 1, 0, '@'}
	,
	{MODE_VOICE, 'v', 1, 0, '+'}
	,
	{MODE_BAN, 'b', 0, 1, 0}
	,
	{MODE_INVITEONLY, 'i', 0, 0, 0}
	,
	{MODE_KEY, 'k', 0, 1, 0}
	,
	{MODE_LIMIT, 'l', 0, 1, 0}
	,
	{MODE_MODERATED, 'm', 0, 0, 0}
	,
	{MODE_NOPRIVMSGS, 'n', 0, 0, 0}
	,
	{MODE_PRIVATE, 'p', 0, 0, 0}
	,
	{MODE_RGSTR, 'r', 0, 0, 0}
	,
	{MODE_SECRET, 's', 0, 0, 0}
	,
	{MODE_TOPICLIMIT, 't', 0, 0, 0}
	,
	{MODE_NOCOLOR, 'x', 0, 0, 0}
	,
	{MODE_OPERONLY, 'O', 0, 0, 0}
	,
	{MODE_RGSTRONLY, 'R', 0, 0, 0}
	,
	/* XXX todo, Mode COLOR */
	{0x0, 0x0, 0x0, 0x0, 0x0}
};


Oper_Modes usr_mds[] = {
	{UMODE_OPER, 'o', 50}
	,
	{UMODE_INVISIBLE, 'i', 0}
	,
	{UMODE_REGNICK, 'r', 10}
	,
	{UMODE_SERVICESADMIN, 'a', 200}
	,
	{UMODE_SERVADMIN, 'A', 100}
	,
	{UMODE_REGONLY, 'R', 0}
	,
	/* this is needed for bot support */
	{UMODE_SERVICES, 'S', 200}
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
sjoin_cmd (const char *who, const char *chan, unsigned long chflag)
{
	char flag;
	char mode[2];
	char **av;
	int ac;
	time_t tstime;
	char tmp[512];
	Chans *c;

	c = findchan ((char *) chan);
	if (!c) {
		tstime = time (NULL);
	} else {
		tstime = c->tstime;
	}
	switch (chflag) {
	case MODE_CHANOP:
		flag = '@';
		strcpy (mode, "o");
		break;
	case MODE_VOICE:
		flag = '+';
		strcpy (mode, "v");
		break;
	default:
		flag = ' ';
		strcpy (mode, "");
	}
	sts (":%s %s %d %s + :%c%s", me.name, MSG_SJOIN, tstime, chan, flag, who);
	join_chan (finduser (who), (char *) chan);
	snprintf (tmp, 512, "%s +%s %s", chan, mode, who);
	ac = split_buf (tmp, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);
	return 1;
}

int
schmode_cmd (const char *who, const char *chan, const char *mode, const char *args)
{
	char **av;
	int ac;
	char tmp[512];

	sts (":%s %s %s %s %s %lu", me.name, MSG_MODE, chan, mode, args, time (NULL));
	snprintf (tmp, 512, "%s %s %s", chan, mode, args);
	ac = split_buf (tmp, &av, 0);
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
	sts ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, time (NULL), newmode, ident, host, me.name, time (NULL), realname);
	AddUser (nick, ident, host, me.name, 0, time (NULL));
	UserMode (nick, newmode, 0);
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
	UserMode (target, newmode, 0);
	return 1;
}

int
snumeric_cmd (const int numeric, const char *target, const char *data, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, data);
	vsnprintf (buf, 512, data, ap);
	sts (":%s %d %s :%s", me.name, numeric, target, buf);
	va_end (ap);
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
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
	sts (":%s %s %s :%s", from, MSG_KILL, target, buf);
	va_end (ap);
	DelUser (target);
	return 1;
}

int
ssvskill_cmd (const char *who, const char *reason, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
	sts (":%s %s %s :%s", me.name, MSG_SVSKILL, who, buf);
	va_end (ap);
	return 1;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	chanalert (s_Services, "Warning, Module %s tried to SMO, which is not supported in Bahamut", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SMO, which is not supported in Bahamut", segvinmodule);
	return 1;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	Change_User (finduser (oldnick), newnick);
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, time (NULL));
	return 1;
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
	sts ("%s %s %s :%d", MSG_SVSNICK, target, newnick, time (NULL));
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

int
swallops_cmd (const char *who, const char *msg, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, msg);
	vsnprintf (buf, 512, msg, ap);
	sts (":%s %s :%s", who, MSG_WALLOPS, buf);
	va_end (ap);
	return 1;
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
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
	sts (":%s %s %s %s %d %s %d :%s", me.name, MSG_AKILL, host, ident, length, setby, time (NULL), buf);
	va_end (ap);
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
	sts ("SVINFO 3 3 0 :%d", time (NULL));
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
chanalert (char *who, char *buf, ...)
{
	va_list ap;
	char tmp[512];
	char out[512];
	va_start (ap, buf);
	vsnprintf (tmp, 512, buf, ap);

	if (me.onchan) {
		snprintf (out, 512, ":%s PRIVMSG %s :%s", who, me.chan, tmp);
		sts ("%s", out);
	}
	va_end (ap);
}

void
prefmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	va_start (ap, fmt);
	vsnprintf (buf2, sizeof (buf2), fmt, ap);
	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}
	if (me.want_privmsg) {
		snprintf (buf, 512, ":%s PRIVMSG %s :%s", from, to, buf2);
	} else {
		snprintf (buf, 512, ":%s NOTICE %s :%s", from, to, buf2);
	}
	sts ("%s", buf);
	va_end (ap);
}

void
privmsg (char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	vsnprintf (buf2, sizeof (buf2), fmt, ap);
	snprintf (buf, 512, ":%s PRIVMSG %s :%s", from, to, buf2);
	sts ("%s", buf);
	va_end (ap);
}

void
notice (char *to, const char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	if (findbot (to)) {
		chanalert (s_Services, "Message From our Bot(%s) to Our Bot(%s), Dropping Message", from, to);
		return;
	}

	va_start (ap, fmt);
	vsnprintf (buf2, sizeof (buf2), fmt, ap);
	snprintf (buf, 512, ":%s NOTICE %s :%s", from, to, buf2);
	sts ("%s", buf);
	va_end (ap);
}


void
privmsg_list (char *to, char *from, const char **text)
{
	while (*text) {
		if (**text)
			prefmsg (to, from, "%s", *text);
		else
			prefmsg (to, from, " ");
		text++;
	}
}


void
globops (char *from, char *fmt, ...)
{
	va_list ap;
	char buf[512], buf2[512];

	va_start (ap, fmt);
	vsnprintf (buf2, sizeof (buf2), fmt, ap);

/* Shmad - have to get rid of nasty term echos :-) */

/* Fish - now that was crackhead coding! */
	if (me.onchan) {
		snprintf (buf, 512, ":%s GLOBOPS :%s", from, buf2);
		sts ("%s", buf);
	} else {
		nlog (LOG_NORMAL, LOG_CORE, "%s", buf2);
	}
	va_end (ap);
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
	if (argc > 4) {
		modes = argv[2];
	} else {
		modes = argv[1];
	}
	if (*modes == '#') {
		join_chan (finduser (origin), modes);
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
					strncpy (m->param, argv[j], PARAMSIZE);
					mn = lnode_create (m);
					if (!list_isfull (tl)) {
						list_append (tl, mn);
					} else {
						nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, tl list is full in Svr_Sjoin(ircd.c)");
						do_exit (0);
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
						i = -1;
					}
				} else {
					/* sjoin's should be at the top of the list */
					ok = 0;
					strncpy (nick, modes, MAXNICK);
					break;
				}
			}
		}
		join_chan (finduser (nick), argv[1]);
		ChangeChanUserMode (findchan (argv[1]), finduser (nick), 1, mode);
		j++;
		ok = 1;
	}
	c = findchan (argv[1]);
	/* update the TS time */
	Change_Chan_Ts (c, atoi (argv[0]));
	c->modes |= mode1;
	if (!list_isempty (tl)) {
		if (!list_isfull (c->modeparms)) {
			list_transfer (c->modeparms, tl, list_first (tl));
		} else {
			/* eeeeeeek, list is full! */
			nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, c->modeparms list is full in Svr_Sjoin(ircd.c)");
			do_exit (0);
		}
	}
	list_destroy (tl);
}

void
Srv_Burst (char *origin, char **argv, int argc)
{
	if (argc > 0) {
		if (ircd_srv.burst == 1) {
			sburst_cmd (0);
			ircd_srv.burst = 0;
			me.synced = 1;
			init_ServBot ();
		}
	} else {
		ircd_srv.burst = 1;
	}
}

void
Srv_Connect (char *origin, char **argv, int argc)
{
}


void
Usr_Stats (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (origin);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Recieved a Message from a Unknown User! (%s)", origin);
		return;
	}
	ShowStats (argv[0], u);
}

void
Usr_Version (char *origin, char **argv, int argc)
{
	snumeric_cmd (351, origin, "%d.%d.%d%s :%s -> %s %s", MAJOR, MINOR, REV, version, me.name, version_date, version_time);
}

void
Usr_ShowMOTD (char *origin, char **argv, int argc)
{
	ShowMOTD (origin);
}

void
Usr_ShowADMIN (char *origin, char **argv, int argc)
{
	ShowADMIN (origin);
}

void
Usr_Showcredits (char *origin, char **argv, int argc)
{
	Showcredits (origin);
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
		UserMode (argv[0], argv[2], 0);
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
		UserMode (argv[0], argv[1], 0);
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
	char *Buf;
	User *u = finduser (origin);
	if (u) {
		if (argc > 0) {
			Buf = joinbuf (argv, argc, 0);
		} else {
			Buf = NULL;
		}
		Do_Away (u, Buf);
		if (argc > 0) {
			free (Buf);
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
	}
}
void
Usr_Topic (char *origin, char **argv, int argc)
{
	char *buf;
	Chans *c;
	c = findchan (argv[0]);
	if (c) {
		buf = joinbuf (argv, argc, 3);
		Change_Topic (argv[1], c, atoi (argv[2]), buf);
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
	if (ircd_srv.burst) {
		sping_cmd (me.name, argv[0], argv[0]);
	}
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
		nlog (LOG_WARNING, LOG_CORE, "Waring, Squit from Unknown Server %s", argv[0]);
	}

}

/* BE REALLY CAREFULL ABOUT THE ORDER OF THESE ifdef's */

void
Srv_Nick (char *origin, char **argv, int argc)
{
	char *realname;
	AddUser (argv[0], argv[4], argv[5], argv[6], strtoul (argv[8], NULL, 10), strtoul (argv[2], NULL, 10));
	realname = joinbuf (argv, argc, 9);
	AddRealName (argv[0], realname);
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3], 0);
}

void
Srv_Svsnick (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (argv[0]);
	if (u) {
		Change_User (u, argv[1]);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Can't find user %s for SVSNICK", argv[0]);
	}

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
		sjoin_cmd (nick, me.chan, MODE_CHANOP);
		/* all bots join */
	}
	return 1;
}
