/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: stats.h,v 1.9 2000/03/02 01:31:24 fishwaldo Exp $
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

#include "Unreal.h"
#include "config.h"

/* Define this to enable Debug Code */

#define CHANLEN			32
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
#define T_TABLE_SIZE	100 /* Number of Timers */
#define B_TABLE_SIZE	100 /* Number of Bots */

#define bzero(x, y)		memset(x, '\0', y);

int servsock;
int times;
extern char s_Debug[MAXNICK], s_Services[MAXNICK];
extern const char version[];
char *recbuf, *segv_location;

typedef struct server_ Server;
typedef struct user_ User;
typedef struct myuser_ MyUser;
typedef struct chans_ Chans;
typedef struct config_mod_ Config_Mod;


struct me {
	char name[MAXHOST];
	char *modpath;
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
	int action;
	char *message;
	char *chan;
	unsigned int onchan : 1;
	unsigned int usesmo : 1;
	Server *s;
	int requests;
} me;


struct Servbot {
	char user[MAXUSER];
	char host[MAXHOST];
} Servbot;

struct chans_ {
	Chans *next, *prev;
	char name[CHANLEN];
	long cur_users;
	long hash;
	char *modes;
	unsigned int is_priv : 1;
	unsigned int is_secret : 1;
	unsigned int is_invite : 1;
	unsigned int is_mod : 1;
	unsigned int is_outside : 1;
	unsigned int is_optopic : 1;
	unsigned int is_regchan : 1;
	unsigned int is_regnick : 1;
	unsigned int is_nocolor : 1;
	unsigned int is_nokick : 1;
	unsigned int is_ircoponly : 1;
	unsigned int is_Svrmode : 1;
	unsigned int is_noknock : 1;
	unsigned int is_noinvite : 1;
	unsigned int is_stripcolor : 1;
	User *users;
	char *topic;
	char *topicowner;
} chans_;


struct server_ {
	Server *next, *prev;
	char name[MAXHOST];
	int hops;
	long hash;
	time_t connected_since;
	time_t last_announce;
	int ping;
	char uplink[MAXHOST];
};

struct user_ {
	User *next, *prev;
	char nick[MAXNICK];
	char *hostname;
	char *username;
	Server *server;
	MyUser *myuser;
	int flood;
	int is_away;
	time_t t_flood;
	long hash;
	char *modes;
	int *ulevel;
	long Umode;
};

struct ping {
    time_t last_sent;
    int ulag;
} ping;


struct myuser_ {
	MyUser *next;
	char *username;
	char *password;
	time_t lastseen;
	unsigned int ison : 1;
};

/* sock.c */
extern int ConnectTo(char *, int);
extern void read_loop();
extern void sts(char *, ...);
extern void log(char *, ...);
extern void ResetLogs();
extern char *sctime(time_t);
extern char *sftime(time_t);
extern void notice(char *,char *, ...);


/* conf.c */
extern void strip(char *);
extern void ConfLoad();
extern void rehash();
extern int init_modules();

/* main.c */
extern void login();
extern void init_statserv();
extern void init_spam();
extern void init_ServBot();
extern void *smalloc(long);
extern char *sstrdup(const char *);
extern unsigned long HASH(const unsigned char *, int);
extern char *strlower(char *);

/* ircd.c */
extern void parse();
extern void privmsg(char *, const char *, char *, ...);
extern void privmsg_list(char *, char *, const char **);
extern void globops(char *, char *, ...);
extern int flood(User *);
extern int init_bot(char *, char *, char *, char *, char *,char *);
extern int del_bot(char *, char *);
extern void Module_Event(char *, void *);
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
extern void Addchan(char *);
extern Chans *findchan(char *);
extern void ChanMode(char *, char *);
extern void ChanTopic(char *, char *, char *);
extern void DelChan(char *);
extern void AddUser(char *, char *, char *, char *);
extern void DelUser(char *);
extern void Change_User(User *, char *);
extern void sendcoders(char *message,...);
extern User *finduser(char *);
extern void UserDump();
extern void UserMode(char *, char *);
extern void init_user_hash();
extern void init_chan_hash();
extern void AddServer(char *, char *,int);
extern void DelServer(char *);
extern Server *findserver(char *);
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

#ifdef ICQSERV
/* icq.c */
extern void IcqServer(char *);
extern int icq_start();
extern int doICQProtocol();
#endif

/* dl.c */
extern void __init_mod_list();
extern int load_module(char *path,User *u);
extern int unload_module(char *module_name,User *u);
extern int add_ld_path(char *path);
extern void list_module(User *);
extern void list_module_bots(User *);
extern int add_mod_user(char *nick, char *mod_name);
extern int del_mod_user(char *nick);
extern int add_mod_timer(char *func_name, char *timer_name, char *mod_name, int interval);
extern int del_mod_timer(char *timer_name);
extern void list_module_timer(User *);
extern int add_socket(char *func_name, char *sock_name, int socknum, char *mod_name);
extern int del_socket(char *sockname);
extern void list_sockets(User *);




/* services.c */
extern void servicesbot(char *nick, char *line);
extern void ns_debug_to_coders(char *);
extern void ns_shutdown(User *, char *);

#endif
