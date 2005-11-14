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
#include "channel.h"

/** Channel table name */
#define CHANNEL_TABLE	"Channel"

/** Channel list */
static list_t *channelstatlist;

/** @brief AverageChannelStatistic
 *
 *  Average channel statistic
 *
 *  @param none
 *
 *  @return none
 */

static void AverageChannelStatistic( channelstat *cs, const void *v )
{
	AverageStatistic( &cs->users );
	AverageStatistic( &cs->kicks );
	AverageStatistic( &cs->topics );
	AverageStatistic( &cs->joins );
}

/** @brief AverageChannelStatistics
 *
 *  Average channel statistics
 *
 *  @param none
 *
 *  @return none
 */

void AverageChannelStatistics( void )
{
	GetChannelStats( AverageChannelStatistic, CHANNEL_SORT_NONE, -1, 0, NULL );
}

/** @brief ResetChannelStatistic
 *
 *  Reset channel statistic
 *
 *  @param none
 *
 *  @return none
 */

void ResetChannelStatistic( channelstat *cs, const void *v )
{
	ResetStatistic( &cs->users );
	ResetStatistic( &cs->kicks );
	ResetStatistic( &cs->topics );
	ResetStatistic( &cs->joins );
}

/** @brief ResetChannelStatistics
 *
 *  Reset channel statistics
 *
 *  @param none
 *
 *  @return none
 */

void ResetChannelStatistics( void )
{
	GetChannelStats( ResetChannelStatistic, CHANNEL_SORT_NONE, -1, 0, NULL );
}

/** @brief findchanstats
 *
 *  Check list for channel
 *
 *  @param name of channel ctcp version to search for
 *
 *  @return pointer to stat found or NULL if none
 */

static channelstat *findchanstats( const char *name )
{
	channelstat *cs;

	cs = lnode_find( channelstatlist, name, comparef );
	if( !cs )
		dlog( DEBUG2, "findchanstats: %s not found", name );
	return cs;
}

/** @brief topcurrentchannel
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

int topcurrentchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->c->users - chan1->c->users );
}

/** @brief topjoinrunningtotalchannel
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

int topjoinrunningtotalchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->users.alltime.runningtotal - chan1->users.alltime.runningtotal );
}

/** @brief topkickrunningtotalchannel
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

int topkickrunningtotalchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->kicks.alltime.runningtotal - chan1->kicks.alltime.runningtotal );
}

/** @brief toptopicrunningtotalchannel
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

int toptopicrunningtotalchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->topics.alltime.runningtotal - chan1->topics.alltime.runningtotal );
}

/** @brief LoadChannel
 *
 *  Load channel stats
 *
 *  @param name of channel to load
 *
 *  @return pointer to stat
 */

static channelstat *LoadChannel( char *name ) 
{
	channelstat *cs;

	SET_SEGV_LOCATION();
	if( list_isfull( channelstatlist ) )
	{
		nlog( LOG_CRITICAL, "StatServ channel hash full" );
		return NULL;
	}
	cs = ns_calloc( sizeof( channelstat ) );
	if( DBAFetch( CHANNEL_TABLE, name, cs, sizeof( channelstat ) ) == NS_SUCCESS )
	{
		dlog( DEBUG2, "Loading channel %s", cs->name );
		PostLoadStatistic( &cs->joins );
		PostLoadStatistic( &cs->kicks );
		PostLoadStatistic( &cs->topics );
		PostLoadStatistic( &cs->users );
		if( ( me.now - cs->ts_lastseen ) > StatServ.channeltime )
		{
			dlog( DEBUG1, "Reset old channel %s", cs->name );
			cs->ts_lastseen = me.now;
		}
	} 
	else
	{
		dlog( DEBUG2, "Creating channel %s", cs->name );
		strlcpy( cs->name, name, MAXCHANLEN );	
	}
	cs->lastsave = me.now;
	lnode_create_append( channelstatlist, cs );
	return cs;
}

/** @brief SaveChannel
 *
 *  Save channel stats
 *
 *  @param cs channel to save
 *
 *  @return pointer to stat
 */

static void SaveChannel( channelstat *cs ) 
{
	PreSaveStatistic( &cs->joins );
	PreSaveStatistic( &cs->kicks );
	PreSaveStatistic( &cs->topics );
	PreSaveStatistic( &cs->users );
	DBAStore( CHANNEL_TABLE, cs->name, ( void * )cs, sizeof( channelstat ) );
	cs->lastsave = me.now;
}

