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

/*	
 *  DBMs issue internal mallocs do these cannot be freed with ns_free
 *  This module handles the copy of data from the db to the variable and frees/
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

#if 1
static void *(*DBMOpenTable) (const char *name);
static int (*DBMCloseTable) (void *handle);
static void *(*DBMGetData) (void *handle, char *key);
static int (*DBMSetData) (void *handle, char *key, void *data, int size);
static int (*DBMGetTableRows) (void *handle, DBRowHandler handler);
static int (*DBMDelData) (void *handle, char * key);
#else
void *DBMOpenTable (const char *name);
int DBMCloseTable (void *handle);
void *DBMGetData (void *handle, char *key);
int DBMSetData (void *handle, char *key, void *data, int size);
int DBMGetTableRows (void *handle, DBRowHandler handler);
int DBMDelData (void *handle, char * key);
#endif

static dbm_sym dbm_sym_table[] = 
{
	{(void *)&DBMOpenTable,		"DBMOpenTable"},
	{(void *)&DBMCloseTable,	"DBMCloseTable"},
	{(void *)&DBMGetData,		"DBMGetData"},
	{(void *)&DBMSetData,		"DBMSetData"},
	{(void *)&DBMGetTableRows,	"DBMGetTableRows"},
	{(void *)&DBMDelData,		"DBMDelData"},
	{NULL, NULL},
};

static hash_t *dbhash;
static char dbname[MAXPATH];

static int
InitDBAMSymbols (void)
{
	static char dbm_path[MAXPATH];
	void *dbm_module_handle;
	dbm_sym * pdbm_sym;

	ircsnprintf (dbm_path, 255, "%s/%s%s", MOD_PATH, me.dbm, MOD_EXT);
	dlog (DEBUG1, "DBA init %s", dbm_path);
	dbm_module_handle = ns_dlopen (dbm_path, RTLD_NOW || RTLD_GLOBAL);
	if (!dbm_module_handle) {
		dlog (DEBUG1, "DBA init %s failed!", dbm_path);
		nlog (LOG_CRITICAL, "Unable to load db module %s\n", dbm_path);
		return NS_FAILURE;	
	}
	pdbm_sym = dbm_sym_table;
	while (pdbm_sym->ptr)
	{
		*pdbm_sym->ptr = ns_dlsym (dbm_module_handle, pdbm_sym->sym);
		pdbm_sym ++;
	}
	return NS_SUCCESS;
}

int InitDBA (void)
{
	if (InitDBAMSymbols() != NS_SUCCESS) {
		return NS_FAILURE;
	}	
	dbhash = hash_create (-1, 0, 0);
	if (!dbhash) {
		nlog (LOG_CRITICAL, "Unable to create db hash");
		return NS_FAILURE;
	}

	DBAOpenDatabase ();
	return NS_SUCCESS;
}

void FiniDBA (void)
{
/*	dbentry *dbe;
	hnode_t *node;
	hscan_t hs;

	hash_scan_begin(&hs, dbhash);
	while ((node = hash_scan_next(&hs)) != NULL ) {
		dbe = (dbentry *) hnode_get (node);
		DBACloseTable (dbe->handle);
		hash_delete (dbhash, node);
		hnode_destroy (node);
		ns_free (dbe);
	}
	hash_destroy(dbhash);*/
}

int DBAOpenDatabase (void)
{
	dbentry *dbe;

	dlog (DEBUG1, "DBAOpenDatabase %s", GET_CUR_MODNAME());
	dbe = ns_calloc (sizeof (dbentry));
	dbe->tablehash = hash_create (-1, 0, 0);
	hnode_create_insert (dbhash, dbe, GET_CUR_MODNAME());
	return NS_SUCCESS;
}

int DBACloseDatabase (void)
{
	tableentry *tbe;
	dbentry *dbe;
	hnode_t *node;
	hnode_t *tnode;
	hscan_t ts;

	dlog (DEBUG1, "DBACloseDatabase %s", GET_CUR_MODNAME());
	node = hash_lookup (dbhash, GET_CUR_MODNAME());
	dbe = (dbentry*)hnode_get (node);
	hash_scan_begin(&ts, dbe->tablehash);
	while ((tnode = hash_scan_next(&ts)) != NULL ) {
		tbe = (tableentry *) hnode_get (tnode);
		DBACloseTable (tbe->table);
		hash_delete (dbe->tablehash, tnode);
		hnode_destroy (tnode);
		ns_free (tbe);
	}
	hash_destroy(dbe->tablehash);
	hash_delete (dbhash, node);
	hnode_destroy (node);
	ns_free (dbe);
	return NS_SUCCESS;
}


