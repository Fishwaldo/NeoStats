/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
#include "neoircd.h"
#include "dl.h"
#include "log.h"
#include "users.h"
#include "server.h"
#include "chans.h"

static void Usr_Version (char *origin, char **argv, int argc, int srv);
static void Usr_MOTD (char *origin, char **argv, int argc, int srv);
static void Usr_Admin (char *origin, char **argv, int argc, int srv);
static void Usr_Credits (char *origin, char **argv, int argc, int srv);
static void Usr_Server (char *origin, char **argv, int argc, int srv);
static void Usr_Squit (char *origin, char **argv, int argc, int srv);
static void Usr_Quit (char *origin, char **argv, int argc, int srv);
static void Usr_Mode (char *origin, char **argv, int argc, int srv);
static void Usr_Kill (char *origin, char **argv, int argc, int srv);
static void Usr_Pong (char *origin, char **argv, int argc, int srv);
static void Usr_Away (char *origin, char **argv, int argc, int srv);
static void Usr_Nick (char *origin, char **argv, int argc, int srv);
static void Usr_Topic (char *origin, char **argv, int argc, int srv);
static void Usr_Kick (char *origin, char **argv, int argc, int srv);
static void Usr_Join (char *origin, char **argv, int argc, int srv);
static void Usr_Part (char *origin, char **argv, int argc, int srv);
static void Usr_Stats (char *origin, char **argv, int argc, int srv);
static void Srv_Ping (char *origin, char **argv, int argc, int srv);
static void Srv_Pass (char *origin, char **argv, int argc, int srv);
static void Srv_Server (char *origin, char **argv, int argc, int srv);
static void Srv_Squit (char *origin, char **argv, int argc, int srv);
static void Srv_Nick (char *origin, char **argv, int argc, int srv);
static void Srv_Svinfo (char *origin, char **argv, int argc, int srv);
static void Srv_Burst (char *origin, char **argv, int argc, int srv);
static void Srv_Sjoin (char *origin, char **argv, int argc, int srv);
static void Srv_Tburst (char *origin, char **argv, int argc, int srv);

static struct ircd_srv_ {
	int burst;
} ircd_srv;

const char ircd_version[] = "(N)";
const char services_bot_modes[]= "+oS";
long services_bot_umode= 0;

/* this is the command list and associated functions to run */
IrcdCommands cmd_list[] = {
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
	{MSG_KILL, Usr_Kill, 1, 0},
	{MSG_PONG, Usr_Pong, 1, 0},
	{MSG_AWAY, Usr_Away, 1, 0},
	{MSG_NICK, Usr_Nick, 1, 0},
	{MSG_TOPIC, Usr_Topic, 1, 0},
	{MSG_TOPIC, Usr_Topic, 0, 0},
	{MSG_KICK, Usr_Kick, 1, 0},
	{MSG_JOIN, Usr_Join, 1, 0},
	{MSG_PART, Usr_Part, 1, 0},
	{MSG_PING, Srv_Ping, 0, 0},
	{MSG_SVINFO, Srv_Svinfo, 0, 0},
	{MSG_PASS, Srv_Pass, 0, 0},
	{MSG_SERVER, Srv_Server, 0, 0},
	{MSG_SQUIT, Srv_Squit, 0, 0},
	{MSG_NICK, Srv_Nick, 0, 0},
	{MSG_EOB, Srv_Burst, 1, 0},
	{MSG_SJOIN, Srv_Sjoin, 1, 0},
	{MSG_TBURST, Srv_Tburst, 1, 0},
};

ChanModes chan_modes[] = {
	{CMODE_HALFOP, 'h', 1, 0, '%'},
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_CHANADMIN, 'a', 1, 0, '!'},
	{CMODE_SECRET, 's', 0, 0, 0},
	{CMODE_PRIVATE, 'p', 0, 0, 0},
	{CMODE_MODERATED, 'm', 0, 0, 0},
	{CMODE_TOPICLIMIT, 't', 0, 0, 0},
	{CMODE_INVITEONLY, 'i', 0, 0, 0},
	{CMODE_NOPRIVMSGS, 'n', 0, 0, 0},
	{CMODE_HIDEOPS, 'A', 0, 0, 0},
	{CMODE_LIMIT, 'l', 0, 1, 0},
	{CMODE_KEY, 'k', 0, 1, 0},
	{CMODE_BAN, 'b', 0, 1, 0},
	{CMODE_EXCEPT, 'e', 0, 1, 0},
	{CMODE_INVEX, 'I', 0, 1, 0},
	{CMODE_REGCHAN, 'r', 0, 0, 0},
	{CMODE_OPERONLY, 'O', 0, 0, 0},
};

