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
#include "Unreal.h"
#include "dl.h"
#include "log.h"


IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, Usr_Stats, 1, 0}
	,
	{TOK_STATS, Usr_Stats, 1, 0}
	,
	{MSG_SETHOST, Usr_Vhost, 1, 0}
	,
	{TOK_SETHOST, Usr_Vhost, 1, 0}
	,
	{MSG_VERSION, Usr_Version, 1, 0}
	,
	{TOK_VERSION, Usr_Version, 1, 0}
	,
	{MSG_MOTD, Usr_ShowMOTD, 1, 0}
	,
	{TOK_MOTD, Usr_ShowMOTD, 1, 0}
	,
	{MSG_ADMIN, Usr_ShowADMIN, 1, 0}
	,
	{TOK_ADMIN, Usr_ShowADMIN, 1, 0}
	,
	{MSG_CREDITS, Usr_Showcredits, 1, 0}
	,
	{TOK_CREDITS, Usr_Showcredits, 1, 0}
	,
	{MSG_SERVER, Usr_AddServer, 1, 0}
	,
	{TOK_SERVER, Usr_AddServer, 1, 0}
	,
	{MSG_SQUIT, Usr_DelServer, 1, 0}
	,
	{TOK_SQUIT, Usr_DelServer, 1, 0}
	,
	{MSG_QUIT, Usr_DelUser, 1, 0}
	,
	{TOK_QUIT, Usr_DelUser, 1, 0}
	,
	{MSG_MODE, Usr_Mode, 1, 0}
	,
	{TOK_MODE, Usr_Mode, 1, 0}
	,
	{MSG_SVSMODE, Usr_Smode, 1, 0}
	,
	{TOK_SVSMODE, Usr_Smode, 1, 0}
	,
	{MSG_SVS2MODE, Usr_Smode, 1, 0}
	,
	{TOK_SVS2MODE, Usr_Smode, 1, 0}
	,
	{MSG_KILL, Usr_Kill, 1, 0}
	,
	{TOK_KILL, Usr_Kill, 1, 0}
	,
	{MSG_PONG, Usr_Pong, 1, 0}
	,
	{TOK_PONG, Usr_Pong, 1, 0}
	,
	{MSG_AWAY, Usr_Away, 1, 0}
	,
	{TOK_AWAY, Usr_Away, 1, 0}
	,
	{MSG_NICK, Usr_Nick, 1, 0}
	,
	{TOK_NICK, Usr_Nick, 1, 0}
	,
	{MSG_TOPIC, Usr_Topic, 1, 0}
	,
	{TOK_TOPIC, Usr_Topic, 1, 0}
	,
	{MSG_TOPIC, Usr_Topic, 0, 0}
	,
	{TOK_TOPIC, Usr_Topic, 0, 0}
	,
	{MSG_KICK, Usr_Kick, 1, 0}
	,
	{TOK_KICK, Usr_Kick, 1, 0}
	,
	{MSG_JOIN, Usr_Join, 1, 0}
	,
	{TOK_JOIN, Usr_Join, 1, 0}
	,
	{MSG_PART, Usr_Part, 1, 0}
	,
	{TOK_PART, Usr_Part, 1, 0}
	,
	{MSG_PING, Srv_Ping, 0, 0}
	,
	{TOK_PING, Srv_Ping, 0, 0}
	,
	{MSG_NETINFO, Srv_Netinfo, 0, 0}
	,
	{TOK_NETINFO, Srv_Netinfo, 0, 0}
	,
	{MSG_PASS, Srv_Pass, 0, 0}
	,
	{TOK_PASS, Srv_Pass, 0, 0}
	,
	{MSG_SERVER, Srv_Server, 0, 0}
	,
	{TOK_SERVER, Srv_Server, 0, 0}
	,
	{MSG_SQUIT, Srv_Squit, 0, 0}
	,
	{TOK_SQUIT, Srv_Squit, 0, 0}
	,
	{MSG_NICK, Srv_Nick, 0, 0}
	,
	{TOK_NICK, Srv_Nick, 0, 0}
	,
	{MSG_SVSNICK, Srv_Svsnick, 0, 0}
	,
	{TOK_SVSNICK, Srv_Svsnick, 0, 0}
	,
	{MSG_KILL, Srv_Kill, 0, 0}
	,
	{TOK_KILL, Srv_Kill, 0, 0}
	,
	{MSG_PROTOCTL, Srv_Connect, 0, 0}
	,
	{TOK_PROTOCTL, Srv_Connect, 0, 0}
	,
	{NULL, NULL, 0, 0}
};



