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

/*	
 *  DBMs issue internal mallocs so these cannot be freed with ns_free
 *  This module handles the copy of data from the db to the variable
 */

#include "neostats.h"
#include "dl.h"

typedef struct dbentry {
	char name[MAX_MOD_NAME];
	hash_t *tablehash;
} dbentry;

typedef struct tableentry {
	char name[MAXPATH];
	char table[MAXPATH];
	void *handle;
} tableentry;

typedef struct dbm_sym {
	void **ptr;
	char *sym;
} dbm_sym;

static void *( *DBMOpenTable )( const char *name );
static int ( *DBMCloseTable )( void *handle );
static int ( *DBMGetData )( void *handle, char *key, void *data, int size );
static int ( *DBMSetData )( void *handle, char *key, void *data, int size );
static int ( *DBMGetTableRows )( void *handle, DBRowHandler handler );
static int ( *DBMDelData )( void *handle, char *key );

static dbm_sym dbm_sym_table[] = 
{
	{ ( void * )&DBMOpenTable,		"DBMOpenTable" },
	{ ( void * )&DBMCloseTable,		"DBMCloseTable" },
	{ ( void * )&DBMGetData,		"DBMGetData" },
	{ ( void * )&DBMSetData,		"DBMSetData" },
	{ ( void * )&DBMGetTableRows,	"DBMGetTableRows" },
	{ ( void * )&DBMDelData,		"DBMDelData" },
	{NULL, NULL},
};

static hash_t *dbhash;
static char dbname[MAXPATH];
void *dbm_module_handle;

/** @brief InitDBAMSymbols
 *
 *  Lookup DBM symbols for DBA layer
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitDBAMSymbols( void )
{
	static char dbm_path[MAXPATH];
	dbm_sym *pdbm_sym;

	ircsnprintf( dbm_path, 255, "%s/%s%s", MOD_PATH, me.dbm, MOD_STDEXT );
	nlog( LOG_NORMAL, "Using dbm module %s", dbm_path );
	dbm_module_handle = ns_dlopen( dbm_path, RTLD_NOW || RTLD_GLOBAL );
	if( !dbm_module_handle ) {
		dlog( DEBUG1, "DBA init %s failed!", dbm_path );
		nlog( LOG_CRITICAL, "Unable to load dbm module %s", dbm_path );
		return NS_FAILURE;	
	}
	pdbm_sym = dbm_sym_table;
	while( pdbm_sym->ptr )
	{
		*pdbm_sym->ptr = ns_dlsym( dbm_module_handle, pdbm_sym->sym );
		pdbm_sym ++;
	}
	return NS_SUCCESS;
}

/** @brief InitDBA
 *
 *  Init DBA layer
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitDBA( void )
{
	if( InitDBAMSymbols() != NS_SUCCESS ) {
		return NS_FAILURE;
	}	
	dbhash = hash_create( -1, 0, 0 );
	if( !dbhash ) {
		nlog( LOG_CRITICAL, "Unable to create db hash" );
		return NS_FAILURE;
	}
	DBAOpenDatabase();
	return NS_SUCCESS;
}

/** @brief FiniDBA
 *
 *  Finish DBA layer
 *
 *  @param none
 *
 *  @return none
 */

void FiniDBA( void )
{
	tableentry *tbe;
	dbentry *dbe;
	hnode_t *node;
	hnode_t *tnode;
	hscan_t ds;
	hscan_t ts;

	hash_scan_begin( &ds, dbhash );
	while(( node = hash_scan_next( &ds ) ) != NULL  ) {
		dbe = (dbentry *) hnode_get( node );
		dlog(DEBUG1, "Closing Database %s", dbe->name);
		hash_scan_begin( &ts, dbe->tablehash );
		while(( tnode = hash_scan_next( &ts ) ) != NULL  ) {
			tbe = (tableentry *) hnode_get( tnode );
			dlog(DEBUG1, "Closing Table %s", tbe->name);
			DBACloseTable( tbe->table );
			hash_scan_delete( dbe->tablehash, tnode );
			hnode_destroy( tnode );
			ns_free( tbe );
		}
		hash_destroy( dbe->tablehash );
		hash_scan_delete( dbhash, node );
		hnode_destroy( node );
		ns_free( dbe );
	}
	hash_destroy( dbhash );
	ns_dlclose(dbm_module_handle);

}

