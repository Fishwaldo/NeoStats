/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
** $Id$
*/

#include "statserv.h"

/* define this if you want the old database format.... but beware, its slow */
#undef OLDDATABASE


int SaveStats()
{
	SStats *s;
	CStats *c;
	hnode_t *sn;
	lnode_t *cn;
	hscan_t ss;
	int count, limit;
	SET_SEGV_LOCATION();

	if (StatServ.newdb == 1) {
		chanalert(ss_bot->nick, "Enabling Record yelling!");
		StatServ.newdb = 0;
	}

	if (StatServ.shutdown == 1) {
		chanalert(ss_bot->nick, "Saving StatServ Database. this *could* take a while");
	}
	/* first thing we do is clear the old database */
	DelTable("ServerStats");
	//DelTable("ChanStats");
	DelTable("NetStats");
	
	/* ok, run through the server stats, and save them */
	hash_scan_begin(&ss, Shead);
	while ((sn = hash_scan_next(&ss))) {
		s = hnode_get(sn);
		nlog(LOG_DEBUG1,
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
		if (StatServ.shutdown == 1) {
			free(s);
			hash_scan_delete(Shead, sn);
			hnode_destroy(sn);
		}

	}
	if (StatServ.shutdown == 1) {
		hash_destroy(Shead);
	}

	/* ok, Now Channel Stats */
	count = 0;
	/* we want to only do 25% each progressive save */
	limit = (list_count(Chead)/4);
	cn = list_first(Chead);
	while (cn) {
		c = lnode_get(cn);
		/* we are not shutting down, so do progressive save if we have more than 100 channels */
		if (StatServ.shutdown == 0 && (limit > 25)) {
			if (count > limit) {
				break;
			}
			/* calc is we save the entire database in the savedb interval plus 1/2 */
			if ((me.now - c->lastsave) < PROGCHANTIME) {
				cn = list_next(Chead, cn);
				continue;
			}
			count++;
		}
/* we need to squeze as much performance out of this as we can */
#if 0
		nlog(LOG_DEBUG1,
		     "Writting Statistics to database for %s (%d)", c->name, count);
#endif
		save_chan(c);
		/* if we are shuting down, clean up */
		if (StatServ.shutdown == 1) {
			free(c);
		}
		cn = list_next(Chead, cn);
	}
	if (StatServ.shutdown == 1) {
		list_destroy_nodes(Chead);
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
	if (StatServ.shutdown == 1) {
		chanalert(ss_bot->nick, "Done");
	}
	return 1;
}

void LoadStats() {
	SStats *s;
	char **row;

	hnode_t *sn;
	int count;
	
	SET_SEGV_LOCATION();
	Chead = list_create(SS_CHAN_SIZE);
	Shead = hash_create(S_TABLE_SIZE, 0, 0);

	if (GetData ((void *) &stats_network.maxopers, CFGINT, "NetStats", "Global", "MaxOpers") <= 0) {
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
			bzero(s, sizeof(SStats));
			strlcpy(s->name, row[count], MAXHOST);
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
			s->users = 0;
			s->opers = 0;
			s->daily_totusers = 0;
			s->lowest_ping = s->highest_ping = s->daily_totusers = 0;

			nlog(LOG_DEBUG1,
			     "LoadStats(): Loaded statistics for %s", s->name);
			sn = hnode_create(s);
			if (hash_isfull(Shead)) {
				nlog(LOG_CRITICAL,
				     "Eeek, StatServ Server Hash is Full!");
			} else {
				hash_insert(Shead, sn, s->name);
			}
		}
	}       
	free(row);                                 
/* we now load channel data dynamically. */
	/* ok, and now the channel stats. */
#if 0
	if (GetTableData("ChanStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			load_chan(row[count]);
		}
	}
			c = malloc(sizeof(CStats));
			strlcpy(c->name, row[count], CHANLEN);	
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
				nlog(LOG_CRITICAL,
				     "Eeek, StatServ Channel Hash is Full!");
			} else {
				nlog(LOG_DEBUG2,
				     "Loading %s Channel Data", c->name);
				if ((me.now - c->lastseen) < 604800) {
					list_append(Chead, cn);
				} else {
					nlog(LOG_DEBUG1,
					     "Deleting Old Channel %s", c->name);
					lnode_destroy(cn);
					free(c);
				}
			}
		}
	}
	free(row);
#endif
	StatServ.newdb = 0;
}

/* @brief load the info for a specific channel from the database 
 * or return null a blank if it does not exist. 
 * 
 * @params name the channel name to load
 * 
 * @returns a CStats struct that contains info for the channel. If its a new Channel, contains the name and thats it.
 */
 
