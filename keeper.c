/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: keeper.c,v 1.3 2003/04/12 06:21:29 fishwaldo Exp $
*/

#include "stats.h"
#include "keeper.h"
#include "conf.h"
#include "log.h"

/** @brief Gets Config Data of Type
 */
int GetConf(void **data, int type, const char *item) {
char keypath[255];
int i = 0;

	/* determine if its a module setting */
	if (strlen(segvinmodule) > 0) {
		snprintf(keypath, 255, "g/%s:/%s", segvinmodule, item);
	} else {
		snprintf(keypath, 255, "g/core:/%s", item);
	}

	switch (type) {
		case CFGSTR:
			i = kp_get_string(keypath, (char **)*&data);
			break;
		case CFGINT:
			i = kp_get_int(keypath, (int *)*&data);
			break;
		case CFGFLOAT:
			i = kp_get_float(keypath, (double *)*&data);
			break;
		case CFGBOOL:
			i = kp_get_bool(keypath, (int *)*&data);
			break;		
		default:
			nlog(LOG_WARNING, LOG_CORE, "Keeper: Called GetConf with invalid datatype %d", type);
			return -1;
	}
	/* check for errors */
	if (i != 0) {
		data = malloc(255);
		nlog(LOG_WARNING, LOG_CORE, "GetConf: %s - Path: %s", kp_strerror(i), keypath);
		return -1;
	} 
	return 1;
}

/** @brief Sets Config Data of Type
*/
int SetConf(void *data, int type, char *item) {
char keypath[255];
int i = 0;

	/* determine if its a module setting */
	if (strlen(segvinmodule) > 0) {
		snprintf(keypath, 255, "g/%s:/%s", segvinmodule, item);
	} else {
		snprintf(keypath, 255, "g/core:/%s", item);
	}

	switch (type) {
		case CFGSTR:
			i = kp_set_string(keypath, (char *)data);
			break;
		case CFGINT:
			i = kp_set_int(keypath, (int)data);
			break;
		case CFGFLOAT:
/*
			i = kp_set_float(keypath, (double *)data);
*/
			break;
		case CFGBOOL:
			i = kp_set_bool(keypath, (int)data);
			break;		
		default:
			nlog(LOG_WARNING, LOG_CORE, "Keeper: Called SetConf with invalid datatype %d", type);
			return -1;
	}
	/* check for errors */
	if (i != 0) {
		nlog(LOG_WARNING, LOG_CORE, "SetConf: %s", kp_strerror(i));
		return -1;
	} 
	kp_flush();
	return 1;
	
}
