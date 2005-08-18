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

#define CTCPVERSION_TABLE "CTCPVERSION"

list_t *versionstatlist;

/** @brief topcurrentversions
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

int topcurrentversions( const void *key1, const void *key2 )
{
	const ctcpversionstat *ver1 = key1;
	const ctcpversionstat *ver2 = key2;
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

static ctcpversionstat *findctcpversion( const char *name )
{
	ctcpversionstat *cv;

	cv = lnode_find( versionstatlist, name, comparef );
	if( cv )
		dlog( DEBUG2, "findctcpversion: found version: %s", name );
	else
		dlog( DEBUG2, "findctcpversion: %s not found", name );	
	return cv;
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
	ctcpversionstat *cv;
	lnode_t *cn;
	
	cn = list_first( versionstatlist );
	while( cn != NULL )
	{
		cv = ( ctcpversionstat * ) lnode_get( cn );
		DBAStore( CTCPVERSION_TABLE, cv->name,( void *)cv, sizeof( ctcpversionstat));
		dlog( DEBUG2, "Save version %s", cv->name );
		cn = list_next( versionstatlist, cn );
	}
}

/** @brief new_ctcpversion
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return
 */

static int new_ctcpversion( void *data, int size )
{
	ctcpversionstat *clientv;
	
	clientv = ns_calloc( sizeof( ctcpversionstat ) );
	os_memcpy( clientv, data, sizeof( ctcpversionstat ));
	lnode_create_append( versionstatlist, clientv );
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

/** @brief ss_cmd_userversion
 *
 *  Command handler for USERVERSION
 *
 *  @param cmdparams
 *    cmdparams->av[0] = optional limit
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_userversion( const CmdParams *cmdparams )
{
	ctcpversionstat *cv;
	lnode_t *cn;
	int i;
	int num;

	num = cmdparams->ac > 0 ? atoi( cmdparams->av[0] ) : 10;
	if( num < 10 )
		num = 10;
	if( list_count( versionstatlist ) == 0 )
	{
		irc_prefmsg( ss_bot, cmdparams->source, "No Stats Available." );
		return NS_SUCCESS;
	}
	if( !list_is_sorted( versionstatlist, topcurrentversions ) )
		list_sort( versionstatlist, topcurrentversions );
	irc_prefmsg( ss_bot, cmdparams->source, "Top %d Client Versions:", num );
	irc_prefmsg( ss_bot, cmdparams->source, "======================" );
	cn = list_first( versionstatlist );
	for( i = 0; i < num && cn; i++ )
	{
		cv = lnode_get( cn );
		irc_prefmsg( ss_bot, cmdparams->source, "%d ) %d ->  %s", i + 1, cv->users.current, cv->name );
		cn = list_next( versionstatlist, cn );
	}
	irc_prefmsg( ss_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_event_ctcpversion
 *
 *  Event handler for CTCP VERSION
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_ctcpversion( const CmdParams *cmdparams )
{
	static char nocols[BUFSIZE];
	ctcpversionstat *clientv;

    strlcpy( nocols, cmdparams->param, BUFSIZE );
	strip_mirc_codes( nocols );
	clientv = findctcpversion( nocols );
	if( clientv )
	{
		IncStatistic( &clientv->users );
		return NS_SUCCESS;
	}
	clientv = ns_calloc( sizeof( ctcpversionstat ) );
	strlcpy( clientv->name, nocols, BUFSIZE );
	IncStatistic( &clientv->users );
	lnode_create_append( versionstatlist, clientv );
	dlog( DEBUG2, "Added version: %s", clientv->name );
	return NS_SUCCESS;
}

/** @brief InitVersionStats
 *
 *  Init version stats
 *
 *  @param none
 *
 *  @return none
 */

void InitVersionStats( void )
{
	versionstatlist = list_create( -1 );
	LoadVersionStats();
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
	list_destroy_auto( versionstatlist );
}
