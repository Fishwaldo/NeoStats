/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "channel.h"
#include "tld.h"
#include "server.h"
#include "version.h"
#include "rta.h"

#if 0

static int zerovalue = 0;

void *display_channel_current_users (void *tbl, char *col, char *sql, void *row) 
{
	channelstat *data = row;

	if (data->c)
		return &data->c->users;
	return &zerovalue;
}                        

COLDEF statserv_chanscols[] = {
	{
		"statserv_chans",
		"name",
		RTA_STR,
		MAXCHANLEN,
		offsetof(struct channelstat, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the channel"
	},
	{
		"statserv_chans",
		"nomems",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_channel_current_users, 
		NULL,
		"The no of users in the channel"
	},
	{
		"statserv_chans",
		"topics",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, topics),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Topic Changes"
	},
	{
		"statserv_chans",
		"totmem",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, totmem),
		RTA_READONLY,
		NULL,
		NULL,
		"The total joins ever on this channel"
	},
	{
		"statserv_chans",
		"kicks",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, kicks),
		RTA_READONLY,
		NULL,
		NULL,
		"The total number of kicks in this channel"
	},
	{	
		"statserv_chans",
		"topicstoday",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, topicstoday),
		RTA_READONLY,
		NULL,
		NULL,
		"The number of topic changes today"
	},
	{	
		"statserv_chans",
		"joinstoday",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, joinstoday),
		RTA_READONLY,
		NULL,
		NULL,
		"The number of joins today"
	},
	{	
		"statserv_chans",
		"kickstoday",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, maxkickstoday),
		RTA_READONLY,
		NULL,
		NULL,
		"The number of kicks today"
	},
	{	
		"statserv_chans",
		"maxmemtoday",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, maxmemtoday),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of members today"
	},
	{	
		"statserv_chans",
		"maxmemtodaytime",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, t_maxmemtoday),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of max no of members today"
	},
	{	
		"statserv_chans",
		"maxmem",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, maxmems),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of members this channel had ever."
	},
	{	
		"statserv_chans",
		"maxmemtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, t_maxmems),
		RTA_READONLY,
		NULL,
		NULL,
		"The time when max no of members this channel had ever."
	},
	{	
		"statserv_chans",
		"maxkicks",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, maxkicks),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of kicks a channel had in a single day."
	},
	{	
		"statserv_chans",
		"maxkickstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, t_maxkicks),
		RTA_READONLY,
		NULL,
		NULL,
		"The time this channel had the Max kicks"
	},
	{	
		"statserv_chans",
		"maxjoins",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, maxjoins),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of joins a channel had in a day"
	},
	{	
		"statserv_chans",
		"maxjoinstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct channelstat, t_maxjoins),
		RTA_READONLY,
		NULL,
		NULL,
		"The time the max no of joins a channel had in a day"
	},

};

TBLDEF statserv_chans = {
	"statserv_chans",
	NULL, 	/* for now */
	sizeof(struct channelstat),
	0,
	TBL_LIST,
	statserv_chanscols,
	sizeof(statserv_chanscols) / sizeof(COLDEF),
	"",
	"The stats of online Channels on the IRC network"
};

void *display_server_current_users (void *tbl, char *col, char *sql, void *row) 
{
	serverstat *data = row;

	if (data->s)
		return &data->s->server->users;
	return &zerovalue;
}                        

COLDEF statserv_serverscols[] = {
	{
		"serverstat",
		"name",
		RTA_STR,
		MAXHOST,
		offsetof(struct serverstat, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the server"
	},
	{
		"serverstat",
		"users",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_server_current_users, 
		NULL,
		"The no of users on the server"
	},
	{
		"serverstat",
		"opers",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, opers),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Opers on this server"
	},
	{
		"serverstat",
		"lowestping",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, lowest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The lowest ping this server had"
	},
	{
		"serverstat",
		"highestping",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, highest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The highest ping this server had"
	},
	{	
		"serverstat",
		"lowestpingtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, t_lowest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the lowest ping"
	},
	{	
		"serverstat",
		"highestpingtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, t_highest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the highest ping"
	},
	{	
		"serverstat",
		"numsplits",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, numsplits),
		RTA_READONLY,
		NULL,
		NULL,
		"The number of splits this server has had"
	},
	{	
		"serverstat",
		"maxusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, maxusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of users this server has had"
	},
	{	
		"serverstat",
		"maxuserstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, t_maxusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of of max users record"
	},
	{	
		"serverstat",
		"maxopers",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, maxopers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of opers this server has ever had"
	},
	{	
		"serverstat",
		"maxoperstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, t_maxopers),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the maxopers record"
	},
	{	
		"serverstat",
		"ts_lastseen",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, ts_lastseen),
		RTA_READONLY,
		NULL,
		NULL,
		"When was this server last seen"
	},
	{	
		"serverstat",
		"ts_start",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, ts_start),
		RTA_READONLY,
		NULL,
		NULL,
		"when was this server first seen"
	},
	{	
		"serverstat",
		"operkills",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, operkills),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of oper issued kills for this server"
	},
	{	
		"serverstat",
		"serverkills",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, serverkills),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Server issued kills"
	},
	{	
		"serverstat",
		"totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, totusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The total no of users ever connected to this server"
	},
	{	
		"serverstat",
		"daily_totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct serverstat, daily_totusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The total no of users connected to this server today"
	},

};

