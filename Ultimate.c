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
#include "Ultimate.h"
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
#ifndef ULTIMATE3
static void Srv_Netinfo (char *origin, char **argv, int argc);
#endif
static void Srv_Pass (char *origin, char **argv, int argc);
static void Srv_Server (char *origin, char **argv, int argc);
static void Srv_Squit (char *, char **, int argc);
static void Srv_Nick (char *, char **, int argc);
static void Srv_Svsnick (char *, char **, int argc);
static void Srv_Kill (char *, char **, int argc);
static void Srv_Protocol (char *, char **, int argc);
#ifdef ULTIMATE3
static void Srv_Svinfo (char *, char **, int argc);
static void Srv_Burst (char *origin, char **argv, int argc);
static void Srv_Sjoin (char *origin, char **argv, int argc);
#endif
static void Srv_Vctrl (char *origin, char **argv, int argc);
#ifdef ULTIMATE3
static void Srv_Client (char *origin, char **argv, int argc);
static void Srv_Smode (char *origin, char **argv, int argc);
#endif
static int vctrl_cmd ();

static struct ircd_srv_ {   
	int uprot;  
	int modex;   
        int nicklg;   
        int gc;   
        char cloak[25];   
        int burst;   
} ircd_srv; 
                                                        



#ifndef ULTIMATE3
const char ircd_version[] = "(UL)";
const char services_bot_modes[]= "+oS";
#else
const char ircd_version[] = "(UL3)";
const char services_bot_modes[]= "+oS";
static char ircd_buf[BUFSIZE];
#endif
long services_bot_umode= 0;

IntCommands cmd_list[] = {
	/* Command      Token          Function       srvmsg */
	{MSG_STATS,     /*TOK_STATS,     */Usr_Stats,     1, 0},
	{MSG_SETHOST,   /*TOK_SETHOST,   */Usr_Vhost,     1, 0},
	{MSG_VERSION,   /*TOK_VERSION,   */Usr_Version,   1, 0},
	{MSG_MOTD,      /*TOK_MOTD,      */Usr_MOTD,      1, 0},
	{MSG_ADMIN,     /*TOK_ADMIN,     */Usr_Admin,     1, 0},
	{MSG_CREDITS,   /*TOK_CREDITS,   */Usr_Credits,   1, 0},
	{MSG_SERVER,    /*TOK_SERVER,    */Usr_Server, 1, 0},
	{MSG_SQUIT,     /*TOK_SQUIT,     */Usr_Squit, 1, 0},
	{MSG_QUIT,      /*TOK_QUIT,      */Usr_Quit,   1, 0},
	{MSG_MODE,      /*TOK_MODE,      */Usr_Mode,      1, 0},
	{MSG_SVSMODE,   /*TOK_SVSMODE,   */Usr_Smode,     1, 0},
	{MSG_KILL,      /*TOK_KILL,      */Usr_Kill,      1, 0},
	{MSG_PONG,      /*TOK_PONG,      */Usr_Pong,      1, 0},
	{MSG_AWAY,      /*TOK_AWAY,      */Usr_Away,      1, 0},
	{MSG_NICK,      /*TOK_NICK,      */Usr_Nick,      1, 0},
	{MSG_TOPIC,     /*TOK_TOPIC,     */Usr_Topic,     1, 0},
	{MSG_KICK,      /*TOK_KICK,      */Usr_Kick,      1, 0},
	{MSG_JOIN,      /*TOK_JOIN,      */Usr_Join,      1, 0},
	{MSG_PART,      /*TOK_PART,      */Usr_Part,      1, 0},
	{MSG_PING,      /*TOK_PING,      */Srv_Ping,      0, 0},
#ifndef ULTIMATE3					 
	{MSG_SNETINFO,  /*TOK_SNETINFO,  */Srv_Netinfo,   0, 0},
#endif
#ifdef ULTIMATE3
	{MSG_SVINFO,    /*NULL,          */Srv_Svinfo,    0, 0},
	{MSG_CAPAB,     /*NULL,          */Srv_Protocol,   0, 0},
	{MSG_BURST,     /*NULL,          */Srv_Burst,     0, 0},
	{MSG_SJOIN,     /*NULL,          */Srv_Sjoin,     1, 0},
	{MSG_CLIENT,    /*NULL,          */Srv_Client,    0, 0},
	{MSG_SMODE,     /*NULL,          */Srv_Smode,     1, 0},
#endif
	{MSG_VCTRL,     /*TOK_VCTRL,     */Srv_Vctrl,     0, 0},
	{MSG_PASS,      /*TOK_PASS,      */Srv_Pass,      0, 0},
	{MSG_SERVER,    /*TOK_SERVER,    */Srv_Server,    0, 0},
	{MSG_SQUIT,     /*TOK_SQUIT,     */Srv_Squit,     0, 0},
	{MSG_NICK,      /*TOK_NICK,      */Srv_Nick,      0, 0},
	{MSG_SVSNICK,   /*TOK_SVSNICK,   */Srv_Svsnick,   0, 0},
	{MSG_KILL,      /*TOK_KILL,      */Srv_Kill,      0, 0},
	{MSG_PROTOCTL,  /*TOK_PROTOCTL,  */Srv_Protocol,   0, 0},
};

