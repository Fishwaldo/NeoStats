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
#include "QuantumIRCd.h"
#include "dl.h"
#include "log.h"
#include "users.h"
#include "server.h"
#include "chans.h"

static void Usr_Version (char *origin, char **argv, int argc);
static void Usr_MOTD (char *origin, char **argv, int argc);
static void Usr_Admin (char *origin, char **argv, int argc);
static void Usr_Credits (char *origin, char **argv, int argc);
static void Usr_AddServer (char *origin, char **argv, int argc);
static void Usr_DelServer (char *origin, char **argv, int argc);
static void Usr_DelUser (char *origin, char **argv, int argc);
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
static void Srv_Vctrl (char *origin, char **argv, int argc);
static void Srv_Client (char *origin, char **argv, int argc);
static void Srv_Smode (char *origin, char **argv, int argc);

static int vctrl_cmd ();

static char ircd_buf[BUFSIZE];

const char ircd_version[] = "(Q)";
const char services_bot_modes[]= "+oS";

IntCommands cmd_list[] = {
	/* Command      Function                srvmsg */
	{MSG_STATS, TOK_STATS, Usr_Stats, 1, 0},
	{MSG_SETHOST, TOK_SETHOST, Usr_Vhost, 1, 0},
	{MSG_VERSION, TOK_VERSION, Usr_Version, 1, 0},
	{MSG_MOTD, TOK_MOTD, Usr_MOTD, 1, 0},
	{MSG_ADMIN, TOK_ADMIN, Usr_Admin, 1, 0},
	{MSG_CREDITS, TOK_CREDITS, Usr_Credits, 1, 0},
	{MSG_SERVER, TOK_SERVER, Usr_AddServer, 1, 0},
	{MSG_SQUIT, TOK_SQUIT, Usr_DelServer, 1, 0},
	{MSG_QUIT, TOK_QUIT, Usr_DelUser, 1, 0},
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
	{MSG_SVINFO, NULL, Srv_Svinfo, 0, 0},
	{MSG_CAPAB, NULL, Srv_Connect, 0, 0},
	{MSG_BURST, NULL, Srv_Burst, 0, 0},
	{MSG_SJOIN, NULL, Srv_Sjoin, 1, 0},
	{MSG_CLIENT, NULL, Srv_Client, 0, 0},
	{MSG_SMODE, NULL, Srv_Smode, 1, 0},
	{MSG_VCTRL, TOK_VCTRL, Srv_Vctrl, 0, 0},
	{MSG_PASS, TOK_PASS, Srv_Pass, 0, 0},
	{MSG_SERVER, TOK_SERVER, Srv_Server, 0, 0},
	{MSG_SQUIT, TOK_SQUIT, Srv_Squit, 0, 0},
	{MSG_NICK, TOK_NICK, Srv_Nick, 0, 0},
	{MSG_SVSNICK, TOK_SVSNICK, Srv_Svsnick, 0, 0},
	{MSG_KILL, TOK_KILL, Srv_Kill, 0, 0},
	{MSG_PROTOCTL, TOK_PROTOCTL, Srv_Connect, 0, 0},
	{NULL, NULL, NULL, 0, 0}
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
	{MODE_VIP, 'V', 1, 0, '='},
	{MODE_SILENCE, 'd', 1, 0, '-'},
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
	{MODE_SEMIMODERATED, 'M', 0, 0, 0},
	{MODE_SUPERMODERATED, 'X', 0, 0, 0},
	{0x0, 0x0, 0x0, 0x0, 0x0}
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
	{UMODE_IRCADMIN, 'Z', NS_ULEVEL_ROOT}
	,
	{UMODE_SERVICESADMIN, 'P', NS_ULEVEL_ADMIN}
	,
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT}
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


