/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
/*
 * KEEPER: A configuration reading and writing library
 *
 * Copyright (C) 1999-2000 Miklos Szeredi
 * Email: mszeredi@inf.bme.hu
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#include "kp_util.h"

#include <errno.h>
#include <sys/stat.h>


/* ------------------------------------------------------------------------- 
 * Checks the key-path for correct syntax
 * ------------------------------------------------------------------------- */
static int kp_check_path(const char *keypath)
{
	int seglen, segbeg;
	int i;
	int colonfound;

	/* Check for the following:
	   - only one / per segment
	   - : only appears once, and only at end or before /
	   - path does not contain . or .. segments
	   - path does not contain non printable characters
	   - path does not end with /
	 */

	colonfound = 0;
	segbeg = 0;
	i = 0;
	do {
		if ((keypath[i] > 0 && keypath[i] < 32)
		    || keypath[i] == 127)
			return KPERR_BADKEY;

		if (keypath[i] == '/' || keypath[i] == ':'
		    || keypath[i] == '\0') {
			seglen = i - segbeg;

			if (keypath[i] == ':') {
				if (colonfound)
					return KPERR_BADKEY;

				if (keypath[i + 1] != '/'
				    && keypath[i + 1] != '\0')
					return KPERR_BADKEY;

				colonfound = 1;
				i++;
			}
			if (i > 0) {
				if (seglen == 0)
					return KPERR_BADKEY;
				if (seglen == 1 && keypath[segbeg] == '.')
					return KPERR_BADKEY;
				if (seglen == 2 && keypath[segbeg] == '.'
				    && keypath[segbeg + 1] == '.')
					return KPERR_BADKEY;
			}
			segbeg = i + 1;
		}

	} while (keypath[i++]);

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Get the value of the key, iterate through the sections
 * ------------------------------------------------------------------------- */
static int kp_get(const char *keypath, kp_key * ck, kpval_t type)
{
	kp_path kpp;
	char *keyname;
	int iskeyfile;
	int res;

	res = kp_check_path(keypath);
	if (res != 0)
		return res;

	do {
		res = _kp_get_path(keypath, &kpp, &keyname, &iskeyfile);
		if (res != 0)
			return res;

		if (iskeyfile && !keyname[0])
			res = KPERR_BADKEY;
		else {
			res = _kp_cache_get(&kpp, keyname, type, ck);
		}
		free(kpp.path);
		keypath++;
	} while (res == KPERR_NOKEY);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Get a string value
 * ------------------------------------------------------------------------- */
int kp_get_string(const char *keypath, char **stringp)
{
	int res;
	kp_key ck;

	res = kp_get(keypath, &ck, KPVAL_STRING);
	if (res != 0)
		return res;

	*stringp = (char *) malloc_check(ck.len + 1);
	memcpy(*stringp, ck.data, ck.len);
	(*stringp)[ck.len] = '\0';
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Get an integer value
 * ------------------------------------------------------------------------- */
int kp_get_int(const char *keypath, int *intp)
{
	int res;
	kp_key ck;

	res = kp_get(keypath, &ck, KPVAL_INT);
	if (res != 0)
		return res;

	*intp = *((int *) ck.data);
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Get a float value
 * ------------------------------------------------------------------------- */
int kp_get_float(const char *keypath, double *floatp)
{
	int res;
	kp_key ck;

	res = kp_get(keypath, &ck, KPVAL_FLOAT);
	if (res != 0)
		return res;

	*floatp = *((double *) ck.data);
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Get an arbitary data (stored as a character array)
 * ------------------------------------------------------------------------- */
int kp_get_data(const char *keypath, void **datap, unsigned int *lenp)
{
	int res;
	kp_key ck;

	res = kp_get(keypath, &ck, KPVAL_DATA);
	if (res != 0)
		return res;

	*datap = malloc_check(ck.len);
	memcpy(*datap, ck.data, ck.len);
	*lenp = ck.len;
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Copy the key-name strings to the end of the array, and free them
 * ------------------------------------------------------------------------- */
static void kp_combine_strings(struct key_array *keys)
{
	char *s;
	int strsize;
	char **kp;

	s = ((char *) keys->array) + (sizeof(char *) * (keys->num + 1));

	for (kp = keys->array; *kp != NULL; kp++) {
		strsize = strlen(*kp) + 1;
		strsize = ROUND_TO(strsize, 8);

		strcpy(s, *kp);
		free(*kp);
		*kp = s;

		s += strsize;
	}
}

/* ------------------------------------------------------------------------- 
 * Free the key-names in the array, and the array itself
 * ------------------------------------------------------------------------- */
static void kp_free_keyarray(struct key_array *keys)
{
	char **kp;

	for (kp = keys->array; *kp != NULL; kp++)
		free(*kp);

	free(keys->array);
}

/* ------------------------------------------------------------------------- 
 * Get the subkeys in a dir, iterate through all sections
 * ------------------------------------------------------------------------- */
static int kp_get_subkeys(const char *keypath, struct key_array *keys)
{
	kp_path kpp;
	char *keyname;
	int iskeyfile;
	int res;

	res = kp_check_path(keypath);
	if (res != 0)
		return res;

	while (keypath[0] != '\0' && keypath[0] != '/') {
		res = _kp_get_path(keypath, &kpp, &keyname, &iskeyfile);
		if (res == 0) {
			_kp_cache_get_subkeys(&kpp, keyname, iskeyfile,
					      keys);
			free(kpp.path);
		}
		keypath++;
	}

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Get the subkeys in a dir, return the compact (one alloc) key array
 * ------------------------------------------------------------------------- */
int kp_get_dir(const char *keypath, char ***keysp, unsigned int *nump)
{
	int res;
	struct key_array keys;

	keys.array = (char **) malloc_check(sizeof(char *));
	keys.array[0] = NULL;
	keys.num = 0;
	keys.strsize = 0;

	res = kp_get_subkeys(keypath, &keys);
	if (res == 0) {
		kp_combine_strings(&keys);
		*keysp = keys.array;
		if (nump != NULL)
			*nump = keys.num;
	} else
		kp_free_keyarray(&keys);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Get the type of a key, iterate through the sections
 * ------------------------------------------------------------------------- */
int kp_get_type(const char *keypath, kpval_t * typep)
{
	kp_path kpp;
	char *keyname;
	int iskeyfile;
	int res;

	res = kp_check_path(keypath);
	if (res != 0)
		return res;

	do {
		res = _kp_get_path(keypath, &kpp, &keyname, &iskeyfile);
		if (res != 0)
			return res;

		res = _kp_cache_get_type(&kpp, keyname, iskeyfile, typep);
		free(kpp.path);
		keypath++;
	} while (res == KPERR_NOKEY);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Set the value of the key, only the first section is set
 * ------------------------------------------------------------------------- */
static int kp_set(const char *keypath, kp_key * ck)
{
	kp_path kpp;
	char *keyname;
	int res;
	int iskeyfile;

	res = kp_check_path(keypath);
	if (res != 0)
		return res;

	res = _kp_get_path(keypath, &kpp, &keyname, &iskeyfile);
	if (res != 0)
		return res;

	if (iskeyfile && !keyname[0])
		res = KPERR_BADKEY;
	else {
		ck->name = strdup_check(keyname);
		res = _kp_cache_set(&kpp, ck);
	}
	free(kpp.path);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Set a string value
 * ------------------------------------------------------------------------- */
int kp_set_string(const char *keypath, const char *string)
{
	int res;
	kp_key ck;
	unsigned int len;

	len = strlen(string);
	kp_value_new(&ck, KPVAL_STRING, len, string);

	res = kp_set(keypath, &ck);
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Set an integer value
 * ------------------------------------------------------------------------- */
int kp_set_int(const char *keypath, int intval)
{
	int res;
	kp_key ck;
	unsigned int len;

	len = sizeof(int);
	kp_value_new(&ck, KPVAL_INT, len, &intval);

	res = kp_set(keypath, &ck);
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Set a float value
 * ------------------------------------------------------------------------- */
int kp_set_float(const char *keypath, double floatval)
{
	int res;
	kp_key ck;
	unsigned int len;

	len = sizeof(double);
	kp_value_new(&ck, KPVAL_FLOAT, len, &floatval);

	res = kp_set(keypath, &ck);
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Set an arbitary data (stored as a character array)
 * ------------------------------------------------------------------------- */
int kp_set_data(const char *keypath, const void *data, unsigned int len)
{
	int res;
	kp_key ck;

	kp_value_new(&ck, KPVAL_DATA, len, data);

	res = kp_set(keypath, &ck);
	kp_value_destroy(&ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Remove a key
 * ------------------------------------------------------------------------- */
int kp_remove(const char *keypath)
{
	int res;
	kp_key ck;

	kp_value_new(&ck, KPVAL_UNKNOWN, 0, NULL);
	ck.flags |= KPFL_REMOVED;
	res = kp_set(keypath, &ck);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Flush changes to disk
 * ------------------------------------------------------------------------- */
int kp_flush()
{
	int res;

	res = _kp_cache_flush();

	return res;
}

/* ------------------------------------------------------------------------- 
 * Return a string corresponding to an error value
 * ------------------------------------------------------------------------- */
char *kp_strerror(int kperr)
{
	switch (kperr) {
	case KPERR_OK:
		return "OK";
	case KPERR_NOKEY:
		return "Key does not exist";
	case KPERR_NOACCES:
		return "Access denied";
	case KPERR_BADKEY:
		return "Bad key";
	case KPERR_BADTYPE:
		return "Bad type";
	case KPERR_BADDB:
		return "Bad database";
	case KPERR_NOSPACE:
		return "Not enough resources";

	default:
		return NULL;
	}
}

/* End of kp_iface.c */
