/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
** Based on:
** KEEPER: A configuration reading and writing library
**
** Copyright (C) 1999-2000 Miklos Szeredi
** Email: mszeredi@inf.bme.hu
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

#include "kp_util.h"

/* ------------------------------------------------------------------------- 
 * Get an enumerated value. The names of the values must be supplied. If
 * no name matches an error is returned
 * ------------------------------------------------------------------------- */
int kp_get_enum(const char *keypath, const char *names[], int *valp)
{
	int res;
	int i;
	char *str = NULL;

	res = kp_get_string(keypath, &str);
	if (res != 0)
		return res;

	for (i = 0; names[i] != NULL; i++) {
		if (strcmp(str, names[i]) == 0) {
			*valp = i;
			free(str);
			return 0;
		}
	}

	free(str);

	return KPERR_BADTYPE;
}

/* ------------------------------------------------------------------------- 
 * Set an enumerated value. The names of the values must be supplied
 * ------------------------------------------------------------------------- */
int kp_set_enum(const char *keypath, const char *names[], int val)
{
	int i;

	if (val >= 0) {
		for (i = 0; names[i] != NULL; i++)
			if (i == val)
				return kp_set_string(keypath, names[val]);
	}

	return KPERR_BADTYPE;
}

static const char *kp_boolnames[] = { "FALSE", "TRUE", NULL };

/* ------------------------------------------------------------------------- 
 * Get a boolean value
 * ------------------------------------------------------------------------- */
int kp_get_bool(const char *keypath, int *boolp)
{
	return kp_get_enum(keypath, kp_boolnames, boolp);
}

/* ------------------------------------------------------------------------- 
 * Set a boolen value 
 * ------------------------------------------------------------------------- */
int kp_set_bool(const char *keypath, int boolval)
{
	return kp_set_enum(keypath, kp_boolnames, boolval ? 1 : 0);
}

/* End of kp_enum.c */