aCtab cFlagTab[] = {
	{MODE_VOICE, 'v', 1, 0, '+'}
	,
	{MODE_HALFOP, 'h', 1, 0, '+'}
	,
	{MODE_CHANOP, 'o', 1, 0, '@'}
	,
	{MODE_LIMIT, 'l', 0, 1}
	,
	{MODE_PRIVATE, 'p', 0, 0}
	,
	{MODE_SECRET, 's', 0, 0}
	,
	{MODE_MODERATED, 'm', 0, 0}
	,
	{MODE_NOPRIVMSGS, 'n', 0, 0}
	,
	{MODE_TOPICLIMIT, 't', 0, 0}
	,
	{MODE_INVITEONLY, 'i', 0, 0}
	,
	{MODE_KEY, 'k', 0, 1}
	,
	{MODE_RGSTR, 'r', 0, 0}
	,
	{MODE_RGSTRONLY, 'R', 0, 0}
	,
	{MODE_NOCOLOR, 'c', 0, 0}
	,
	{MODE_CHANPROT, 'a', 1, 0}
	,
	{MODE_CHANOWNER, 'q', 1, 0}
	,
	{MODE_OPERONLY, 'O', 0, 0}
	,
	{MODE_ADMONLY, 'A', 0, 0}
	,
	{MODE_LINK, 'L', 0, 1}
	,
	{MODE_NOKICKS, 'Q', 0, 0}
	,
	{MODE_BAN, 'b', 0, 1}
	,
	{MODE_STRIP, 'S', 0, 0}
	,			/* works? */
	{MODE_EXCEPT, 'e', 0, 1}
	,			/* exception ban */
	{MODE_NOKNOCK, 'K', 0, 0}
	,			/* knock knock (no way!) */
	{MODE_NOINVITE, 'V', 0, 0}
	,			/* no invites */
	{MODE_FLOODLIMIT, 'f', 0, 1}
	,			/* flood limiter */
	{MODE_NOHIDING, 'H', 0, 0}
	,			/* no +I joiners */
	{MODE_STRIPBADWORDS, 'G', 0, 0}
	,			/* no badwords */
	{MODE_NOCTCP, 'C', 0, 0}
	,			/* no CTCPs */
	{MODE_AUDITORIUM, 'u', 0, 0}
	,
	{MODE_ONLYSECURE, 'z', 0, 0}
	,
	{MODE_NONICKCHANGE, 'N', 0, 0}
	,
	{0x0, 0x0, 0x0}
};


Oper_Modes usr_mds[] = {
	{UMODE_OPER, 'o', 50}
	,
	{UMODE_LOCOP, 'O', 40}
	,
	{UMODE_INVISIBLE, 'i', 0}
	,
	{UMODE_WALLOP, 'w', 0}
	,
	{UMODE_FAILOP, 'g', 0}
	,
	{UMODE_HELPOP, 'h', 30}
	,
	{UMODE_SERVNOTICE, 's', 0}
	,
	{UMODE_KILLS, 'k', 0}
	,
	{UMODE_SERVICES, 'S', 200}
	,
	{UMODE_SADMIN, 'a', 100}
	,
	{UMODE_COADMIN, 'C', 60}
	,
	{UMODE_EYES, 'e', 0}
	,
	{UMODE_KIX, 'q', 0}
	,
	{UMODE_BOT, 'B', 0}
	,
	{UMODE_FCLIENT, 'F', 0}
	,
	{UMODE_DEAF, 'd', 0}
	,
	{UMODE_HIDING, 'I', 0}
	,
	{UMODE_ADMIN, 'A', 70}
	,
	{UMODE_NETADMIN, 'N', 185}
	,
	{UMODE_TECHADMIN, 'T', 190}
	,
	{UMODE_CLIENT, 'c', 0}
	,
	{UMODE_FLOOD, 'f', 0}
	,
	{UMODE_REGNICK, 'r', 0}
	,
	{UMODE_HIDE, 'x', 0}
	,
	{UMODE_CHATOP, 'b', 0}
	,
	{UMODE_WHOIS, 'W', 0}
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
	DelUser (who);
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
	char tmp[512];

	sts (":%s %s %s %s %s %lu", who, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, time (NULL));
	snprintf (tmp, 512, "%s %s %s", chan, mode, args);
	ac = split_buf (tmp, &av, 0);
	ChanMode ("", av, ac);
	return 1;
}

int
snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname)
{
	sts ("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time (NULL), ident, host, me.name, realname);
	AddUser (nick, ident, host, me.name, 0, time (NULL));
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
	sts (":%s %s %s :%s", who, (me.token ? TOK_MODE : MSG_MODE), target, newmode);
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
	sts ("%s %s", (me.token ? TOK_PONG : MSG_PONG), reply);
	return 1;
}

int
snetinfo_cmd ()
{
	sts (":%s %s 0 %d %d %s 0 0 0 :%s", me.name, (me.token ? TOK_NETINFO : MSG_NETINFO), time (NULL), ircd_srv.uprot, ircd_srv.cloak, me.netname);
	return 1;
}

int
skill_cmd (const char *from, const char *target, const char *reason, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
	sts (":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, buf);
	va_end (ap);
	DelUser (target);
	return 1;
}

int
ssmo_cmd (const char *from, const char *umodetarget, const char *msg)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_SMO : MSG_SMO), umodetarget, msg);
	return 1;
}

int
snick_cmd (const char *oldnick, const char *newnick)
{
	Change_User (finduser (oldnick), newnick);
	sts (":%s %s %s %d", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, time (NULL));
	return 1;
}

