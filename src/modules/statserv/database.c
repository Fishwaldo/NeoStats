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

#include "statserv.h"

void SaveServerStats(void)
{
	SStats *s;
	hnode_t *sn;
	hscan_t ss;

	/* clear the old database */
	DelTable("ServerStats");
	/* run through stats and save them */
	hash_scan_begin(&ss, Shead);
	while ((sn = hash_scan_next(&ss))) {
		s = hnode_get(sn);
		dlog(DEBUG1, "Writing statistics to database for %s", s->name);
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
}

void SaveNetworkStats(void)
{
	/* clear the old database */
	DelTable("NetStats");
	/* run through stats and save them */
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

void SaveChanStats(void)
{
	CStats *c;
	lnode_t *cn;
	int limit;
    int count = 0;

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
		save_chan(c);
		cn = list_next(Chead, cn);
	}
}

int SaveStats(void)
{
	SET_SEGV_LOCATION();
	SaveServerStats();
	SaveChanStats();
	SaveNetworkStats();
	return 1;
}

int LoadNetworkStats(void) 
{
	if (GetData ((void *) &stats_network.maxopers, CFGINT, "NetStats", "Global", "MaxOpers") <= 0) {
		return 0;
	}
	GetData((void *)&stats_network.maxusers, CFGINT, "NetStats", "Global", "MaxUsers");
	GetData((void *)&stats_network.maxservers, CFGINT, "NetStats", "Global", "MaxServers");
	GetData((void *)&stats_network.t_maxopers, CFGINT, "NetStats", "Global", "MaxOpersTime");
	GetData((void *)&stats_network.t_maxusers, CFGINT, "NetStats", "Global", "MaxUsersTime");
	GetData((void *)&stats_network.t_maxservers, CFGINT, "NetStats", "Global", "MaxServersTime");
	GetData((void *)&stats_network.totusers, CFGINT, "NetStats", "Global", "TotalUsers");
	GetData((void *)&stats_network.maxchans, CFGINT, "NetStats", "Global", "MaxChans");
	GetData((void *)&stats_network.t_chans, CFGINT, "NetStats", "Global", "MaxChansTime");
	return 1;
}

void LoadServerStats(void) 
{
	SStats *s;
	char **row;
	hnode_t *sn;
	int count;
	if (GetTableData("ServerStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			s = scalloc(sizeof(SStats));
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
			dlog(DEBUG1, "Loaded statistics for %s", s->name);
			if (hash_isfull(Shead)) {
				nlog(LOG_CRITICAL, "StatServ server hash full");
				break;
			} else {
				sn = hnode_create(s);
				hash_insert(Shead, sn, s->name);
			}
		}
	}       
	sfree(row);                                 
}

void LoadStats(void) 
{
	SET_SEGV_LOCATION();
	if(LoadNetworkStats() == 0)
		return;
	LoadServerStats();
}

/* @brief load the info for a specific channel from the database 
 * or return null a blank if it does not exist. 
 * 
 * @params name the channel name to load
 * 
 * @returns a CStats struct that contains info for the channel. If its a new Channel, contains the name and thats it.
 */
 
CStats *load_chan(char *name) 
{
	lnode_t *cn;
	char *data;
	CStats *c;

	SET_SEGV_LOCATION();
	c = smalloc(sizeof(CStats));
#ifdef USE_BERKELEY
	if ((data = DBGetData(name)) != NULL) {
		memcpy(c, data, sizeof(CStats));
#else
	strlcpy(c->name, name, MAXCHANLEN);	
	if (GetData((void *)&data, CFGSTR, "ChanStats", c->name, "ChanData") > 0) {
		sscanf(data, "%ld %ld %ld %ld %ld %ld %ld %ld %ld", &c->topics, &c->totmem, &c->kicks, &c->maxmems, &c->t_maxmems, &c->maxkicks, &c->t_maxkicks, &c->maxjoins, &c->t_maxjoins);
		GetData((void *)&c->lastseen, CFGINT, "ChanStats", c->name, "LastSeen");
		sfree(data);
#endif
	} else {
		strlcpy(c->name, name, MAXCHANLEN);	
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
		nlog(LOG_CRITICAL, "StatServ channel hash full");
	} else {
		dlog(DEBUG2, "Loading channel %s", c->name);
		if ((me.now - c->lastseen) > 604800) {
			dlog(DEBUG1, "Reset old channel %s", c->name);
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
 
void save_chan(CStats *c) 
{
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
	c->lastsave = me.now;
}

/* @brief run through database deleting old channels 
 *  
 * 
 * @params nothing
 * 
 * @returns nothing
 */

int DelOldChan(void)
{
	char **row;
	int count = 0;
	time_t lastseen;
	time_t start;
	
	start = time(NULL);
	dlog(DEBUG1, "Deleting old channels");
	if (GetTableData("ChanStats", &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			if (GetData((void *)&lastseen, CFGINT, "ChanStats", row[count], "LastSeen") > 0) {
				/* delete it if its old and not online 
				 * use findchan, instead of findchanstats, and findchan is based on hashes, so its faster 
				 */
				if (((me.now - lastseen) > 604800) && (!findchan(row[count]))) {
					dlog(DEBUG1, "Deleting Channel %s", row[count]);
					DelRow("ChanStats", row[count]);
				}
			} else {
				/* database corruption? */
				nlog(LOG_WARNING, "Channel %s corrpted: deleting record", row[count]);
				DelRow("ChanStats", row[count]);
			}
		}
	}
	sfree(row);
	dlog(DEBUG1, "DelOldChan: %d seconds %d channels", (int)(time(NULL) - start), count);
	return 1;
}