/** @brief DBAOpenDatabase
 *
 *  Open NeoStats database
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAOpenDatabase( void )
{
	dbentry *dbe;

	dlog( DEBUG1, "DBAOpenDatabase %s", GET_CUR_MODNAME() );
	dbe = ns_calloc( sizeof( dbentry ) );
	strlcpy(dbe->name, GET_CUR_MODNAME(), MAX_MOD_NAME);
	dbe->tablehash = hash_create( -1, 0, 0 );
	hnode_create_insert( dbhash, dbe, dbe->name);
	return NS_SUCCESS;
}

/** @brief DBACloseDatabase
 *
 *  Close NeoStats database
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBACloseDatabase( void )
{
	tableentry *tbe;
	dbentry *dbe;
	hnode_t *node;
	hnode_t *tnode;
	hscan_t ts;

	dlog( DEBUG1, "DBACloseDatabase %s", GET_CUR_MODNAME() );
	node = hash_lookup( dbhash, GET_CUR_MODNAME() );
	if (node) {
		dbe = ( dbentry* )hnode_get( node );
		dlog(DEBUG1, "Closing Database %s", dbe->name);
		hash_scan_begin( &ts, dbe->tablehash );
		while(( tnode = hash_scan_next( &ts ) ) != NULL  ) {
			tbe = (tableentry *) hnode_get( tnode );
			dlog(DEBUG1, "Closing Table %s", tbe->name);
			DBMCloseTable( tbe->handle );
			hash_delete( dbe->tablehash, tnode );
			hnode_destroy( tnode );
			ns_free( tbe );
		}
		hash_destroy( dbe->tablehash );
		hash_delete( dbhash, node );
		hnode_destroy( node );
		ns_free( dbe );
	}
	return NS_SUCCESS;
}

/** @brief DBAOpenTable
 *
 *  Open table in NeoStats database
 *
 *  @param table name
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAOpenTable( char *table )
{
	dbentry *dbe;
	tableentry *tbe;

	dlog( DEBUG1, "DBAOpenTable %s", table );
	dbe = (dbentry *)hnode_find( dbhash, GET_CUR_MODNAME() );
	if( !dbe ) {
		return NS_FAILURE;
	}
	tbe = ns_calloc( sizeof( tableentry ) );
	strlcpy( tbe->table, table, MAX_MOD_NAME );
	ircsnprintf( tbe->name, MAXPATH, "data/%s%s", GET_CUR_MODNAME(), table ? table : "" );
	tbe->handle = DBMOpenTable( tbe->name );
	if( !tbe->handle ) {
		ns_free( tbe );
		FATAL_ERROR( "DBAOpenTable failed. Check log file for details" );
		return NS_FAILURE;
	}
	hnode_create_insert( dbe->tablehash, tbe, tbe->name );
	return NS_SUCCESS;
}

/** @brief DBAFetchTableEntry
 *
 *  Get table entry info
 *
 *  @param table name
 *
 *  @return table entry or NULL for none
 */

static tableentry *DBAFetchTableEntry( char *table )
{
	dbentry *dbe;
	tableentry *tbe;

	dbe = (dbentry *)hnode_find( dbhash, GET_CUR_MODNAME() );
	ircsnprintf( dbname, MAXPATH, "data/%s%s", GET_CUR_MODNAME(), table ? table : "" );
	tbe = (tableentry *)hnode_find( dbe->tablehash, dbname );
	if( !tbe ) {
		DBAOpenTable( table );
		tbe = (tableentry *)hnode_find( dbe->tablehash, dbname );
	}
	return tbe;
}

/** @brief DBACloseTable
 *
 *  Close table in NeoStats database
 *
 *  @param table name
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBACloseTable( char *table )
{
	dbentry *dbe;
	tableentry *tbe;
	hnode_t *node;

	dlog( DEBUG1, "DBACloseTable %s", table );
	dbe = (dbentry *)hnode_find( dbhash, GET_CUR_MODNAME() );
	if( !dbe ) {
		return NS_FAILURE;
	}
	ircsnprintf( dbname, MAXPATH, "data/%s%s", GET_CUR_MODNAME(), table ? table : "" );
	node = hash_lookup( dbhash, dbname );
	if( node ) {
		tbe = (tableentry *)hnode_get( node );
		DBMCloseTable( tbe->handle );
		hash_delete( dbhash, node );
		hnode_destroy( node );
		ns_free( tbe );
	}
	return NS_SUCCESS;
}

/** @brief DBAFetch
 *
 *  Fetch data from table record
 *
 *  @param table name
 *  @param record key
 *  @param pointer to data to fetch data into
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAFetch( char *table, char *key, void *data, int size )
{
	tableentry *tbe;

	dlog( DEBUG1, "DBAFetch %s %s", table, key );
	tbe = DBAFetchTableEntry( table );
	if( tbe ) {
		return DBMGetData( tbe->handle, key, data, size );
	}
	return NS_FAILURE;
}

/** @brief DBAStore
 *
 *  Store data in table record
 *
 *  @param table name
 *  @param record key
 *  @param pointer to data to store
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAStore( char *table, char *key, void *data, int size )
{
	tableentry *tbe;

	dlog( DEBUG1, "DBAStore %s %s", table, key );
	tbe = DBAFetchTableEntry( table );
	if( !tbe ) {
		return NS_FAILURE;
	}
	DBMSetData( tbe->handle, key, data, size );
	return NS_SUCCESS;
}

/** @brief DBAFetchRows
 *
 *  Walk through database and pass records in turn to handler
 *
 *  @param table name
 *  @param handler for records
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAFetchRows( char *table, DBRowHandler handler )
{
	tableentry *tbe;

	dlog( DEBUG1, "DBAFetchRows %s", table );
	tbe = DBAFetchTableEntry( table );
	if( !tbe ) {
		return 0;
	}
	return DBMGetTableRows( tbe->handle, handler );	
}

/** @brief DBADelete
 *
 *  delete table row
 *
 *  @param table name
 *  @param record key
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBADelete( char *table, char *key )
{
	tableentry *tbe;

	dlog( DEBUG1, "DBADelete %s %s", table, key );
	tbe = DBAFetchTableEntry( table );
	if( !tbe ) {
		return NS_FAILURE;
	}
	DBMDelData( tbe->handle, key );
	return NS_SUCCESS;
}