ChanModes chan_modes[] = {
	{CMODE_CHANOP, 'o', 1, 0, '@'},
	{CMODE_HALFOP, 'h', 1, 0, '%'},
	{CMODE_CHANADMIN, 'a', 1, 0, '!'},
	{CMODE_VOICE, 'v', 1, 0, '+'},
	{CMODE_BAN, 'b', 0, 1, 0},
	{CMODE_EXCEPT, 'e', 0, 1, 0},
	{CMODE_FLOODLIMIT, 'f', 0, 1, 0},			/* Flood limiter */
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
	{CMODE_ADMONLY, 'A', 0, 0, 0},
	{CMODE_NOINVITE, 'I', 0, 0, 0},			/* no invites */
	{CMODE_NOKNOCK, 'K', 0, 0, 0},			/* knock knock (no way!) */
	{CMODE_LINK, 'L', 0, 1, 0},
	{CMODE_OPERONLY, 'O', 0, 0, 0},
	{CMODE_RGSTRONLY, 'R', 0, 0, 0},
	{CMODE_STRIP, 'S', 0, 0, 0},			/* works? */
};

#ifdef ULTIMATE3
UserModes user_umodes[] = {
	{UMODE_IRCADMIN, 'Z', NS_ULEVEL_ROOT},
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_SERVICESADMIN, 'P', NS_ULEVEL_ADMIN},
	{UMODE_SERVICESOPER, 'a', NS_ULEVEL_OPER},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_LOCOP, 'O', NS_ULEVEL_OPER},
	{UMODE_REGNICK, 'r', NS_ULEVEL_REG},
	{UMODE_INVISIBLE, 'i', 0},
	{UMODE_WALLOP, 'w', 0},
	{UMODE_SERVNOTICE, 's', 0},
	{UMODE_CLIENT, 'c', 0},
	{UMODE_KILLS, 'k', 0},
	{UMODE_FAILOP, 'g', 0},
	{UMODE_HELPOP, 'h', 0},
	{UMODE_FLOOD, 'f', 0},
	{UMODE_SPY, 'y', 0},
	{UMODE_DCC, 'D', 0},
	{UMODE_GLOBOPS, 'g', 0},
	{UMODE_CHATOP, 'c', 0},
	{UMODE_REJ, 'j', 0},
	{UMODE_ROUTE, 'n', 0},
	{UMODE_SPAM, 'm', 0},
	{UMODE_HIDE, 'x', 0},
	{UMODE_PROT, 'p', 0},
	{UMODE_GLOBCON, 'F', 0},
#if 0
	/* useless modes, ignore them as services use these modes for services ID */
	{UMODE_DEBUG, 'd', 0},
	{UMODE_DCCWARN, 'd', 0},
#endif
	{UMODE_WHOIS, 'W', 0},
};

UserModes user_smodes[] = {
	{SMODE_NETADMIN, 'N', 190},
	{SMODE_CONET, 'n', 175},
	{SMODE_TECHADMIN, 'T', 150},
	{SMODE_COTECH, 't', 125},
	{SMODE_SERVADMIN, 'A', 100},
	{SMODE_GUEST, 'G', 100},
	{SMODE_COADMIN, 'a', 75},
	{SMODE_SSL, 's', 0},
};

#elif ULTIMATE
UserModes user_umodes[] = {
	{UMODE_SERVICES, 'S', NS_ULEVEL_ROOT},
	{UMODE_SERVICESADMIN, 'P', NS_ULEVEL_ROOT},
	{UMODE_TECHADMIN, 'T', 190},
	{UMODE_NETADMIN, 'N', NS_ULEVEL_ADMIN},
	{UMODE_SERVICESOPER, 'a', 100},
	{UMODE_IRCADMIN, 'Z', 100},
	{UMODE_ADMIN, 'z', 70},
	{UMODE_OPER, 'o', NS_ULEVEL_OPER},
	{UMODE_SUPER, 'p', NS_ULEVEL_LOCOPER},
	{UMODE_LOCOP, 'O', NS_ULEVEL_LOCOPER},
	{UMODE_REGNICK, 'r', NS_ULEVEL_REG},
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
	{UMODE_HIDE, 'x', 0},
	{UMODE_WATCHER, 'W', 0},
};
#endif

