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
#include <fcntl.h>

/* ------------------------------------------------------------------------- 
 * Code data (character array) to a string
 * ------------------------------------------------------------------------- */
static char *kp_code_data(kp_key * ck)
{
	char *buf, *ds;
	int i;
	int val;

	buf = (char *) smalloc(ck->len * 3 + 64);
	ds = buf;

	*ds++ = 'D';
	*ds++ = SEP2;
	for (i = 0; i < (int) ck->len; i++) {
		if (i != 0)
			*ds++ = ' ';
		val = ((unsigned char *) ck->data)[i];
		sprintf(ds, "%02x", val);
		ds += 2;
	}
	*ds = '\0';

	return buf;
}

/* ------------------------------------------------------------------------- 
 * Code arbitary string to an escaped representation
 * ------------------------------------------------------------------------- */
static char *kp_code_string(kp_key * ck)
{
	char *buf, *ds;
	unsigned char *os;
	int i;

	buf = (char *) smalloc(ck->len * 4 + 64);
	ds = buf;
	*ds++ = 'S';
	*ds++ = SEP2;
	*ds++ = '"';

	os = (unsigned char *) ck->data;

	for (i = 0; i < (int) ck->len; i++) {
		switch (os[i]) {
		case '\b':
			*ds++ = '\\';
			*ds++ = 'b';
			break;

		case '\f':
			*ds++ = '\\';
			*ds++ = 'f';
			break;

		case '\n':
			*ds++ = '\\';
			*ds++ = 'n';
			break;

		case '\r':
			*ds++ = '\\';
			*ds++ = 'r';
			break;

		case '\t':
			*ds++ = '\\';
			*ds++ = 't';
			break;

		case '\v':
			*ds++ = '\\';
			*ds++ = 'v';
			break;

		case '\\':
			*ds++ = '\\';
			*ds++ = '\\';
			break;

		case '"':
			*ds++ = '\\';
			*ds++ = '"';
			break;

		default:
			if (os[i] < 32 || os[i] >= 127) {
				*ds++ = '\\';
				*ds++ = '0' + ((os[i] >> 6) & 3);
				*ds++ = '0' + ((os[i] >> 3) & 7);
				*ds++ = '0' + (os[i] & 7);
			} else
				*ds++ = os[i];
		}
	}

	*ds++ = '"';
	*ds = '\0';
	return buf;
}

/* ------------------------------------------------------------------------- 
 * Return the string representation of an integer
 * ------------------------------------------------------------------------- */
static char *kp_code_int(kp_key * ck)
{
	char *buf, *ds;
	int val;

	buf = (char *) smalloc(64);
	ds = buf;
	*ds++ = 'I';
	*ds++ = SEP2;

	val = *(int *) ck->data;
	sprintf(ds, "%i", val);

	return buf;
}

/* ------------------------------------------------------------------------- 
 * Return the string representation of a float
 * ------------------------------------------------------------------------- */
static char *kp_code_float(kp_key * ck)
{
	char *buf, *ds;
	double val;

	buf = (char *) smalloc(64);
	ds = buf;
	*ds++ = 'F';
	*ds++ = SEP2;

	val = *(double *) ck->data;
	sprintf(ds, "%.20g", val);

	return buf;
}


/* ------------------------------------------------------------------------- 
 * Code a value to a string depending on it's type
 * ------------------------------------------------------------------------- */
static char *kp_code_value(kp_key * ck)
{
	char *buf;

	switch (ck->type) {
	case KPVAL_DATA:
		buf = kp_code_data(ck);
		break;

	case KPVAL_STRING:
		buf = kp_code_string(ck);
		break;

	case KPVAL_INT:
		buf = kp_code_int(ck);
		break;

	case KPVAL_FLOAT:
		buf = kp_code_float(ck);
		break;

	default:
		/* Internal error */
		fprintf(stderr,
			"keeper: Internal Error: Illegal type: %i\n",
			ck->type);
		abort();
	}

	return buf;
}

/* ------------------------------------------------------------------------- 
 * Remove empty directories in a path
 * ------------------------------------------------------------------------- */
static void kp_delete_path(kp_path * kpp)
{
	char *ipath;
	int res;
	int i;
	int ibeg = _kp_get_ibeg(kpp->dbindex);

	ipath = sstrdup(kpp->path);

	i = strlen(ipath) - 1;
	while (1) {
		while (i >= 0 && ipath[i] != '/')
			i--;
		while (i >= 0 && ipath[i] == '/')
			i--;

		if (i < ibeg)
			break;

		ipath[i + 1] = '\0';
		res = rmdir(ipath);
		if (res == -1)
			break;
	}
	free(ipath);
}

/* ------------------------------------------------------------------------- 
 * Return the creation mode of a file or directory
 * ------------------------------------------------------------------------- */
static int kp_get_cmode(const char *path, int isdir)
{
	int lastchar;
	int pathlen;

	pathlen = strlen(path);
	if (pathlen == 0)
		return 0;

	lastchar = path[pathlen - 1];


	if (lastchar == '-') {
		if (isdir)
			return 0700;
		else
			return 0600;
	} else {
		if (isdir)
			return 0755;
		else
			return 0644;
	}
}

/* ------------------------------------------------------------------------- 
 * Create the nonexistent directories in a path
 * ------------------------------------------------------------------------- */
