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

/* this is the max number of statserv channels our database can hold... */
#define SS_CHAN_SIZE 10240


extern char s_StatServ[MAXNICK];

typedef struct tld_ TLD;
typedef struct server_stats SStats;
typedef struct chan_stats CStats;
hash_t *Shead;
list_t *Chead;
TLD *tldhead;


#define ok_to_wallop	!StatServ.newdb && StatServ.onchan && me.synced


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
	int newdb;
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
 void statserv(char *);
 void re_init_bot();
int topchan(const void *key1, const void *key2);
int topjoin(const void *key1, const void *key2);
int topkick(const void *key1, const void *key2);
int toptopics(const void *key1, const void *key2);

/* stats.c */
 TLD *tldhead;
 TLD *findtld(char *);
 TLD *AddTLD(User *);
 void LoadTLD();
 void init_tld();
 void AddStats(Server *);
 SStats *findstats(char *);
 void SaveStats();
 void LoadStats();
 int Online(char **av, int ac);
 int pong(char **av, int ac);
 int s_user_away(char **av, int ac);
 int s_new_server(char **av, int ac);
 int s_del_server(char **av, int ac);
 int s_new_user(char **av, int ac);
 int s_del_user(char **av, int ac);
 int s_user_modes(char **av, int ac);
 int s_user_kill(char **av, int ac);
 int s_chan_new(char **av, int ac);
 int s_chan_del(char **av, int ac);
 int s_chan_join(char **av, int ac);
 int s_chan_part(char **av, int ac);
CStats *findchanstats(char *);
CStats *AddChanStats(char *);
void DelOldChan();
 int s_topic_change(char **av, int ac);
 int s_chan_kick(char **av, int ac);


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
extern const char *ss_chan_help[];
/* tld.c */
 void DelTLD(User *u);



#endif
