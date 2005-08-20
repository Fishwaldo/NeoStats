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
#include "server.h"

/** Server table name */
#define SERVER_TABLE	"Server"

/** Server hash */
static hash_t *serverstathash;

/** @brief AverageServerStatistics
 *
 *  Average server statistics
 *
 *  @param none
 *
 *  @return none
 */

void AverageServerStatistics( void )
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	hash_scan_begin( &hs, serverstathash );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		ss = hnode_get( sn );
		AverageStatistic( &ss->users );
		AverageStatistic( &ss->opers );
	}
}

/** @brief ResetServerStatistics
 *
 *  Reset server statistics
 *
 *  @param none
 *
 *  @return none
 */

void ResetServerStatistics( void )
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	hash_scan_begin( &hs, serverstathash );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		ss = hnode_get( sn );
		ResetStatistic( &ss->users );
		ResetStatistic( &ss->opers );
	}
}

/** @brief new_server_stat
 *
 *  Create new server stat
 *
 *  @param name of server to search for
 *
 *  @return pointer to stat created or NULL if unable
 */

static serverstat *new_server_stat( const char *name )
{
	serverstat *ss;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "new_server_stat( %s )", name );
	if( hash_isfull( serverstathash ) )
	{
		nlog( LOG_CRITICAL, "StatServ Server hash is full!" );
		return NULL;
	}
	ss = ns_calloc( sizeof( serverstat ) );
	memcpy( ss->name, name, MAXHOST );
	hnode_create_insert( serverstathash, ss, ss->name );
	return ss;
}

/** @brief findserverstats
 *
 *  Check list for server
 *
 *  @param name of server to search for
 *
 *  @return pointer to stat found or NULL if none
 */

static serverstat *findserverstats( const char *name )
{
	serverstat *stats;

	stats =( serverstat * )hnode_find( serverstathash, name );
	if( !stats )
		dlog( DEBUG2, "findserverstats( %s ) - not found", name );
	return stats;
}

/** @brief AddServerUser
 *
 *  Add user stat
 *
 *  @param u pointer to client to update
 *
 *  @return none
 */

void AddServerUser( const  Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	if( IncStatistic( &ss->users ) )
	{
		announce_record( "\2NEW SERVER RECORD!\2 %d users on server %s",
			ss->s->server->users, ss->name );
	}
}

/** @brief DelServerUser
 *
 *  Delete server user
 *
 *  @param u pointer to client to update
 *
 *  @return none
 */

void DelServerUser( const Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	DecStatistic( &ss->users );
}

/** @brief AddServerOper
 *
 *  Add server oper
 *
 *  @param u pointer to client to update
 *
 *  @return none
 */

void AddServerOper( const Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	if( IncStatistic( &ss->opers ) )
	{
		announce_record( "\2NEW SERVER RECORD!\2 %d opers on %s",
			ss->opers.alltime.runningtotal, ss->name );
	}
}

/** @brief DelServerOper
 *
 *  Delete server oper
 *
 *  @param s pointer to client to update
 *
 *  @return none
 */

void DelServerOper( const Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	DecStatistic( &ss->opers );
}

/** @brief AddServerStat
 *
 *  Add server stat
 *
 *  @param s pointer to client to update
 *  @param v not used
 *
 *  @return none
 */

static int AddServerStat( Client *s, void *v )
{
	serverstat *ss;

	dlog( DEBUG2, "AddServerStat( %s )", s->name );
	ss = findserverstats( s->name );
	if( !ss )
		ss = new_server_stat( s->name );
	ss->ts_start = ss->ts_lastseen = me.now;
	AddNetworkServer();
	SetServerModValue( s,( void * )ss );
	ss->s = s;
	return NS_FALSE;
}

/** @brief ss_event_server
 *
 *  SERVER event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_server( CmdParams *cmdparams )
{
	AddServerStat( cmdparams->source, NULL );
	return NS_SUCCESS;
}

/** @brief DelServerStat
 *
 *  Delete server stat
 *
 *  @param s pointer to client to update
 *
 *  @return none
 */

static void DelServerStat( Client* s )
{
	serverstat *ss;
	
	ss =( serverstat * ) GetServerModValue( s );
	ss->ts_lastseen = me.now;
	IncStatistic( &ss->splits );
	ClearServerModValue( s );
	ss->s = NULL;
}

/** @brief ss_event_squit
 *
 *  SQUIT event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_squit( CmdParams *cmdparams )
{
	DelServerStat( cmdparams->source );
	DelNetworkServer();
	return NS_SUCCESS;
}

/** @brief UpdatePingStats
 *
 *  Update ping stats
 *
 *  @param s pointer to client to update
 *
 *  @return none
 */

