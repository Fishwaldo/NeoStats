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
 * Do a function recursively on all non-directory keys under the given key.
 * If the stop_on_err flag is set, and an error is encountered while
 * reading the database, or the given function returns non-zero, then the
 * iteration is aborted and the error is returnes. If the flag is not set,
 * then the last error encountered is retured. 
 * ------------------------------------------------------------------------- */
int kp_recursive_do(const char *key, kp_func func, int stop_on_err,
		    void *user_data)
{
	kpval_t type;
	int res;
	int finalres;

	res = kp_get_type(key, &type);
	if (res != 0)
		return res;

	finalres = 0;
	if (type != KPVAL_DIR) {
		finalres = (*func) (key, user_data);
	} else {
		char **keys;
		char **kp;
		KPDIR *dir;

		res = kp_get_dir(key, &keys, NULL);
		if (res != 0)
			return res;

		dir = kp_dir_open(key);
		for (kp = keys; *kp != NULL; kp++) {
			res =
			    kp_recursive_do(KP_PATH(dir, *kp), func,
					    stop_on_err, user_data);
			if (res != 0) {
				finalres = res;
				if (stop_on_err)
					break;
			}
		}
		kp_dir_close(dir);
		free(keys);
	}

	return finalres;
}

/* End of kp_recur.c */
