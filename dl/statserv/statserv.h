/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

#include "neostats.h"
#include "m_stats.h"

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
typedef struct region_ Region;
typedef struct server_stats SStats;
typedef struct chan_stats CStats;
typedef struct irc_client_version CVersions;
hash_t *Shead;
list_t *Chead;
list_t *Thead;
list_t *Vhead;

struct stats_network_ {
	long opers;
	long chans;
	long maxopers;
	long users;
	long totusers;
	long maxusers;
	long away;
	long servers;
	long maxservers;
	time_t t_maxopers;
	time_t t_maxusers;
	time_t t_maxservers;
	long maxchans;
	time_t t_chans;
} stats_network;


struct StatServ { 
	char user[MAXUSER]; 
	char host[MAXHOST]; 
	char rname[MAXREALNAME]; 
	int lagtime; 
	int lagalert; 
	int recordalert; 
	int html; 
	char htmlpath[MAXPATH]; 
	int onchan; 
	int newdb; 
	int msginterval; 
	int msglimit; 
	int shutdown; 
	int exclusions; 
	int GeoDBtypes;
} StatServ;


struct server_stats {
	SStats *next, *prev;
	char name[MAXHOST];
	unsigned int users;
	int opers;
	long lowest_ping;
	long highest_ping;
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
	int chans;
	time_t t_chans;
} daily;

struct tld_ {
	char tld[5];
	char *country;
	int users;
	int daily_users;
	/* for region/isp edition of GeoIP */
	list_t *rl;
};

struct region_ {
	char *region;
	int users;
	int daily_users;
};

/* statserv.c */
void statserv(char *);
int topchan(const void *key1, const void *key2);
int topjoin(const void *key1, const void *key2);
int topkick(const void *key1, const void *key2);
int toptopics(const void *key1, const void *key2);
int topversions(const void *key1, const void *key2);
/* stats.c */
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

/* database.c */
void save_chan(CStats *c);
CStats *load_chan(char *name);

/* ss_help.c */
extern const char *ss_help_about[];
extern const char *ss_help_server[];
extern const char *ss_help_map[];
extern const char *ss_help_netstats[];
extern const char *ss_help_daily[];
extern const char *ss_help_tld[];
extern const char *ss_help_tldmap[];
extern const char *ss_help_operlist[];
#ifdef GOTBOTMODE
extern const char *ss_help_botlist[];
#endif
extern const char *ss_help_version[];
extern const char *ss_help_stats[];
extern const char *ss_help_htmlstats[];
extern const char *ss_help_forcehtml[];
extern const char *ss_help_chan[];
extern const char *ss_help_set_htmlpath[];
extern const char *ss_help_set_html[];
extern const char *ss_help_set_exclusions[];
extern const char *ss_help_set_msginterval[];
extern const char *ss_help_set_msglimit[];
extern const char *ss_help_set_lagtime[];
extern const char *ss_help_set_lagalert[];
extern const char *ss_help_set_recordalert[];
extern const char *ss_help_clientversions[];

extern const char ss_help_about_oneline[];
extern const char ss_help_version_oneline[];
extern const char ss_help_server_oneline[];
extern const char ss_help_map_oneline[];
extern const char ss_help_chan_oneline[];
extern const char ss_help_netstats_oneline[];
extern const char ss_help_daily_oneline[];
extern const char ss_help_tld_oneline[];
extern const char ss_help_tldmap_oneline[];
extern const char ss_help_operlist_oneline[];
#ifdef GOTBOTMODE
extern const char ss_help_botlist_oneline[];
#endif
extern const char ss_help_clientversions_oneline[];
extern const char ss_help_forcehtml_oneline[];
extern const char ss_help_stats_oneline[];

/* tld.c */
void DelTLD(User * u);
int sortusers(const void *v, const void *v2);
void ResetTLD();
void DisplayTLDmap(User *u);
void AddTLD(User *);
void init_tld();
void fini_tld();
/* htmlstats.c */
void ss_html(void);

#endif
