/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      statserv.h, 
** Version: 1.6
** Date:    29/03/2001
*/

#ifndef STATSERV_H
#define STATSERV_H

#include "dl.h"
#include "m_stats.h"
#include "stats.h"
#include "dotconf.h"

#define SSMNAME "statserv"


extern char s_StatServ[MAXNICK];

typedef struct tld_ TLD;
typedef struct server_stats SStats;
typedef struct chan_stats CStats;
hash_t *Shead;
list_t *Chead;
TLD *tldhead;


extern const char version_date[], version_time[];

struct stats_network_ {
	int opers;
	int chans;
	int maxopers;
	long users;
	long totusers;
	long maxusers;
	long away;
	int servers;
	int maxservers;
	time_t t_maxopers;
	time_t t_maxusers;
	time_t t_maxservers;
	int requests;
	long maxchans;
	time_t t_chans;
} stats_network;


struct StatServ {
	char nick[MAXNICK];
	char user[MAXUSER];
	char host[MAXHOST];
	int lag;
	int html;
	char htmlpath[BUFSIZE];
	int onchan;
} StatServ;


struct server_stats {
	SStats *next, *prev;
	char name[MAXHOST];
	unsigned int users;
	int opers;
	int lowest_ping;
	int highest_ping;
	time_t t_lowest_ping;
	time_t t_highest_ping;
	int numsplits;
	long maxusers;
	time_t t_maxusers;
	int maxopers;
	time_t t_maxopers;
	time_t lastseen;
	time_t starttime;
	unsigned int operkills;
	unsigned int serverkills;
	long totusers;
	long daily_totusers;
};

struct chan_stats {
	char name[CHANLEN];
	long members;
	long topics;
	long totmem;
	long kicks;
	long topicstoday;
	long joinstoday;
	long maxkickstoday;
	long maxmemtoday;
	time_t t_maxmemtoday;
	time_t lastseen;
	long maxmems;
	time_t t_maxmems;
	long maxkicks;
	time_t t_maxkicks;
	long maxjoins;
	time_t t_maxjoins;
};

struct daily_ {
	int servers;
	time_t t_servers;
	int users;
	time_t t_users;
	int opers;
	time_t t_opers;
	int tot_users;
	long chans;
	time_t t_chans;
} daily;

struct tld_ {
	TLD *next;
	char tld[4];
	char *country;
	int users;
	int daily_users;
};

/* statserv.c */
extern void statserv(char *);
extern void re_init_bot();

/* stats.c */
extern TLD *tldhead;
extern TLD *findtld(char *);
extern TLD *AddTLD(User *);
extern void LoadTLD();
extern void init_tld();
extern void AddStats(Server *);
extern SStats *findstats(char *);
extern void SaveStats();
extern void LoadStats();
extern int Online(void *);
extern int pong(Server *);
extern int s_user_away(User *);
extern int s_new_server(Server *);
extern int s_del_server(Server *);
extern int s_new_user(User *);
extern int s_del_user(User *);
extern int s_user_modes(User *);
extern int s_user_kill(User *);
extern void s_chan_new(Chans *);
extern void s_chan_del(Chans *);
void s_chan_join(Chans *);
void s_chan_part(Chans *);
CStats *findchanstats(char *);
CStats *AddChanStats(char *);
void DelOldChan();
void s_topic_change(Chans *);
void s_chan_kick(Chans *c);


/* ss_help.c */
extern const char *ss_help[];
extern const char *ss_myuser_help[];
extern const char *ss_server_help[];
extern const char *ss_map_help[];
extern const char *ss_netstats_help[];
extern const char *ss_daily_help[];
extern const char *ss_tld_help[];
extern const char *ss_tld_map_help[];
extern const char *ss_operlist_help[];
extern const char *ss_botlist_help[];
extern const char *ss_version_help[];
extern const char *ss_reset_help[];
extern const char *ss_stats_help[];
extern const char *ss_join_help[];
extern const char *ns_join_help[];
extern const char *ns_raw_help[];
extern const char *ns_debug_help[];
extern const char *ns_userdump_help[];
extern const char *ns_chandump_help[];
extern const char *ns_serverdump_help[];
extern const char *icq_help[];
extern const char *ss_htmlstats_help[];
extern const char *ss_forcehtml_help[];
extern const char *ss_notices_help[];

/* tld.c */
extern void DelTLD(User *u);



#endif
