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
#include "gdbmdefs.h"
#include "gdbmerrno.h"
#include "nsdbm.h"

extern const char *gdbm_strerror __P( ( gdbm_error ) );

/** @brief DBMOpenTable
 *
 *  Open gdbm table
 *
 *  @param table name
 *
 *  @return handle to table or NULL on error
 */

void *DBMOpenTable( const char *name )
{
	static char filename[MAXPATH];
	gdbm_file_info *gdbm_file;
	int cache_size = DEFAULT_CACHESIZE;

	dlog( DEBUG4, "DBMOpenTable %s", name );
	ircsprintf( filename, "%s.gdbm", name );
	gdbm_file = gdbm_open( filename, 0, GDBM_WRCREAT | GDBM_NOLOCK, 00664, NULL );
	if( gdbm_file == NULL )
	{
		nlog( LOG_ERROR, "gdbm_open fail: %s", gdbm_strerror( gdbm_errno ) );
		return NULL;
	}
	if( gdbm_setopt( gdbm_file, GDBM_CACHESIZE, &cache_size, sizeof( int ) ) == -1 )
	{
		nlog( LOG_ERROR, "gdbm_setopt fail: %s", gdbm_strerror( gdbm_errno ) );
		return NULL;
	}
	return ( void * )gdbm_file;
}

/** @brief DBMCloseTable
 *
 *  Close gdbm table
 *
 *  @param handle of table to close
 *
 *  @return none
 */

void DBMCloseTable( void *handle )
{
	if( handle ) 
	{
		gdbm_close( ( gdbm_file_info * )handle ); 
	}
}

/** @brief DBMFetch
 *
 *  Fetch data from table record
 *
 *  @param handle of table
 *  @param record key
 *  @param pointer to data to fetch data into
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBMFetch( void *handle, char *key, void *data, int size )
{
	datum dbkey;
	datum dbdata;

	dbkey.dptr = key;
	dbkey.dsize = strlen( key ) + 1;
	dbdata = gdbm_fetch( ( gdbm_file_info * )handle, dbkey );
	if( dbdata.dptr != NULL )
	{
		if( dbdata.dsize != size )
		{
			dlog( DEBUG1, "DBMFetch: gdbm_fetch fail: %s data size mismatch", key );
			free( dbdata.dptr );
			return NS_FAILURE;
		}
		os_memcpy( data, dbdata.dptr, size );
		free( dbdata.dptr );
		return NS_SUCCESS;
	}
	dlog( DEBUG1, "DBMFetch: gdbm_fetch fail: %s %s", key, gdbm_strerror( gdbm_errno ) );
	return NS_FAILURE;
}

/** @brief DBMStore
 *
 *  Store data in table record
 *
 *  @param handle of table
 *  @param record key
 *  @param pointer to data to fetch data into
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBMStore( void *handle, char *key, void *data, int size )
{
	datum dbkey;
	datum dbdata;

	dbkey.dptr = key;
	dbkey.dsize = strlen( key ) + 1;
	dbdata.dptr = data;
	dbdata.dsize = size;
	if( gdbm_store( ( gdbm_file_info * )handle, dbkey, dbdata, GDBM_REPLACE ) != 0 )
	{
		dlog( DEBUG1, "DBMStore: gdbm_store fail: %s %s", key, gdbm_strerror( gdbm_errno ) );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief DBMFetchRows
 *
 *  Walk through table and pass records in turn to handler
 *
 *  @param table handle
 *  @param handler for records
 *
 *  @return number of rows processed by handler
 */

int DBMFetchRows( void *handle, DBRowHandler handler )
{
	datum dbkey;
	datum dbdata;
	int rowcount = 0;

	dbkey = gdbm_firstkey( ( gdbm_file_info * )handle );
	while( dbkey.dptr != NULL )
	{
		rowcount++;
		dlog( DEBUG4, "DBMFetchRows: key %s", dbkey.dptr );
		dbdata = gdbm_fetch( ( gdbm_file_info * )handle, dbkey );
		/* Allow handler to exit the fetch loop */
		if( handler( dbdata.dptr, dbdata.dsize ) != 0 )
		{
			free( dbdata.dptr );
			free( dbkey.dptr );
			break;
		}
		free( dbdata.dptr );
		dbdata = gdbm_nextkey( ( gdbm_file_info * )handle, dbkey );
		free( dbkey.dptr );
		dbkey = dbdata;
	}
	return rowcount;
}

/** @brief DBMDelete
 *
 *  delete table row
 *
 *  @param table handle
 *  @param record key
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBMDelete( void *handle, char *key )
{
	datum dbkey;

	dbkey.dptr = key;
	dbkey.dsize = strlen( key ) + 1;
	if( gdbm_delete( ( gdbm_file_info * )handle, dbkey ) != 0 )
	{
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}
