/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
** $Id: database.c,v 1.17 2003/09/15 10:39:39 fishwaldo Exp $
*/


#include "stats.h"
#include "conf.h"
#include "statserv.h"
#include "log.h"

void LoadOldStats();

void SaveStats()
{
	SStats *s;
	CStats *c;
	hnode_t *sn;
	lnode_t *cn;
	hscan_t ss;
	strcpy(segv_location, "StatServ-SaveStats");

	if (StatServ.newdb == 1) {
		chanalert(s_StatServ, "Enabling Record yelling!");
		StatServ.newdb = 0;
	}

	/* first thing we do is clear the old database */
	DelTable("ServerStats");
	DelTable("ChanStats");
	DelTable("NetStats");
	
	/* ok, run through the server stats, and save them */
	hash_scan_begin(&ss, Shead);
	while ((sn = hash_scan_next(&ss))) {
		s = hnode_get(sn);
		nlog(LOG_DEBUG1, LOG_MOD,
		     "Writing statistics to database for %s", s->name);
		SetData((void *)s->numsplits, CFGINT, "ServerStats", s->name, "Splits");
		SetData((void *)s->maxusers, CFGINT, "ServerStats", s->name, "MaxUsers");
		SetData((void *)s->t_maxusers, CFGINT, "ServerStats", s->name, "MaxUsersTime");
		SetData((void *)s->lastseen, CFGINT, "ServerStats", s->name, "LastSeen");
		SetData((void *)s->starttime, CFGINT, "ServerStats", s->name, "StartTime");
		SetData((void *)s->operkills, CFGINT, "ServerStats", s->name, "OperKills");
		SetData((void *)s->serverkills, CFGINT, "ServerStats", s->name, "ServerKills");
		SetData((void *)s->totusers, CFGINT, "ServerStats", s->name, "TotalUsers");
		SetData((void *)s->maxopers, CFGINT, "ServerStats", s->name, "MaxOpers");
		SetData((void *)s->t_maxopers, CFGINT, "ServerStats", s->name, "MaxOpersTime");
	}

	/* ok, Now Channel Stats */

	cn = list_first(Chead);
	while (cn) {
		c = lnode_get(cn);
		nlog(LOG_DEBUG1, LOG_MOD,
		     "Writting Statistics to database for %s", c->name);
		SetData((void *)c->topics, CFGINT, "ChanStats", c->name, "Topics");
		SetData((void *)c->totmem, CFGINT, "ChanStats", c->name, "TotalMems");
		SetData((void *)c->kicks, CFGINT, "ChanStats", c->name, "Kicks");
		SetData((void *)c->lastseen, CFGINT, "ChanStats", c->name, "LastSeen");
		SetData((void *)c->maxmems, CFGINT, "ChanStats", c->name, "MaxMems");
		SetData((void *)c->t_maxmems, CFGINT, "ChanStats", c->name, "MaxMemsTime");
		SetData((void *)c->maxkicks, CFGINT, "ChanStats", c->name, "MaxKicks");
		SetData((void *)c->t_maxkicks, CFGINT, "ChanStats", c->name, "MaxKicksTime");
		SetData((void *)c->maxjoins, CFGINT, "ChanStats", c->name, "MaxJoins");
		SetData((void *)c->t_maxjoins, CFGINT, "ChanStats", c->name, "MaxJoinsTime");
		cn = list_next(Chead, cn);
	}

	/* and finally, the network data */

	SetData((void *)stats_network.maxopers, CFGINT, "NetStats", "Global", "MaxOpers");
	SetData((void *)stats_network.maxusers, CFGINT, "NetStats", "Global", "MaxUsers");
	SetData((void *)stats_network.maxservers, CFGINT, "NetStats", "Global", "MaxServers");
	SetData((void *)stats_network.t_maxopers, CFGINT, "NetStats", "Global", "MaxOpersTime");
	SetData((void *)stats_network.t_maxusers, CFGINT, "NetStats", "Global", "MaxUsersTime");
	SetData((void *)stats_network.t_maxservers, CFGINT, "NetStats", "Global", "MaxServersTime");
	SetData((void *)stats_network.totusers, CFGINT, "NetStats", "Global", "TotalUsers");
	SetData((void *)stats_network.maxchans, CFGINT, "NetStats", "Global", "MaxChans");
	SetData((void *)stats_network.t_chans, CFGINT, "NetStats", "Global", "MaxChansTime");
}