static void UpdatePingStats( const Client* s )
{
	serverstat *ss;

	ss =( serverstat * ) GetServerModValue( s );
	if( !ss )
		return;
	if( s->server->ping > ss->highest_ping )
	{
		ss->highest_ping = s->server->ping;
		ss->ts_highest_ping = me.now;
	}
	if( s->server->ping < ss->lowest_ping )
	{
		ss->lowest_ping = s->server->ping;
		ss->ts_lowest_ping = me.now;
	}
	/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if( s->server->ping > StatServ.lagtime )
		announce_lag( "\2%s\2 is lagged out with a ping of %d", s->name, s->server->ping );
}

/** @brief ss_event_pong
 *
 *  PONG event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_pong( CmdParams *cmdparams )
{
	/* we don't want negative pings! */
	if( cmdparams->source->server->ping > 0 )
		UpdatePingStats( cmdparams->source );
	return NS_SUCCESS;
}

/** @brief makemap
 *
 *  MAP command handler
 *  Recursively transverse map and report results
 *
 *  @param uplink current point in map
 *  @param u client to report to
 *  @param level in map
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static void makemap( const char *uplink, const Client * u, int level )
{
	static char buf[256];
	hscan_t hs;
	hnode_t *sn;
	Client *s;
	serverstat *ss;
	int i;
	hash_scan_begin( &hs, GetServerHash() );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		s = hnode_get( sn );
		printf( "%d %s %s( %s )\n", level, s->name, s->uplink ? s->uplink->name : "", uplink );
		ss =( serverstat * ) GetServerModValue( s );
		if( ( level == 0 ) &&( s->uplinkname[0] == 0 ) )
		{
			/* its the root server */
			if( StatServ.exclusions && IsExcluded( s ) )
				makemap( s->name, u, level );
			irc_prefmsg( ss_bot, u,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %d/%ld ]",
				ss->name, s->server->users,( int )ss->users.alltime.max,
				ss->opers.current, ss->opers.alltime.max,( int )s->server->ping, ss->highest_ping );
			makemap( s->name, u, level + 1 );
		} else if( ( level > 0 ) &&( s->uplink ) &&  !ircstrcasecmp( s->uplink->name, uplink ) )
		{
			if( StatServ.exclusions && IsExcluded( s ) )
				makemap( s->name, u, level );
			/* its not the root server */
			if( StatServ.flatmap )
			{
				irc_prefmsg( ss_bot, u,
					"\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %d/%ld ]", 
					ss->name, s->server->users,( int )ss->users.alltime.max,
					ss->opers.current, ss->opers.alltime.max,( int )s->server->ping, ss->highest_ping );
			}
			else
			{
				buf[0]='\0';
				for( i = 1; i < level; i++ )
					strlcat( buf, "     |", 256 );
				irc_prefmsg( ss_bot, u,
					"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %d/%ld ]",
					buf, ss->name, s->server->users,( int )ss->users.alltime.max,
					ss->opers.current, ss->opers.alltime.max,( int )s->server->ping, ss->highest_ping );
			}
			makemap( s->name, u, level + 1 );
		}
	}
}

