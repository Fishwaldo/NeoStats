/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
		announce_record ("\2NEW NETWORK RECORD\2 %ld servers on the network",
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
		announce_record ("\2NEW NETWORK RECORD\2 %ld channels on the network",
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
		announce_record ("\2NEW NETWORK RECORD!\2 %ld users on the network",
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
	irc_prefmsg(ss_bot, cmdparams->source, "Current Users: %ld", networkstats.users.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Users: %ld [%s]",
		networkstats.users.alltime.max, sftime(networkstats.users.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Total Users Connected: %ld",
		networkstats.users.alltime.runningtotal);
	irc_prefmsg(ss_bot, cmdparams->source, "Current Channels %ld", networkstats.channels.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Channels %ld [%s]",
		networkstats.channels.alltime.max, sftime(networkstats.channels.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Current Opers: %ld", networkstats.opers.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Opers: %ld [%s]",
		networkstats.opers.alltime.max, sftime(networkstats.opers.alltime.ts_max));
	irc_prefmsg(ss_bot, cmdparams->source, "Users Set Away: %ld", me.awaycount);
	irc_prefmsg(ss_bot, cmdparams->source, "Current Servers: %ld", networkstats.servers.current);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Servers: %ld [%s]",
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
	LoadStatistic (&networkstats.servers, "NetStats", "Global", "servers");
	LoadStatistic (&networkstats.channels, "NetStats", "Global", "channels");
	LoadStatistic (&networkstats.users, "NetStats", "Global", "users");
	LoadStatistic (&networkstats.opers, "NetStats", "Global", "opers");
	LoadStatistic (&networkstats.kills, "NetStats", "Global", "kills");
}

void SaveNetworkStats(void)
{
	/* clear the old database */
	DelTable("NetStats");
	/* save stats */
	SaveStatistic (&networkstats.servers, "NetStats", "Global", "servers");
	SaveStatistic (&networkstats.channels, "NetStats", "Global", "channels");
	SaveStatistic (&networkstats.users, "NetStats", "Global", "users");
	SaveStatistic (&networkstats.opers, "NetStats", "Global", "opers");
	SaveStatistic (&networkstats.kills, "NetStats", "Global", "kills");
}

void InitNetworkStats (void)
{
	LoadNetworkStats();
}

void FiniNetworkStats (void)
{
	SaveNetworkStats();
}
