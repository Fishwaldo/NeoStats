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

#include "neostats.h"
#include "statserv.h"
#include "rta.h"

COLDEF statserv_chanscols[] = {
	{
		"statserv_chans",
		"name",
		RTA_STR,
		MAXCHANLEN,
		offsetof(struct CStats, name),
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
		offsetof(struct CStats, members),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users in the channel"
	},
	{
		"statserv_chans",
		"topics",
		RTA_INT,
		sizeof(int),
		offsetof(struct CStats, topics),
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
		offsetof(struct CStats, totmem),
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
		offsetof(struct CStats, kicks),
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
		offsetof(struct CStats, topicstoday),
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
		offsetof(struct CStats, joinstoday),
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
		offsetof(struct CStats, maxkickstoday),
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
		offsetof(struct CStats, maxmemtoday),
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
		offsetof(struct CStats, t_maxmemtoday),
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
		offsetof(struct CStats, maxmems),
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
		offsetof(struct CStats, t_maxmems),
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
		offsetof(struct CStats, maxkicks),
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
		offsetof(struct CStats, t_maxkicks),
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
		offsetof(struct CStats, maxjoins),
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
		offsetof(struct CStats, t_maxjoins),
		RTA_READONLY,
		NULL,
		NULL,
		"The time the max no of joins a channel had in a day"
	},

};

TBLDEF statserv_chans = {
	"statserv_chans",
	NULL, 	/* for now */
	sizeof(struct CStats),
	0,
	TBL_LIST,
	statserv_chanscols,
	sizeof(statserv_chanscols) / sizeof(COLDEF),
	"",
	"The stats of online Channels on the IRC network"
};

COLDEF statserv_serverscols[] = {
	{
		"SStats",
		"name",
		RTA_STR,
		MAXHOST,
		offsetof(struct SStats, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the server"
	},
	{
		"SStats",
		"users",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users on the server"
	},
	{
		"SStats",
		"opers",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, opers),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Opers on this server"
	},
	{
		"SStats",
		"lowestping",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, lowest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The lowest ping this server had"
	},
	{
		"SStats",
		"highestping",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, highest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The highest ping this server had"
	},
	{	
		"SStats",
		"lowestpingtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, t_lowest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the lowest ping"
	},
	{	
		"SStats",
		"highestpingtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, t_highest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the highest ping"
	},
	{	
		"SStats",
		"numsplits",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, numsplits),
		RTA_READONLY,
		NULL,
		NULL,
		"The number of splits this server has had"
	},
	{	
		"SStats",
		"maxusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, maxusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of users this server has had"
	},
	{	
		"SStats",
		"maxuserstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, t_maxusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of of max users record"
	},
	{	
		"SStats",
		"maxopers",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, maxopers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of opers this server has ever had"
	},
	{	
		"SStats",
		"maxoperstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, t_maxopers),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the maxopers record"
	},
	{	
		"SStats",
		"t_lastseen",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, t_lastseen),
		RTA_READONLY,
		NULL,
		NULL,
		"When was this server last seen"
	},
	{	
		"SStats",
		"t_start",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, t_start),
		RTA_READONLY,
		NULL,
		NULL,
		"when was this server first seen"
	},
	{	
		"SStats",
		"operkills",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, operkills),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of oper issued kills for this server"
	},
	{	
		"SStats",
		"serverkills",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, serverkills),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Server issued kills"
	},
	{	
		"SStats",
		"totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, totusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The total no of users ever connected to this server"
	},
	{	
		"SStats",
		"daily_totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct SStats, daily_totusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The total no of users connected to this server today"
	},

};

TBLDEF statserv_servers = {
	"statserv_servers",
	NULL, 	/* for now */
	sizeof(struct SStats),
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
		offsetof(struct CVersions, name),
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
		offsetof(struct CVersions, count),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of hits on this client version"
	},
};

TBLDEF statserv_versions = {
	"statserv_versions",
	NULL, 	/* for now */
	sizeof(struct CVersions),
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
		offsetof(struct stats_network_, chans),
		RTA_READONLY,
		NULL, 
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
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, t_maxopers),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of max no of opers on the network"
	},
	{
		"statserv_network",
		"users",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, users),
		RTA_READONLY,
		NULL, 
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
		offsetof(struct stats_network_, away),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users marked away"
	},
	{
		"statserv_network",
		"servers",
		RTA_INT,
		sizeof(int),
		offsetof(struct stats_network_, servers),
		RTA_READONLY,
		NULL, 
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

void statserv_rta_init (void)
{
	/* ok, now export the server and chan data into the sql emulation layers */
	/* for the network and daily stats, we use a fake list, so we can easily import into rta */

	fakedaily = list_create(-1);
	lnode_create_append (fakedaily, &daily);
	fakenetwork = list_create(-1);
	lnode_create_append (fakenetwork, &stats_network);
	
	/* find the address of each list/hash, and export to rta */
	 
	statserv_chans.address = Chead;
	rtaserv_add_table (&statserv_chans);
	statserv_tld.address = Thead;
	rtaserv_add_table (&statserv_tld);
	statserv_servers.address = Shead;
	rtaserv_add_table (&statserv_servers);
	statserv_versions.address = Vhead;
	rtaserv_add_table (&statserv_versions);
	statserv_network.address = fakenetwork;
	rtaserv_add_table (&statserv_network);
	statserv_daily.address = fakedaily;
	rtaserv_add_table (&statserv_daily);
}

void statserv_rta_fini (void)
{
	list_destroy_nodes (fakedaily);
	list_destroy (fakedaily);
	list_destroy_nodes (fakenetwork);
	list_destroy (fakenetwork);
}
