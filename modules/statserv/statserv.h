/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* this is the how often to save a portion of the DB. Don't alter this unless you need to */
/* DO NOT set PROGCHANTIME less than ((DBSAVETIME + (DBSAVETIME/2)) * 4) otherwise you will not have the enitre database progressively saved! */

/* try a save every 10 minutes */
#define DBSAVETIME 600
/* but only save data older than 1 hour! */
#define PROGCHANTIME 3600

extern Bot *ss_bot;
extern Module* ss_module;

extern hash_t *Shead;
extern list_t *Chead;
extern list_t *Thead;
extern list_t *Vhead;

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
	int lagtime; 
	int lagalert; 
	int recordalert; 
	int html; 
	char htmlpath[MAXPATH]; 
	int msginterval; 
	int msglimit; 
	int shutdown; 
	int exclusions; 
	int GeoDBtypes;
} StatServ;


typedef struct SStats {
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
	time_t t_lastseen;
	time_t t_start;
	unsigned int operkills;
	unsigned int serverkills;
	long totusers;
	long daily_totusers;
}SStats;

typedef struct CVersions {
	char name[BUFSIZE];
	int count;
}CVersions;

typedef struct CStats {
	char name[MAXCHANLEN];
	long members;
	long topics;
	long totmem;
	long kicks;
	long topicstoday;
	long joinstoday;
	long maxkickstoday;
	long maxmemtoday;
	time_t t_maxmemtoday;
	time_t t_lastseen;
	long maxmems;
	time_t t_maxmems;
	long maxkicks;
	time_t t_maxkicks;
	long maxjoins;
	time_t t_maxjoins;
	time_t lastsave;
}CStats;

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

typedef struct TLD {
	char tld[5];
	char country[32];
	int users;
	int daily_users;
	/* for region/isp edition of GeoIP */
	list_t *rl;
}TLD;

typedef struct Region {
	char *region;
	int users;
	int daily_users;
}Region;

int StatsMidnight(void);
/* statserv.c */
int topchan(const void *key1, const void *key2);
int topjoin(const void *key1, const void *key2);
int topkick(const void *key1, const void *key2);
int toptopics(const void *key1, const void *key2);
int topversions(const void *key1, const void *key2);
/* stats.c */
void list_client_versions(Client * u, int num);
int load_client_versions(void);
int save_client_versions(void);
void StatsAddServer(Client *s);
void StatsDelServer(Client *s);
void StatsServerPong(Client *s);
SStats *findserverstats(char *name);
void StatsAddCTCPVersion(char* version);
void StatsAddUser(Client * u);
void StatsQuitUser(Client * u);
void StatsKillUser(Client * u);
void StatsUserMode(Client * u, char *modes);
void StatsUserAway(Client * u);
void InitStats(void);
void FiniStats(void);

int SaveStats(void);
void LoadStats(void);

CStats *findchanstats(char *name);
void StatsAddChan(Channel* c);
void StatsDelChan(Channel* c);
void StatsJoinChan(Client * u, Channel* c);
void StatsPartChan(Client * u, Channel* c);
void StatsChanTopic(Channel* c);
void StatsChanKick(Channel* c);
int DelOldChan(void);

/* database.c */
void save_chan(CStats *c);
CStats *load_chan(char *name);

/* ss_help.c */
extern const char *ss_about[];
extern const char *ss_help_server[];
extern const char *ss_help_map[];
extern const char *ss_help_netstats[];
extern const char *ss_help_daily[];
extern const char *ss_help_tld[];
extern const char *ss_help_tldmap[];
extern const char *ss_help_operlist[];
extern const char *ss_help_botlist[];
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

extern const char ss_help_server_oneline[];
extern const char ss_help_map_oneline[];
extern const char ss_help_chan_oneline[];
extern const char ss_help_netstats_oneline[];
extern const char ss_help_daily_oneline[];
extern const char ss_help_tld_oneline[];
extern const char ss_help_tldmap_oneline[];
extern const char ss_help_operlist_oneline[];
extern const char ss_help_botlist_oneline[];
extern const char ss_help_clientversions_oneline[];
extern const char ss_help_forcehtml_oneline[];
extern const char ss_help_stats_oneline[];

/* tld.c */
void DelTLD(Client * u);
int sortusers(const void *v, const void *v2);
void ResetTLD();
void DisplayTLDmap(Client *u);
void AddTLD(Client *);
void InitTLD(void);
void FiniTLD(void);
/* htmlstats.c */
int ss_html(void);

#endif
