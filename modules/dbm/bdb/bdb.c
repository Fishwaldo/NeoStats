/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2007 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "nsdbm.h"

#ifdef DB_HEADER
#include DB_HEADER
#include <sys/types.h>
#include <dirent.h>
          
static DBT dbkey;
static DBT dbdata;

static DB_ENV *db_env;

static int dbopened = 0;

void db_log_cb(const char *prefix, char *msg) {
	dlog(DEBUG1, "%s", msg);
}
/* some older versions of BDB dun have this */
#ifndef DB_VERB_REGISTER
#define DB_VERB_REGISTER 0
#endif
#ifndef DB_REGISTER
#define DB_REGISTER 0
#endif

void *DBMOpenDB (const char *name)
{
	int dbret;
	
	if (!db_env) {
		if ((dbret = db_env_create(&db_env, 0)) != 0) {
			nlog(LOG_WARNING, "db_env_create failed: %s", db_strerror(dbret));
			return NULL;
		}
		db_env->set_verbose(db_env, DB_VERB_RECOVERY|DB_VERB_REGISTER, 1);
#ifdef DEBUG
		db_env->set_errfile(db_env, stderr);
		db_env->set_msgfile(db_env, stderr);		
#endif
		if ((dbret = db_env->open(db_env, "data/", DB_RECOVER|DB_REGISTER|DB_CREATE|DB_INIT_TXN|DB_INIT_MPOOL, 0600)) != 0) {
			nlog(LOG_WARNING, "db evn open failed: %s", db_strerror(dbret));
			db_env->close(db_env, 0);
			return NULL;
		}
#if 0
		db_env->set_errcall(db_env, db_log_cb);
		db_env->set_msgcall(db_env, db_log_cb);
		db_env->stat_print(db_env, DB_STAT_ALL|DB_STAT_SUBSYSTEM);
#endif
	}
	dbopened++;
	return strdup(name);
}

void DBMCloseDB (void *dbhandle)
{
	ns_free(dbhandle);
	dbopened--;
	if (dbopened <= 0) {
		db_env->close(db_env, 0);
		db_env = NULL;
	} else {
		dlog(DEBUG5, "DBMClose: Databases still opened, not destroying enviroment");
	}
	return;
}

void *DBMOpenTable (void *dbhandle, const char *name)
{
	static char filename[MAXPATH];
	int dbret;
	DB *dbp;

	dlog (DEBUG1, "DBMOpenTable %s", name);
	if (db_env == NULL) {
		nlog(DEBUG1, "DataBase Enviroment is not created\n");
		return NULL;
	}
	ircsprintf (filename, "%s.bdb", (char *)dbhandle);
	if ((dbret = db_create(&dbp, db_env, 0)) != 0) {
		dlog(DEBUG1, "db_create: %s", db_strerror(dbret));
		return NULL;
	}
#if (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
	if ((dbret = dbp->open(dbp, NULL, filename, name, DB_BTREE, DB_CREATE, 0600)) != 0) {
#else
	if ((dbret = dbp->open(dbp, filename, name, DB_BTREE, DB_CREATE, 0600)) != 0) {
#endif
		dlog(DEBUG1, "dbp->open: %s", db_strerror(dbret));
		return NULL;
	}
	return (void *)dbp;
}

void DBMCloseTable (void *dbhandle, void *tbhandle)
{
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG1, "DBACloseTable");
	dbp->close(dbp, 0); 
}

int DBMFetch (void *dbhandle, void *tbhandle, char *key, void *data, int size)
{
	int dbret;
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG1, "DBAFetch %s", key);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = dbp->get(dbp, NULL, &dbkey, &dbdata, 0)) == 0)
	{
		os_memcpy (data, dbdata.data, size);
		return NS_SUCCESS;
	}
	dlog(DEBUG1, "dbp->get fail: %s", db_strerror(dbret));
	return NS_FAILURE;
}

int DBMStore (void *dbhandle, void *tbhandle, char *key, void *data, int size)
{
	int dbret;
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG1, "DBAStore %s %s", key, (char *)data);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	dbdata.data = data;
	dbdata.size = size;
	if ((dbret = dbp->put(dbp, NULL, &dbkey, &dbdata, 0)) != 0) {
		if (dbret != DB_NOTFOUND) {
			dlog(DEBUG1, "dbp->put: %s", db_strerror(dbret));
			return NS_FAILURE;
		} else {
			return NS_SUCCESS;
		}
	}
	return NS_SUCCESS;
}