UserModes user_umodes[] = {
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_DEBUG, 'd', NS_ULEVEL_ROOT},
	{UMODE_ADMIN, 'A', NS_ULEVEL_ADMIN},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_LOCOPS, 'l', NS_ULEVEL_OPER},
	{UMODE_BOTS, 'b', 0},
	{UMODE_CCONN, 'c', 0},
	{UMODE_FULL, 'f', 0},
	{UMODE_CALLERID, 'g', 0},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_SKILL, 'k', 0},
	{UMODE_NCHANGE, 'n', 0},
	{UMODE_REJ, 'R', 0},
	{UMODE_SERVNOTICE, 's', 0},
	{UMODE_UNAUTH, 'u', 0},
	{UMODE_WALLOP, 'w', 0},
	{UMODE_EXTERNAL, 'x', 0},
	{UMODE_SPY, 'y', 0},
	{UMODE_OPERWALL, 'z', 0},
};

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_eob (const char *server)
{
	sts (":%s %s", server, MSG_EOB);
}

void
send_server (const char *name, const int numeric, const char *infoline)
{
	sts (":%s %s %s 2 0 :%s", me.name, MSG_SERVER, name, infoline);
}

void
send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass)
{
	sts ("%s %s :TS", MSG_PASS, pass);
	sts ("CAPAB :TS EOB HUB PARA");
	sts ("%s %s 0 0 :%s", MSG_SERVER, name, infoline);
}

void
send_squit (const char *server, const char *quitmsg)
{
	sts ("%s %s :%s", MSG_SQUIT, server, quitmsg);
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

void 
send_join (const char *who, const char *chan)
{
	sts (":%s %s %d %s + :%s", me.name, MSG_SJOIN, (int)me.now, chan, who);
}

void 
send_sjoin (const char *who, const char *chan, const char flag, time_t tstime)
{
}

void 
send_cmode (const char *who, const char *chan, const char *mode, const char *args)
{
	sts (":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, me.now);
}

void
send_nick (const char *nick, const char *ident, const char *host, const char *realname, const char* newmode, time_t tstime)
{
	sts ("%s %s 1 %lu %s %s %s * %s 0 :%s", MSG_NICK, nick, tstime, newmode, ident, host, me.name, realname);
}

void
send_ping (const char *from, const char *reply, const char *to)
{
	sts (":%s %s %s :%s", from, MSG_PING, reply, to);
}

void 
send_umode (const char *who, const char *target, const char *mode)
{
	sts (":%s %s %s :%s", who, MSG_MODE, target, mode);
}

void 
send_numeric (const int numeric, const char *target, const char *buf)
{
	sts (":%s %d %s :%s", me.name, numeric, target, buf);
}

void
send_pong (const char *reply)
{
	sts ("%s %s", MSG_PONG, reply);
}


void 
send_kill (const char *from, const char *target, const char *reason)
{
	sts (":%s %s %s :%s", from, MSG_KILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick)
{
	sts (":%s %s %s %d", oldnick, MSG_NICK, newnick, (int)me.now);
}

void
send_swhois (const char *target, const char *swhois)
{
	sts (":%s SWHOIS %s :%s", me.name, target, swhois);
}

void 
send_svsnick (const char *target, const char *newnick)
{
	sts (":%s %s %s %s :%lu", me.name, MSG_SVSNICK, target, newnick, me.now);
}

void
send_svsjoin (const char *target, const char *chan)
{
	sts (":%s SVSJOIN %s %s", me.name, target, chan);
}

void
send_svspart (const char *target, const char *chan)
{
	sts (":%s SVSPART %s %s", me.name, target, chan);
}

void 
send_kick (const char *who, const char *target, const char *chan, const char *reason)
{
	sts (":%s %s %s %s :%s", who, MSG_KICK, chan, target, (reason ? reason : "No Reason Given"));
}

void send_wallops (char *who, char *buf)
{
	sts (":%s %s :%s", who, MSG_WALLOPS, buf);
}

void
send_svshost (const char *who, const char *vhost)
{
		sts (":%s SVSHOST %s :%s", me.name, who, vhost);
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	sts (":%s %s %s %s", from, MSG_INVITE, to, chan);
}

void
send_svinfo (void)
{
	sts ("SVINFO 5 3 0 :%d", (int)me.now);
}

void
send_burst (int b)
{
	if (b == 0) {
		sts ("BURST 0");
	} else {
		sts ("BURST");
	}
}

/* there isn't an akill on Hybrid, so we send a kline to all servers! */
void 
send_akill (const char *host, const char *ident, const char *setby, const int length, const char *reason)
{
	sts (":%s GLINE %s %s %d :%s", me.name, ident, host, (int)(me.now + length), reason);
}

void 
send_rakill (const char *host, const char *ident)
{
	sts (":%s UNGLINE %s@%s", me.name, ident, host);
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
	sts (":%s %s :%s", from, MSG_WALLOPS, buf);
}

/* XXXX sjoin sucks man Should do as much of this in a chans.c function*/


static void
Srv_Sjoin (char *origin, char **argv, int argc, int srv)
{
	handle_sjoin (argv[1], argv[0], ((argc <= 2) ? argv[1] : argv[2]), 3, argv[4], argv, argc);
}


static void
Srv_Burst (char *origin, char **argv, int argc, int srv)
{
	if (argc > 0) {
		if (ircd_srv.burst == 1) {
			send_burst (0);
			ircd_srv.burst = 0;
			me.synced = 1;
			init_services_bot ();
		}
	} else {
		ircd_srv.burst = 1;
	}
	send_eob (me.name);
	init_services_bot ();

}

static void
Usr_Stats (char *origin, char **argv, int argc, int srv)
{
	ns_usr_stats (origin, argv, argc);
}

static void
Usr_Version (char *origin, char **argv, int argc, int srv)
{
	ns_usr_version (origin, argv, argc);
}

static void
Usr_MOTD (char *origin, char **argv, int argc, int srv)
{
	ns_usr_motd (origin, argv, argc);
}

static void
Usr_Admin (char *origin, char **argv, int argc, int srv)
{
	ns_usr_admin (origin, argv, argc);
}

static void
Usr_Credits (char *origin, char **argv, int argc, int srv)
{
	ns_usr_credits (origin, argv, argc);
}

static void
Usr_Server (char *origin, char **argv, int argc, int srv)
{
	AddServer (argv[0], origin, atoi (argv[1]), NULL);
}

static void
Usr_Squit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	SquitServer (argv[0], tmpbuf);
	free(tmpbuf);
}

static void
Usr_Quit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 0);
	UserQuit (origin, tmpbuf);
	free(tmpbuf);
}

