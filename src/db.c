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

#if 0 /* Temp since berkeley is causing too many problems */
#ifndef WIN32 /* Temp since open call no longer compiles */
#ifdef HAVE_DB_H
#include <db.h>

static DBT dbkey;
static DBT dbdata;
static int dbret;

typedef struct db_entry {
	DB* dbp;
	char dbname[MAXPATH];
}db_entry;

db_entry db_list[NUM_MODULES];

int DBOpenDatabase(void)
{
	int index;

	dlog(DEBUG1, "DBOpenDatabase");
	index = GET_CUR_MODNUM();
	ircsnprintf(db_list[index].dbname, MAXPATH, "data/%s.db", GET_CUR_MODNAME());
	if ((dbret = db_create(&db_list[index].dbp, NULL, 0)) != 0) {
		dlog(DEBUG1, "db_create: %s", db_strerror(dbret));
		return -1;
	}
	if ((dbret = db_list[index].dbp->open(db_list[index].dbp, db_list[index].dbname, "Data", DB_BTREE, DB_CREATE, 0664)) != 0) {
		dlog(DEBUG1, "dbp->open: %s", db_strerror(dbret));
		return -1;
	}
	return 1;
}

void DBCloseDatabase(void)
{
	int index;

	dlog(DEBUG1, "DBCloseDatabase");
	index = GET_CUR_MODNUM();
	db_list[index].dbp->close(db_list[index].dbp, 0); 
}

void* DBGetData(char* key)
{
	int index;

	dlog(DEBUG1, "DBGetData %s", key);
	index = GET_CUR_MODNUM();
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = db_list[index].dbp->get(db_list[index].dbp, NULL, &dbkey, &dbdata, 0)) == 0)
	{
		return dbdata.data;
	}
	dlog(DEBUG1, "dbp->get: fail");
	return NULL;
}

void DBSetData(char* key, void* data, int size)
{
	int index;

	dlog(DEBUG1, "DBSetData %s %s", key, (char *)data);
	index = GET_CUR_MODNUM();
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	dbdata.data = data;
	dbdata.size = size;
	if ((dbret = db_list[index].dbp->put(db_list[index].dbp, NULL, &dbkey, &dbdata, 0)) != 0) {
		dlog(DEBUG1, "dbp->put: %s", db_strerror(dbret));
	}
}

#endif /* HAVE_DB_H */
#endif /* WIN32 */
#endif /* 0 */
