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

#include "neostats.h"
#include "nsdbm.h"

/*#ifdef HAVE_DB_H*/
#if 0
#include <db.h>

static DBT dbkey;
static DBT dbdata;

void *DBMOpenTable (const char *name)
{
	static char filename[MAXPATH];
	int dbret;
	DB *dbp;

	dlog (DEBUG1, "DBMOpenTable");
	ircsprintf (filename, "%s.bdb", name);
	if ((dbret = db_create(&dbp, NULL, 0)) != 0) {
		dlog(DEBUG1, "db_create: %s", db_strerror(dbret));
		return NULL;
	}
#if 0 /* Temp since it does not compile */
	if ((dbret = dbp->open(dbp, filename, "Data", DB_BTREE, DB_CREATE, 0664)) != 0) {
		dlog(DEBUG1, "dbp->open: %s", db_strerror(dbret));
		return NULL;
	}
	return (void *)dbp;
#else
	return NULL;
#endif
}

int DBMCloseTable (void *handle)
{
	DB *dbp = (DB *)handle;

	dlog(DEBUG1, "DBACloseTable");
	dbp->close(dbp, 0); 
	return NS_SUCCESS;
}

void* DBMGetData (void *handle, char *key)
{
	int dbret;
	DB *dbp = (DB *)handle;

	dlog(DEBUG1, "DBAFetch %s", key);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = dbp->get(dbp, NULL, &dbkey, &dbdata, 0)) == 0)
	{
		return dbdata.data;
	}
	dlog(DEBUG1, "dbp->get: fail");
	return NULL;
}

int DBMSetData (void *handle, char *key, void *data, int size)
{
	int dbret;
	DB *dbp = (DB *)handle;

	dlog(DEBUG1, "DBAStore %s %s", key, (char *)data);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	dbdata.data = data;
	dbdata.size = size;
	if ((dbret = dbp->put(dbp, NULL, &dbkey, &dbdata, 0)) != 0) {
		dlog(DEBUG1, "dbp->put: %s", db_strerror(dbret));
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

int DBMGetTableRows (void *handle, DBRowHandler handler)
{
	/* TODO */
	return NS_SUCCESS;
}

int DBMDelData (void *handle, char * key)
{
	/* TODO */
	return NS_SUCCESS;
}

#else
void *DBMOpenTable (const char *name)
{
	return NULL;
}

int DBMCloseTable (void *handle)
{
	return NS_FAILURE;
}

void* DBMGetData (void *handle, char *key)
{
	return NULL;
}

int DBMSetData (void *handle, char *key, void *data, int size)
{
	return NS_FAILURE;
}

int DBMGetTableRows (void *handle, DBRowHandler handler)
{
	return NS_FAILURE;
}

int DBMDelData (void *handle, char * key)
{
	return NS_FAILURE;
}
#endif