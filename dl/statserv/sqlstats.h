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

#ifndef SQLSTATS_H
#define SQLSTATS_H

#include "stats.h"

/* only include the following code if SQLSRV is included */
#ifdef SQLSRV


#include "dl.h"
#include "statserv.h"
#include "sqlsrv/rta.h"


COLDEF statserv_chanscols[] = {
	{
		"statserv_chans",
		"name",
		RTA_STR,
		CHANLEN,
		offsetof(struct chan_stats, name),
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
		offsetof(struct chan_stats, members),
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
		offsetof(struct chan_stats, topics),
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
		offsetof(struct chan_stats, totmem),
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
		offsetof(struct chan_stats, kicks),
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
		offsetof(struct chan_stats, topicstoday),
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
		offsetof(struct chan_stats, joinstoday),
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
		offsetof(struct chan_stats, maxkickstoday),
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
		offsetof(struct chan_stats, maxmemtoday),
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
		offsetof(struct chan_stats, t_maxmemtoday),
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
		offsetof(struct chan_stats, maxmems),
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
		offsetof(struct chan_stats, t_maxmems),
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
		offsetof(struct chan_stats, maxkicks),
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
		offsetof(struct chan_stats, t_maxkicks),
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
		offsetof(struct chan_stats, maxjoins),
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
		offsetof(struct chan_stats, t_maxjoins),
		RTA_READONLY,
		NULL,
		NULL,
		"The time the max no of joins a channel had in a day"
	},

};

TBLDEF statserv_chans = {
	"statserv_chans",
	NULL, 	/* for now */
	sizeof(struct chan_stats),
	0,
	TBL_LIST,
	statserv_chanscols,
	sizeof(statserv_chanscols) / sizeof(COLDEF),
	"",
	"The stats of online Channels on the IRC network"
};

COLDEF statserv_serverscols[] = {
	{
		"server_stats",
		"name",
		RTA_STR,
		MAXHOST,
		offsetof(struct server_stats, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the server"
	},
	{
		"server_stats",
		"users",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users on the server"
	},
	{
		"server_stats",
		"opers",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, opers),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Opers on this server"
	},
	{
		"server_stats",
		"lowestping",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, lowest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The lowest ping this server had"
	},
	{
		"server_stats",
		"highestping",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, highest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The highest ping this server had"
	},
	{	
		"server_stats",
		"lowestpingtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, t_lowest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the lowest ping"
	},
	{	
		"server_stats",
		"highestpingtime",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, t_highest_ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the highest ping"
	},
	{	
		"server_stats",
		"numsplits",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, numsplits),
		RTA_READONLY,
		NULL,
		NULL,
		"The number of splits this server has had"
	},
	{	
		"server_stats",
		"maxusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, maxusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of users this server has had"
	},
	{	
		"server_stats",
		"maxuserstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, t_maxusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of of max users record"
	},
	{	
		"server_stats",
		"maxopers",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, maxopers),
		RTA_READONLY,
		NULL,
		NULL,
		"The Max no of opers this server has ever had"
	},
	{	
		"server_stats",
		"maxoperstime",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, t_maxopers),
		RTA_READONLY,
		NULL,
		NULL,
		"The time of the maxopers record"
	},
	{	
		"server_stats",
		"lastseen",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, lastseen),
		RTA_READONLY,
		NULL,
		NULL,
		"When was this server last seen"
	},
	{	
		"server_stats",
		"starttime",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, starttime),
		RTA_READONLY,
		NULL,
		NULL,
		"when was this server first seen"
	},
	{	
		"server_stats",
		"operkills",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, operkills),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of oper issued kills for this server"
	},
	{	
		"server_stats",
		"serverkills",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, serverkills),
		RTA_READONLY,
		NULL,
		NULL,
		"The no of Server issued kills"
	},
	{	
		"server_stats",
		"totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, totusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The total no of users ever connected to this server"
	},
	{	
		"server_stats",
		"daily_totalusers",
		RTA_INT,
		sizeof(int),
		offsetof(struct server_stats, daily_totusers),
		RTA_READONLY,
		NULL,
		NULL,
		"The total no of users connected to this server today"
	},

};

TBLDEF statserv_servers = {
	"server_stats",
	NULL, 	/* for now */
	sizeof(struct server_stats),
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
		MAX_CLIENT_VERSION_NAME,
		offsetof(struct irc_client_version, name),
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
		offsetof(struct irc_client_version, count),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of hits on this client version"
	},
};

TBLDEF statserv_versions = {
	"statserv_versions",
	NULL, 	/* for now */
	sizeof(struct irc_client_version),
	0,
	TBL_LIST,
	statserv_versionscols,
	sizeof(statserv_versionscols) / sizeof(COLDEF),
	"",
	"The stats of IRC client versions on the IRC network"
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
		offsetof(struct daily_, servers),
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
		offsetof(struct daily_, t_servers),
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
		offsetof(struct daily_, users),
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
		offsetof(struct daily_, t_users),
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
		offsetof(struct daily_, opers),
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
		offsetof(struct daily_, t_opers),
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
		offsetof(struct daily_, tot_users),
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
		offsetof(struct daily_, chans),
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
		offsetof(struct daily_, t_chans),
		RTA_READONLY,
		NULL, 
		NULL,
		"The time of the max no of channels on the network today"
	},
};

TBLDEF statserv_daily = {
	"statserv_daily",
	NULL, 	/* for now */
	sizeof(struct daily_),
	0,
	TBL_LIST, /* will be faked up */
	statserv_dailycols,
	sizeof(statserv_dailycols) / sizeof(COLDEF),
	"",
	"The daily network  stats"
};

list_t *fakedaily;


#endif /* SQLSRV */
#endif /* SQLSTATS_H */
