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
#include "Ultimate.h"
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
#ifndef ULTIMATE3
	{MSG_SNETINFO, Srv_Netinfo, 0, 0}
	,
	{TOK_SNETINFO, Srv_Netinfo, 0, 0}
	,

#endif
#ifdef ULTIMATE3
	{MSG_SVINFO, Srv_Svinfo, 0, 0}
	,
	{MSG_CAPAB, Srv_Connect, 0, 0}
	,
	{MSG_BURST, Srv_Burst, 0, 0}
	,
	{MSG_SJOIN, Srv_Sjoin, 1, 0}
	,
	{MSG_CLIENT, Srv_Client, 0, 0}
	,
	{MSG_SMODE, Srv_Smode, 1, 0}
	,
#endif
	{MSG_VCTRL, Srv_Vctrl, 0, 0}
	,
	{TOK_VCTRL, Srv_Vctrl, 0, 0}
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
	{MODE_CHANOP, 'o', 1, 0, '@'}
	,
	{MODE_HALFOP, 'h', 1, 0, '%'}
	,
	{MODE_CHANADMIN, 'a', 1, 0, '!'}
	,
	{MODE_VOICE, 'v', 1, 0, '+'}
	,
	{MODE_BAN, 'b', 0, 1, 0}
	,
	{MODE_EXCEPT, 'e', 0, 1, 0}
	,
	{MODE_FLOODLIMIT, 'f', 0, 1, 0}
	,			/* Flood limiter */
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
	{MODE_ADMONLY, 'A', 0, 0, 0}
	,
	{MODE_NOINVITE, 'I', 0, 0, 0}
	,			/* no invites */
	{MODE_NOKNOCK, 'K', 0, 0, 0}
	,			/* knock knock (no way!) */
	{MODE_LINK, 'L', 0, 1, 0}
	,
	{MODE_OPERONLY, 'O', 0, 0, 0}
	,
	{MODE_RGSTRONLY, 'R', 0, 0, 0}
	,
	{MODE_STRIP, 'S', 0, 0, 0}
	,			/* works? */
	{0x0, 0x0, 0x0, 0x0, 0x0}
};


#ifdef ULTIMATE3
Oper_Modes usr_mds[] = {
	{UMODE_OPER, 'o', 50}
	,
	{UMODE_LOCOP, 'O', 40}
	,
	{UMODE_INVISIBLE, 'i', 0}
	,
	{UMODE_WALLOP, 'w', 0}
	,
	{UMODE_SERVNOTICE, 's', 0}
	,
	{UMODE_CLIENT, 'c', 0}
	,
	{UMODE_REGNICK, 'r', 10}
	,
	{UMODE_KILLS, 'k', 0}
	,
	{UMODE_FAILOP, 'g', 0}
	,
	{UMODE_HELPOP, 'h', 30}
	,
	{UMODE_FLOOD, 'f', 0}
	,
	{UMODE_SPY, 'y', 0}
	,
	{UMODE_DCC, 'D', 0}
	,
	{UMODE_GLOBOPS, 'g', 0}
	,
	{UMODE_CHATOP, 'c', 0}
	,
	{UMODE_SERVICESOPER, 'a', 100}
	,
	{UMODE_REJ, 'j', 0}
	,
	{UMODE_ROUTE, 'n', 0}
	,
	{UMODE_SPAM, 'm', 0}
	,
	{UMODE_HIDE, 'x', 0}
	,
	{UMODE_IRCADMIN, 'Z', 200}
	,
	{UMODE_SERVICESADMIN, 'P', 185}
	,
	{UMODE_SERVICES, 'S', 200}
	,
	{UMODE_PROT, 'p', 0}
	,
	{UMODE_GLOBCON, 'F', 0}
	,
	{UMODE_DEBUG, 'd', 0}
	,
	{UMODE_DCCWARN, 'd', 0}
	,
	{UMODE_WHOIS, 'W', 0}
	,
	{0, 0, 0}
};

Oper_Modes susr_mds[] = {
	{SMODE_SSL, 's', 0}
	,
	{SMODE_COADMIN, 'a', 75}
	,
	{SMODE_SERVADMIN, 'A', 100}
	,
	{SMODE_COTECH, 't', 125}
	,
	{SMODE_TECHADMIN, 'T', 150}
	,
	{SMODE_CONET, 'n', 175}
	,
	{SMODE_NETADMIN, 'N', 190}
	,
	{SMODE_GUEST, 'G', 100}
	,
	{0, 0, 0}
};