/** @brief AddChannel
 *
 *  Add channel
 *
 *  @param c pointer to channel
 *  @param v not used
 *
 *  @return none
 */

static int AddChannel( Channel* c, void *v )
{
	channelstat *cs;

	cs = LoadChannel( c->name );
	AddNetworkChannel();
    SetChannelModValue( c, ( void * )cs );
	cs->c = c;
	return NS_FALSE;
}

/** @brief ss_event_newchan
 *
 *  NEWCHAN event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_newchan( const CmdParams *cmdparams )
{
	AddChannel( cmdparams->channel, NULL );
	return NS_SUCCESS;
}

/** @brief ss_event_delchan
 *
 *  DELCHAN event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_delchan( const CmdParams *cmdparams )
{
	channelstat *cs;
	lnode_t *ln;
	
	ClearChannelModValue( cmdparams->channel );
	DelNetworkChannel();
	ln = list_find( channelstatlist, cmdparams->channel->name, comparef );
	if( !ln )
	{
		nlog( LOG_WARNING, "Couldn't find channel %s when deleting from stats", cmdparams->channel->name );
		return NS_SUCCESS;
	}
	cs = lnode_get( ln );
	SaveChannel( cs );
	list_delete_destroy_node( channelstatlist, ln );
	ns_free( cs );
	return NS_SUCCESS;
}

/** @brief ss_event_join
 *
 *  JOIN event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_join( const CmdParams *cmdparams )
{										   
	channelstat *cs;

	cs = GetChannelModValue( cmdparams->channel );
	if( !cs )
	{
		dlog( DEBUG4, "Cannot find stats for channel %s", cmdparams->channel->name );
		return NS_SUCCESS;
	}
	IncStatistic( &cs->users );
	IncStatistic( &cs->joins );
	return NS_SUCCESS;
}

/** @brief ss_event_part
 *
 *  PART event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_part( const CmdParams *cmdparams )
{
	channelstat *cs;

	cs = GetChannelModValue( cmdparams->channel );
	if( !cs )
	{
		dlog( DEBUG4, "Cannot find stats for channel %s", cmdparams->channel->name );
		return NS_SUCCESS;
	}
	DecStatistic( &cs->users );
	cs->ts_lastseen = me.now;
	return NS_SUCCESS;
}

/** @brief ss_event_kick
 *
 *  KICK event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_topic( const CmdParams *cmdparams )
{
	channelstat *cs;

	cs = GetChannelModValue( cmdparams->channel );
	if( !cs )
	{
		dlog( DEBUG4, "Cannot find stats for channel %s", cmdparams->channel->name );
		return NS_SUCCESS;
	}
	IncStatistic( &cs->topics );
	return NS_SUCCESS;
}

/** @brief ss_event_kick
 *
 *  KICK event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_kick( const CmdParams *cmdparams )
{
	channelstat *cs;

	cs = GetChannelModValue( cmdparams->channel );
	if( !cs )
	{
		dlog( DEBUG4, "Cannot find stats for channel %s", cmdparams->channel->name );
		return NS_SUCCESS;
	}
	IncStatistic( &cs->kicks );
	DecStatistic( &cs->users );
	return NS_SUCCESS;
}

/** @brief top10membershandler
 *
 *  Report topics
 *
 *  @param cs pointer to channel stat
 *  @param v pointer to client to send to
 *
 *  @return none
 */

static void top10membershandler( channelstat *cs, const void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Members %d", 
		cs->name, cs->c->users );
}

/** @brief top10joinshandler
 *
 *  Report topics
 *
 *  @param cs pointer to channel stat
 *  @param v pointer to client to send to
 *
 *  @return none
 */

static void top10joinshandler( channelstat *cs, const void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Joins %d", 
		cs->name, cs->users.alltime.runningtotal );
}

/** @brief top10kickshandler
 *
 *  Report topics
 *
 *  @param cs pointer to channel stat
 *  @param v pointer to client to send to
 *
 *  @return none
 */

static void top10kickshandler( channelstat *cs, const void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Kicks %d", 
		cs->name, cs->kicks.alltime.runningtotal );
}

/** @brief top10topicshandler
 *
 *  Report topics
 *
 *  @param cs pointer to channel stat
 *  @param v pointer to client to send to
 *
 *  @return none
 */

static void top10topicshandler( channelstat *cs, const void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Topics %d",
		cs->name, cs->topics.alltime.runningtotal );
}