TBLDEF statserv_servers = {
	"statserv_servers",
	NULL, 	/* for now */
	sizeof(struct serverstat),
	0,
	TBL_HASH,
	statserv_serverscols,
	sizeof(statserv_serverscols) / sizeof(COLDEF),
	"",
	"The stats of servers on the IRC network"
};

COLDEF statserv_versionscols[] = {
	{
		"statserv_versions",
		"name",
		RTA_STR,
		BUFSIZE,
		offsetof(struct ctcpversionstat, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The client version"
	},
	{
		"statserv_versions",
		"count",
		RTA_INT,
		sizeof(int),
		offsetof(struct ctcpversionstat, count),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of hits on this client version"
	},
};

TBLDEF statserv_versions = {
	"statserv_versions",
	NULL, 	/* for now */
	sizeof(struct ctcpversionstat),
	0,
	TBL_LIST,
	statserv_versionscols,
	sizeof(statserv_versionscols) / sizeof(COLDEF),
	"",
	"The stats of IRC client versions on the IRC network"
};

COLDEF statserv_tldcols[] = {
	{
		"statserv_tld",
		"tld",
		RTA_STR,
		5,
		offsetof(TLD, tld),
		RTA_READONLY,
		NULL,
		NULL,
		"The tld name"
	},
	{
		"statserv_tld",
		"country",
		RTA_STR,
		32,
		offsetof(TLD, country),
		RTA_READONLY,
		NULL, 
		NULL,
		"The Country name"
	},
	{
		"statserv_tld",
		"users",
		RTA_INT,
		sizeof(int),
		offsetof(TLD, users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The No of Online Users"
	},
#if 0
	{
		"statserv_tld",
		"daily_users",
		RTA_INT,
		sizeof(int),
		offsetof(TLD, daily_users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The No of Users connected today"
	},
#endif
};

TBLDEF statserv_tld = {
	"statserv_tld",
	NULL, 	/* for now */
	sizeof(TLD),
	0,
	TBL_LIST,
	statserv_tldcols,
	sizeof(statserv_tldcols) / sizeof(COLDEF),
	"",
	"The TLD stats of the IRC network"
};

void *display_network_current_users (void *tbl, char *col, char *sql, void *row) 
{
	return &networkstats.users.current;
}                        

void *display_network_current_away (void *tbl, char *col, char *sql, void *row) 
{
	return &me.awaycount;
}                        

void *display_network_current_channels (void *tbl, char *col, char *sql, void *row) 
{
	return &networkstats.channels.current;
}                        

void *display_network_current_servers (void *tbl, char *col, char *sql, void *row) 
{
	return &networkstats.servers.current;
}                        

static char fmtime[TIMEBUFSIZE];

void *display_timestamp_maxopers (void *tbl, char *col, char *sql, void *row) 
{
	struct tm *ltm;
	struct stats_network_ *data = row;
	ltm = localtime (&data->t_maxopers);
	strftime (fmtime, TIMEBUFSIZE, "%a %b %d %Y %I:%M", ltm);
	return fmtime;
}

COLDEF statserv_networkcols[] = {
	{
		"statserv_network",
		"opers",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, opers),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of opers on the network"
	},
	{
		"statserv_network",
		"chans",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_network_current_channels,
		NULL,
		"The no of channels on the network"
	},
	{
		"statserv_network",
		"maxopers",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, maxopers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The max no of opers on the network"
	},
	{
		"statserv_network",
		"maxoperstime",
		RTA_STR,//RTA_INT,
		sizeof(int),
		0, //offsetof(struct stats_network_, t_maxopers),
		RTA_READONLY,
		display_timestamp_maxopers, 
		NULL,
		"The time of max no of opers on the network"
	},
	{
		"statserv_network",
		"users",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_network_current_users,
		NULL,
		"The max no of users on the network"
	},
	{
		"statserv_network",
		"totusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, totusers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users ever connected to the network"
	},
	{
		"statserv_network",
		"maxusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, maxusers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The max no of users connected to the network"
	},
	{
		"statserv_network",
		"maxuserstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, t_maxusers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of max no of users connected to the network"
	},
	{
		"statserv_network",
		"away",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_network_current_away,
		NULL,
		"The no of users marked away"
	},
	{
		"statserv_network",
		"servers",
		RTA_INT,
		sizeof(int),
		0,
		RTA_READONLY,
		display_network_current_servers,
		NULL,
		"The no of servers connected to the network"
	},
	{
		"statserv_network",
		"maxservers",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, maxservers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The Max no of servers ever connected to the network"
	},
	{
		"statserv_network",
		"maxserverstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, t_maxservers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of Max no of servers ever connected to the network"
	},
	{
		"statserv_network",
		"maxchans",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, maxchans),
		RTA_READONLY,
		NULL, 
		NULL,
		"The Max no of channels ever on the network"
	},
	{
		"statserv_network",
		"maxchanstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, t_chans),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of Max no of channels ever on the network"
	},
};

TBLDEF statserv_network = {
	"statserv_network",
	NULL, 	/* for now */
	sizeof(struct stats_network_),
	0,
	TBL_LIST, /* this list will be faked up */
	statserv_networkcols,
	sizeof(statserv_networkcols) / sizeof(COLDEF),
	"",
	"The network stats"
};

list_t *fakenetwork;


COLDEF statserv_dailycols[] = {
	{
		"statserv_daily",
		"servers",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, servers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of servers connected today"
	},
	{
		"statserv_daily",
		"serverstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, t_servers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of the Max no of servers today"
	},
	{
		"statserv_daily",
		"maxusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The max no of users on the network today"
	},
	{
		"statserv_daily",
		"maxuserstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, t_users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of max no of users today"
	},
	{
		"statserv_daily",
		"maxopers",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, opers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The max no of opers on the network today"
	},
	{
		"statserv_daily",
		"maxoperstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, t_opers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of opers connected to the network today"
	},
	{
		"statserv_daily",
		"totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, tot_users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users connected to the network today"
	},
	{
		"statserv_daily",
		"maxchans",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, chans),
		RTA_READONLY,
		NULL, 
		NULL,
		"The max no of channels on the network today"
	},
	{
		"statserv_daily",
		"maxchanstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct Daily, t_chans),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of the max no of channels on the network today"
	},
};

