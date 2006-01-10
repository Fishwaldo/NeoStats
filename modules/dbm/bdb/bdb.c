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
#include "nsdbm.h"

#ifdef HAVE_DB_H*/
#include <db.h>

static DBT dbkey;
static DBT dbdata;

static DB_ENV *db_env;

static int dbopened = 0;

void db_log_cb(const char *prefix, char *msg) {
	dlog(DEBUG1, "%s", msg);
}

void *DBMOpenDB (const char *name)
{
	int dbret;
	
	if (!db_env) {
		if ((dbret = db_env_create(&db_env, 0)) != 0) {
			nlog(LOG_WARNING, "db_env_create failed: %s", db_strerror(dbret));
			return NULL;
		}
		db_env->set_verbose(db_env, DB_VERB_RECOVERY, 1);
		
		if ((dbret = db_env->open(db_env, "data/", DB_RECOVER|DB_CREATE|DB_INIT_TXN|DB_INIT_MPOOL, 0600)) != 0) {
			nlog(LOG_WARNING, "db evn open failed: %s", db_strerror(dbret));
			db_env->close(db_env, 0);
			return NULL;
		}
		db_env->set_errcall(db_env, db_log_cb);
#if 0
		db_env->set_msgcall(db_env, db_log_cb)
		db_env->stat_print(db_env, DB_STAT_ALL|DB_STAT_SUBSYSTEM);
#endif
	}
	dbopened++;
	return strndup(name, strlen(name));
}

void DBMCloseDB (void *dbhandle)
{
	ns_free(dbhandle);
	dbopened--;
	if (dbopened <= 0) {
		db_env->close(db_env, 0);
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

#endif
