/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#define MODE_TABLE_SIZE	256

#define NICKPARAM	0x00000001
#define MODEPARAM	0x00000002
#define MULTIPARAM	0x00000004

#define MSGTOK(a) ((ircd_srv.protocol & PROTOCOL_TOKEN) ? TOK_##a: MSG_##a)

typedef void (*ircd_cmd_handler) (char *origin, char **argv, int argc, int srv);

typedef struct ircd_cmd{
	const char *name;
	const char *token;
	ircd_cmd_handler function;
	int usage;
}ircd_cmd;

typedef struct cmode_init {
	char modechar;
	long mode;
	unsigned flags;
} cmode_init;

typedef struct ChanModes {
	long mode;
	unsigned flags;
	char sjoin;
} ChanModes;

typedef struct cumode_init {
	char modechar;
	long mode;
	char sjoin;
} cumode_init;

typedef struct umode_init {
	char modechar;
	unsigned long umode;
} umode_init;

typedef struct UserModes {
	unsigned long umode;
} UserModes;

typedef struct ircd_server {
	int burst;
	int uprot;
	int modex;
	int nicklen;
	int gc;
	char cloak[35];
	int maxglobalcnt;
	int tsendsync;
	unsigned int protocol;
	unsigned int features;
} ircd_server;

typedef struct ProtocolInfo {
	/* Minimum protocols that are required for the selected protocol
	* e.g. IRCu requires NOQUIT */
	const unsigned int minprotocol;
	/* Optional protocols that are negotiated during connection that the
	* protocol module supports but will work when not available e.g. SJOIN */
	const unsigned int optprotocol;
	/* Features provided by this protocol module e.g. SVSNICK support. */
	const unsigned int features;
	const char* services_umode;
	const char* services_cmode;
} ProtocolInfo;

#ifndef NEOSTATSCORE
MODULEVAR extern ircd_cmd cmd_list[];
MODULEVAR extern cumode_init chan_umodes[];
MODULEVAR extern cmode_init chan_modes[];
MODULEVAR extern umode_init user_umodes[];
MODULEVAR extern umode_init user_smodes[];
#endif

extern char* services_cmode;
extern char* services_umode;

EXPORTVAR extern ircd_server ircd_srv;

#ifdef NEOSTATSCORE
extern ProtocolInfo* protocol_info;
#else
MODULEVAR extern ProtocolInfo protocol_info;
#endif

extern const int proto_maxhost;
extern const int proto_maxpass;
extern const int proto_maxnick;
extern const int proto_maxuser;
extern const int proto_maxrealname;
extern const int proto_chanlen;
extern const int proto_topiclen;

extern long service_umode_mask;

EXPORTVAR extern ChanModes ircd_cmodes[MODE_TABLE_SIZE];
extern UserModes ircd_umodes[MODE_TABLE_SIZE];
extern UserModes ircd_smodes[MODE_TABLE_SIZE];

char* UmodeMaskToString(const long Umode);
long UmodeStringToMask(const char* UmodeString, long Umode);
char* SmodeMaskToString(const long Umode);
long SmodeStringToMask(const char* UmodeString, long Smode);
EXPORTFUNC int init_services_bot (void);
EXPORTFUNC void m_private (char* origin, char **av, int ac, int cmdptr);
EXPORTFUNC void m_notice (char* origin, char **av, int ac, int cmdptr);
EXPORTFUNC void do_motd (const char* nick, const char *remoteserver);
EXPORTFUNC void do_admin (const char* nick, const char *remoteserver);
EXPORTFUNC void do_credits (const char* nick, const char *remoteserver);
EXPORTFUNC void do_stats (const char* nick, const char *what);
EXPORTFUNC void do_ping (const char* origin, const char *destination);
EXPORTFUNC void do_pong (const char* origin, const char* destination);
EXPORTFUNC void do_version (const char* nick, const char *remoteserver);
EXPORTFUNC void do_protocol (char *origin, char **argv, int argc);
EXPORTFUNC void do_sjoin (char* tstime, char* channame, char *modes, char *sjoinnick, char **argv, int argc);
EXPORTFUNC void do_netinfo (const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname);
EXPORTFUNC void do_snetinfo (const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname);
EXPORTFUNC void do_join (const char* nick, const char* chanlist, const char* keys);
EXPORTFUNC void do_part (const char* nick, const char* chan, const char* reason);
EXPORTFUNC void do_nick (const char *nick, const char *hopcount, const char *TS, 
		const char *user, const char *host, const char *server, 
		const char *ip, const char *servicestamp, const char *modes, 
		const char *vhost, const char *realname, const char *numeric, 
		const char *smodes);
