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
#include "channel.h"

#define CHANNEL_TABLE	"Channel"

list_t *channelstatlist;

void save_chan (channelstat *c);
channelstat *load_chan (char *name);

void AverageChannelStatistics (void)
{
	lnode_t *cn;
	channelstat *cs;

	cn = list_first (channelstatlist);
	while (cn) {
		cs = lnode_get (cn);
		AverageStatistic (&cs->users);
		AverageStatistic (&cs->kicks);
		AverageStatistic (&cs->topics);
		AverageStatistic (&cs->joins);
		cn = list_next (channelstatlist, cn);
	}
}

void ResetChannelStatistics (void)
{
	lnode_t *cn;
	channelstat *cs;

	cn = list_first (channelstatlist);
	while (cn) {
		cs = lnode_get (cn);
		ResetStatistic (&cs->users);
		ResetStatistic (&cs->kicks);
		ResetStatistic (&cs->topics);
		ResetStatistic (&cs->joins);
		cn = list_next (channelstatlist, cn);
	}
}

static channelstat *findchanstats(char *name)
{
	channelstat *cs;

	cs = lnode_find (channelstatlist, name, comparef);
	if (!cs) {
		dlog(DEBUG2, "findchanstats: %s not found", name);
	}	
	return cs;
}

int topcurrentchannel(const void *key1, const void *key2)
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return (chan2->c->users - chan1->c->users);
}

int topjoinrunningtotalchannel(const void *key1, const void *key2)
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return (chan2->users.alltime.runningtotal - chan1->users.alltime.runningtotal);
}

int topkickrunningtotalchannel(const void *key1, const void *key2)
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return (chan2->kicks.alltime.runningtotal - chan1->kicks.alltime.runningtotal);
}

int toptopicrunningtotalchannel(const void *key1, const void *key2)
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return (chan2->topics.alltime.runningtotal - chan1->topics.alltime.runningtotal);
}

static void AddChannel (Channel* c, void *v)
{
	channelstat *cs;

	cs = load_chan (c->name);
	AddNetworkChannel ();
    SetChannelModValue (c, (void *)cs);
	cs->c = c;
}

int ss_event_newchan (CmdParams *cmdparams)
{
	AddChannel (cmdparams->channel, NULL);
	return NS_SUCCESS;
}

int ss_event_delchan (CmdParams *cmdparams)
{
	channelstat *cs;
	lnode_t *ln;
	
	ClearChannelModValue (cmdparams->channel);
	DelNetworkChannel ();
	ln = list_find (channelstatlist, cmdparams->channel->name, comparef);
	if (!ln) {
		nlog (LOG_WARNING, "Couldn't find channel %s when deleting from stats", cmdparams->channel->name);
		return NS_SUCCESS;
	}
	cs = lnode_get (ln);
	save_chan (cs);
	list_delete (channelstatlist, ln);
	lnode_destroy (ln);
	ns_free (cs);
	return NS_SUCCESS;
}

int ss_event_join(CmdParams *cmdparams)
{										   
	channelstat *cs;

	cs = GetChannelModValue (cmdparams->channel);
	IncStatistic (&cs->users);
	IncStatistic (&cs->joins);
	return NS_SUCCESS;
}

int ss_event_part (CmdParams *cmdparams)
{
	channelstat *cs;

	cs = GetChannelModValue (cmdparams->channel);
	DecStatistic (&cs->users);
	cs->ts_lastseen = me.now;
	return NS_SUCCESS;
}

int ss_event_topic(CmdParams *cmdparams)
{
	channelstat *cs;

	cs = GetChannelModValue (cmdparams->channel);
	IncStatistic (&cs->topics);
	return NS_SUCCESS;
}

int ss_event_kick(CmdParams *cmdparams)
{
	channelstat *cs;

	cs = GetChannelModValue (cmdparams->channel);
	IncStatistic (&cs->kicks);
	DecStatistic (&cs->users);
	return NS_SUCCESS;
}