TBLDEF statserv_daily = {
	"statserv_daily",
	NULL, 	/* for now */
	sizeof(struct Daily),
	0,
	TBL_LIST, /* will be faked up */
	statserv_dailycols,
	sizeof(statserv_dailycols) / sizeof(COLDEF),
	"",
	"The daily network  stats"
};

list_t *fakedaily;

#endif

void InitRTAStats (void)
{
#if 0
	/* ok, now export the server and chan data into the sql emulation layers */
	/* for the network and daily stats, we use a fake list, so we can easily import into rta */

	fakedaily = list_create(-1);
	lnode_create_append (fakedaily, &daily);
	fakenetwork = list_create(-1);
	lnode_create_append (fakenetwork, &stats_network);
	
	/* find the address of each list/hash, and export to rta */
	 
	statserv_chans.address = channelstatlist;
	rtaserv_add_table (&statserv_chans);
	statserv_tld.address = tldstatlist;
	rtaserv_add_table (&statserv_tld);
	statserv_servers.address = serverstathash;
	rtaserv_add_table (&statserv_servers);
	statserv_versions.address = versionstatlist;
	rtaserv_add_table (&statserv_versions);
	statserv_network.address = fakenetwork;
	rtaserv_add_table (&statserv_network);
	statserv_daily.address = fakedaily;
	rtaserv_add_table (&statserv_daily);
#endif
}

void FiniRTAStats (void)
{
#if 0
	list_destroy_nodes (fakedaily);
	list_destroy (fakedaily);
	list_destroy_nodes (fakenetwork);
	list_destroy (fakenetwork);
#endif
}