int DBAOpenTable (char *table)
{
	dbentry *dbe;
	tableentry *tbe;

	dlog (DEBUG1, "DBAOpenTable %s", table);
	dbe = (dbentry *)hnode_find (dbhash, GET_CUR_MODNAME());
	if (!dbe) {
		return NS_FAILURE;
	}
	tbe = ns_calloc (sizeof (tableentry));
	strlcpy (tbe->table, table, MAX_MOD_NAME);
	ircsnprintf (tbe->name, MAXPATH, "data/%s%s", GET_CUR_MODNAME(), table ? table : "");
	tbe->handle = DBMOpenTable (tbe->name);
	if (!tbe->handle) {
		ns_free (tbe);
		return NS_FAILURE;
	}
	hnode_create_insert (dbe->tablehash, tbe, tbe->name);
	return NS_SUCCESS;
}

static tableentry *DBAFetchTableEntry (char *table)
{
	dbentry *dbe;
	tableentry *tbe;

	dbe = (dbentry *)hnode_find (dbhash, GET_CUR_MODNAME());
	ircsnprintf (dbname, MAXPATH, "data/%s%s", GET_CUR_MODNAME(), table ? table : "");
	tbe = (tableentry *)hnode_find (dbe->tablehash, dbname);
	if (!tbe) {
		DBAOpenTable (table);
		tbe = (tableentry *)hnode_find (dbe->tablehash, dbname);
	}
	return tbe;
}

int DBACloseTable (char *table)
{
	dbentry *dbe;
	tableentry *tbe;
	hnode_t *node;

	dlog (DEBUG1, "DBACloseTable %s", table);
	dbe = (dbentry *)hnode_find (dbhash, GET_CUR_MODNAME());
	if (!dbe) {
		return NS_FAILURE;
	}
	ircsnprintf (dbname, MAXPATH, "data/%s%s", GET_CUR_MODNAME(), table ? table : "");
	node = hash_lookup (dbhash, dbname);
	if (node) {
		tbe = (tableentry *)hnode_get (node);
		DBMCloseTable (tbe->handle);
		hash_delete (dbhash, node);
		hnode_destroy (node);
		ns_free (tbe);
	}
	return NS_SUCCESS;
}

int DBAFetch (char *table, char *key, void *data, int size)
{
	tableentry *tbe;
	void *dptr = NULL;

	dlog (DEBUG1, "DBAFetch %s %s", table, key);
	tbe = DBAFetchTableEntry (table);
	if (tbe) {
		dptr = DBMGetData (tbe->handle, key);
		if (dptr)
		{
			os_memcpy (data, dptr, size);
			return NS_SUCCESS;
		}
	}
	return NS_FAILURE;
}

int DBAStore (char *table, char *key, void *data, int size)
{
	tableentry *tbe;

	dlog (DEBUG1, "DBAStore %s %s", table, key);
	tbe = DBAFetchTableEntry (table);
	if (!tbe) {
		return NS_FAILURE;
	}
	DBMSetData (tbe->handle, key, data, size);
	return NS_SUCCESS;
}

int DBAFetchRows (char *table, DBRowHandler handler)
{
	tableentry *tbe;

	dlog (DEBUG1, "DBAFetchRows %s", table);
	tbe = DBAFetchTableEntry (table);
	if (!tbe) {
		return 0;
	}
	return DBMGetTableRows (tbe->handle, handler);	
}

int DBADelete (char *table, char * key)
{
	tableentry *tbe;

	dlog (DEBUG1, "DBADelete %s %s", table, key);
	tbe = DBAFetchTableEntry (table);
	if (!tbe) {
		return NS_FAILURE;
	}
	DBMDelData (tbe->handle, key);
	return NS_SUCCESS;
}

#if 0
void DBTest (void)
{
	int i = 10;
	int b = 1;
	char *s = "test";
	char *s2;
	
	DBAOpenTable ("Test");
	DBAStoreStr ("Test", "string", s);
	DBAStoreBool ("Test", "boolean", b);
	DBAStoreInt ("Test", "integer", i);
	i = 0;
	b = 0;
	DBAFetchInt ("Test", "integer", &i);
	DBAFetchBool ("Test", "boolean", &b);
	DBAFetchStr ("Test", "string", &s2);
	free (s2);
	DBACloseTable ("Test");
}
#endif