static void top10membershandler (channelstat *cs, void *v)
{
	CmdParams *cmdparams = (CmdParams *) v;
	irc_prefmsg(ss_bot,cmdparams->source, "Channel %s Members %ld", 
		cs->name, cs->c->users);
}

static void top10joinshandler (channelstat *cs, void *v)
{
	CmdParams *cmdparams = (CmdParams *) v;
	irc_prefmsg(ss_bot, cmdparams->source, "Channel %s Joins %ld", 
		cs->name, cs->users.alltime.runningtotal);
}

static void top10kickshandler (channelstat *cs, void *v)
{
	CmdParams *cmdparams = (CmdParams *) v;
	irc_prefmsg(ss_bot, cmdparams->source, "Channel %s Kicks %ld", 
		cs->name, cs->kicks.alltime.runningtotal);
}

static void top10topicshandler (channelstat *cs, void *v)
{
	CmdParams *cmdparams = (CmdParams *) v;
	irc_prefmsg(ss_bot, cmdparams->source, "Channel %s Topics %ld",
		cs->name, cs->topics.alltime.runningtotal);
}

int ss_cmd_channel (CmdParams *cmdparams)
{
	channelstat *cs;

	if (cmdparams->ac == 0) {
		/* they want the top 10 Channels online atm */
		irc_prefmsg(ss_bot, cmdparams->source, "Top 10 Online Channels:");
		irc_prefmsg(ss_bot, cmdparams->source, "======================");
		GetChannelStats (top10membershandler, CHANNEL_SORT_MEMBERS, 10, (UserLevel(cmdparams->source) < NS_ULEVEL_OPER), (void *)cmdparams);
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else if (!ircstrcasecmp(cmdparams->av[0], "POP")) {
		/* they want the top 10 Popular Channels (based on joins) */
		irc_prefmsg(ss_bot, cmdparams->source, "Top 10 Channels (Ever):");
		irc_prefmsg(ss_bot, cmdparams->source, "======================");
		GetChannelStats (top10joinshandler, CHANNEL_SORT_JOINS, 10, (UserLevel(cmdparams->source) < NS_ULEVEL_OPER), (void *)cmdparams);
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else if (!ircstrcasecmp(cmdparams->av[0], "KICKS")) {
		/* they want the top 10 most unwelcome channels (based on kicks) */
		irc_prefmsg(ss_bot,cmdparams->source, "Top 10 Most un-welcome Channels (Ever):");
		irc_prefmsg(ss_bot,cmdparams->source, "======================================");
		GetChannelStats (top10kickshandler, CHANNEL_SORT_KICKS, 10, (UserLevel(cmdparams->source) < NS_ULEVEL_OPER), (void *)cmdparams);
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else if (!ircstrcasecmp(cmdparams->av[0], "TOPICS")) {
		/* they want the top 10 most undecisive channels (based on topics) */
		irc_prefmsg(ss_bot, cmdparams->source, "Top 10 Most undecisive Channels (Ever):");
		irc_prefmsg(ss_bot, cmdparams->source, "======================================");
		GetChannelStats (top10topicshandler, CHANNEL_SORT_TOPICS, 10, (UserLevel(cmdparams->source) < NS_ULEVEL_OPER), (void *)cmdparams);
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else {
		cs = findchanstats(cmdparams->av[0]);
		if (!cs) {
			irc_prefmsg(ss_bot,cmdparams->source, 
				"Error, Can't find any information about Channel %s", cmdparams->av[0]);
			return NS_SUCCESS;
		}
		irc_prefmsg(ss_bot, cmdparams->source, "\2Channel Information for %s (%s)\2", 
			cmdparams->av[0], (find_channel(cmdparams->av[0]) ? "Online" : "Offline"));
		irc_prefmsg(ss_bot, cmdparams->source, "Current Members: %ld (Max %ld on %s)",
			cs->c->users, cs->users.alltime.max, sftime(cs->users.alltime.ts_max));
		irc_prefmsg(ss_bot,cmdparams->source, "Max Members today: %ld at %s", 
			cs->users.daily.max, sftime(cs->users.daily.ts_max));
		irc_prefmsg(ss_bot,cmdparams->source, "Total Number of Channel Joins: %ld", 
			cs->users.alltime.runningtotal);
		irc_prefmsg(ss_bot, cmdparams->source, "Total Member Joins today: %ld (Max %ld on %s)",
			cs->joins.daily.runningtotal, cs->joins.alltime.max, sftime(cs->joins.alltime.ts_max));
		irc_prefmsg(ss_bot,cmdparams->source, "Total Topic Changes %ld (Today %ld)", 
			cs->topics, cs->topics.daily.runningtotal);
		irc_prefmsg(ss_bot, cmdparams->source, "Total Kicks: %ld", cs->kicks);
		irc_prefmsg(ss_bot, cmdparams->source, "Total Kicks today %ld (Max %ld on %s)",
			cs->kicks.daily.max, cs->kicks.alltime.max, sftime(cs->kicks.alltime.ts_max));
		if (!find_channel(cmdparams->av[0]))
			irc_prefmsg(ss_bot, cmdparams->source, "Channel was last seen at %s",
				sftime(cs->ts_lastseen));
	}
	return NS_SUCCESS;
}

void SaveChanStats(void)
{
	channelstat *c;
	lnode_t *cn;
	int limit;
    int count = 0;

	/* we want to only do 25% each progressive save */
	limit = (list_count(channelstatlist)/4);
	cn = list_first(channelstatlist);
	while (cn) {
		c = lnode_get(cn);
		/* we are not shutting down, so do progressive save if we have more than 100 channels */
		if (StatServ.shutdown == 0 && (limit > 25)) {
			if (count > limit) {
				break;
			}
			/* calc is we save the entire database in the savedb interval plus 1/2 */
			if ((me.now - c->lastsave) < PROGCHANTIME) {
				cn = list_next(channelstatlist, cn);
				continue;
			}
			count++;
		}
		save_chan(c);
		cn = list_next(channelstatlist, cn);
	}
}

/* @brief load the info for a specific channel from the database 
 * or return null a blank if it does not exist. 
 * 
 * @params name the channel name to load
 * 
 * @returns a channelstat struct that contains info for the channel. If its a new Channel, contains the name and thats it.
 */
 
channelstat *load_chan(char *name) 
{
	char *data;
	channelstat *c;

	SET_SEGV_LOCATION();
	if (list_isfull (channelstatlist)) {
		nlog (LOG_CRITICAL, "StatServ channel hash full");
		return NULL;
	}
	c = ns_calloc (sizeof (channelstat));
#ifdef USE_BERKELEY
	if ((data = DBGetData (name)) != NULL) {
		memcpy (c, data, sizeof(channelstat));
		dlog (DEBUG2, "Loading channel %s", c->name);
#else
	strlcpy (c->name, name, MAXCHANLEN);	
	if (GetData ((void *)&data, CFGSTR, CHANNEL_TABLE, c->name, "ChanData") > 0) {
		dlog (DEBUG2, "Loading channel %s", c->name);
		LoadStatistic (&c->users, CHANNEL_TABLE, c->name, "users");
		LoadStatistic (&c->kicks, CHANNEL_TABLE, c->name, "kicks");
		LoadStatistic (&c->topics, CHANNEL_TABLE, c->name, "topics");
		LoadStatistic (&c->joins, CHANNEL_TABLE, c->name, "joins");
		ns_free (data);
#endif
	} else {
		dlog (DEBUG2, "Creating channel %s", c->name);
		strlcpy (c->name, name, MAXCHANLEN);	
	}
	c->lastsave = me.now;
	if ((me.now - c->ts_lastseen) > 604800) {
		dlog (DEBUG1, "Reset old channel %s", c->name);
		c->ts_lastseen = 0;
		c->lastsave = me.now;
	}
	lnode_create_append (channelstatlist, c);
	return c;
}

/* @brief save the info for a specific channel to the database 
 *  
 * 
 * @params c the channelstat struct to save
 * 
 * @returns nothing
 */
 
void save_chan(channelstat *c) 
{
#ifdef USE_BERKELEY
	char data[BUFSIZE];

	memcpy (data, c, sizeof(channelstat));
	DBSetData (c->name, data, sizeof(channelstat));
#else
	/* we keep this seperate so we can easily delete old channels */
	SetData ((void *)c->ts_lastseen, CFGINT, CHANNEL_TABLE, c->name, "ts_lastseen");
	SaveStatistic (&c->users, CHANNEL_TABLE, c->name, "users");
	SaveStatistic (&c->kicks, CHANNEL_TABLE, c->name, "kicks");
	SaveStatistic (&c->topics, CHANNEL_TABLE, c->name, "topics");
	SaveStatistic (&c->joins, CHANNEL_TABLE, c->name, "joins");
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
	time_t ts_lastseen;
	time_t start;
	
	start = time (NULL);
	dlog (DEBUG1, "Deleting old channels");
	if (GetTableData (CHANNEL_TABLE, &row) > 0) {
		for (count = 0; row[count] != NULL; count++) {
			if (GetData ((void *)&ts_lastseen, CFGINT, CHANNEL_TABLE, row[count], "ts_lastseen") > 0) {
				/* delete it if its old and not online 
				 * use find_channel, instead of findchanstats, and find_channel is based on hashes, so its faster 
				 */
				if (((me.now - ts_lastseen) > 604800) && (!find_channel(row[count]))) {
					dlog (DEBUG1, "Deleting Channel %s", row[count]);
					DelRow(CHANNEL_TABLE, row[count]);
				}
			} else {
				/* database corruption? */
				nlog (LOG_WARNING, "Channel %s corrupted: deleting record", row[count]);
				DelRow (CHANNEL_TABLE, row[count]);
			}
		}
	}
	ns_free (row);
	dlog (DEBUG1, "DelOldChan: %d seconds %d channels", (int)(time(NULL) - start), count);
	return NS_SUCCESS;
}

void InitChannelStats (void)
{
	channelstatlist = list_create(-1);
	GetChannelList (AddChannel, NULL);
}

void FiniChannelStats (void)
{
	lnode_t *ln;
	channelstat *cs;

	SaveChanStats();
	ln = list_first (channelstatlist);
	while (ln) {
		cs = (channelstat *)lnode_get (ln);
		ClearChannelModValue (cs->c);
		ns_free (cs);
		ln = list_next (channelstatlist, ln);
	}
	list_destroy_nodes (channelstatlist);
	list_destroy (channelstatlist);
}

void GetChannelStats (ChannelStatHandler handler, channelsort sortstyle, int maxcount, int ignorehidden, void *v)
{
	int i = 0;
	lnode_t *ln;
	channelstat *cs;

	switch (sortstyle) {
		case CHANNEL_SORT_MEMBERS:
			if (!list_is_sorted(channelstatlist, topcurrentchannel)) {
				list_sort(channelstatlist, topcurrentchannel);
			}
			break;
		case CHANNEL_SORT_JOINS:
			if (!list_is_sorted(channelstatlist, topjoinrunningtotalchannel)) {
				list_sort(channelstatlist, topjoinrunningtotalchannel);
			}
			break;
		case CHANNEL_SORT_KICKS:
			if (!list_is_sorted(channelstatlist, topkickrunningtotalchannel)) {
				list_sort(channelstatlist, topkickrunningtotalchannel);
			}
			break;
		case CHANNEL_SORT_TOPICS:
			if (!list_is_sorted(channelstatlist, toptopicrunningtotalchannel)) {
				list_sort(channelstatlist, toptopicrunningtotalchannel);
			}
			break;
		default:
			break;
	}

	ln = list_first (channelstatlist);
	while (ln) {
		cs = (channelstat *)lnode_get (ln);
		if (i >= maxcount)
			break;
		if (!ignorehidden || !is_hidden_chan(cs->c))
		{
			i++;
			handler (cs, v);
		}
		ln = list_next (channelstatlist, ln);
	}
}