/** @brief ss_cmd_channel
 *
 *  CHANNEL command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *    cmdparams->av[0] = optionally POP, KICKS, TOPICS, channel name
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_channel( const CmdParams *cmdparams )
{
	channelstat *cs;

	if( cmdparams->ac == 0 )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "Top 10 Online Channels:" );
		irc_prefmsg( ss_bot, cmdparams->source, "=======================" );
		GetChannelStats( top10membershandler, CHANNEL_SORT_MEMBERS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	}
	else if( ircstrcasecmp( cmdparams->av[0], "POP" ) == 0 )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "Top 10 Join Channels (Ever):" );
		irc_prefmsg( ss_bot, cmdparams->source, "============================" );
		GetChannelStats( top10joinshandler, CHANNEL_SORT_JOINS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	}
	else if( ircstrcasecmp( cmdparams->av[0], "KICKS" ) == 0 )
	{
		irc_prefmsg( ss_bot,cmdparams->source, "Top 10 Kick Channels (Ever):" );
		irc_prefmsg( ss_bot,cmdparams->source, "============================" );
		GetChannelStats( top10kickshandler, CHANNEL_SORT_KICKS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	}
	else if( ircstrcasecmp( cmdparams->av[0], "TOPICS" ) == 0 )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "Top 10 Topic Channels (Ever):" );
		irc_prefmsg( ss_bot, cmdparams->source, "=============================" );
		GetChannelStats( top10topicshandler, CHANNEL_SORT_TOPICS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	}
	else
	{
		cs = findchanstats( cmdparams->av[0] );
		if( !cs )
		{
			irc_prefmsg( ss_bot,cmdparams->source, 
				"No statistics for %s", cmdparams->av[0] );
			return NS_SUCCESS;
		}
		irc_prefmsg( ss_bot, cmdparams->source, "\2Channel statistics for %s (%s)\2", 
			cmdparams->av[0], cs->c ? "Online" : "Offline" );
		irc_prefmsg( ss_bot, cmdparams->source, "Current Members: %d (Max %d on %s)",
			cs->c->users, cs->users.alltime.max, sftime( cs->users.alltime.ts_max ) );
		irc_prefmsg( ss_bot,cmdparams->source, "Max Members today: %d at %s", 
			cs->users.daily.max, sftime( cs->users.daily.ts_max ) );
		irc_prefmsg( ss_bot,cmdparams->source, "Total Channel Joins: %d", 
			cs->users.alltime.runningtotal );
		irc_prefmsg( ss_bot, cmdparams->source, "Total Joins today: %d (Max %d on %s)",
			cs->joins.daily.runningtotal, cs->joins.alltime.max, sftime( cs->joins.alltime.ts_max ) );
		irc_prefmsg( ss_bot,cmdparams->source, "Total Topic Changes %d (Today %d)", 
			cs->topics.day, cs->topics.daily.runningtotal );
		irc_prefmsg( ss_bot, cmdparams->source, "Total Kicks: %d", cs->kicks.alltime.runningtotal );
		irc_prefmsg( ss_bot, cmdparams->source, "Total Kicks today %d (Max %d on %s)",
			cs->kicks.daily.max, cs->kicks.alltime.max, sftime( cs->kicks.alltime.ts_max ) );
		if( !cs->c )
			irc_prefmsg( ss_bot, cmdparams->source, "Channel last seen at %s",
				sftime( cs->ts_lastseen ) );
	}
	return NS_SUCCESS;
}

/** @brief SaveChanStats
 *
 *  Save channel stats
 *
 *  @param none
 *
 *  @return none
 */

void SaveChanStats( void )
{
	channelstat *cs;
	lnode_t *cn;

	cn = list_first( channelstatlist );
	while( cn )
	{
		cs = lnode_get( cn );
		SaveChannel( cs );
		cn = list_next( channelstatlist, cn );
	}
}

/** @brief SaveChanStatsProgressive
 *
 *  Save channel stats progressively
 *  Do we still need this functionality?
 *
 *  @param none
 *
 *  @return none
 */

