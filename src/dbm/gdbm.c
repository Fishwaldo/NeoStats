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
#include "gdbmdefs.h"
#include "gdbmerrno.h"
#include "extern.h"
#include "nsdbm.h"

extern const char *gdbm_strerror __P((gdbm_error));

static datum dbkey;
static datum dbdata;

void *DBMOpenTable (const char *name)
{
	static char filename[MAXPATH];
	gdbm_file_info *gdbm_file;
	int cache_size = DEFAULT_CACHESIZE;

	dlog (DEBUG1, "DBMOpenTable");
	ircsprintf (filename, "%s.gdbm", name);
	gdbm_file = gdbm_open (filename, 0, GDBM_WRCREAT, 00664, NULL);
	if (gdbm_file == NULL)
	{
		dlog (DEBUG1, "gdbm_open fail: %s", gdbm_strerror(gdbm_errno));
		return NULL;
	}
	if (gdbm_setopt(gdbm_file, GDBM_CACHESIZE, &cache_size, sizeof(int)) == -1)
	{
		dlog (DEBUG1, "gdbm_setopt fail: %s", gdbm_strerror(gdbm_errno));
		return NULL;
	}
	return (void *)gdbm_file;
}

int DBMCloseTable (void *handle)
{
	if (!handle) {
		return NS_FAILURE;
	}
	gdbm_close((gdbm_file_info *)handle); 
	return NS_SUCCESS;
}

void *DBMGetData (void *handle, char *key)
{
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.dptr = key;
	dbkey.dsize = strlen(key) + 1;
	dbdata = gdbm_fetch ((gdbm_file_info *)handle, dbkey);
	if (dbdata.dptr != NULL)
	{
		return  (dbdata.dptr);
	}
	dlog (DEBUG1, "gdbm_fetch fail: %s %s", key, gdbm_strerror(gdbm_errno));
	return NULL;
}

int DBMSetData (void *handle, char *key, void *data, int size)
{
	dbkey.dptr = key;
	dbkey.dsize = strlen(key) + 1;
	dbdata.dptr = data;
	dbdata.dsize = size;
	if (gdbm_store ((gdbm_file_info *)handle, dbkey, dbdata, GDBM_REPLACE) != 0)
	{
		dlog (DEBUG1, "gdbm_store fail: %s %s", key, gdbm_strerror(gdbm_errno));
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

int DBMGetTableRows (void *handle, DBRowHandler handler)
{
	int rowcount = 0;

	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey = gdbm_firstkey ((gdbm_file_info *)handle);
	while (dbkey.dptr != NULL)
	{
		rowcount ++;
		dlog (DEBUG1, "DBMGetTableRows: key %s", dbkey.dptr);
		dbdata = gdbm_fetch ((gdbm_file_info *)handle, dbkey);
		handler (dbdata.dptr);
		free (dbdata.dptr);
		dbdata = gdbm_nextkey ((gdbm_file_info *)handle, dbkey);
		free (dbkey.dptr);
		dbkey = dbdata;
	}
	return rowcount;
}

int DBMDelData (void *handle, char * key)
{
	dbkey.dptr = key;
	dbkey.dsize = strlen(key) + 1;
	if (gdbm_delete((gdbm_file_info *)handle, dbkey) != 0)
	{
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}