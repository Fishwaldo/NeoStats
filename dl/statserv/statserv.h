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

#ifndef STATSERV_H
#define STATSERV_H

#include "dl.h"
#include "m_stats.h"
#include "stats.h"
#include "dotconf.h"

/* this is the max number of statserv channels our database can hold... */
#define SS_CHAN_SIZE -1

/* this is the how often to save a portion of the DB. Don't alter this unless you need to */
/* DO NOT set PROGCHANTIME less than ((DBSAVETIME + (DBSAVETIME/2)) * 4) otherwise you will not have the enitre database progressively saved! */

/* try a save every 10 minutes */
#define DBSAVETIME 600
/* but only save data older than 1 hour! */
#define PROGCHANTIME 3600

extern char s_StatServ[MAXNICK];
extern ModuleInfo __module_info;

typedef struct tld_ TLD;
typedef struct server_stats SStats;
typedef struct chan_stats CStats;
typedef struct irc_client_version CVersions;
hash_t *Shead;
list_t *Chead;
TLD *tldhead;
list_t *Vhead;

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
	char user[MAXUSER];
	char host[MAXHOST];
	char rname[MAXREALNAME];
	int lag;
	int html;
	char htmlpath[255];
	int onchan;
	int newdb;
	int interval;
	int shutdown;
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

#define MAX_CLIENT_VERSION_NAME	512

struct irc_client_version {
	char name[MAX_CLIENT_VERSION_NAME];
	int count;
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
	time_t lastsave;
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
	char tld[5];
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
int topversions(const void *key1, const void *key2);
/* stats.c */
TLD *tldhead;
TLD *findtld(char *);
TLD *AddTLD(User *);
void LoadTLD();
void init_tld();
int s_client_version(char **av, int ac);
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
#if 0
CStats *AddChanStats(char *);
#endif
void DelOldChan();
int s_topic_change(char **av, int ac);
int s_chan_kick(char **av, int ac);
int ok_to_wallop();

/* database.c */
void save_chan(CStats *c);
CStats *load_chan(char *name);

/* ss_help.c */
extern const char *ss_help[];
extern const char *ss_help_on_help[];
extern const char *ss_myuser_help[];
extern const char *ss_about_help[];
extern const char *ss_server_help[];
extern const char *ss_map_help[];
extern const char *ss_netstats_help[];
extern const char *ss_daily_help[];
extern const char *ss_tld_help[];
extern const char *ss_tld_map_help[];
extern const char *ss_operlist_help[];
extern const char *ss_botlist_help[];
extern const char *ss_version_help[];
extern const char *ss_stats_help[];
extern const char *ss_htmlstats_help[];
extern const char *ss_forcehtml_help[];
extern const char *ss_chan_help[];
extern const char *ss_set_help[];
extern const char *ss_clientversions_help[];
/* tld.c */
void DelTLD(User * u);



#endif