void SaveChanStatsProgressive( void )
{
	channelstat *cs;
	lnode_t *cn;
	int limit;
    int count = 0;

	/* we want to only do 25% each progressive save */
	limit = ( list_count( channelstatlist ) / 4 );
	cn = list_first( channelstatlist );
	while( cn )
	{
		cs = lnode_get( cn );
		/* do progressive save if we have more than 100 channels */
		if( limit > 25 )
		{
			if( count > limit )
				break;
			/* calc is we save the entire database in the savedb interval plus 1/2 */
			if( ( me.now - cs->lastsave ) < PROGCHANTIME )
			{
				cn = list_next( channelstatlist, cn );
				continue;
			}
			count++;
		}
		SaveChannel( cs );
		cn = list_next( channelstatlist, cn );
	}
}

/** @brief del_chan
 *
 *  Delete old channel stats table row handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE if deleted else NS_FALSE
 */

static int del_chan( void *data, int size )
{
	channelstat *cs;
	
	if( size != sizeof( channelstat ) )
		return NS_FALSE;
	cs = ( channelstat * )data;
	if( ( ( me.now - cs->ts_lastseen ) > StatServ.channeltime ) && ( !cs->c ) )
	{
		dlog( DEBUG1, "Deleting Channel %s", cs->name );
		DBADelete( CHANNEL_TABLE, cs->name );
		/* Delete only one channel per loop */
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief DelOldChanTimer
 *
 *  Delete old channel stats timer handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int DelOldChanTimer( void *userptr )
{
	time_t start;

	start = time( NULL );
	dlog( DEBUG1, "Deleting old channels" );
	DBAFetchRows( CHANNEL_TABLE, del_chan );
	dlog( DEBUG1, "DelOldChanTimer: %d seconds", ( int )( time( NULL ) - start ) );
	return NS_SUCCESS;
}

/** @brief GetChannelStats
 *
 *  Walk through list passing each channel to handler
 *
 *  @param handler pointer to handler function
 *  @param sortstyle
 *  @param maxcount
 *  @param ignorehidden
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void GetChannelStats( const ChannelStatHandler handler, CHANNEL_SORT sortstyle, int maxcount, int ignorehidden, const void *v )
{
	int count = 0;
	lnode_t *ln;
	channelstat *cs;

	switch( sortstyle )
	{
		case CHANNEL_SORT_MEMBERS:
			if( !list_is_sorted( channelstatlist, topcurrentchannel ) )
				list_sort( channelstatlist, topcurrentchannel );
			break;
		case CHANNEL_SORT_JOINS:
			if( !list_is_sorted( channelstatlist, topjoinrunningtotalchannel ) )
				list_sort( channelstatlist, topjoinrunningtotalchannel );
			break;
		case CHANNEL_SORT_KICKS:
			if( !list_is_sorted( channelstatlist, topkickrunningtotalchannel ) )
				list_sort( channelstatlist, topkickrunningtotalchannel );
			break;
		case CHANNEL_SORT_TOPICS:
			if( !list_is_sorted( channelstatlist, toptopicrunningtotalchannel ) )
				list_sort( channelstatlist, toptopicrunningtotalchannel );
			break;
		default:
			break;
	}

	ln = list_first( channelstatlist );
	while( ln )
	{
		cs = ( channelstat * )lnode_get( ln );
		if( maxcount != -1 && count >= maxcount )
			break;
		if( !ignorehidden || !is_hidden_chan( cs->c ) )
		{
			count++;
			handler( cs, v );
		}
		ln = list_next( channelstatlist, ln );
	}
}

/** @brief InitChannelStats
 *
 *  Init channel stats
 *
 *  @param none
 *
 *  @return NS_SUCCESS on success, NS_FAILURE on failure
 */

int InitChannelStats( void )
{
	channelstatlist = list_create( LISTCOUNT_T_MAX );
	if( !channelstatlist )
	{
		nlog( LOG_CRITICAL, "Unable to create channel stat list" );
		return NS_FAILURE;
	}
	DBAOpenTable( CHANNEL_TABLE );
	ProcessChannelList( AddChannel, NULL );
	return NS_SUCCESS;
}

/** @brief FiniChannelStats
 *
 *  Fini channel stats
 *
 *  @param none
 *
 *  @return none
 */

void FiniChannelStats( void )
{
	lnode_t *ln;
	channelstat *cs;

	SaveChanStats();
	DBACloseTable( CHANNEL_TABLE );
	ln = list_first( channelstatlist );
	while( ln )
	{
		cs = ( channelstat * )lnode_get( ln );
		ClearChannelModValue( cs->c );
		ns_free( cs );
		ln = list_next( channelstatlist, ln );
	}
	list_destroy_nodes( channelstatlist );
	list_destroy( channelstatlist );
}