/** @brief ss_cmd_map
 *
 *  MAP command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_map( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_prefmsg( ss_bot, cmdparams->source, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2", "\2[LAG/MAX]\2" );
	makemap( "", cmdparams->source, 0 );
	irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_cmd_server_list
 *
 *  SERVER LIST command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_server_list( CmdParams *cmdparams )
{
	serverstat *ss;
	hscan_t hs;
	hnode_t *sn;

	irc_prefmsg( ss_bot, cmdparams->source, "Server listing:" );
	hash_scan_begin( &hs, serverstathash );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		ss = hnode_get( sn );
		irc_prefmsg( ss_bot, cmdparams->source, "%s( %s )", ss->name, 
			( ss->s ) ? "ONLINE" : "OFFLINE" );
	}
	irc_prefmsg( ss_bot,cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_server_del
 *
 *  SERVER DEL command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ss_server_del( const CmdParams *cmdparams )
{
	serverstat *ss;
	hnode_t *node;

	if( cmdparams->ac < 2 )
		return NS_ERR_NEED_MORE_PARAMS;
	ss = findserverstats( cmdparams->av[1] );
	if( !ss )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "%s is not in the database", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	if( ss->s )
	{
		irc_prefmsg( ss_bot, cmdparams->source, 
			"Cannot remove %s from the database, it is online!!", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	node = hash_lookup( serverstathash, cmdparams->av[1] );
	if( node )
	{
		ss =( serverstat * )hnode_get( node );
		hash_delete( serverstathash, node );
		hnode_destroy( node );
		ns_free( ss );
		irc_prefmsg( ss_bot, cmdparams->source, "Removed %s from the database.",
			cmdparams->av[1] );
		nlog( LOG_NOTICE, "%s deleted stats for %s", cmdparams->source->name, cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief ss_server_copy
 *
 *  SERVER COPY command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ss_server_copy( const CmdParams *cmdparams )
{
	serverstat *dest;
	serverstat *src;

	if( UserLevel( cmdparams->source ) < NS_ULEVEL_ADMIN )
		return NS_ERR_NO_PERMISSION;
	if( cmdparams->ac < 3 )
		return NS_ERR_NEED_MORE_PARAMS;
	dest = findserverstats( cmdparams->av[2] );
	if( dest )
	{
		if( dest->s )
		{
			irc_prefmsg( ss_bot, cmdparams->source, "Server %s is online!", cmdparams->av[2] );
			return NS_SUCCESS;
		}
		ns_free( dest );
	}
	src = findserverstats( cmdparams->av[1] );
	if( !src )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "%s is not in the database", 
			cmdparams->av[1] );
		return NS_SUCCESS;
	}
	memcpy( dest, src, sizeof( serverstat ) );
	strlcpy( dest->name, cmdparams->av[2], MAXHOST );
	irc_prefmsg( ss_bot, cmdparams->source, "Copied database entry for %s to %s", 
		cmdparams->av[1], cmdparams->av[2] );
	nlog( LOG_NOTICE, "%s requested STATS COPY %s to %s", cmdparams->source->name, 
		cmdparams->av[1], cmdparams->av[2] );
	return NS_SUCCESS;
}

/** @brief ss_cmd_server_stats
 *
 *  SERVER command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ss_cmd_server_stats( CmdParams *cmdparams )
{
	serverstat *ss;
	Client *s;
	char *server;

	if( cmdparams->ac == 0 )
		return NS_ERR_SYNTAX_ERROR;
	server = cmdparams->av[0];
	ss = findserverstats( server );
	if( !ss )
	{
		nlog( LOG_CRITICAL, "Unable to find server statistics for %s", server );
		irc_prefmsg( ss_bot, cmdparams->source, "Unable to find server statistics for %s", server );
		return NS_SUCCESS;
	}
	irc_prefmsg( ss_bot, cmdparams->source, "Statistics for \2%s\2 since %s",
		ss->name, sftime( ss->ts_start ) );
	s = ss->s;
	if( !s )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "Server Last Seen: %s", 
			sftime( ss->ts_lastseen ) );
	}
	else
	{
		/* Calculate uptime as uptime from server plus uptime of NeoStats */
		time_t uptime;

		uptime = s->server->uptime +( me.now - me.ts_boot );
		irc_prefmsg( ss_bot, cmdparams->source, "Version: %s", s->version );
		irc_prefmsg( ss_bot, cmdparams->source, "Uptime:  %ld day%s, %02ld:%02ld:%02ld",( uptime / TS_ONE_DAY ),( ( uptime / TS_ONE_DAY ) == 1 ) ? "" : "s",( ( uptime / TS_ONE_HOUR ) % 24 ),( ( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE ),( uptime % 60 ) );
		irc_prefmsg( ss_bot, cmdparams->source, "Current Users: %-3d( %d%% )", 
			s->server->users, 
			( int )( ( float ) s->server->users /( float ) networkstats.users.current * 100 ) );
	}
	irc_prefmsg( ss_bot, cmdparams->source, "Maximum users: %-3d at %s",
		ss->users.alltime.max, sftime( ss->users.alltime.ts_max ) );
	irc_prefmsg( ss_bot, cmdparams->source, "Total users connected: %-3d", ss->users.alltime.runningtotal );
	if( s )
		irc_prefmsg( ss_bot, cmdparams->source, "Current opers: %-3d", ss->opers.current );
	irc_prefmsg( ss_bot, cmdparams->source, "Maximum opers: %-3d at %s",
		ss->opers.alltime.max, sftime( ss->opers.alltime.ts_max ) );
	irc_prefmsg( ss_bot, cmdparams->source, "IRCop kills: %d", ss->operkills.alltime.runningtotal );
	irc_prefmsg( ss_bot, cmdparams->source, "Server kills: %d", ss->serverkills.alltime.runningtotal );
	irc_prefmsg( ss_bot, cmdparams->source, "Lowest ping: %-3d at %s",
		( int )ss->lowest_ping, sftime( ss->ts_lowest_ping ) );
	irc_prefmsg( ss_bot, cmdparams->source, "Higest ping: %-3d at %s",
		( int )ss->highest_ping, sftime( ss->ts_highest_ping ) );
	if( s )
		irc_prefmsg( ss_bot, cmdparams->source, "Current Ping: %-3d",( int )s->server->ping );
	if( ss->splits.alltime.runningtotal >= 1 )
	{
		irc_prefmsg( ss_bot, cmdparams->source, 
			"%s has split from the network %d time %s",
			ss->name, ss->splits.alltime.runningtotal,( ss->splits.alltime.runningtotal == 1 ) ? "" : "s" );
	}
	else
	{
		irc_prefmsg( ss_bot, cmdparams->source, "%s has never split from the network.", 
			ss->name );
	}
	irc_prefmsg( ss_bot, cmdparams->source, "***** End of Statistics *****" );
	return NS_SUCCESS;
}

