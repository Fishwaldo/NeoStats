/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: statserv.h,v 1.3 2000/02/06 07:12:46 fishwaldo Exp $
*/

#ifndef STATSERV_H
#define STATSERV_H

#include "dl.h"
#include "stats.h"
#include "dotconf.h"


extern char s_StatServ[MAXNICK];

typedef struct tld_ TLD;
typedef struct server_stats SStats;

struct stats_network_ {
	int opers;
	int chans;
	int maxopers;
	long users;
	long totusers;
	long maxusers;
	int servers;
	int maxservers;
	time_t t_maxopers;
	time_t t_maxusers;
	time_t t_maxservers;
	int requests;
} stats_network;


struct StatServ {
	char nick[MAXNICK];
	char user[MAXUSER];
	char host[MAXHOST];
	int lag;
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

struct daily {
	int servers;
	time_t t_servers;
	int users;
	time_t t_users;
	int opers;
	time_t t_opers;
	int tot_users;
} daily;

struct tld_ {
	TLD *next;
	char tld[4];
	char *country;
	int users;
};

/* statserv.c */
extern void statserv(char *);

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


#endif