CStats *load_chan(char *name) {
	lnode_t *cn;
	char *data;

	CStats *c;


	SET_SEGV_LOCATION();
	c = malloc(sizeof(CStats));
#ifdef USE_BERKELEY
	if ((data = DBGetData(name)) != NULL) {
		memcpy(c, data, sizeof(CStats));
#else
	strlcpy(c->name, name, CHANLEN);	
	if (GetData((void *)&data, CFGSTR, "ChanStats", c->name, "ChanData") > 0) {
		/* its the new database format... Good */
		sscanf(data, "%ld %ld %ld %ld %ld %ld %ld %ld %ld", &c->topics, &c->totmem, &c->kicks, &c->maxmems, &c->t_maxmems, &c->maxkicks, &c->t_maxkicks, &c->maxjoins, &c->t_maxjoins);
		GetData((void *)&c->lastseen, CFGINT, "ChanStats", c->name, "LastSeen");
		free(data);
	} else if (GetData((void *)&c->topics, CFGINT, "ChanStats", c->name, "Topics") > 0) {
		GetData((void *)&c->totmem, CFGINT, "ChanStats", c->name, "TotalMems");
		GetData((void *)&c->kicks, CFGINT, "ChanStats", c->name, "Kicks");
		GetData((void *)&c->lastseen, CFGINT, "ChanStats", c->name, "LastSeen");
		GetData((void *)&c->maxmems, CFGINT, "ChanStats", c->name, "MaxMems");
		GetData((void *)&c->t_maxmems, CFGINT, "ChanStats", c->name, "MaxMemsTime");
		GetData((void *)&c->maxkicks, CFGINT, "ChanStats", c->name, "MaxKicks");
		GetData((void *)&c->t_maxkicks, CFGINT, "ChanStats", c->name, "MaxKicksTime");
		GetData((void *)&c->maxjoins, CFGINT, "ChanStats", c->name, "MaxJoins");
		GetData((void *)&c->t_maxjoins, CFGINT, "ChanStats", c->name, "MaxJoinsTime");
		/* delete so when we save, we only save relevent information */
		DelRow("ChanStats", c->name);
#endif
	} else {
		strlcpy(c->name, name, CHANLEN);	
		c->totmem = 0;
		c->topics = 0;
		c->kicks = 0;
		c->lastseen = 0;
		c->maxmems = 0;
		c->t_maxmems = me.now;
		c->maxkicks = 0;
		c->t_maxkicks = 0;
		c->maxjoins = 0;
		c->t_maxjoins = 0;
	}
	c->maxmemtoday = 0;
	c->maxkickstoday = 0;
	c->topicstoday = 0;
	c->joinstoday = 0;
	c->members = 0;
	c->lastsave = me.now;
	cn = lnode_create(c);
	if (list_isfull(Chead)) {
		nlog(LOG_CRITICAL, "Eeek, StatServ Channel Hash is Full!");
	} else {
		nlog(LOG_DEBUG2, "Loading %s Channel Data", c->name);
		if ((me.now - c->lastseen) > 604800) {
			nlog(LOG_DEBUG1,
			     "Resetting Old Channel %s", c->name);
			c->totmem = 0;
			c->topics = 0;
			c->kicks = 0;
			c->lastseen = 0;
			c->maxmems = 0;
			c->t_maxmems = me.now;
			c->maxkicks = 0;
			c->t_maxkicks = 0;
			c->maxjoins = 0;
			c->t_maxjoins = 0;
			c->maxmemtoday = 0;
			c->maxkickstoday = 0;
			c->topicstoday = 0;
			c->joinstoday = 0;
			c->members = 0;
			c->lastsave = me.now;
		}
	}
	list_append(Chead, cn);
	return c;
}

/* @brief save the info for a specific channel to the database 
 *  
 * 
 * @params c the CStats struct to save
 * 
 * @returns nothing
 */
 
void save_chan(CStats *c) {
#ifndef OLDDATABASE
	char data[BUFSIZE];

#ifdef USE_BERKELEY
	SET_SEGV_LOCATION();
	memcpy(data, c, sizeof(CStats));
	DBSetData(c->name, data, sizeof(CStats));
#else
	SET_SEGV_LOCATION();
	ircsnprintf(data, BUFSIZE, "%d %d %d %d %d %d %d %d %d", 
		(int)c->topics, (int)c->totmem, (int)c->kicks, 
		(int)c->maxmems, (int)c->t_maxmems, (int)c->maxkicks, 
		(int)c->t_maxkicks, (int)c->maxjoins, (int)c->t_maxjoins);
	SetData((void *)data, CFGSTR, "ChanStats", c->name, "ChanData");
	/* we keep this seperate so we can easily delete old channels */
	SetData((void *)c->lastseen, CFGINT, "ChanStats", c->name, "LastSeen");
#endif

#else 
	SET_SEGV_LOCATION();
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
#endif
	c->lastsave = me.now;
}

/* @brief run through database deleting old channels 
 *  
 * 
 * @params nothing
 * 
 * @returns nothing
 */

int DelOldChan()
{
	char **row;
	int count = 0;
	time_t lastseen;
	time_t start;
	
	start = time(NULL);
	nlog(LOG_DEBUG1, "Starting To Clean Channel Database");
	if (GetTableData("ChanStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			if (GetData((void *)&lastseen, CFGINT, "ChanStats", row[count], "LastSeen") > 0) {
				/* delete it if its old and not online 
				 * use findchan, instead of findchanstats, and findchan is based on hashes, so its faster 
				 */
				if (((me.now - lastseen) > 604800) && (!findchan(row[count]))) {
					nlog(LOG_DEBUG1, "Deleting Channel %s", row[count]);
					DelRow("ChanStats", row[count]);
				}
			} else {
				/* database corruption? */
				nlog(LOG_WARNING, "Hrm, Database Corruption for Channel %s?. Deleting Record", row[count]);
				DelRow("ChanStats", row[count]);
			}
		}
	}
	free(row);
	nlog(LOG_INFO, "Took %d seconds to clean %d channel stats", (int)(time(NULL) - start), count);
	return 1;
}


