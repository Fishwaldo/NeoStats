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

#ifndef WIN32
#include <dirent.h>
#endif

/* ------------------------------------------------------------------------- 
 * Decode the type of a key
 * ------------------------------------------------------------------------- */
static kpval_t kp_type_from_code(int c)
{
	switch (c) {
	case 'D':
		return KPVAL_DATA;

	case 'S':
		return KPVAL_STRING;

	case 'I':
		return KPVAL_INT;

	case 'F':
		return KPVAL_FLOAT;

	default:
		return KPVAL_UNKNOWN;
	}
}

/* ------------------------------------------------------------------------- 
 * Decode the data type
 * ------------------------------------------------------------------------- */
static int kp_decode_data(const char *buf, kp_key * ck)
{
	unsigned int len;
	unsigned int i;
	const char *s;
	int val;

	len = 0;
	s = buf;
	while (*s) {
		if (!isxdigit((int) *s))
			return -1;
		s++;
		if (!isxdigit((int) *s))
			return -1;
		s++;

		if (*s) {
			if (*s != ' ')
				return -1;
			s++;
		}

		len++;
	}

	kp_value_new(ck, KPVAL_DATA, len, NULL);

	for (i = 0; i < len; i++) {
		val = (int) strtol(buf + (i * 3), NULL, 16);
		((unsigned char *) ck->data)[i] = val;
	}

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Decode the escaped representation of the string
 * ------------------------------------------------------------------------- */
static int kp_decode_string(const char *buf, kp_key * ck)
{
	unsigned int len;
	int i;
	const char *s;
	int val;

	len = 0;
	s = buf;
	if (*s++ != '"')
		return -1;

	while (*s && *s != '"') {
		if (*s == '\\') {
			s++;

			switch (*s) {
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
			case 'v':
			case '\\':
			case '"':
				break;

			default:
				if (*s >= '0' && *s < '4') {
					s++;
					if (*s < '0' || *s >= '8')
						return -1;
					s++;
					if (*s < '0' || *s >= '8')
						return -1;
				} else
					return -1;
			}
		}
		s++;

		len++;
	}

	if (*s != '"')
		return -1;

	kp_value_new(ck, KPVAL_STRING, len, NULL);

	s = buf + 1;
	for (i = 0; *s != '"'; i++) {
		if (*s != '\\')
			val = *s;
		else {
			s++;

			switch (*s) {
			case 'b':
				val = '\b';
				break;

			case 'f':
				val = '\f';
				break;

			case 'n':
				val = '\n';
				break;

			case 'r':
				val = '\r';
				break;

			case 't':
				val = '\t';
				break;

			case 'v':
				val = '\v';
				break;

			case '\\':
				val = '\\';
				break;

			case '"':
				val = '"';
				break;

			default:
				val = (*s - '0') << 6;
				s++;
				val += (*s - '0') << 3;
				s++;
				val += (*s - '0');
			}
		}
		s++;

		((unsigned char *) ck->data)[i] = val;
	}

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Decode the string representation of an integer type
 * ------------------------------------------------------------------------- */
static int kp_decode_int(const char *buf, kp_key * ck)
{
	int val;
	char *end;

	val = (int) strtol(buf, &end, 10);
	if (end == buf)
		return -1;

	kp_value_new(ck, KPVAL_INT, sizeof(int), &val);

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Decode the string representation of a float type
 * ------------------------------------------------------------------------- */
static int kp_decode_float(const char *buf, kp_key * ck)
{
	double val;
	char *end;

	val = strtod(buf, &end);
	if (end == buf)
		return -1;

	kp_value_new(ck, KPVAL_FLOAT, sizeof(double), &val);

	return 0;

}

/* ------------------------------------------------------------------------- 
 * Decode the string representation to the type and value of the key
 * ------------------------------------------------------------------------- */
static int kp_decode_value(const char *buf, kp_key * ck)
{
	int res;
	kpval_t type;

	if (strlen(buf) < 2)
		return -1;

	if (buf[1] != SEP2)
		return -1;

	type = kp_type_from_code(buf[0]);

	switch (type) {
	case KPVAL_DATA:
		res = kp_decode_data(buf + 2, ck);
		break;

	case KPVAL_STRING:
		res = kp_decode_string(buf + 2, ck);
		break;

	case KPVAL_INT:
		res = kp_decode_int(buf + 2, ck);
		break;

	case KPVAL_FLOAT:
		res = kp_decode_float(buf + 2, ck);
		break;

	default:
		res = -1;
	}

	return res;

}

/* ------------------------------------------------------------------------- 
 * Read a database file, and insert the keys into the key list
 * ------------------------------------------------------------------------- */
int _kp_read_file(char *path, kp_key ** ksp)
{
	FILE *fp;
	char *buf, *value;
	int res;
	int finished;
	kp_key *keys = *ksp;
	kp_key ck;

	fp = fopen(path, "r");
	if (fp == NULL)
		return _kp_errno_to_kperr(errno);

	finished = 0;
	do {
		buf = _kp_get_line(fp, &value);
		if (buf == NULL)
			finished = 1;
		else {
			kp_key *newkey;

			/* FIXME: check key for uniqueness, etc... */

			res = kp_decode_value(value, &ck);
			if (res != 0) {
				kp_value_new(&ck, KPVAL_UNKNOWN, 0, NULL);
				ck.flags |= KPFL_BADDB;
			}

			newkey = (kp_key *) smalloc(sizeof(kp_key));
			*newkey = ck;
			newkey->name = sstrdup(buf);
			newkey->next = keys;
			keys = newkey;
		}
		free(buf);
	} while (!finished);

	fclose(fp);
	*ksp = keys;

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Check if a database file contains a single value (no key name)
 * ------------------------------------------------------------------------- */
static int kp_is_singleval(const char *path)
{
	FILE *fp;
	char *buf, *value;
	int is_singleval;


	fp = fopen(path, "r");
	if (fp == NULL)
		is_singleval = 0;
	else {
		buf = _kp_get_line(fp, &value);
		if (buf == NULL)
			is_singleval = 0;
		else {
			if (buf[0] != '\0')
				is_singleval = 0;
			else
				is_singleval = 1;

			free(buf);
		}
		fclose(fp);
	}

	return is_singleval;
}

/* ------------------------------------------------------------------------- 
 * Get the subkeys in a directory. In case the directory entry is a
 * directory or a single-value file, the entries name is added to the
 * key array. If it is a multilple-value file a colon is appended to the
 * key name before inserting it into the array
 * ------------------------------------------------------------------------- */
int _kp_get_subkeys_dir(char *path, struct key_array *keys)
{
#ifndef WIN32
	DIR *dirp;
	struct dirent *dirent;
	struct stat stbuf;
	char *fullpath;
	int res;

	dirp = opendir(path);
	if (dirp == NULL)
		return _kp_errno_to_kperr(errno);

	do {
		dirent = readdir(dirp);
		if (dirent != NULL && strcmp(dirent->d_name, ".") != 0 &&
		    strcmp(dirent->d_name, "..") != 0 &&
		    dirent->d_name[0] != ':') {

			fullpath = (char *) smalloc(strlen(path) + 1 +
							 strlen(dirent->
								d_name) +
							 1);
			sprintf(fullpath, "%s/%s", path, dirent->d_name);

			res = stat(fullpath, &stbuf);
			if (res == 0) {
				if (S_ISDIR(stbuf.st_mode))
					_kp_add_subkey_check(keys,
							     dirent->
							     d_name);
				else if (S_ISREG(stbuf.st_mode)
					 && kp_is_singleval(fullpath))
					_kp_add_subkey_check(keys,
							     dirent->
							     d_name);
				else {
					char *realname;
					realname = (char *)
					    smalloc(strlen
							 (dirent->d_name) +
							 2);
					sprintf(realname, "%s:",
						dirent->d_name);
					_kp_add_subkey_check(keys,
							     realname);
					free(realname);
				}
			}
			free(fullpath);
		}
	} while (dirent != NULL);

	closedir(dirp);
#endif
	return 0;
}

/* End of kp_get.c */