int DBMFetchRows (void *dbhandle, void *tbhandle, DBRowHandler handler)
{
	int rowcount = 0;
	int dbret;
	DB *dbp = (DB *)tbhandle;
	DBC *dbcp;

	dlog(DEBUG1, "DBMFetchRows here");
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* initilize the cursors */
	if ((dbret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		nlog(LOG_WARNING, "DB Cursor failed: %s", db_strerror(dbret));
		return rowcount;
	}
	
	while ((dbret = dbcp->c_get(dbcp, &dbkey, &dbdata, DB_NEXT)) == 0)
	{
		rowcount++;
                if( handler( dbdata.data, dbdata.size ) != 0 ) {
                        break;
		}
	
	} 
	if (dbret != 0 && dbret != DB_NOTFOUND) {
		dlog(DEBUG1, "dbp->c_get failed: %s", db_strerror(dbret));
	}
	if ((dbret = dbcp->c_close(dbcp)) != 0) {
		dlog(DEBUG1, "dbcpp->close failed: %s", db_strerror(dbret));
	}	
	return rowcount;
}

int DBMFetchRows2 (void *dbhandle, void *tbhandle, DBRowHandler2 handler)
{
	int rowcount = 0;
	int dbret;
	DB *dbp = (DB *)tbhandle;
	DBC *dbcp;

	dlog(DEBUG1, "DBMFetchRows2 here");
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* initilize the cursors */
	if ((dbret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		nlog(LOG_WARNING, "DB Cursor failed: %s", db_strerror(dbret));
		return rowcount;
	}
	
	while ((dbret = dbcp->c_get(dbcp, &dbkey, &dbdata, DB_NEXT)) == 0)
	{

		rowcount++;
                if( handler( dbkey.data, dbdata.data, dbdata.size ) != 0 ) {
                        break;
		}
	} 
	if (dbret != 0 && dbret != DB_NOTFOUND) {
		dlog(DEBUG1, "dbp->c_get failed: %s", db_strerror(dbret));
	}
	if ((dbret = dbcp->c_close(dbcp)) != 0) {
		dlog(DEBUG1, "dbcpp->close failed: %s", db_strerror(dbret));
	}	
	return rowcount;
}

int DBMDelete (void *dbhandle, void *tbhandle, char * key)
{
	int dbret;
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG1, "DBMDelete %s", key);
	memset(&dbkey, 0, sizeof(dbkey));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = dbp->del(dbp, NULL, &dbkey, 0)) != 0)
	{
		if (dbret != DB_NOTFOUND) {
			nlog(LOG_WARNING, "dbp->del failed: %s", db_strerror(dbret));
			return NS_FAILURE;
		}
		return NS_SUCCESS;
	}
	return NS_SUCCESS;
}

int file_select (struct dirent *entry) {
	char *ptr;
	if ((ircstrcasecmp(entry->d_name, ".")==0) || (ircstrcasecmp(entry->d_name, "..")==0)) 
		return 0;
	/* check filename extension */
	ptr = strrchr(entry->d_name, '.');
	if ((ptr) && !(ircstrcasecmp(ptr, ".bdb"))) {
		return NS_SUCCESS;
	}
	return 0;	
}

char **DBMListDB()
{
	struct dirent **files;
	int count, i, sl = 0;
	char *filename;
	char **DBList = NULL;;
	
	count = scandir ("data/", &files, file_select, alphasort);
	for (i = 2; i <= count; i++) 
	{
		
		filename = ns_malloc(strlen(files[i-1]->d_name) - 3);
		strlcpy(filename, files[i-1]->d_name, strlen(files[i-1]->d_name) - 3);
		AddStringToList(&DBList, filename, &sl);
	}
	AddStringToList(&DBList, '\0', &sl);
	return DBList;;
}

char **DBMListTables(char *Database)
{
	static char filename[MAXPATH];
	int dbret;
	DB *dbp;
	int rowcount = 0;
	DBC *dbcp;
	char *table;
	char **Tables = NULL;;
	int tl = 0;

	dlog(DEBUG1, "DBMListTables %s\n", Database);
	ircsprintf (filename, "data/%s.bdb", Database);
	if ((dbret = db_create(&dbp, NULL, 0)) != 0) {
		nlog(LOG_WARNING, "db_create: %s\n", db_strerror(dbret));
		return NULL;
	}
#if (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
	if ((dbret = dbp->open(dbp, NULL, filename, NULL, DB_UNKNOWN, DB_RDONLY, 0600)) != 0) {
#else
	if ((dbret = dbp->open(dbp, filename, NULL, DB_UNKNOWN, DB_RDONLY, 0600)) != 0) {
#endif
		nlog(LOG_WARNING,"dbp->open: %s\n", db_strerror(dbret));
		return NULL;
	}

	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* initilize the cursors */
	if ((dbret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		nlog(LOG_WARNING, "DB Cursor failed: %s\n", db_strerror(dbret));
		return NULL;
	}
	
	while ((dbret = dbcp->c_get(dbcp, &dbkey, &dbdata, DB_NEXT)) == 0)
	{
		rowcount++;
		if (dbkey.size > 0) {
			table = ns_malloc(dbkey.size+1);
			strlcpy(table, dbkey.data, dbkey.size+1);
			AddStringToList(&Tables, table, &tl);
		} else {
			nlog(LOG_WARNING, "Hrm, Table name is null?\n");
		}
	} 
	if (dbret != 0 && dbret != DB_NOTFOUND) {
		nlog(LOG_WARNING, "dbp->c_get failed: %s\n", db_strerror(dbret));
	}
	if ((dbret = dbcp->c_close(dbcp)) != 0) {
		nlog(LOG_WARNING, "dbcpp->close failed: %s\n", db_strerror(dbret));
	}	
	dbp->close(dbp, 0); 
	AddStringToList(&Tables, '\0', &tl);
	return Tables;

}


#endif