int
sswhois_cmd (const char *target, const char *swhois)
{
	sts ("%s %s :%s", (me.token ? TOK_SWHOIS : MSG_SWHOIS), target, swhois);
	return 1;
}

int
ssvsnick_cmd (const char *target, const char *newnick)
{
	sts ("%s %s %s :%d", (me.token ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, time (NULL));
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

int
swallops_cmd (const char *who, const char *msg, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, msg);
	vsnprintf (buf, 512, msg, ap);
	sts (":%s %s :%s", who, (me.token ? TOK_WALLOPS : MSG_WALLOPS), buf);
	va_end (ap);
	return 1;
}

int
ssvshost_cmd (const char *who, const char *vhost)
{
	User *u;
	u = finduser (who);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Can't Find user %s for ssvshost_cmd", who);
		return 0;
	} else {
		strncpy (u->vhost, vhost, MAXHOST);
		sts (":%s %s %s %s", me.name, (me.token ? TOK_CHGHOST : MSG_CHGHOST), who, vhost);
		return 1;
	}
}

int
ssvsmode_cmd (const char *target, const char *modes)
{
	User *u;
	u = finduser (target);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Can't find user %s for ssvsmode_cmd", target);
		return 0;
	} else {
		sts (":%s %s %s %s", me.name, (me.token ? TOK_SVSMODE : MSG_SVSMODE), target, modes);
		UserMode (target, modes, 0);
	}
	return 1;
}

int
ssvskill_cmd (const char *target, const char *reason, ...)
{
	User *u;
	va_list ap;
	char buf[512];
	u = finduser (target);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Cant find user %s for ssvskill_cmd", target);
		return 0;
	} else {
		va_start (ap, reason);
		vsnprintf (buf, 512, reason, ap);
		sts (":%s %s %s :%s", me.name, (me.token ? TOK_SVSKILL : MSG_SVSKILL), target, buf);
		va_end (ap);
		return 1;
	}
}

/* akill is gone in the latest Unreals, so we set Glines instead */

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
	sts (":%s %s + G %s %s %s %d %d :%s", me.name, (me.token ? TOK_TKL : MSG_TKL), ident, host, setby, time (NULL) + length, time (NULL), buf);
	va_end (ap);
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
	sts (":%s %s - G %s %s %s", me.name, (me.token ? TOK_TKL : MSG_TKL), ident, host, me.name);
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
Srv_Connect (char *origin, char **argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcasecmp ("TOKEN", argv[i])) {
			me.token = 1;
		}
	}
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
		UserMode (argv[0], argv[1], 0);
	} else {
		/* its a channel svsmode change */
		ChanMode (origin, argv, argc);
	}
}
void
Usr_Mode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
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
Usr_Vhost (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (origin);
	if (u) {
		strncpy (u->vhost, argv[0], MAXHOST);
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
	char *buf;
	User *u = finduser (origin);
	if (u) {
		if (argc > 0) {
			buf = joinbuf (argv, argc, 0);
		} else {
			buf = NULL;
		}
		Do_Away (u, buf);
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
		nlog (LOG_NOTICE, LOG_CORE, "Warning, Unable to find user %s for User_nick", origin);
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
		nlog (LOG_WARNING, LOG_CORE, "Waring, Can't find user %s for Kick %s", argv[1], argv[0]);
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
Srv_Netinfo (char *origin, char **argv, int argc)
{
	me.onchan = 1;
	ircd_srv.uprot = atoi (argv[2]);
	strncpy (ircd_srv.cloak, argv[3], 10);
	strncpy (me.netname, argv[7], MAXPASS);

	snetinfo_cmd ();
	init_ServBot ();
	globops (me.name, "Link with Network \2Complete!\2");
	if (ircd_srv.uprot == 2109) {
		me.usesmo = 1;
	}
	Module_Event ("NETINFO", NULL, 0);
	me.synced = 1;
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
	char **av;
	int ac = 0;
	char *realname;
	AddStringToList (&av, argv[0], &ac);
	AddUser (argv[0], argv[3], argv[4], argv[5], 0, strtol (argv[2], NULL, 10));
	realname = joinbuf (argv, argc, 7);
	AddRealName (argv[0], realname);
	free (realname);
}

void
Srv_Svsnick (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (argv[0]);
	if (u) {
		Change_User (u, argv[1]);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Can't find user %s for svsnick", argv[0]);
	}

}
void
Srv_Kill (char *origin, char **argv, int argc)
{
	nlog (LOG_WARNING, LOG_CORE, "Got Kill, but its unhandled.");
}

extern int
SignOn_NewBot (const char *nick, const char *user, const char *host, const char *rname, long Umode)
{

	snewnick_cmd (nick, user, host, rname);
	sumode_cmd (nick, nick, Umode);
	if ((me.allbots > 0) || (Umode & UMODE_SERVICES)) {
		sjoin_cmd (nick, me.chan);
		schmode_cmd (me.name, me.chan, "+o", nick);
	}
	return 1;
}