void LoadStats() {
	SStats *s;
	CStats *c;
	char *name;
	char **row;

	hnode_t *sn;
	lnode_t *cn;
	int count;
	strcpy(segv_location, "StatServ-LoadStats");
	Chead = list_create(SS_CHAN_SIZE);
	Shead = hash_create(S_TABLE_SIZE, 0, 0);

        if (GetData ((void *) &stats_network.maxopers, CFGINT, "NetStats", "Global", "MaxOpers") <= 0) {
		/* if this doesn't exist, then try LoadOldStats() */
		nlog(LOG_WARNING, LOG_MOD, "Trying to load StatServ stats from old database");
		LoadOldStats();
		return;
        }
	/* the rest don't need such valid checking */
	GetData((void *)&stats_network.maxusers, CFGINT, "NetStats", "Global", "MaxUsers");
	GetData((void *)&stats_network.maxservers, CFGINT, "NetStats", "Global", "MaxServers");
	GetData((void *)&stats_network.t_maxopers, CFGINT, "NetStats", "Global", "MaxOpersTime");
	GetData((void *)&stats_network.t_maxusers, CFGINT, "NetStats", "Global", "MaxUsersTime");
	GetData((void *)&stats_network.t_maxservers, CFGINT, "NetStats", "Global", "MaxServersTime");
	GetData((void *)&stats_network.totusers, CFGINT, "NetStats", "Global", "TotalUsers");
	GetData((void *)&stats_network.maxchans, CFGINT, "NetStats", "Global", "MaxChans");
	GetData((void *)&stats_network.t_chans, CFGINT, "NetStats", "Global", "MaxChansTime");


	/* ok, now load the server stats */
	if (GetTableData("ServerStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			s = malloc(sizeof(SStats));
			name = strncpy(s->name, row[count], MAXHOST);
			GetData((void *)&s->numsplits, CFGINT, "ServerStats", s->name, "Splits");
			GetData((void *)&s->maxusers, CFGINT, "ServerStats", s->name, "MaxUsers");
			GetData((void *)&s->t_maxusers, CFGINT, "ServerStats", s->name, "MaxUsersTime");
			GetData((void *)&s->lastseen, CFGINT, "ServerStats", s->name, "LastSeen");
			GetData((void *)&s->starttime, CFGINT, "ServerStats", s->name, "StartTime");
			GetData((void *)&s->operkills, CFGINT, "ServerStats", s->name, "OperKills");
			GetData((void *)&s->serverkills, CFGINT, "ServerStats", s->name, "ServerKills");
			GetData((void *)&s->totusers, CFGINT, "ServerStats", s->name, "TotalUsers");
			GetData((void *)&s->maxopers, CFGINT, "ServerStats", s->name, "MaxOpers");
			GetData((void *)&s->t_maxopers, CFGINT, "ServerStats", s->name, "MaxOpersTime");
			nlog(LOG_DEBUG1, LOG_MOD,
			     "LoadStats(): Loaded statistics for %s", s->name);
			sn = hnode_create(s);
			if (hash_isfull(Shead)) {
				nlog(LOG_CRITICAL, LOG_MOD,
				     "Eeek, StatServ Server Hash is Full!");
			} else {
				hash_insert(Shead, sn, s->name);
			}
		}
	}                                        
				
	/* ok, and now the channel stats. */
	if (GetTableData("ChanStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			c = malloc(sizeof(CStats));
			strncpy(c->name, row[count], CHANLEN);	
			GetData((void *)&c->topics, CFGINT, "ChanStats", c->name, "Topics");
			GetData((void *)&c->totmem, CFGINT, "ChanStats", c->name, "TotalMems");
			GetData((void *)&c->kicks, CFGINT, "ChanStats", c->name, "Kicks");
			GetData((void *)&c->lastseen, CFGINT, "ChanStats", c->name, "LastSeen");
			GetData((void *)&c->maxmems, CFGINT, "ChanStats", c->name, "MaxMems");
			GetData((void *)&c->t_maxmems, CFGINT, "ChanStats", c->name, "MaxMemsTime");
			GetData((void *)&c->maxkicks, CFGINT, "ChanStats", c->name, "MaxKicks");
			GetData((void *)&c->t_maxkicks, CFGINT, "ChanStats", c->name, "MaxKicksTime");
			GetData((void *)&c->maxjoins, CFGINT, "ChanStats", c->name, "MaxJoins");
			GetData((void *)&c->t_maxjoins, CFGINT, "ChanStats", c->name, "MaxJoinsTime");
			c->topicstoday = 0;
			c->joinstoday = 0;
			c->members = 0;
			cn = lnode_create(c);
			if (list_isfull(Chead)) {
				nlog(LOG_CRITICAL, LOG_MOD,
				     "Eeek, StatServ Channel Hash is Full!");
			} else {
				nlog(LOG_DEBUG2, LOG_MOD,
				     "Loading %s Channel Data", c->name);
				if ((time(NULL) - c->lastseen) < 604800) {
					list_append(Chead, cn);
				} else {
					nlog(LOG_DEBUG1, LOG_MOD,
					     "Deleting Old Channel %s", c->name);
					lnode_destroy(cn);
					free(c);
				}
			}
		}
	}
	StatServ.newdb = 0;
}


