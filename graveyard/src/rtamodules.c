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
#include "rta.h"

char sqlbuf[BUFSIZE];

void *display_module_name (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->name, MAX_MOD_NAME);
	return sqlbuf;	
}

void *display_module_desc (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->description, BUFSIZE);
	return sqlbuf;
}

void *display_module_version (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->version, BUFSIZE);
	return sqlbuf;
}

void *display_module_builddate (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	ircsnprintf(sqlbuf, BUFSIZE, "%s - %s", mod_ptr->info->build_date, mod_ptr->info->build_time);
	return sqlbuf;
}

void *display_core_info (void *tbl, char *col, char *sql, void *row) 
{
	ircsnprintf(sqlbuf, BUFSIZE, "%s", me.version);
	return sqlbuf;	
}

COLDEF neo_modulecols[] = {
	{
		"modules",
		"name",
		RTA_STR,
		MAX_MOD_NAME,
		offsetof(struct _Module, info),
		RTA_READONLY,
		display_module_name,
		NULL,
		"The name of the Module"
	},
	{
		"modules",
		"description",
		RTA_STR,
		BUFSIZE,
		offsetof(struct _Module, info),
		RTA_READONLY,
		display_module_desc, 
		NULL,
		"The Module Description"
	},
	{
		"modules",
		"version",
		RTA_STR,
		BUFSIZE,
		offsetof(struct _Module, info),
		RTA_READONLY,
		display_module_version,
		NULL,
		"The module version"
	},
	{
		"modules",
		"builddate",
		RTA_STR,
		BUFSIZE,
		offsetof(struct _Module, info),
		RTA_READONLY,
		display_module_builddate,
		NULL,
		"The module build date"
	},
	{
		"modules",
		"coreinfo",
		RTA_STR,
		BUFSIZE,
		offsetof(struct _Module, info),
		RTA_READONLY,
		display_core_info,
		NULL,
		"The NeoStats core Version"
	},
};

TBLDEF neo_modules = {
	"modules",
	NULL, 	/* for now */
	sizeof(struct _Module),
	0,
	TBL_HASH,
	neo_modulecols,
	sizeof(neo_modulecols) / sizeof(COLDEF),
	"",
	"The list of Modules loaded by NeoStats"
};

