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
#ifndef IRCD_H
#define IRCD_H

typedef void (*ircd_cmd_handler) (char *origin, char **argv, int argc, int srv);

typedef struct ircd_cmd{
	const char *name;
#ifdef GOTTOKENSUPPORT
	const char *token;
#endif
	ircd_cmd_handler function;
	int usage;
}ircd_cmd;

typedef struct ChanModes {
	long mode;
	char flag;
	unsigned nickparam:1;	/* 1 = yes 0 = no */
	unsigned parameters:1;
	char sjoin;
} ChanModes;

typedef struct {
	unsigned long umode;
	char mode;
	int level;
} UserModes;

typedef struct ircd_server {
	int burst;
	int unkline;
	int uprot;
	int modex;
	int nicklen;
	int gc;
	char cloak[25];
	int maxglobalcnt;
	int tsendsync;
	unsigned int token:1;
} ircd_server;

extern UserModes user_umodes[];
#ifdef GOTUSERSMODES
extern UserModes user_smodes[];
#endif

extern ircd_server ircd_srv;
extern ircd_cmd cmd_list[];
extern ChanModes chan_modes[];
extern const int ircd_cmdcount;
extern const int ircd_umodecount;
extern const int ircd_smodecount;
extern const int ircd_cmodecount;
extern long services_bot_umode;

char* UmodeMaskToString(const long Umode);
long UmodeStringToMask(const char* UmodeString, long Umode);
char* SmodeMaskToString(const long Umode);
long SmodeStringToMask(const char* UmodeString, long Smode);
int init_services_bot (void);
void m_private (char* origin, char **av, int ac, int cmdptr);
void m_notice (char* origin, char **av, int ac, int cmdptr);
void do_motd (const char* nick, const char *remoteserver);
void do_admin (const char* nick, const char *remoteserver);
void do_credits (const char* nick, const char *remoteserver);
void do_stats (const char* nick, const char *what);
void do_ping (const char* origin, const char *destination);
void do_pong (const char* origin, const char* destination);
void do_version (const char* nick, const char *remoteserver);
void do_protocol (char *origin, char **argv, int argc);
void do_sjoin (char* tstime, char* channame, char *modes, char *sjoinnick, char **argv, int argc);
void do_netinfo (const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname);
void do_snetinfo (const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname);
void do_join (const char* nick, const char* chanlist, const char* keys);
void do_part (const char* nick, const char* chan, const char* reason);
void do_nick (const char *nick, const char *hopcount, const char *TS, 
		const char *user, const char *host, const char *server, 
		const char *ip, const char *servicestamp, const char *modes, 
		const char *vhost, const char *realname, const char *numeric
#ifdef GOTUSERSMODES
		, const char *smodes
#endif
		 );
void do_client (const char *nick, const char *arg1, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *arg9, 
		const char *ip, const char *realname);
void do_quit (const char *nick, const char *quitmsg);
void do_kill (const char *nick, const char *killmsg);
void do_squit (const char *name, const char* reason);
void do_kick (const char *kickby, const char *chan, const char *kicked, const char *kickreason);
#ifdef MSG_SVINFO
void do_svinfo (void);
#endif
#ifdef MSG_VCTRL
void do_vctrl (const char* uprot, const char* nicklen, const char* modex, const char* gc, const char* netname);
#endif
#ifdef GOTUSERSMODES
void do_smode (const char* nick, const char* modes);
#endif
void do_mode_user (const char* nick, const char* modes);
void do_mode_channel (char *origin, char **argv, int argc);
void do_svsmode_user (const char* nick, const char* modes, const char* ts);
/* These are the same for now but we might need to be different in the 
 * future so use macros
 */