#elif ULTIMATE
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
	{UMODE_SERVICESADMIN, 'P', 200}
	,
	{UMODE_RBOT, 'B', 0}
	,
	{UMODE_SBOT, 'b', 0}
	,
	{UMODE_ADMIN, 'z', 70}
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
	{UMODE_WATCHER, 'W', 0}
	,
	{UMODE_SERVICESOPER, 'a', 100}
	,
	{UMODE_SUPER, 'p', 40}
	,
	{UMODE_IRCADMIN, 'Z', 100}
	,
	{0, 0, 0}
};
#endif

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
#ifndef ULTIMATE3
	sts ("%s %s", (me.token ? TOK_PASS : MSG_PASS), pass);
#else
	sts ("%s %s :TS", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts ("CAPAB TS5 BURST SSJ5 NICKIP CLIENT");
#endif
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
#ifndef ULTIMATE3
	sts ("%s %s", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL), option);
#endif
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

#ifdef ULTIMATE3
int
sjoin_cmd (const char *who, const char *chan, unsigned long chflag)
{
	char flag;
	char mode;
	char **av;
	int ac;
	char tmp[512];
	switch (chflag) {
	case MODE_CHANOP:
		flag = '@';
		mode= '0';
		break;
	case MODE_HALFOP:
		flag = '%';
		mode= 'h';
		break;
	case MODE_VOICE:
		flag = '+';
		mode= 'v';
		break;
	case MODE_CHANADMIN:
		flag = '!';
		mode= 'a';
		break;
	default:
		flag = ' ';
		mode= '\0';
	}
	sts (":%s %s 0 %s + :%c%s", me.name, MSG_SJOIN, chan, flag, who);
	join_chan (finduser (who), (char *) chan);
	snprintf (tmp, 512, "%s +%c %s", chan, mode, who);
	ac = split_buf (tmp, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);

#else
int
sjoin_cmd (const char *who, const char *chan)
{
	sts (":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
	join_chan (finduser (who), (char *) chan);
#endif
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
	free (av);
	return 1;
}

#ifndef ULTIMATE3
int
snewnick_cmd (const char *nick, const char *ident, const char *host, const char *realname)
{
	sts ("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time (NULL), ident, host, me.name, realname);
	AddUser (nick, ident, host, me.name, 0, time (NULL));
#else
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
	sts ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", (me.token ? TOK_NICK : MSG_NICK), nick, time (NULL), newmode, ident, host, me.name, time (NULL), realname);
	AddUser (nick, ident, host, me.name, 0, time (NULL));
	UserMode (nick, newmode, 0);
#endif
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
	sts (":%s %s 0 %d %d %s 0 0 0 :%s", me.name, MSG_SNETINFO, time (NULL), ircd_srv.uprot, ircd_srv.cloak, me.netname);
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
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
	sts (":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, buf);
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
	chanalert (s_Services, "Warning, Module %s tried to SMO, which is not supported in Ultimate", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning, Module %s tried to SMO, which is not supported in Ultimate", segvinmodule);
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
	chanalert (s_Services, "Warning Module %s tried to SWHOIS, which is not supported in Ultimate", segvinmodule);
	nlog (LOG_NOTICE, LOG_CORE, "Warning. Module %s tried to SWHOIS, which is not supported in Ultimate", segvinmodule);
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
#ifdef ULTIMATE3
		sts (":%s %s %s %s", me.name, (me.token ? TOK_SETHOST : MSG_SETHOST), who, vhost);
#elif ULTIMATE
		sts (":%s CHGHOST %s %s", me.name, who, vhost);
#endif
		return 1;
	}
}
int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;
	char buf[512];
	va_start (ap, reason);
	vsnprintf (buf, 512, reason, ap);
#ifdef ULTIMATE3
	sts (":%s %s %s %s %d %s %d :%s", me.name, (me.token ? TOK_AKILL : MSG_AKILL), host, ident, length, setby, time (NULL), buf);
#elif ULTIMATE
	sts (":%s %s %s@%s %d %d %s :%s", me.name, MSG_GLINE, ident, host, time (NULL) + length, time (NULL), setby, buf);
#endif
	va_end (ap);
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
#ifdef ULTIMATE3
	sts (":%s %s %s %s", me.name, (me.token ? TOK_RAKILL : MSG_RAKILL), host, ident);
#elif ULTIMATE
	/* ultimate2 needs an oper to remove */
	sts (":%s %s :%s@%s", s_Services, MSG_REMGLINE, host, ident);
#endif
	return 1;
}


int
ssvinfo_cmd ()
{
	sts ("SVINFO 5 3 0 :%d", time (NULL));
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
		nlog (LOG_DEBUG3, LOG_CORE, "SENT: %s", out);
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
	if (argc <= 2) {
		modes = argv[1];
	} else {
		modes = argv[2];
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
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcasecmp ("TOKEN", argv[i])) {
			me.token = 1;
		}
#ifdef ULTIMATE3
		if (!strcasecmp ("CLIENT", argv[i])) {
			me.client = 1;
		}
#endif
	}
}


void
Usr_Stats (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (origin);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Received a Message from an Unknown User! (%s)", origin);
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
#ifdef ULTIMATE3
		UserMode (argv[0], argv[2], 0);
#else
		UserMode (argv[0], argv[1], 0);
#endif
	} else {
		/* its a channel svsmode change */
		ChanMode (origin, argv, argc);
	}
}
void
Usr_Mode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
		nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s %s", argv[0], argv[1]);
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
#ifndef ULTIMATE3
	u = finduser (origin);
#else
	u = finduser (argv[0]);
#endif
	if (u) {
#ifndef ULTIMATE3
		strncpy (u->vhost, argv[0], MAXHOST);
#else
		strncpy (u->vhost, argv[1], MAXHOST);
#endif
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
	if ((u) && (k)) {
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
#ifdef ULTIMATE3
	if (ircd_srv.burst) {
		sping_cmd (me.name, argv[0], argv[0]);
	}
#endif
}

void
Srv_Vctrl (char *origin, char **argv, int argc)
{
	ircd_srv.uprot = atoi (argv[0]);
	ircd_srv.nicklg = atoi (argv[1]);
	ircd_srv.modex = atoi (argv[2]);
	ircd_srv.gc = atoi (argv[3]);
	strncpy (me.netname, argv[14], MAXPASS);
	vctrl_cmd ();
}

void
Srv_Svinfo (char *origin, char **argv, int argc)
{
	ssvinfo_cmd ();
}

#ifndef ULTIMATE3
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
#ifdef DEBUG
	ns_debug_to_coders (me.chan);
#endif
	if (ircd_srv.uprot == 2109) {
		me.usesmo = 1;
	}
	Module_Event ("NETINFO", NULL, 0);
	me.synced = 1;
}
#endif

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
#if ULTIMATE3
	AddUser (argv[0], argv[4], argv[5], argv[6], strtoul (argv[8], NULL, 10), strtoul (argv[2], NULL, 10));
	realname = joinbuf (argv, argc, 9);
	AddRealName (argv[0], realname);
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3], 0);
#elif ULTIMATE
	AddUser (argv[0], argv[3], argv[4], argv[5], 0, strtoul (argv[2], NULL, 10));
	realname = joinbuf (argv, argc, 7);
	AddRealName (argv[0], realname);
	free (realname);
