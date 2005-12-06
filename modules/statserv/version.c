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
#include "version.h"

/** CTCP version table name */
#define CTCPVERSION_TABLE "CTCPVERSION"

/** Client version list */
static list_t *ctcp_version_list;

/** @brief topcurrentversions
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

static int topcurrentversions( const void *key1, const void *key2 )
{
	const ss_ctcp_version *ver1 = key1;
	const ss_ctcp_version *ver2 = key2;
	return( ver2->users.current - ver1->users.current );
}

/** @brief findctcpversion
 *
 *  Check list for ctcp version
 *
 *  @param name ctcp version to search for
 *
 *  @return pointer to stat found or NULL if none
 */

static ss_ctcp_version *findctcpversion( const char *name )
{
	ss_ctcp_version *cv;

	cv = lnode_find( ctcp_version_list, name, comparef );
	if( cv )
		dlog( DEBUG2, "findctcpversion: found version: %s", name );
	else
		dlog( DEBUG2, "findctcpversion: %s not found", name );	
	return cv;
}

/** @brief SaveClientVersion
 *
 *  Save client version stat
 *
 *  @param none
 *
 *  @return none
 */
static void SaveClientVersion( const ss_ctcp_version *cv, const void *v )
{
	DBAStore( CTCPVERSION_TABLE, cv->name, ( void *)cv, sizeof( ss_ctcp_version));
	dlog( DEBUG2, "Save version %s", cv->name );
}

/** @brief SaveClientVersions
 *
 *  Save client version stats
 *
 *  @param none
 *
 *  @return none
 */

static void SaveClientVersions( void )
{
	GetClientStats( SaveClientVersion, -1, NULL );
}

/** @brief new_ctcpversion
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int new_ctcpversion( void *data, int size )
{
	ss_ctcp_version *cv;
	
	if( size != sizeof( ss_ctcp_version ) )
	{
		nlog( LOG_CRITICAL, "CTCP version data size invalid" );		
		return NS_FALSE;
	}
	cv = ns_calloc( sizeof( ss_ctcp_version ) );
	os_memcpy( cv, data, sizeof( ss_ctcp_version ));
	lnode_create_append( ctcp_version_list, cv );
	return NS_FALSE;
}

/** @brief LoadVersionStats
 *
 *  Load version stats
 *
 *  @param none
 *
 *  @return none
 */

static void LoadVersionStats( void )
{
	DBAFetchRows( CTCPVERSION_TABLE, new_ctcpversion );
}

/** @brief ClientVersionReport
 *
 *  Report client version
 *
 *  @param cv pointer to version to report
 *  @param v pointer to client to send to
 *
 *  @return none
 */

static void ClientVersionReport( const ss_ctcp_version *cv, const void *v )
{
	irc_prefmsg( ss_bot, ( Client * ) v, "%d ->  %s", cv->users.current, cv->name );
}

/** @brief GetClientStats
 *
 *  Walk through list passing each client version to handler
 *
 *  @param handler pointer to handler function
 *  @param limit max entries to handle
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void GetClientStats( const CTCPVersionHandler handler, int limit, const void *v )
{
	ss_ctcp_version *cv;
	lnode_t *cn;
	int count = 0;
	
	if( !list_is_sorted( ctcp_version_list, topcurrentversions ) )
		list_sort( ctcp_version_list, topcurrentversions );
	cn = list_first( ctcp_version_list );
	while( cn != NULL )
	{
		cv = ( ss_ctcp_version * ) lnode_get( cn );
		handler( cv, v );	
		cn = list_next( ctcp_version_list, cn );
		count++;
		if( limit != -1 && count >= limit )
			break;
	}

}

/** @brief ss_cmd_ctcpversion
 *
 *  CTCPVERSION command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *    cmdparams->av[0] = optional limit
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_ctcpversion( const CmdParams *cmdparams )
{
	int num;

	num = cmdparams->ac > 0 ? atoi( cmdparams->av[0] ) : 10;
	if( num < 10 )
		num = 10;
	if( list_count( ctcp_version_list ) == 0 )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "No Stats Available." );
		return NS_SUCCESS;
	}
	irc_prefmsg( ss_bot, cmdparams->source, "Top %d Client Versions:", num );
	irc_prefmsg( ss_bot, cmdparams->source, "======================" );
	GetClientStats( ClientVersionReport, num, ( void * )cmdparams->source );
	irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_event_ctcpversionbc
 *
 *  CTCP VERSION event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_ctcpversionbc( const CmdParams *cmdparams )
{
	static char nocols[BUFSIZE];
	ss_ctcp_version *cv;

    strlcpy( nocols, cmdparams->param, BUFSIZE );
	strip_mirc_codes( nocols );
	cv = findctcpversion( nocols );
	if( !cv )
	{
		cv = ns_calloc( sizeof( ss_ctcp_version ) );
		strlcpy( cv->name, nocols, BUFSIZE );
		lnode_create_append( ctcp_version_list, cv );
		dlog( DEBUG2, "Added version: %s", cv->name );
	}
	IncStatistic( &cv->users );
	return NS_SUCCESS;
}

/** @brief InitVersionStats
 *
 *  Init version stats
 *
 *  @param none
 *
 *  @return NS_SUCCESS on success, NS_FAILURE on failure
 */

int InitVersionStats( void )
{
	ctcp_version_list = list_create( LISTCOUNT_T_MAX );
	if( !ctcp_version_list )
	{
		nlog( LOG_CRITICAL, "Unable to create version stat list" );
		return NS_FAILURE;
	}
	DBAOpenTable( CTCPVERSION_TABLE );
	LoadVersionStats();
	return NS_SUCCESS;
}

/** @brief FiniVersionStats
 *
 *  Fini version stats
 *
 *  @param none
 *
 *  @return none
 */

void FiniVersionStats( void )
{
	SaveClientVersions();
	DBACloseTable( CTCPVERSION_TABLE );
	list_destroy_auto( ctcp_version_list );
}
