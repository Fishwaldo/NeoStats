/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      stats.h, 
** Version: 1.11
** Date:    17/11/2001
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
#include "list.h"
#include "hash.h"
#include "config.h"

#if UNREAL == 1
#include "Unreal.h"
#elif ULTIMATE == 1
#include "Ultimate.h"
#else
#error Error, you must select a IRCD to use. See ./configure --help for more information
#endif


/* Define this to enable Recived Line Logging - Only enable if Coders ask you to! */

#define RECVLOG 

/* this is a security hack to give the coders the right levels to debug a NeoStats. Don't define unless we ask you to */

#undef CODERHACK


#define CHANLEN			50
#define BUFSIZE			512
#define CONFIG_NAME		"stats.cfg"
#define MAXHOST			63
#define MAXPASS			32
#define MAXNICK			32
#define MAXUSER			10
#define NUM_MODULES		255
#define MAX_SOCKS		10
#define S_TABLE_SIZE	97
#define U_TABLE_SIZE	1999
#define D_TABLE_SIZE	1999
#define C_TABLE_SIZE	1999
#define CHAN_MEM_SIZE	1000
#define MAXJOINCHANS	1000
#define T_TABLE_SIZE	100 /* Number of Timers */
#define B_TABLE_SIZE	100 /* Number of Bots */
#define MAXMODES	CHAN_MEM_SIZE



#define bzero(x, y)		memset(x, '\0', y);
#define is_synced	me.synced



int servsock;
int times;
extern char s_Debug[MAXNICK], s_Services[MAXNICK];
extern const char version[];
char recbuf[BUFSIZE], segv_location[255];
char segvinmodule[30];
jmp_buf sigvbuf;

hash_t *sh;
hash_t *uh;
hash_t *ch;


typedef struct server_ Server;
typedef struct user_ User;
typedef struct myuser_ MyUser;
typedef struct chans_ Chans;
typedef struct config_mod_ Config_Mod;
typedef struct chanmem_ Chanmem;
typedef struct modeparms_ ModesParm;

struct me {
	char name[MAXHOST];
	char modpath[BUFSIZE];
	int port;
	int r_time;
	int lag_time;
	char uplink[MAXHOST];
	char pass[MAXPASS];
	char services_name[MAXHOST];
        char infoline[MAXHOST];
        char netname[MAXPASS];
	time_t t_start;
	unsigned int enable_spam : 1;
	unsigned int want_privmsg : 1;
	unsigned int send_extreme_lag_notices : 1;
	unsigned int onlyopers : 1;
	unsigned int collisions;
	unsigned int enable_proxy : 1;
	unsigned int coder_debug : 1;
	unsigned int noticelag : 1;
	unsigned int token : 1;
	int action;
	char message[BUFSIZE];
	char chan[BUFSIZE];
	unsigned int onchan : 1;
	unsigned int usesmo : 1;
	unsigned int synced : 1;
	Server *s;
	int requests;
	long SendM;
	long SendBytes;
	long RcveM;
	long RcveBytes;
	time_t lastmsg;
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
	time_t last_announce;
	int ping;
	char uplink[MAXHOST];
};

struct user_ {
	char nick[MAXNICK];
	char hostname[BUFSIZE];
	char username[BUFSIZE];
	char vhost[BUFSIZE];
	Server *server;
	MyUser *myuser;
	int flood;
	int is_away;
	time_t t_flood;
	long hash;
	char modes[BUFSIZE];
	int ulevel;
	long Umode;
	list_t *chans;
};

struct chans_ {
	char name[CHANLEN];
	long cur_users;
	long modes;
	list_t *chanmembers;
	list_t *modeparms;
	char topic[BUFSIZE];
	char topicowner[BUFSIZE];
	time_t topictime;
} chans_;

struct chanmem_ {
	char nick[MAXNICK];
	time_t joint;
	long flags;
} chanmem_;

struct modeparms_ {
	long mode;
	char param[512];
} modeparms_;


struct ping {
    time_t last_sent;
    int ulag;
} ping;



/* sock.c */
extern int ConnectTo(char *, int);
extern void read_loop();
extern void log(char *, ...);
extern void ResetLogs();
extern char *sctime(time_t);
extern char *sftime(time_t);



/* conf.c */
extern void strip(char *);
extern void ConfLoad();
extern void rehash();
extern int init_modules();

/* main.c */
extern void login();
/* extern void init_statserv(); */
extern void init_spam();
extern void init_ServBot();
extern void *smalloc(long);
extern char *sstrdup(const char *);
extern unsigned long HASH(const unsigned char *, int);
extern char *strlower(char *);
extern void AddStringToList(char ***List,char S[],int *C);
void FreeList(char **List,int C);

/* ircd.c */
extern void parse();
extern char *joinbuf(char **av, int ac, int from);
extern int split_buf(char *buf, char ***argv, int colon_special);
extern void privmsg(char *, const char *, char *, ...);
extern void privmsg_list(char *, char *, const char **);
extern void globops(char *, char *, ...);
extern int flood(User *);
extern int init_bot(char *, char *, char *, char *, char *,char *);
extern int del_bot(char *, char *);
extern void Module_Event(char *, char **av, int ac);
extern int bot_nick_change(char *, char *);

/* timer.c */
extern void chk();
extern void TimerReset();
extern void TimerSpam();
extern void TimerPings();
extern void TimerMidnight();
extern int is_midnight();

/* users.c */
extern Server *serverlist[S_TABLE_SIZE];
extern User *userlist[U_TABLE_SIZE];
extern MyUser *myuhead;
extern void AddUser(const char *, const char *, const char *, const char *);
extern void DelUser(const char *);
extern void Change_User(User *, const char *);
extern void sendcoders(char *message,...);
extern User *finduser(const char *);
extern void UserDump(char *);
extern void UserMode(const char *, const char *);
extern void init_user_hash();
extern void init_chan_hash();
extern void AddServer(char *, char *,int);
extern void DelServer(char *);
extern Server *findserver(const char *);
extern void ServerDump();
extern void ChanDump();
extern void init_server_hash();
extern void LoadMyUsers();
extern void SaveMyUsers();
extern void DeleteMyUser(char *);
extern MyUser *findmyuser(char *);
extern int UserLevel(User *);


/* ns_help.c */
extern const char *ns_help[];
extern const char *ns_myuser_help[];
extern const char *ns_coder_help[];
extern const char *ns_shutdown_help[];
extern const char *ns_reload_help[];
extern const char *ns_logs_help[];
extern const char *ns_join_help[];
extern const char *ns_raw_help[];
extern const char *ns_debug_help[];
extern const char *ns_userdump_help[];
extern const char *ns_chandump_help[];
extern const char *ns_serverdump_help[];
extern const char *ns_version_help[];
extern const char *ns_load_help[];
extern const char *ns_unload_help[];
extern const char *ns_modlist_help[];
extern const char *ns_raw_help[];
extern const char *ns_jupe_help[];
extern const char *ns_level_help[];



/* services.c */
extern void servicesbot(char *nick, char **av, int ac);
extern void ns_debug_to_coders(char *);
extern void ns_shutdown(User *, char *);


/* chans.c */
extern void chandump(char *chan);
extern void part_chan(User *u, char *chan);
extern void join_chan(User *u, char *chan);
extern void change_user_nick(Chans *c, char *newnick, char *oldnick);
extern Chans *findchan(char *chan);
extern int ChanMode(char *origin, char **av, int ac);
extern void Change_Topic(char *, Chans *, time_t t, char *);
extern void ChangeChanUserMode(Chans *c, User *u, int add, long mode);
#endif