EXPORTFUNC void do_client (const char *nick, const char *arg1, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *arg9, 
		const char *ip, const char *realname);
EXPORTFUNC void do_quit (const char *nick, const char *quitmsg);
EXPORTFUNC void do_kill (const char *nick, const char *killmsg);
EXPORTFUNC void do_squit (const char *name, const char* reason);
EXPORTFUNC void do_kick (const char *kickby, const char *chan, const char *kicked, const char *kickreason);
EXPORTFUNC void do_svinfo (void);
EXPORTFUNC void do_vctrl (const char* uprot, const char* nicklen, const char* modex, const char* gc, const char* netname);
EXPORTFUNC void do_smode (const char* nick, const char* modes);
EXPORTFUNC void do_mode_user (const char* nick, const char* modes);
EXPORTFUNC void do_mode_channel (char *origin, char **argv, int argc);
EXPORTFUNC void do_svsmode_user (const char* nick, const char* modes, const char* ts);
/* These are the same for now but we might need to be different in the 
 * future so use macros
 */
#define do_svsmode_channel do_mode_channel
EXPORTFUNC void do_away (const char* nick, const char *reason);
EXPORTFUNC void do_vhost (const char* nick, const char *vhost);
EXPORTFUNC void do_nickchange (const char * oldnick, const char *newnick, const char * ts);
EXPORTFUNC void do_topic (const char* chan, const char *owner, const char* ts, const char *topic);
EXPORTFUNC void do_server (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline, int srv);
EXPORTFUNC void do_burst (char *origin, char **argv, int argc);
EXPORTFUNC void do_swhois (char *who, char *swhois);
EXPORTFUNC void do_tkl(const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason);
EXPORTFUNC void do_eos(const char *name);

