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

#ifndef STATS_H
#define STATS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <assert.h>
#include <adns.h>
#include "list.h"
#include "hash.h"
#include "config.h"

#if UNREAL == 1
#include "Unreal.h"
#elif ULTIMATE == 1
#include "Ultimate.h"
#elif HYBRID7 == 1
#include "hybrid7.h"
#elif NEOIRCD == 1
#include "neoircd.h"
#elif MYSTIC == 1
#include "mystic.h"
#elif IRCU == 1
#include "Ircu.h"
#elif BAHAMUT == 1
#include "Bahamut.h"
#elif QUANTUM == 1
#include "QuantumIRCd.h"
#else
#error Error, you must select an IRCD to use. See ./configure --help for more information
#endif

/**  this is a security hack to give the coders the right levels to debug NeoStats. 
  *  Don't define unless we ask you to 
  */
#undef CODERHACK

#define CHANLEN			50
#define BUFSIZE			512
#define CONFIG_NAME		"neostats.cfg"
#define MOD_PATH		"dl"
#define RECV_LOG		"logs/recv.log"
#define MAXHOST			128
#define MAXPASS			32
#define MAXNICK			32
#define MAXUSER			15
#define MAXREALNAME		50
#define MODESIZE		53
#define PARAMSIZE		MAXNICK+MAXUSER+MAXHOST+10

/* MAX_MOD_NAME
   ModuleInfo will allow any length since it is merely a char *
   functions displaying ModuleInfo contents will display the full string.
   NeoStats core will truncate to this length for use internally. 
*/
#define MAX_MOD_NAME	32

/* doesn't have to be so big atm */
#define NUM_MODULES		20
#define S_TABLE_SIZE	-1
#define U_TABLE_SIZE	-1
#define C_TABLE_SIZE	-1
#define CHAN_MEM_SIZE	-1
#define MAXJOINCHANS	-1
#define T_TABLE_SIZE	300	/* Number of Timers */
#define B_TABLE_SIZE	100	/* Number of Bots */
#define MAXMODES		-1
#define DNS_QUEUE_SIZE  100	/* number on concurrent DNS lookups */

#define bzero(x, y)		memset(x, '\0', y);
#define is_synced		me.synced

/* do_exit call exit type definitions */
enum {
	NS_EXIT_NORMAL=0,
	NS_EXIT_SEGFAULT,
	NS_EXIT_RESTART,
}NS_EXIT_TYPE;

#define SEGV_LOCATION_BUFSIZE	255
#ifdef LEAN_AND_MEAN
#define SET_SEGV_LOCATION()
#define SET_SEGV_LOCATION_EXTRA(debug_text)
#define CLEAR_SEGV_LOCATION()
#else
#define SET_SEGV_LOCATION() snprintf(segv_location,SEGV_LOCATION_BUFSIZE,"%s %d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__); 
#define SET_SEGV_LOCATION_EXTRA(debug_text) snprintf(segv_location,SEGV_LOCATION_BUFSIZE,"%s %d %s %s", __FILE__, __LINE__, __PRETTY_FUNCTION__,(debug_text)); 
#define CLEAR_SEGV_LOCATION() segv_location[0]='\0';
#endif

#define SEGV_INMODULE_BUFSIZE	MAX_MOD_NAME
#define SET_SEGV_INMODULE(module_name) strncpy(segv_inmodule,(module_name),SEGV_INMODULE_BUFSIZE);
#define CLEAR_SEGV_INMODULE() segv_inmodule[0]='\0';

/* temporary define while module settings are ported to macro system */
#define segvinmodule segv_inmodule

int servsock;
extern char s_Services[MAXNICK];
extern const char version[];
char recbuf[BUFSIZE];
char segv_location[SEGV_LOCATION_BUFSIZE];
char segvinmodule[SEGV_INMODULE_BUFSIZE];
jmp_buf sigvbuf;

hash_t *sh;
hash_t *uh;
hash_t *ch;

/* this is the dns structure */
adns_state ads;

/* version info */
extern const char version_date[], version_time[];

typedef struct server_ Server;
typedef struct user_ User;
typedef struct chans_ Chans;
typedef struct chanmem_ Chanmem;
typedef struct modeparms_ ModesParm;

struct me {
	char name[MAXHOST];
	int port;
	int r_time;
	int lag_time;
	int numeric; /* For Unreal and any other server that needs a numeric */
	char uplink[MAXHOST];
	char pass[MAXPASS];
	char services_name[MAXHOST];
	char infoline[MAXHOST];
	char netname[MAXPASS];
	char local[MAXHOST];
	time_t t_start;
	unsigned int allbots;
	unsigned int maxsocks;
	unsigned int cursocks;
	unsigned int want_privmsg:1;
	unsigned int send_extreme_lag_notices:1;
	unsigned int onlyopers:1;
	unsigned int die:1;
	unsigned int collisions;
	unsigned int enable_proxy:1;
	unsigned int coder_debug:1;
	unsigned int noticelag:1;
	unsigned int token:1;
#if defined(ULTIMATE3) || defined(QUANTUM)
	unsigned int client:1;
#endif
	int action;
	char message[BUFSIZE];
	char chan[BUFSIZE];
	unsigned int onchan:1;
	unsigned int synced:1;
	Server *s;
	int requests;
	long SendM;
	long SendBytes;
	long RcveM;
	long RcveBytes;
	time_t lastmsg;
	int pingtime;
} me;


struct Servbot {
	char user[MAXUSER];
	char host[MAXHOST];
} Servbot;



struct server_ {
	char name[MAXHOST];
	int hops;
	long hash;
	time_t connected_since;
	int ping;
	char uplink[MAXHOST];
	void *moddata[NUM_MODULES];
};

