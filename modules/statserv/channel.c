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

#define CHANNEL_TABLE	"Channel"

list_t *channelstatlist;

void AverageChannelStatistics( void )
{
	lnode_t *cn;
	channelstat *cs;

	cn = list_first( channelstatlist );
	while( cn ) {
		cs = lnode_get( cn );
		AverageStatistic( &cs->users );
		AverageStatistic( &cs->kicks );
		AverageStatistic( &cs->topics );
		AverageStatistic( &cs->joins );
		cn = list_next( channelstatlist, cn );
	}
}

void ResetChannelStatistics( void )
{
	lnode_t *cn;
	channelstat *cs;

	cn = list_first( channelstatlist );
	while( cn ) {
		cs = lnode_get( cn );
		ResetStatistic( &cs->users );
		ResetStatistic( &cs->kicks );
		ResetStatistic( &cs->topics );
		ResetStatistic( &cs->joins );
		cn = list_next( channelstatlist, cn );
	}
}

static channelstat *findchanstats( char *name )
{
	channelstat *cs;

	cs = lnode_find( channelstatlist, name, comparef );
	if( !cs ) {
		dlog( DEBUG2, "findchanstats: %s not found", name );
	}	
	return cs;
}

int topcurrentchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->c->users - chan1->c->users );
}

int topjoinrunningtotalchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->users.alltime.runningtotal - chan1->users.alltime.runningtotal );
}

int topkickrunningtotalchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->kicks.alltime.runningtotal - chan1->kicks.alltime.runningtotal );
}

int toptopicrunningtotalchannel( const void *key1, const void *key2 )
{
	const channelstat *chan1 = key1;
	const channelstat *chan2 = key2;
	return( chan2->topics.alltime.runningtotal - chan1->topics.alltime.runningtotal );
}

/* @brief load the info for a specific channel from the database 
 * or return null a blank if it does not exist. 
 * 
 * @params name the channel name to load
 * 
 * @returns a channelstat struct that contains info for the channel. If its a new Channel, contains the name and thats it.
 */
 
static channelstat *LoadChannel( char *name ) 
{
	channelstat *cs;

	SET_SEGV_LOCATION();
	if( list_isfull( channelstatlist ) ) {
		nlog( LOG_CRITICAL, "StatServ channel hash full" );
		return NULL;
	}
	cs = ns_calloc( sizeof( channelstat ) );
	if( DBAFetch( CHANNEL_TABLE, name, cs, sizeof( channelstat ) ) == NS_SUCCESS ) {
		dlog( DEBUG2, "Loading channel %s", cs->name );
		PostLoadStatistic( &cs->joins );
		PostLoadStatistic( &cs->kicks );
		PostLoadStatistic( &cs->topics );
		PostLoadStatistic( &cs->users );
		if( ( me.now - cs->ts_lastseen ) > StatServ.channeltime ) {
			dlog( DEBUG1, "Reset old channel %s", cs->name );
			cs->ts_lastseen = me.now;
		}
	} else {
		dlog( DEBUG2, "Creating channel %s", cs->name );
		strlcpy( cs->name, name, MAXCHANLEN );	
	}
	cs->lastsave = me.now;
	lnode_create_append( channelstatlist, cs );
	return cs;
}