#endif
}

/* Ultimate3 Client Support */
#ifdef ULTIMATE3
void
Srv_Client (char *origin, char **argv, int argc)
{
	char *realname;

	AddUser (argv[0], argv[5], argv[6], argv[8], strtoul (argv[10], NULL, 10), strtoul (argv[2], NULL, 10));
	realname = joinbuf (argv, argc, 11);
	AddRealName (argv[0], realname);
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3], 0);
	nlog (LOG_DEBUG1, LOG_CORE, "Smode: SMode: %s", argv[4]);
	UserMode (argv[0], argv[4], 1);

}

void
Srv_Smode (char *origin, char **argv, int argc)
{
	UserMode (argv[0], argv[1], 1);
};

/* ultimate 3 */
#endif

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

#ifdef ULTIMATE3
	snewnick_cmd (nick, user, host, rname, Umode);
#else
	snewnick_cmd (nick, user, host, rname);
	sumode_cmd (nick, nick, Umode);
#endif
	if ((me.allbots > 0) || (Umode & UMODE_SERVICES)) {
#ifdef ULTIMATE3
		sjoin_cmd (nick, me.chan, MODE_CHANADMIN);
		schmode_cmd (nick, me.chan, "+a", nick);
#else /* ulitmate3 */
		sjoin_cmd (nick, me.chan);
		schmode_cmd (nick, me.chan, "+o", nick);
#endif
		/* all bots join */
	}
	return 1;
}