void
init_ircd ()
{
	/* count the number of commands */
	ircd_srv.cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])) - 1);
	ircd_srv.umodecount = ((sizeof (usr_mds) / sizeof (usr_mds[0])) - 1);
	ircd_srv.usmodecount = ((sizeof (susr_mds) / sizeof (susr_mds[0])) - 1);
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
	sts ("%s %s :TS", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts ("CAPAB TS5 BURST SSJ5 NICKIP CLIENT");
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
sjoin_cmd (const char *who, const char *chan, unsigned long chflag)
{
	char flag;
	char mode;
	char **av;
	int ac;
	
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
	case MODE_VIP:
		flag = '=';
		mode= 'V';
		break;
	case MODE_SILENCE:
		flag = '-';
		mode= 'd';
		break;
	default:
		flag = ' ';
		mode= '\0';
	}
	sts (":%s %s 0 %s + :%c%s", me.name, MSG_SJOIN, chan, flag, who);
	join_chan (finduser (who), (char *) chan);
	ircsnprintf (ircd_buf, BUFSIZE, "%s +%c %s", chan, mode, who);
	ac = split_buf (ircd_buf, &av, 0);
	ChanMode (me.name, av, ac);
	free (av);

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
	char* newmode;
	
	newmode = UmodeMaskToString(mode);
	sts ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", (me.token ? TOK_NICK : MSG_NICK), nick, me.now, newmode, ident, host, me.name, me.now, realname);
	AddUser (nick, ident, host, realname, me.name, 0, me.now);
	UserMode (nick, newmode);
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
	sts (":%s %s %s :%s", me.name, MSG_SVSKILL, who, ircd_buf);
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
	UserNick (oldnick, newnick);
	sts (":%s %s %s %d", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, (int)me.now);
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

int
swallops_cmd (const char *who, const char *msg, ...)
{
	va_list ap;

	va_start (ap, msg);
	ircvsnprintf (ircd_buf, BUFSIZE, msg, ap);
	va_end (ap);
	sts (":%s %s :%s", who, (me.token ? TOK_WALLOPS : MSG_WALLOPS), ircd_buf);
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
		strlcpy (u->vhost, vhost, MAXHOST);
		sts (":%s %s %s %s", me.name, (me.token ? TOK_SETHOST : MSG_SETHOST), who, vhost);
		return 1;
	}
}
int 
sinvite_cmd (const char *from, const char *to, const char *chan) {
	sts (":%s INVITE %s %s", from, to, chan);
	return 1;
}

int
sakill_cmd (const char *host, const char *ident, const char *setby, const int length, const char *reason, ...)
{
	va_list ap;

	va_start (ap, reason);
	ircvsnprintf (ircd_buf, BUFSIZE, reason, ap);
	va_end (ap);
	sts (":%s %s %s %s %d %s %d :%s", me.name, (me.token ? TOK_AKILL : MSG_AKILL), host, ident, length, setby, (int)me.now, ircd_buf);
	return 1;
}

int
srakill_cmd (const char *host, const char *ident)
{
	sts (":%s %s %s %s", me.name, (me.token ? TOK_RAKILL : MSG_RAKILL), host, ident);
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
		sts (":%s GLOBOPS :%s", from, ircd_buf);
	} else {
		nlog (LOG_NORMAL, LOG_CORE, ircd_buf);
	}
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
}

static void
Srv_Connect (char *origin, char **argv, int argc)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcasecmp ("TOKEN", argv[i])) {
			me.token = 1;
		}
		if (!strcasecmp ("CLIENT", argv[i])) {
			me.client = 1;
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
	snumeric_cmd (RPL_VERSION, origin, "%d.%d.%d%s :%s -> %s %s", MAJOR, MINOR, REV, ircd_version, me.name, version_date, version_time);
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
Usr_AddServer (char *origin, char **argv, int argc)
{
	AddServer (argv[0], origin, atoi (argv[1]));
}

static void
Usr_DelServer (char *origin, char **argv, int argc)
{
	DelServer (argv[0]);
}

static void
Usr_DelUser (char *origin, char **argv, int argc)
{
	DelUser (origin);
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
		nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s %s", argv[0], argv[1]);
		UserMode (argv[0], argv[1]);
	} else {
		ChanMode (origin, argv, argc);
	}
}
static void
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
static void
Usr_Vhost (char *origin, char **argv, int argc)
{
	User *u;
	u = finduser (argv[0]);
	if (u) {
		strlcpy (u->vhost, argv[1], MAXHOST);
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
	User *u, *k;
	u = finduser (argv[1]);
	k = finduser (origin);
	if ((u) && (k)) {
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
	if (ircd_srv.burst) {
		sping_cmd (me.name, argv[0], argv[0]);
	}
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

	realname = joinbuf (argv, argc, 9);
	AddUser (argv[0], argv[4], argv[5], realname, argv[6], strtoul (argv[8], NULL, 10), strtoul (argv[2], NULL, 10));
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3]);
}

/* Ultimate3 Client Support */
static void
Srv_Client (char *origin, char **argv, int argc)
{
	char *realname;

	realname = joinbuf (argv, argc, 11);
	AddUser (argv[0], argv[5], argv[6], realname, argv[8], strtoul (argv[10], NULL, 10), strtoul (argv[2], NULL, 10));
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3]);
	nlog (LOG_DEBUG1, LOG_CORE, "Smode: SMode: %s", argv[4]);
	UserSMode (argv[0], argv[4]);

}

static void
Srv_Smode (char *origin, char **argv, int argc)
{
	UserSMode (argv[0], argv[1]);
};

/* ultimate 3 */

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
	if ((me.allbots > 0) || (Umode & UMODE_SERVICES)) {
		sjoin_cmd (nick, me.chan, MODE_CHANADMIN);
		schmode_cmd (nick, me.chan, "+a", nick);
		/* all bots join */
	}
	return 1;
}