/* @brief save the info for a specific channel to the database 
 *  
 * 
 * @params cs the channelstat struct to save
 * 
 * @returns nothing
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

static int AddChannel( Channel* c, void *v )
{
	channelstat *cs;

	cs = LoadChannel( c->name );
	AddNetworkChannel();
    SetChannelModValue( c, ( void * )cs );
	cs->c = c;
	return NS_FALSE;
}

int ss_event_newchan( CmdParams *cmdparams )
{
	AddChannel( cmdparams->channel, NULL );
	return NS_SUCCESS;
}

int ss_event_delchan( CmdParams *cmdparams )
{
	channelstat *cs;
	lnode_t *ln;
	
	ClearChannelModValue( cmdparams->channel );
	DelNetworkChannel();
	ln = list_find( channelstatlist, cmdparams->channel->name, comparef );
	if( !ln ) {
		nlog( LOG_WARNING, "Couldn't find channel %s when deleting from stats", cmdparams->channel->name );
		return NS_SUCCESS;
	}
	cs = lnode_get( ln );
	SaveChannel( cs );
	list_delete( channelstatlist, ln );
	lnode_destroy( ln );
	ns_free( cs );
	return NS_SUCCESS;
}

int ss_event_join( CmdParams *cmdparams )
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

int ss_event_part( CmdParams *cmdparams )
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

int ss_event_topic( CmdParams *cmdparams )
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

int ss_event_kick( CmdParams *cmdparams )
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

static void top10membershandler( channelstat *cs, void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot,cmdparams->source, "Channel %s Members %d", 
		cs->name, cs->c->users );
}

static void top10joinshandler( channelstat *cs, void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Joins %d", 
		cs->name, cs->users.alltime.runningtotal );
}

static void top10kickshandler( channelstat *cs, void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Kicks %d", 
		cs->name, cs->kicks.alltime.runningtotal );
}

static void top10topicshandler( channelstat *cs, void *v )
{
	CmdParams *cmdparams = ( CmdParams * ) v;

	irc_prefmsg( ss_bot, cmdparams->source, "Channel %s Topics %d",
		cs->name, cs->topics.alltime.runningtotal );
}

int ss_cmd_channel( CmdParams *cmdparams )
{
	channelstat *cs;

	if( cmdparams->ac == 0 ) {
		irc_prefmsg( ss_bot, cmdparams->source, "Top 10 Online Channels:" );
		irc_prefmsg( ss_bot, cmdparams->source, "=======================" );
		GetChannelStats( top10membershandler, CHANNEL_SORT_MEMBERS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	} else if( !ircstrcasecmp( cmdparams->av[0], "POP" ) ) {
		irc_prefmsg( ss_bot, cmdparams->source, "Top 10 Join Channels (Ever):" );
		irc_prefmsg( ss_bot, cmdparams->source, "============================" );
		GetChannelStats( top10joinshandler, CHANNEL_SORT_JOINS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	} else if( !ircstrcasecmp( cmdparams->av[0], "KICKS" ) ) {
		irc_prefmsg( ss_bot,cmdparams->source, "Top 10 Kick Channels (Ever):" );
		irc_prefmsg( ss_bot,cmdparams->source, "============================" );
		GetChannelStats( top10kickshandler, CHANNEL_SORT_KICKS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	} else if( !ircstrcasecmp( cmdparams->av[0], "TOPICS" ) ) {
		irc_prefmsg( ss_bot, cmdparams->source, "Top 10 Topic Channels (Ever):" );
		irc_prefmsg( ss_bot, cmdparams->source, "=============================" );
		GetChannelStats( top10topicshandler, CHANNEL_SORT_TOPICS, 10, ( UserLevel( cmdparams->source ) < NS_ULEVEL_OPER ), ( void * )cmdparams );
		irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	} else {
		cs = findchanstats( cmdparams->av[0] );
		if( !cs ) {
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

void SaveChanStats( void )
{
	channelstat *cs;
	lnode_t *cn;
	int limit;
    int count = 0;

	/* we want to only do 25% each progressive save */
	limit = ( list_count( channelstatlist ) /4 );
	cn = list_first( channelstatlist );
	while( cn ) {
		cs = lnode_get( cn );
		/* we are not shutting down, so do progressive save if we have more than 100 channels */
		if( StatServ.shutdown == 0 && ( limit > 25 ) ) {
			if( count > limit ) {
				break;
			}
			/* calc is we save the entire database in the savedb interval plus 1/2 */
			if( ( me.now - cs->lastsave ) < PROGCHANTIME ) {
				cn = list_next( channelstatlist, cn );
				continue;
			}
			count++;
		}
		SaveChannel( cs );
		cn = list_next( channelstatlist, cn );
	}
}

/* @brief run through database deleting old channels 
 *  
 * 
 * @params nothing
 * 
 * @returns nothing
 */

static int del_chan( void *data )
{
	channelstat *cs;
	
	cs = ( channelstat * )data;
	if( ( ( me.now - cs->ts_lastseen ) > StatServ.channeltime ) && ( !cs->c ) ) {
		dlog( DEBUG1, "Deleting Channel %s", cs->name );
		DBADelete( CHANNEL_TABLE, cs->name );
		/* Delete only one channel per loop */
		return NS_TRUE;
	}
	return NS_FALSE;
}

int DelOldChan( void )
{
	time_t start;

	start = time( NULL );
	dlog( DEBUG1, "Deleting old channels" );
	DBAFetchRows( CHANNEL_TABLE, del_chan );
	dlog( DEBUG1, "DelOldChan: %d seconds", ( int )( time( NULL ) - start ) );
	return NS_SUCCESS;
}

void InitChannelStats( void )
{
	channelstatlist = list_create( -1 );
	GetChannelList( AddChannel, NULL );
}

void FiniChannelStats( void )
{
	lnode_t *ln;
	channelstat *cs;

	SaveChanStats();
	ln = list_first( channelstatlist );
	while( ln ) {
		cs = ( channelstat * )lnode_get( ln );
		ClearChannelModValue( cs->c );
		ns_free( cs );
		ln = list_next( channelstatlist, ln );
	}
	list_destroy_nodes( channelstatlist );
	list_destroy( channelstatlist );
}

void GetChannelStats( ChannelStatHandler handler, CHANNEL_SORT sortstyle, int maxcount, int ignorehidden, void *v )
{
	int i = 0;
	lnode_t *ln;
	channelstat *cs;

	switch( sortstyle ) {
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
	while( ln ) {
		cs = ( channelstat * )lnode_get( ln );
		if( i >= maxcount )
			break;
		if( !ignorehidden || !is_hidden_chan( cs->c ) )
		{
			i++;
			handler( cs, v );
		}
		ln = list_next( channelstatlist, ln );
	}
}