/** @brief ss_cmd_server
 *
 *  SERVER command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *    cmdparams->av[0] = optionally LIST, DEL, COPY
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_server( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( !ircstrcasecmp( cmdparams->av[0], "LIST" ) )
		return ss_cmd_server_list( cmdparams );
	if( !ircstrcasecmp( cmdparams->av[0], "DEL" ) )
		return ss_server_del( cmdparams );
	if( !ircstrcasecmp( cmdparams->av[0], "COPY" ) )
		return ss_server_copy( cmdparams );	
	return ss_cmd_server_stats( cmdparams );
}

/** @brief SaveServer
 *
 *  Save server stat
 *
 *  @param ss pointer to server stat to save
 *
 *  @return none
 */

static void SaveServer( serverstat *ss )
{
	dlog( DEBUG1, "Writing statistics to database for %s", ss->name );
	PreSaveStatistic( &ss->users );
	PreSaveStatistic( &ss->opers );
	PreSaveStatistic( &ss->operkills );
	PreSaveStatistic( &ss->serverkills );
	PreSaveStatistic( &ss->splits );
	DBAStore( SERVER_TABLE, ss->name,( void * )ss, sizeof( serverstat ) );
}

/** @brief SaveServerStats
 *
 *  Save server stats
 *
 *  @param none
 *
 *  @return none
 */

void SaveServerStats( void )
{
	serverstat *ss;
	hnode_t *sn;
	hscan_t hs;

	/* run through stats and save them */
	hash_scan_begin( &hs, serverstathash );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		ss = hnode_get( sn );
		SaveServer( ss );
	}
}

/** @brief LoadVersionStats
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return none
 */

int LoadServerStats( void *data, int size ) 
{
	serverstat *ss;

	if( size != sizeof( serverstat ) )
	{
		nlog( LOG_CRITICAL, "server data size invalid" );		
		return NS_FALSE;
	}
	ss = ns_calloc( sizeof( serverstat ) );
	os_memcpy( ss, data, sizeof( serverstat ) );
	dlog( DEBUG1, "Loaded statistics for %ss", ss->name );
	hnode_create_insert( serverstathash, ss, ss->name );
	PostLoadStatistic( &ss->users );
	PostLoadStatistic( &ss->opers );
	PostLoadStatistic( &ss->operkills );
	PostLoadStatistic( &ss->serverkills );
	PostLoadStatistic( &ss->splits );
	return NS_FALSE;
}

/** @brief GetServerStats
 *
 *  Walk through list passing each server to handler
 *
 *  @param handler pointer to handler function
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void GetServerStats( const ServerStatHandler handler, const void *v )
{
	serverstat *ss;
	hnode_t *sn;
	hscan_t hs;

	hash_scan_begin( &hs, serverstathash );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		ss = hnode_get( sn );
		handler( ss, v );
	}
}

/** @brief InitServerStats
 *
 *  Init server stats
 *
 *  @param none
 *
 *  @return NS_SUCCESS on success, NS_FAILURE on failure
 */

int InitServerStats( void )
{
	serverstathash = hash_create( -1, 0, 0 );
	if( !serverstathash )
	{
		nlog( LOG_CRITICAL, "Unable to create server hash list" );
		return NS_FAILURE;
	}
	DBAFetchRows( SERVER_TABLE, LoadServerStats );
	GetServerList( AddServerStat, NULL );
	return NS_SUCCESS;
}

/** @brief FiniServerStats
 *
 *  Fini server stats
 *
 *  @param none
 *
 *  @return none
 */

void FiniServerStats( void )
{
	serverstat *ss;
	hnode_t *sn;
	hscan_t hs;

	SaveServerStats();
	hash_scan_begin( &hs, serverstathash );
	while( ( sn = hash_scan_next( &hs ) ) )
	{
		ss = hnode_get( sn );
		ClearServerModValue( ss->s );
		hash_scan_delete( serverstathash, sn );
		hnode_destroy( sn );
		ns_free( ss );
	}
	hash_destroy( serverstathash );
}
