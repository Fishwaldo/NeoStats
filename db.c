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

#include "stats.h"
#include "log.h"

#ifdef USE_BERKELEY
#include <db.h>

#define	DATABASE "data/statserv.db"
static DB *dbp;
static DBT dbkey;
static DBT dbdata;
static int dbret;

void DBOpenDatabase(void)
{
	nlog(LOG_DEBUG1, LOG_MOD, "DBOpenDatabase");
	if ((dbret = db_create(&dbp, NULL, 0)) != 0) {
		nlog(LOG_DEBUG1, LOG_MOD, "db_create: %s", db_strerror(dbret));
		return;
	}
	if ((dbret = dbp->open(dbp, NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		nlog(LOG_DEBUG1, LOG_MOD, "dbp->open: %s", db_strerror(dbret));
		return;
	}
}

void DBCloseDatabase(void)
{
	nlog(LOG_DEBUG1, LOG_MOD, "DBCloseDatabase");
	dbp->close(dbp, 0); 
}

char* DBGetData(char* key)
{
	nlog(LOG_DEBUG1, LOG_MOD, "DBGetData %s", key);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = dbp->get(dbp, NULL, &dbkey, &dbdata, 0)) == 0)
	{
		nlog(LOG_DEBUG1, LOG_MOD, "DBGetData %s", dbdata.data);
		return dbdata.data;
	}
	nlog(LOG_DEBUG1, LOG_MOD, "dbp->get: fail");
	return NULL;
}

void DBSetData(char* key, char * data)
{
	nlog(LOG_DEBUG1, LOG_MOD, "DBSetData %s %s", key, data);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	dbdata.data = data;
	dbdata.size = strlen(data);
	if ((dbret = dbp->put(dbp, NULL, &dbkey, &dbdata, 0)) != 0) {
		nlog(LOG_DEBUG1, LOG_MOD, "dbp->put: %s", db_strerror(dbret));
	}
}

#endif /* USE_BERKELEY */