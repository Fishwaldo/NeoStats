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

#define MAX_CMD_LINE_LENGTH		350

typedef void (*int_cmd_handler) (char *origin, char **argv, int argc);

typedef struct IntCommands{
	const char *name;
#ifdef GOTTOKENSUPPORT
	const char *token;
#endif
	int_cmd_handler function;
	int srvmsg;		/* Should this be a Server Message(1), or a User Message?(0) */
	int usage;
}IntCommands;

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

extern UserModes user_umodes[];
#ifdef GOTUSERSMODES
extern UserModes user_smodes[];
#endif

extern IntCommands cmd_list[];
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
void ns_usr_motd (char *nick, char **argv, int argc);
void ns_usr_admin (char *nick, char **argv, int argc);
void ns_usr_credits (char *nick, char **argv, int argc);
void ns_usr_stats (char *origin, char **argv, int argc);
void ns_usr_pong (char *origin, char **argv, int argc);
void ns_usr_version (char *origin, char **argv, int argc);
void ns_srv_protocol(char *origin, char **argv, int argc);
void server_sjoin (char* channame, char* tstime, char *modes, int offset, char *origin, char **argv, int argc);

/* Defined in ircd specific files but common to all */
void init_ircd (void);
void send_privmsg (char *to, const char *from, char *buf);
void send_notice (char *to, const char *from, char *buf);
void send_globops (char *from, char *buf);
void send_wallops (char *from, char *buf);
void send_numeric (const int numeric, const char *target, const char *buf);
void send_umode (const char *who, const char *target, const char *mode);
void send_join (const char *who, const char *chan);
void send_sjoin (const char *who, const char *chan, const char flag, time_t tstime);
void send_part (const char *who, const char *chan);
void send_nickchange (const char *oldnick, const char *newnick);
void send_cmode (const char *who, const char *chan, const char *mode, const char *args);
void send_quit (const char *who, const char *quitmsg);
void send_kill (const char *from, const char *target, const char *reason);
void send_kick (const char *who, const char *target, const char *chan, const char *reason);
void send_invite(const char *from, const char *to, const char *chan);
#ifdef GOTSVSKILL
void send_svskill (const char *target, const char *reason);
#endif
#ifdef GOTSVSMODE
void send_svsmode (const char *target, const char *modes);
#endif
#ifdef GOTSVSHOST 
void send_svshost (const char *who, const char *vhost);
#endif
#ifdef GOTSVSJOIN 
void send_svsjoin (const char *target, const char *chan);
#endif
#ifdef GOTSVSPART
void send_svspart (const char *target, const char *chan);
#endif
#ifdef GOTSVSNICK
void send_svsnick (const char *target, const char *newnick);
#endif
#ifdef GOTSWHOIS
void send_swhois (const char *target, const char *swhois);
#endif
#ifdef GOTSMO
void send_smo (const char *from, const char *umodetarget, const char *msg);
#endif
void send_akill (const char *host, const char *ident, const char *setby, const int length, const char *reason);
void send_rakill (const char *host, const char *ident);
void send_ping (const char *from, const char *reply, const char *to);
void send_pong (const char *reply);
void send_server (const char *name, const int numeric, const char *infoline);
void send_squit (const char *server, const char *quitmsg);
void send_nick (const char *nick, const char *ident, const char *host, const char *realname, const char* newmode, time_t tstime);

int sserver_cmd (const char *name, const int numeric, const char *infoline);
int slogin_cmd (const char *name, const int numeric, const char *infoline, const char *pass);
int ssquit_cmd (const char *server, const char *quitmsg);
/*int snetinfo_cmd (void);*/
/*int ssvinfo_cmd (void);*/
int sburst_cmd (int b);
int seob_cmd (const char *server);
int ssmo_cmd (const char *from, const char *umodetarget, const char *msg);

#endif
