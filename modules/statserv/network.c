/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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

#define NETWORK_TABLE	"Network"

networkstat networkstats;

void AverageNetworkStatistics (void)
{
	AverageStatistic (&networkstats.servers);
	AverageStatistic (&networkstats.channels);
	AverageStatistic (&networkstats.users);
	AverageStatistic (&networkstats.opers);
	AverageStatistic (&networkstats.kills);
}

void ResetNetworkStatistics (void)
{
	ResetStatistic (&networkstats.servers);
	ResetStatistic (&networkstats.channels);
	ResetStatistic (&networkstats.users);
	ResetStatistic (&networkstats.opers);
	ResetStatistic (&networkstats.kills);
}

void AddNetworkServer (void)
{
	if (IncStatistic (&networkstats.servers)) {
		announce_record ("\2NEW NETWORK RECORD\2 %d servers on the network",
			networkstats.servers.current);
	}
}

void DelNetworkServer (void)
{
	DecStatistic (&networkstats.servers);
}

void AddNetworkChannel (void)
{
	if (IncStatistic (&networkstats.channels)) {
		announce_record ("\2NEW NETWORK RECORD\2 %d channels on the network",
		    networkstats.channels.current);
	}
}

void DelNetworkChannel (void)
{
	DecStatistic (&networkstats.channels);
}

void AddNetworkUser (void)
{
	if (IncStatistic (&networkstats.users)) {
		announce_record ("\2NEW NETWORK RECORD!\2 %d users on the network",
			networkstats.users.current);
	}
}

void DelNetworkUser (void)
{
	DecStatistic (&networkstats.users);
}

void AddNetworkOper (void)
{
	IncStatistic (&networkstats.opers);
}

void DelNetworkOper (void)
{
	DecStatistic (&networkstats.opers);
}

void AddNetworkKill (void)
{
	IncStatistic (&networkstats.kills);
}

void DelNetworkKill (void)
{
	DecStatistic (&networkstats.kills);
}

int ss_cmd_netstats (CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg(ss_bot, cmdparams->source, "Network Statistics:-----");
	irc_prefmsg(ss_bot, cmdparams->source, "Current Users: %d", networkstats.users.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Users: %d [%s]",
		networkstats.users.alltime.max, sftime(networkstats.users.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Total Users Connected: %d",
		networkstats.users.alltime.runningtotal);
	irc_prefmsg(ss_bot, cmdparams->source, "Current Channels %d", networkstats.channels.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Channels %d [%s]",
		networkstats.channels.alltime.max, sftime(networkstats.channels.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Current Opers: %d", networkstats.opers.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Opers: %d [%s]",
		networkstats.opers.alltime.max, sftime(networkstats.opers.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Users Set Away: %d", me.awaycount);
	irc_prefmsg(ss_bot, cmdparams->source, "Current Servers: %d", networkstats.servers.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Servers: %d [%s]",
		networkstats.servers.alltime.max, sftime(networkstats.servers.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

int ss_cmd_daily (CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg(ss_bot, cmdparams->source, "Daily Network Statistics:");
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Servers: %-2d %s",
		networkstats.servers.daily.max, sftime(networkstats.servers.daily.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Users: %-2d %s", 
		networkstats.users.daily.max, sftime(networkstats.users.daily.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Channel: %-2d %s", 
		networkstats.channels.daily.max, sftime(networkstats.channels.daily.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Opers: %-2d %s", 
		networkstats.opers.daily.max, sftime(networkstats.opers.daily.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Total Users Connected: %-2d",
		networkstats.users.daily.runningtotal);
	irc_prefmsg(ss_bot, cmdparams->source, "End of Information.");
	return NS_SUCCESS;
}

void LoadNetworkStats(void) 
{
	if (DBAFetch (NETWORK_TABLE, NETWORK_TABLE, &networkstats, sizeof (networkstats)) == NS_SUCCESS) 
	{
		PostLoadStatistic (&networkstats.servers);
		PostLoadStatistic (&networkstats.channels);
		PostLoadStatistic (&networkstats.users);
		PostLoadStatistic (&networkstats.opers);
		PostLoadStatistic (&networkstats.kills);
	}
}

void SaveNetworkStats(void)
{
	DBAStore (NETWORK_TABLE, NETWORK_TABLE, &networkstats, sizeof(networkstats));
	PreSaveStatistic (&networkstats.servers);
	PreSaveStatistic (&networkstats.channels);
	PreSaveStatistic (&networkstats.users);
	PreSaveStatistic (&networkstats.opers);
	PreSaveStatistic (&networkstats.kills);
}

void InitNetworkStats (void)
{
	LoadNetworkStats();
}

void FiniNetworkStats (void)
{
	SaveNetworkStats();
}