/* @brief This loads the old database format for statistics. This is depreciated and only
 * retained for backwards compatibility with old DB formats. will go away one day.
 */
void LoadOldStats()
{
	FILE *fp = fopen("data/nstats.db", "r");
	SStats *s;
	CStats *c;
	char buf[BUFSIZE];
	char *tmp;
	char *name, *numsplits, *maxusers, *t_maxusers,
	    *maxopers, *t_maxopers, *lastseen, *starttime,
	    *operkills, *serverkills, *totusers;
	char *topics, *totmem, *kicks, *maxmems, *t_maxmems, *maxkicks,
	    *t_maxkicks, *maxjoins, *t_maxjoins;


	hnode_t *sn;
	lnode_t *cn;
	int count;
	strcpy(segv_location, "StatServ-LoadStats");

	if (fp) {
		while (fgets(buf, BUFSIZE, fp)) {
			stats_network.maxopers = atoi(strtok(buf, " "));
			stats_network.maxusers = atol(strtok(NULL, " "));
			stats_network.maxservers = atoi(strtok(NULL, " "));
			stats_network.t_maxopers = atoi(strtok(NULL, " "));
			stats_network.t_maxusers = atol(strtok(NULL, " "));
			stats_network.t_maxservers =
			    atoi(strtok(NULL, " "));
			tmp = strtok(NULL, " ");
			if (tmp == NULL) {
				fprintf(stderr,
					"Detected Old Version(1.0) of Network Database, Upgrading\n");
				stats_network.totusers =
				    stats_network.maxusers;
			} else {
				stats_network.totusers = atoi(tmp);
			}
			tmp = strtok(NULL, " ");
			if (tmp == NULL) {
				nlog(LOG_NOTICE, LOG_MOD,
				     "Detected Old version (3.0) of Network Database, Upgrading");
				stats_network.maxchans = 0;
				stats_network.t_chans = time(NULL);
			} else {
				stats_network.maxchans = atol(tmp);
				tmp = strtok(NULL, "");
				stats_network.t_chans = atol(tmp);
			}
		}
		StatServ.newdb = 0;
		fclose(fp);
		unlink("data/nstats.db");
	} else {
		StatServ.newdb = 1;
	}
	if ((fp = fopen("data/stats.db", "r")) == NULL)
		return;

	memset(buf, '\0', BUFSIZE);
	while (fgets(buf, BUFSIZE, fp)) {
		s = malloc(sizeof(SStats));
		name = strtok(buf, " ");
		numsplits = strtok(NULL, " ");
		maxusers = strtok(NULL, " ");
		t_maxusers = strtok(NULL, " ");
		maxopers = strtok(NULL, " ");
		t_maxopers = strtok(NULL, " ");
		lastseen = strtok(NULL, " ");
		starttime = strtok(NULL, " ");
		operkills = strtok(NULL, " ");
		serverkills = strtok(NULL, " ");
		totusers = strtok(NULL, " ");

		memcpy(s->name, name, MAXHOST);
		s->numsplits = atoi(numsplits);
		s->maxusers = atol(maxusers);
		s->t_maxusers = atol(t_maxusers);
		s->maxopers = atoi(maxopers);
		s->t_maxopers = atol(t_maxopers);
		s->lastseen = atol(lastseen);
		s->starttime = atol(starttime);
		s->operkills = atoi(operkills);
		s->serverkills = atol(serverkills);
		s->users = 0;
		s->opers = 0;
		s->daily_totusers = 0;
		if (totusers == NULL) {
			s->totusers = 0;
		} else {
			s->totusers = atol(totusers);
		}

		nlog(LOG_DEBUG1, LOG_MOD,
		     "LoadStats(): Loaded statistics for %s", s->name);
		sn = hnode_create(s);
		if (hash_isfull(Shead)) {
			nlog(LOG_CRITICAL, LOG_MOD,
			     "Eeek, StatServ Server Hash is Full!");
		} else {
			hash_insert(Shead, sn, s->name);
		}
	}
	fclose(fp);
	unlink("data/stats.db");
	
	if ((fp = fopen("data/cstats.db", "r")) == NULL)
		return;
	memset(buf, '\0', BUFSIZE);
	name = malloc(CHANLEN);
	topics = malloc(BUFSIZE);
	totmem = malloc(BUFSIZE);
	kicks = malloc(BUFSIZE);
	lastseen = malloc(BUFSIZE);
	maxmems = malloc(BUFSIZE);
	t_maxmems = malloc(BUFSIZE);
	maxkicks = malloc(BUFSIZE);
	t_maxkicks = malloc(BUFSIZE);
	maxjoins = malloc(BUFSIZE);
	t_maxjoins = malloc(BUFSIZE);
	while (fgets(buf, BUFSIZE, fp)) {
		c = malloc(sizeof(CStats));

		count =
		    sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s\n", name,
			   topics, totmem, kicks, lastseen, maxmems,
			   t_maxmems, maxkicks, t_maxkicks, maxjoins,
			   t_maxjoins);
		strcpy(c->name, name);
		c->topics = atol(topics);
		c->totmem = atol(totmem);
		c->kicks = atol(kicks);
		c->lastseen = (time_t) atol(lastseen);
		c->maxmems = atol(maxmems);
		c->t_maxmems = (time_t) atol(t_maxmems);
		c->maxkicks = atol(maxkicks);
		c->t_maxkicks = (time_t) atol(t_maxkicks);
		c->maxjoins = atol(maxjoins);
		c->t_maxjoins = (time_t) atol(t_maxjoins);
		c->topicstoday = 0;
		c->joinstoday = 0;
		c->members = 0;
		cn = lnode_create(c);
		if (list_isfull(Chead)) {
			nlog(LOG_CRITICAL, LOG_MOD,
			     "Eeek, StatServ Channel Hash is Full!");
		} else {
			nlog(LOG_DEBUG2, LOG_MOD,
			     "Loading %s Channel Data", c->name);
			if ((time(NULL) - c->lastseen) < 604800) {
				list_append(Chead, cn);
			} else {
				nlog(LOG_DEBUG1, LOG_MOD,
				     "Deleting Old Channel %s", c->name);
				lnode_destroy(cn);
				free(c);
			}
		}
	}
	free(name);
	free(topics);
	free(totmem);
	free(kicks);
	free(lastseen);
	free(maxmems);
	free(t_maxmems);
	free(maxkicks);
	free(t_maxkicks);
	free(maxjoins);
	free(t_maxjoins);
	fclose(fp);
	unlink("data/cstats.db");
	SaveStats();



}
