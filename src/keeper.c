/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "keeper.h"
#include "conf.h"

/** @brief Gets Config Data of Type
 */
int
GetConf (void **data, int type, const char *item)
{
	char keypath[255];
	int i = 0;

	ircsnprintf (keypath, 255, "g/%s:/%s", GET_CUR_MODNAME(), item);

	switch (type) {
	case CFGSTR:
		i = kp_get_string (keypath, (char **) *&data);
		break;
	case CFGINT:
		i = kp_get_int (keypath, (int *) *&data);
		break;
	case CFGFLOAT:
		i = kp_get_float (keypath, (double *) *&data);
		break;
	case CFGBOOL:
		i = kp_get_bool (keypath, (int *) *&data);
		break;
	default:
		nlog (LOG_WARNING, "Keeper: Called GetConf with invalid datatype %d", type);
		return -1;
	}
	/* check for errors */
	if (i != 0) {
		nlog (LOG_DEBUG1, "GetConf: %s - Path: %s", kp_strerror (i), keypath);
		return -1;
	}
	return 1;
}

/* @brief return an array of strings containing all subkeys in a directory */

int
GetDir (char *item, char ***data)
{
	int i;
	char keypath[255];
	char **data1;

	ircsnprintf (keypath, 255, "g/%s:/%s", GET_CUR_MODNAME(), item);
	i = kp_get_dir (keypath, &data1, NULL);
	if (i == 0) {
		*data = data1;
		return 1;
	}
	*data = NULL;
	nlog (LOG_DEBUG1, "GetDir: %s - Path: %s", kp_strerror (i), keypath);
	return -1;

}



/** @brief Sets Config Data of Type
*/
int
SetConf (void *data, int type, char *item)
{
	char keypath[255];
	int i = 0;

	ircsnprintf (keypath, 255, "g/%s:/%s", GET_CUR_MODNAME(), item);

	switch (type) {
	case CFGSTR:
		i = kp_set_string (keypath, (char *) data);
		break;
	case CFGINT:
		i = kp_set_int (keypath, (int) data);
		break;
	case CFGFLOAT:
/*
			i = kp_set_float(keypath, (double *)data);
*/
		break;
	case CFGBOOL:
		i = kp_set_bool (keypath, (int) data);
		break;
	default:
		nlog (LOG_WARNING, "Keeper: Called SetConf with invalid datatype %d", type);
		return -1;
	}
	/* check for errors */
	if (i != 0) {
		nlog (LOG_WARNING, "SetConf: %s", kp_strerror (i));
		return -1;
	}
	return 1;

}


/** @brief removes Config Data
*/
int
DelConf (char *item)
{
	char keypath[255];
	int i = 0;

	/* determine if its a module setting */
	ircsnprintf (keypath, 255, "g/%s:/%s", GET_CUR_MODNAME(), item);
	i = kp_recursive_do(keypath, (kp_func) kp_remove, 0, NULL);
	/* check for errors */
	if (i != 0) {
		nlog (LOG_WARNING, "DelConf: %s (%s)", kp_strerror (i), keypath);
		return -1;
	}
	return 1;
}	


/** @brief Gets Data of Type
 */
int
GetData (void **data, int type, const char *table, const char *row, const char *field)
{
	char keypath[255];
	int i = 0;

	/* determine if its a module setting */
	ircsnprintf (keypath, 255, "l/%s:/%s/%s/%s", GET_CUR_MODNAME(), table, row, field);
	switch (type) {
	case CFGSTR:
		i = kp_get_string (keypath, (char **) *&data);
		break;
	case CFGINT:
		i = kp_get_int (keypath, (int *) *&data);
		break;
	case CFGFLOAT:
		i = kp_get_float (keypath, (double *) *&data);
		break;
	case CFGBOOL:
		i = kp_get_bool (keypath, (int *) *&data);
		break;
	default:
		nlog (LOG_WARNING, "Keeper: Called GetData with invalid datatype %d", type);
		return -1;
	}
	/* check for errors */
	if (i != 0) {
		nlog (LOG_DEBUG1, "GetData: %s - Path: %s", kp_strerror (i), keypath);
		return -1;
	}
	return 1;
}

/* @brief return an array of strings containing all rows in a database */

int
GetTableData (char *table, char ***data)
{
	int i;
	char keypath[255];
	char **data1;

	/* determine if its a module setting */
	ircsnprintf (keypath, 255, "l/%s:/%s", GET_CUR_MODNAME(), table);
	i = kp_get_dir (keypath, &data1, NULL);
	if (i == 0) {
		*data = data1;
		return 1;
	}
	*data = NULL;
	nlog (LOG_DEBUG1, "GetTableData: %s - Path: %s", kp_strerror (i), keypath);
	return -1;

}



/** @brief Sets Config Data of Type
*/
int
SetData (void *data, int type, char *table, char *row, char *field)
{
	char keypath[255];
	int i = 0;

	ircsnprintf (keypath, 255, "l/%s:/%s/%s/%s", GET_CUR_MODNAME(), table, row, field);
	switch (type) {
	case CFGSTR:
		i = kp_set_string (keypath, (char *) data);
		break;
	case CFGINT:
		i = kp_set_int (keypath, (int) data);
		break;
	case CFGFLOAT:
/*
			i = kp_set_float(keypath, (double *)data);
*/
		break;
	case CFGBOOL:
		i = kp_set_bool (keypath, (int) data);
		break;
	default:
		nlog (LOG_WARNING, "Keeper: Called SetData with invalid datatype %d", type);
		return -1;
	}
	/* check for errors */
	if (i != 0) {
		nlog (LOG_WARNING, "SetData: %s", kp_strerror (i));
		return -1;
	}
	return 1;

}


/** @brief removes a row from the Database
*/
int
DelRow (char *table, char *row)
{
	char keypath[255];
	int i = 0;

	/* determine if its a module setting */
	ircsnprintf (keypath, 255, "l/%s:/%s/%s", GET_CUR_MODNAME(), table, row);
	i = kp_recursive_do(keypath, (kp_func) kp_remove, 0, NULL);
	/* check for errors */
	if (i != 0) {
		nlog (LOG_WARNING, "DelRow: %s (%s)", kp_strerror (i), keypath);
		return -1;
	}
	return 1;
}	

/** @brief removes a row from the Database
*/
int
DelTable (char *table)
{
	char keypath[255];
	int i = 0;

	/* determine if its a module setting */
	ircsnprintf (keypath, 255, "l/%s:/%s", GET_CUR_MODNAME(), table);
	i = kp_recursive_do(keypath, (kp_func) kp_remove, 0, NULL);
	/* check for errors */
	if (i != 0) {
		nlog (LOG_WARNING, "DelTable: %s (%s)", kp_strerror (i), keypath);
		return -1;
	}
	return 1;
}	
/** @brief flushes the Keeper Database out 
*/
void
flush_keeper() {
	kp_flush();
}