struct user_ {
	char nick[MAXNICK];
	char hostname[MAXHOST];
	char username[MAXUSER];
	char realname[MAXREALNAME];
	char vhost[MAXHOST];
	Server *server;
	int flood;
	int is_away;
	time_t t_flood;
	long hash;
	char modes[MODESIZE];
	int ulevel;
	long Umode;
	list_t *chans;
	struct in_addr ipaddr;
	time_t TS;
	long Smode;
	void *moddata[NUM_MODULES];
};

struct chans_ {
	char name[CHANLEN];
	long cur_users;
	long modes;
	list_t *chanmembers;
	list_t *modeparms;
	char topic[BUFSIZE];
	char topicowner[MAXHOST];	/* becuase a "server" can be a topic owner */
	time_t topictime;
	void *moddata[NUM_MODULES];
	time_t tstime;
} chans_;

struct chanmem_ {
	char nick[MAXNICK];
	time_t joint;
	long flags;
	void *moddata[NUM_MODULES];
} chanmem_;

struct modeparms_ {
	long mode;
	char param[PARAMSIZE];
	void *moddata[NUM_MODULES];
} modeparms_;


struct ping {
	time_t last_sent;
	int ulag;
} ping;



/* sock.c */
int ConnectTo (char * host, int port);
void read_loop (void);
int getmaxsock (void);
int sock_connect (int socktype, unsigned long ipaddr, int port, char *sockname, char *module, char *func_read, char *func_write, char *func_error);
int sock_disconnect (char *sockname);

/* conf.c */
void strip (char * line);
int ConfLoad (void);
void rehash (void);
int init_modules (void);

/* main.c */
void *smalloc (long size);
char *sstrdup (const char * s);
char *strlower (char * s);
void AddStringToList (char ***List, char S[], int *C);
void FreeList (char **List, int C);
void do_exit (int exitcode);
void strip_mirc_codes(char *text);
char *sctime (time_t t);
char *sftime (time_t t);

/* ircd.c */
void parse (char* line);
char *joinbuf (char **av, int ac, int from);
int split_buf (char *buf, char ***argv, int colon_special);
int flood (User * u);
int init_bot (char * nick, char * user, char * host, char * rname, char *modes, char * modname);
int del_bot (char * nick, char * reason);
void Module_Event (char * event, char **av, int ac);

/* ircd specific files */
void prefmsg (char * to, const char * from, char * fmt, ...);
void privmsg (char *to, const char *from, char *fmt, ...);
void notice (char *to, const char *from, char *fmt, ...);
void privmsg_list (char *to, char *from, const char **text);
void globops (char * from, char * fmt, ...);

/* dl.c */
int bot_nick_change (char * oldnick, char *newnick);

/* timer.c */
void chk (void);
void TimerReset (void);
void TimerPings (void);
void TimerMidnight (void);
int is_midnight (void);

/* users.c */
void AddUser (const char *nick, const char *user, const char *host, const char *server, const unsigned long ip, const unsigned long TS);
void DelUser (const char *nick);
void AddRealName (const char *nick, const char *realname);
void Change_User (User *u, const char * newnick);
void sendcoders (char *message, ...);
User *finduser (const char *nick);
void UserDump (char *nick);
void part_u_chan (list_t *list, lnode_t *node, void *v);
void UserMode (const char *nick, const char *modes, int smode);
void init_user_hash (void);
int UserLevel (User *u);
void Do_Away (User *u, const char *awaymsg);
void KillUser (const char *nick);

/* server.c */
void AddServer (char *name, char *uplink, int hops);
void DelServer (char *name);
Server *findserver (const char *name);
void ServerDump (void);
void init_server_hash (void);

/* ns_help.c */
extern const char *ns_help[];
extern const char *ns_help_on_help[];
extern const char *ns_myuser_help[];
extern const char *ns_shutdown_help[];
extern const char *ns_reload_help[];
extern const char *ns_logs_help[];
#ifdef USE_RAW
extern const char *ns_raw_help[];
#endif
extern const char *ns_debug_help[];
extern const char *ns_userdump_help[];
extern const char *ns_chandump_help[];
extern const char *ns_serverdump_help[];
extern const char *ns_version_help[];
extern const char *ns_load_help[];
extern const char *ns_unload_help[];
extern const char *ns_modlist_help[];
extern const char *ns_jupe_help[];
extern const char *ns_level_help[];
extern const char *ns_modbotlist_help[];
extern const char *ns_modsocklist_help[];
extern const char *ns_modtimerlist_help[];
extern const char *ns_modbotchanlist_help[];
extern const char *ns_info_help[];

/* services.c */
void servicesbot (char *nick, char **av, int ac);
void ns_debug_to_coders (char *u);
void ns_shutdown (User * u, char *reason);

/* chans.c */
void chandump (char *chan);
void part_chan (User * u, char *chan);
void join_chan (User * u, char *chan);
void change_user_nick (Chans * c, char *newnick, char *oldnick);
Chans *findchan (char *chan);
int ChanMode (char *origin, char **av, int ac);
void Change_Topic (char *, Chans *, time_t t, char *);
void ChangeChanUserMode (Chans * c, User * u, int add, long mode);
void kick_chan (User *, char *, User *);
void Change_Chan_Ts (Chans * c, time_t tstime);
int CheckChanMode (Chans * c, long mode);
int IsChanMember(Chans *c, User *u);
void init_chan_hash (void);

/* dns.c */
int dns_lookup (char *str, adns_rrtype type, void (*callback) (char *data, adns_answer * a), char *data);
int init_dns (void);
void do_dns (void);

#endif