#define do_svsmode_channel do_mode_channel
void do_away (const char* nick, const char *reason);
void do_vhost (const char* nick, const char *vhost);
void do_nickchange (const char * oldnick, const char *newnick, const char * ts);
void do_topic (const char* chan, const char *owner, const char* ts, const char *topic);
void do_server (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline, int srv);
#ifdef MSG_BURST
void do_burst (char *origin, char **argv, int argc);
#endif
#ifdef MSG_SWHOIS
void do_swhois (char *who, char *swhois);
#endif
#ifdef MSG_TKL
void do_tkl(const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason);
#endif
#ifdef MSG_EOS
void do_eos(const char *name);
#endif

/* Defined in ircd specific files but common to all */
void init_ircd (void);
void send_privmsg (const char *from, const char *to, const char *buf);
void send_notice (const char *from, const char *to, const char *buf);
void send_globops (const char *from, const char *buf);
void send_wallops (const char *who, const char *buf);
void send_numeric (const char *from, const int numeric, const char *target, const char *buf);
void send_umode (const char *who, const char *target, const char *mode);
void send_join (const char *sender, const char *who, const char *chan, const unsigned long ts);
void send_sjoin (const char *sender, const char *who, const char *chan, const unsigned long ts);
void send_part (const char *who, const char *chan);
void send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts);
void send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts);
void send_quit (const char *who, const char *quitmsg);
void send_kill (const char *from, const char *target, const char *reason);
void send_kick (const char *who, const char *chan, const char *target, const char *reason);
void send_invite(const char *from, const char *to, const char *chan);
#ifdef GOTSVSKILL
void send_svskill (const char *sender, const char *target, const char *reason);
#endif
#ifdef GOTSVSMODE
void send_svsmode (const char *sender, const char *target, const char *modes);
#endif
#ifdef GOTSVSHOST 
void send_svshost (const char *sender, const char *who, const char *vhost);
#endif
#ifdef GOTSVSJOIN 
void send_svsjoin (const char *sender, const char *target, const char *chan);
#endif
#ifdef GOTSVSPART
void send_svspart (const char *sender, const char *target, const char *chan);
#endif
#ifdef GOTSVSNICK
void send_svsnick (const char *sender, const char *target, const char *newnick, const unsigned long ts);
#endif
#ifdef GOTSWHOIS
void send_swhois (const char *sender, const char *target, const char *swhois);
#endif
#ifdef GOTSMO
void send_smo (const char *from, const char *umodetarget, const char *msg);
#endif
void send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, unsigned long ts);
void send_rakill (const char *sender, const char *host, const char *ident);
void send_ping (const char *from, const char *reply, const char *to);
void send_pong (const char *reply);
void send_server (const char *sender, const char *name, const int numeric, const char *infoline);
void send_squit (const char *server, const char *quitmsg);
void send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname);
void send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink);
void send_netinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
void send_snetinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
#ifdef MSG_SVINFO
void send_svinfo (const int tscurrent, const int tsmin, const unsigned long tsnow);
#endif
#ifdef MSG_VCTRL
void send_vctrl (const int uprot, const int nicklen, const int modex, const int gc, const char* netname);
#endif
#ifdef MSG_BURST
void send_burst (int b);
#endif
#ifdef GOTSVSTIME
void send_svstime (const char *sender, const unsigned long ts);
#endif

int sserver_cmd (const char *name, const int numeric, const char *infoline);
int ssquit_cmd (const char *server, const char *quitmsg);

/*int snetinfo_cmd (void);*/
/*int ssvinfo_cmd (void);*/
/*int sburst_cmd (int b);*/
/*int seob_cmd (const char *server);*/
int ssmo_cmd (const char *from, const char *umodetarget, const char *msg);

void send_cmd (char *fmt, ...) __attribute__((format(printf,1,2))); /* 2=format 3=params */

#ifdef BASE64SERVERNAME
void setserverbase64 (const char *name, const char* num);
char* servertobase64 (const char* name);
char* base64toserver (const char* num);
#endif
#ifdef BASE64NICKNAME
void setnickbase64 (const char *nick, const char* num);
char* nicktobase64 (const char* nick);
char* base64tonick (const char* num);
#endif

#endif