/* Defined in ircd specific files */
MODULEFUNC void send_privmsg (const char *from, const char *to, const char *buf);
MODULEFUNC void send_notice (const char *from, const char *to, const char *buf);
MODULEFUNC void send_globops (const char *from, const char *buf);
MODULEFUNC void send_wallops (const char *who, const char *buf);
MODULEFUNC void send_numeric (const char *from, const int numeric, const char *target, const char *buf);
MODULEFUNC void send_umode (const char *who, const char *target, const char *mode);
MODULEFUNC void send_join (const char *sender, const char *who, const char *chan, const unsigned long ts);
MODULEFUNC void send_sjoin (const char *sender, const char *who, const char *chan, const unsigned long ts);
MODULEFUNC void send_part (const char *who, const char *chan);
MODULEFUNC void send_nickchange (const char *oldnick, const char *newnick, const unsigned long ts);
MODULEFUNC void send_cmode (const char *sender, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts);
MODULEFUNC void send_quit (const char *who, const char *quitmsg);
MODULEFUNC void send_kill (const char *from, const char *target, const char *reason);
MODULEFUNC void send_kick (const char *who, const char *chan, const char *target, const char *reason);
MODULEFUNC void send_invite(const char *from, const char *to, const char *chan);
MODULEFUNC void send_svskill (const char *sender, const char *target, const char *reason);
MODULEFUNC void send_svsmode (const char *sender, const char *target, const char *modes);
MODULEFUNC void send_svshost (const char *sender, const char *who, const char *vhost);
MODULEFUNC void send_svsjoin (const char *sender, const char *target, const char *chan);
MODULEFUNC void send_svspart (const char *sender, const char *target, const char *chan);
MODULEFUNC void send_svsnick (const char *sender, const char *target, const char *newnick, const unsigned long ts);
MODULEFUNC void send_swhois (const char *sender, const char *target, const char *swhois);
MODULEFUNC void send_smo (const char *from, const char *umodetarget, const char *msg);
MODULEFUNC void send_akill (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, unsigned long ts);
MODULEFUNC void send_rakill (const char *sender, const char *host, const char *ident);
MODULEFUNC void send_ping (const char *from, const char *reply, const char *to);
MODULEFUNC void send_pong (const char *reply);
MODULEFUNC void send_server (const char *sender, const char *name, const int numeric, const char *infoline);
MODULEFUNC void send_squit (const char *server, const char *quitmsg);
MODULEFUNC void send_nick (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname);
MODULEFUNC void send_server_connect (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink);
MODULEFUNC void send_netinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
MODULEFUNC void send_snetinfo (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
MODULEFUNC void send_svinfo (const int tscurrent, const int tsmin, const unsigned long tsnow);
MODULEFUNC void send_vctrl (const int uprot, const int nicklen, const int modex, const int gc, const char* netname);
MODULEFUNC void send_burst (int b);
MODULEFUNC void send_svstime (const char *sender, const unsigned long ts);

/* Pointers to the IRCd defined functions */
extern void (*irc_send_privmsg) (const char *from, const char *to, const char *buf);
extern void (*irc_send_notice) (const char *from, const char *to, const char *buf);
extern void (*irc_send_globops) (const char *from, const char *buf);
extern void (*irc_send_wallops) (const char *who, const char *buf);
extern void (*irc_send_numeric) (const char *from, const int numeric, const char *target, const char *buf);
extern void (*irc_send_umode) (const char *who, const char *target, const char *mode);
extern void (*irc_send_join) (const char *sender, const char *who, const char *chan, const unsigned long ts);
extern void (*irc_send_sjoin) (const char *sender, const char *who, const char *chan, const unsigned long ts);
extern void (*irc_send_part) (const char *who, const char *chan);
extern void (*irc_send_nickchange) (const char *oldnick, const char *newnick, const unsigned long ts);
extern void (*irc_send_cmode) (const char *sender, const char *who, const char *chan, const char *mode, const char *args, unsigned long ts);
extern void (*irc_send_quit) (const char *who, const char *quitmsg);
extern void (*irc_send_kill) (const char *from, const char *target, const char *reason);
extern void (*irc_send_kick) (const char *who, const char *chan, const char *target, const char *reason);
extern void (*irc_send_invite) (const char *from, const char *to, const char *chan);
extern void (*irc_send_svskill) (const char *sender, const char *target, const char *reason);
extern void (*irc_send_svsmode) (const char *sender, const char *target, const char *modes);
extern void (*irc_send_svshost) (const char *sender, const char *who, const char *vhost);
extern void (*irc_send_svsjoin) (const char *sender, const char *target, const char *chan);
extern void (*irc_send_svspart) (const char *sender, const char *target, const char *chan);
extern void (*irc_send_svsnick) (const char *sender, const char *target, const char *newnick, const unsigned long ts);
extern void (*irc_send_swhois) (const char *sender, const char *target, const char *swhois);
extern void (*irc_send_smo) (const char *from, const char *umodetarget, const char *msg);
extern void (*irc_send_akill) (const char *sender, const char *host, const char *ident, const char *setby, const int length, const char *reason, unsigned long ts);
extern void (*irc_send_rakill) (const char *sender, const char *host, const char *ident);
extern void (*irc_send_ping) (const char *from, const char *reply, const char *to);
extern void (*irc_send_pong) (const char *reply);
extern void (*irc_send_server) (const char *sender, const char *name, const int numeric, const char *infoline);
extern void (*irc_send_squit) (const char *server, const char *quitmsg);
extern void (*irc_send_nick) (const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname);
extern void (*irc_send_server_connect) (const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink);
extern void (*irc_send_netinfo) (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
extern void (*irc_send_snetinfo) (const char* from, const int prot, const char* cloak, const char* netname, const unsigned long ts);
extern void (*irc_send_svinfo) (const int tscurrent, const int tsmin, const unsigned long tsnow);
extern void (*irc_send_vctrl) (const int uprot, const int nicklen, const int modex, const int gc, const char* netname);
extern void (*irc_send_burst) (int b);
extern void (*irc_send_svstime) (const char *sender, const unsigned long ts);

void InitIrcd (void);
int signon_newbot (const char *nick, const char *user, const char *host, const char *realname, const char *modes, long Umode);
int sserver_cmd (const char *name, const int numeric, const char *infoline);
int ssquit_cmd (const char *server, const char *quitmsg);

/*int snetinfo_cmd (void);*/
/*int ssvinfo_cmd (void);*/
/*int sburst_cmd (int b);*/
/*int seob_cmd (const char *server);*/
int ssmo_cmd (const char *from, const char *umodetarget, const char *msg);

EXPORTFUNC void send_cmd (char *fmt, ...) __attribute__((format(printf,1,2))); /* 2=format 3=params */

EXPORTFUNC void setserverbase64 (const char *name, const char* num);
EXPORTFUNC char* servertobase64 (const char* name);
EXPORTFUNC char* base64toserver (const char* num);

EXPORTFUNC void setnickbase64 (const char *nick, const char* num);
EXPORTFUNC char* nicktobase64 (const char* nick);
EXPORTFUNC char* base64tonick (const char* num);

MODULEFUNC void parse (char* line);
int flood (User * u);

void (*irc_parse) (char* line);

#endif