static void
Usr_Mode (char *origin, char **argv, int argc, int srv)
{
	if (!strchr (argv[0], '#')) {
		UserMode (argv[0], argv[1]);
	} else {
		ChanMode (origin, argv, argc);
	}
}
static void
Usr_Kill (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	KillUser (argv[0], tmpbuf);
	free(tmpbuf);
}
static void
Usr_Pong (char *origin, char **argv, int argc, int srv)
{
	ns_usr_pong (origin, argv, argc);
}
static void
Usr_Away (char *origin, char **argv, int argc, int srv)
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
Usr_Nick (char *origin, char **argv, int argc, int srv)
{
	UserNick (origin, argv[0]);
}
static void
Usr_Topic (char *origin, char **argv, int argc, int srv)
{
	char *buf;

	buf = joinbuf (argv, argc, 2);
	ChanTopic (argv[1], argv[0], atoi (argv[2]), buf);
	free (buf);
}

static void
Usr_Kick (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 2);
	kick_chan(argv[0], argv[1], origin, tmpbuf);
	free(tmpbuf);
}
static void
Usr_Join (char *origin, char **argv, int argc, int srv)
{
	UserJoin (origin, argv[0]);
}
static void
Usr_Part (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	part_chan (finduser (origin), argv[0], tmpbuf);
	free(tmpbuf);
}

static void
Srv_Ping (char *origin, char **argv, int argc, int srv)
{
	send_pong (argv[0]);
}


static void
Srv_Svinfo (char *origin, char **argv, int argc, int srv)
{
	send_svinfo ();
}

static void
Srv_Pass (char *origin, char **argv, int argc, int srv)
{
}
static void
Srv_Server (char *origin, char **argv, int argc, int srv)
{
	if (*origin == 0) {
		me.s = AddServer (argv[0], me.name, atoi (argv[1]), NULL);
	} else {
		me.s = AddServer (argv[0], origin, atoi (argv[1]), NULL);
	}
}

static void
Srv_Squit (char *origin, char **argv, int argc, int srv)
{
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	SquitServer (argv[0], tmpbuf);
	free(tmpbuf);
}

static void
Srv_Nick (char *origin, char **argv, int argc, int srv)
{
	char *realname;
	User *u;

	realname = joinbuf (argv, argc, 9);
	AddUser (argv[0], argv[4], argv[5], realname, argv[7], 0, strtoul (argv[2], NULL, 10));
	free (realname);
	u = finduser (argv[0]);
	if (u) {
		strlcpy (u->vhost, argv[6], MAXHOST);
	}
	UserMode (argv[0], argv[3]);
}

/* Topic Bursting for NeoIRCD */
/* R: :fish.dynam.ac TBURST 1034639893 #ircop 1034652780 ChanServ!services@neostats.net :NeoIRCd Test Oper Channel */

static void
Srv_Tburst (char *origin, char **argv, int argc, int srv)
{
	char *buf;

	buf = joinbuf (argv, argc, 4);
	ChanTopic (argv[3], argv[1], atoi (argv[2]), buf);
	free (buf);
}