const int ircd_cmdcount = ((sizeof (cmd_list) / sizeof (cmd_list[0])));
const int ircd_umodecount = ((sizeof (user_umodes) / sizeof (user_umodes[0])));
#ifdef GOTUSERSMODES
const int ircd_smodecount = ((sizeof (user_smodes) / sizeof (user_smodes[0])));
#endif
const int ircd_cmodecount = ((sizeof (chan_modes) / sizeof (chan_modes[0])));

void
send_server (const char *name, const int numeric, const char *infoline)
{
	sts (":%s %s %s %d :%s", me.name, (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
}

int
slogin_cmd (const char *name, const int numeric, const char *infoline, const char *pass)
{
#ifndef ULTIMATE3
	sts ("%s %s", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts ("%s %s %d :%s", (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
	sts ("%s TOKEN CLIENT", (me.token ? TOK_PROTOCTL : MSG_PROTOCTL));
#else
	sts ("%s %s :TS", (me.token ? TOK_PASS : MSG_PASS), pass);
	sts ("CAPAB TS5 BURST SSJ5 NICKIP CLIENT");
	sts ("%s %s %d :%s", (me.token ? TOK_SERVER : MSG_SERVER), name, numeric, infoline);
#endif
	return 1;
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

#ifdef ULTIMATE3
void 
send_sjoin (const char *who, const char *chan, const char flag, time_t tstime)
{
	sts (":%s %s %ld %s + :%c%s", me.name, MSG_SJOIN, tstime, chan, flag, who);
}
#else
void 
send_join (const char *who, const char *chan)
{
	sts (":%s %s %s", who, (me.token ? TOK_JOIN : MSG_JOIN), chan);
}
void 
send_sjoin (const char *who, const char *chan, const char flag, time_t tstime)
{
}
#endif

void 
send_cmode (const char *who, const char *chan, const char *mode, const char *args)
{
	sts (":%s %s %s %s %s %lu", who, (me.token ? TOK_MODE : MSG_MODE), chan, mode, args, me.now);
}

void
send_nick (const char *nick, const char *ident, const char *host, const char *realname, const char* newmode, time_t tstime)
{
#ifndef ULTIMATE3
	sts ("%s %s 1 %lu %s %s %s 0 :%s", (me.token ? TOK_NICK : MSG_NICK), nick, tstime, ident, host, me.name, realname);
#else
	sts ("%s %s 1 %lu %s %s %s %s 0 %lu :%s", (me.token ? TOK_NICK : MSG_NICK), nick, tstime, newmode, ident, host, me.name, tstime, realname);
#endif
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
	sts (":%s %s 0 %ld %d %s 0 0 0 :%s", me.name, MSG_SNETINFO, (long)me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname);
}

int
vctrl_cmd ()
{
	sts ("%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, ircd_srv.uprot, ircd_srv.nicklg, ircd_srv.modex, ircd_srv.gc, me.netname);
	return 1;
}

void 
send_kill (const char *from, const char *target, const char *reason)
{
	sts (":%s %s %s :%s", from, (me.token ? TOK_KILL : MSG_KILL), target, reason);
}

void 
send_svskill (const char *target, const char *reason)
{
	sts (":%s %s %s :%s", me.name, MSG_SVSKILL, target, reason);
}

void 
send_nickchange (const char *oldnick, const char *newnick)
{
	sts (":%s %s %s %ld", oldnick, (me.token ? TOK_NICK : MSG_NICK), newnick, (long)me.now);
}

void 
send_svsnick (const char *target, const char *newnick)
{
	sts ("%s %s %s :%ld", (me.token ? TOK_SVSNICK : MSG_SVSNICK), target, newnick, (long)me.now);
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

void send_wallops (char *who, char *buf)
{
	sts (":%s %s :%s", who, (me.token ? TOK_WALLOPS : MSG_WALLOPS), buf);
}

void
send_svshost (const char *who, const char *vhost)
{
#ifdef ULTIMATE3
	sts (":%s %s %s %s", me.name, (me.token ? TOK_SETHOST : MSG_SETHOST), who, vhost);
#elif ULTIMATE
	sts (":%s %s %s %s", me.name, (me.token ? TOK_CHGHOST : MSG_CHGHOST), who, vhost);
#endif
}

void
send_invite (const char *from, const char *to, const char *chan) 
{
	sts (":%s %s %s %s", from, (me.token ? TOK_INVITE : MSG_INVITE), to, chan);
}

void 
send_akill (const char *host, const char *ident, const char *setby, const int length, const char *reason)
{
#ifdef ULTIMATE3
	sts (":%s %s %s %s %d %s %ld :%s", me.name, (me.token ? TOK_AKILL : MSG_AKILL), host, ident, length, setby, (long)me.now, reason);
#elif ULTIMATE
	sts (":%s %s %s@%s %d %d %s :%s", me.name, MSG_GLINE, ident, host, (int)(me.now + length), (int)me.now, setby, reason);
#endif
}

void 
send_rakill (const char *host, const char *ident)
{
#ifdef ULTIMATE3
	sts (":%s %s %s %s", me.name, (me.token ? TOK_RAKILL : MSG_RAKILL), host, ident);
#elif ULTIMATE
	/* ultimate2 needs an oper to remove */
	sts (":%s %s :%s@%s", s_Services, MSG_REMGLINE, host, ident);
#endif
}


void
send_svinfo (void)
{
	sts ("SVINFO 5 3 0 :%ld", (long)me.now);
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

#ifdef ULTIMATE3
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
		join_chan (origin, modes);
		return;
	}
	tl = list_create (10); 
	if (*modes == '+') {
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
	}
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
		ChangeChanUserMode (argv[1], nick, 1, mode);
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
#endif

#ifdef ULTIMATE3
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
#endif

static void
Srv_Protocol (char *origin, char **argv, int argc)
{
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
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 0);
	UserQuit (origin, tmpbuf);
	free(tmpbuf);
}

static void
Usr_Smode (char *origin, char **argv, int argc)
{
	if (!strchr (argv[0], '#')) {
		/* its user svsmode change */
#ifdef ULTIMATE3
		UserMode (argv[0], argv[2]);
#else
		UserMode (argv[0], argv[1]);
#endif
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
	char *tmpbuf;
	tmpbuf = joinbuf(argv, argc, 1);
	KillUser (argv[0], tmpbuf);
	free(tmpbuf);
}
static void
Usr_Vhost (char *origin, char **argv, int argc)
{
#ifdef ULTIMATE3
	SetUserVhost(argv[0], argv[1]);
#else
	SetUserVhost(origin, argv[0]);
#endif
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
#ifdef ULTIMATE3
	if (ircd_srv.burst) {
		send_ping (me.name, argv[0], argv[0]);
	}
#endif
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

#ifdef ULTIMATE3
static void
Srv_Svinfo (char *origin, char **argv, int argc)
{
	send_svinfo ();
}
#endif

#ifndef ULTIMATE3
static void
Srv_Netinfo (char *origin, char **argv, int argc)
{
	ircd_srv.uprot = atoi (argv[2]);
	strlcpy (ircd_srv.cloak, argv[3], 10);
	strlcpy (me.netname, argv[7], MAXPASS);

	send_netinfo ();
	init_services_bot ();
	globops (me.name, "Link with Network \2Complete!\2");
#ifdef DEBUG
	me.debug_mode = 1;
#endif
	ModuleEvent (EVENT_NETINFO, NULL, 0);
	me.synced = 1;
}
#endif

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

#if ULTIMATE3
	realname = joinbuf (argv, argc, 9);
	AddUser (argv[0], argv[4], argv[5], realname, argv[6], strtoul (argv[8], NULL, 10), strtoul (argv[2], NULL, 10));
	free (realname);
	nlog (LOG_DEBUG1, LOG_CORE, "Mode: UserMode: %s", argv[3]);
	UserMode (argv[0], argv[3]);
#elif ULTIMATE
	realname = joinbuf (argv, argc, 7);
	AddUser (argv[0], argv[3], argv[4], realname, argv[5], 0, strtoul (argv[2], NULL, 10));
	free (realname);
#endif
}

/* Ultimate3 Client Support */
#ifdef ULTIMATE3
static void
Srv_Client (char *origin, char **argv, int argc)
{
	char *realname;
	realname = joinbuf (argv, argc, 11);
	AddUser (argv[0], argv[5], argv[6], realname, argv[8], strtoul (argv[10], NULL, 10), strtoul (argv[2], NULL, 10));
	SetUserVhost(argv[0], argv[7]);
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
#endif

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