static int kp_create_path(kp_path * kpp)
{
	char *ipath;
	int res;
	int i;
	struct stat stbuf;
	int cmode;
	int ibeg = _kp_get_ibeg(kpp->dbindex);

	ipath = sstrdup(kpp->path);

	i = strlen(ipath) - 1;
	while (1) {
		if (i < ibeg) {
			res = KPERR_NOKEY;
			break;
		}

		while (i >= 0 && ipath[i] != '/')
			i--;
		while (i >= 0 && ipath[i] == '/')
			i--;

		if (i < 0) {
			res = KPERR_BADKEY;
			break;
		}

		ipath[i + 1] = '\0';
		res = stat(ipath, &stbuf);
		ipath[i + 1] = '/';

		if (res == -1 && errno != ENOENT) {
			res = _kp_errno_to_kperr(errno);
			break;
		} else if (res == 0) {
			if (!S_ISDIR(stbuf.st_mode)) {
				res = KPERR_BADKEY;
				break;
			}

			res = 0;
			break;
		}
	}

	if (res != 0) {
		free(ipath);
		return res;
	}

	i++;
	while (1) {
		while (ipath[i] == '/')
			i++;

		while (ipath[i] && ipath[i] != '/')
			i++;

		if (!ipath[i]) {
			res = 0;
			break;
		}

		ipath[i] = '\0';
		cmode = kp_get_cmode(ipath, 1);
#ifdef WIN32
		res = mkdir(ipath);
#else
		res = mkdir(ipath, cmode);
#endif
		ipath[i] = '/';

		if (res == -1) {
			res = _kp_errno_to_kperr(errno);
			break;
		}
	}

	free(ipath);

	return res;
}


/* ------------------------------------------------------------------------- 
 * Check if a key-name can be found in the key list
 * ------------------------------------------------------------------------- */
static int kp_keys_match(char *buf, kp_key * keys)
{
	kp_key *k;

	for (k = keys; k != NULL; k = k->next) {
		if (k->flags & KPFL_DIRTY) {
			if (strcmp(buf, k->name) == 0)
				return 1;
			if (!(k->flags & KPFL_REMOVED) &&
			    (kp_is_subkey(buf, k->name) ||
			     kp_is_subkey(k->name, buf))) {

				/* This key cannot be set. */
				k->flags &= ~KPFL_DIRTY;
			}
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- 
 * Copy entries from the old database file to the temporary file, excluding
 * those which are found in the key list
 * ------------------------------------------------------------------------- */
static int kp_remove_keys(kp_path * kpp, FILE * tmpfp, kp_key * keys,
			  int *emptyp)
{
	FILE *fp;
	char *buf, *value;
	int finished;
	int empty;

	fp = fopen(kpp->path, "r");
	if (fp == NULL)
		return _kp_errno_to_kperr(errno);

	empty = 1;
	finished = 0;
	do {
		buf = _kp_get_line(fp, &value);
		if (buf == NULL)
			finished = 1;
		else if (!kp_keys_match(buf, keys)) {
			fprintf(tmpfp, "%s%c%s\n", buf, SEP1, value);
			empty = 0;
		}
		free(buf);
	} while (!finished);

	fclose(fp);

	*emptyp = empty;

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Write entries to the file for all keys in the key list
 * ------------------------------------------------------------------------- */
static void kp_write_keys(FILE * fp, kp_key * keys, int *emptyp)
{
	kp_key *k;
	char *buf;

	for (k = keys; k != NULL; k = k->next) {
		if (k->flags & KPFL_DIRTY) {
			if (!(k->flags & KPFL_REMOVED)) {
				*emptyp = 0;

				buf = kp_code_value(k);
				fprintf(fp, "%s%c%s\n", k->name, SEP1,
					buf);
				free(buf);
			}

			k->flags &= ~KPFL_DIRTY;
		}
	}
}

/* ------------------------------------------------------------------------- 
 * Rewrite the database file, changing the value of the keys in the given
 * key list. If the file did not exist, it is created along with the
 * missing directories. If at the end the file becomes empty, it is
 * deleted, and all empty directories above it are also deleted.
 * ------------------------------------------------------------------------- */
int _kp_write_file(kp_path * kpp, kp_key * keys)
{
	FILE *tmpfp;
	struct stat stbuf;
	int res;
	char *tmppath;
	int empty = 1;
	int delpath = 0;
	int fd;
	int cmode;

	tmppath = _kp_get_tmpfile(kpp->dbindex);

	tmpfp = NULL;
	cmode = kp_get_cmode(kpp->path, 0);
	fd = open(tmppath, O_CREAT | O_WRONLY | O_TRUNC, cmode);
	if (fd != -1)
		tmpfp = fdopen(fd, "w");

	if (tmpfp == NULL)
		res = _kp_errno_to_kperr(errno);
	else {
		fprintf(tmpfp, "# KP 1.0\n");

		res = stat(kpp->path, &stbuf);
		if (res == -1 && errno == ENOENT)
			res = kp_create_path(kpp);
		else
			res = kp_remove_keys(kpp, tmpfp, keys, &empty);

		if (res != 0)
			fclose(tmpfp);
		else {
			kp_write_keys(tmpfp, keys, &empty);
			fclose(tmpfp);

			if (empty) {
				res = unlink(kpp->path);
				if (res == -1)
					res = _kp_errno_to_kperr(errno);
				else
					delpath = 1;
			} else {
				res = rename(tmppath, kpp->path);
				if (res != 0)
					res = _kp_errno_to_kperr(errno);
			}
		}
		unlink(tmppath);
	}
	free(tmppath);

	if (delpath)
		kp_delete_path(kpp);

	return res;
}

/* End of kp_set.c */
