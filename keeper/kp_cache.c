/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

/* One physical file's cached data */
typedef struct _kp_fil {
	kp_path kpp;
	kp_key *keys;
	int dirty;
	time_t modif;
	time_t use;
	time_t check;
	struct _kp_fil *next;
} kp_fil;

/* The cache */
static kp_fil *kp_cachef = NULL;

/* ------------------------------------------------------------------------- 
 * Free a key list
 * ------------------------------------------------------------------------- */
static void kp_free_keys(kp_key * k)
{
	kp_key *nextk;

	while (k != NULL) {
		nextk = k->next;
		kp_value_destroy(k);
		free(k);
		k = nextk;
	}
}

/* ------------------------------------------------------------------------- 
 * If the file is in the cache, return it, otherwise return NULL
 * ------------------------------------------------------------------------- */
static kp_fil *kp_get_cached_file(kp_path * kpp)
{
	kp_fil *fil;
	char *path = kpp->path;

	for (fil = kp_cachef; fil != NULL; fil = fil->next)
		if (strcmp(fil->kpp.path, path) == 0) {
#ifdef KPDEBUG
			printf("kp_get_cached_file: %s\n", fil->kpp.path);
#endif
			break;

		}
	return fil;
}

/* ------------------------------------------------------------------------- 
 * Create a new chaced file, and insert it into the cache
 * ------------------------------------------------------------------------- */
static kp_fil *kp_new_cached_file(kp_path * kpp)
{
	kp_fil *fil;

	fil = (kp_fil *) malloc_check(sizeof(kp_fil));
	fil->kpp.path = strdup_check(kpp->path);
	fil->kpp.dbindex = kpp->dbindex;
	fil->keys = NULL;
	fil->dirty = 0;
	fil->modif = 0;
	fil->use = 0;
	fil->check = 0;
	fil->next = kp_cachef;
	kp_cachef = fil;
#ifdef KPDEBUG
	printf("kp_new_cached_file: %s\n", fil->kpp.path);
#endif

	return fil;
}

/* ------------------------------------------------------------------------- 
 * Free data associated with a cached file
 * ------------------------------------------------------------------------- */
static void kp_free_file(kp_fil * fil)
{
#ifdef KPDEBUG
	printf("kp_free_file: %s\n", fil->kpp.path);
#endif
	free(fil->kpp.path);
	kp_free_keys(fil->keys);
	free(fil);
}

/* ------------------------------------------------------------------------- 
 * Remove a file from a cache and free it
 * ------------------------------------------------------------------------- */
static void kp_remove_file_from_cache(kp_fil * rfil)
{
	kp_fil **filp;

	for (filp = &kp_cachef; *filp != NULL; filp = &(*filp)->next) {
		if (*filp == rfil) {
			*filp = rfil->next;
#ifdef KPDEBUG
			printf("kp_remove_file_from_cache: %s\n", rfil->kpp.path);
#endif
			kp_free_file(rfil);
			return;
		}
	}

	/* Should never happen */
	fprintf(stderr, "keeper: Internal Error: Corrupted cache data\n");
	abort();
}

#ifdef ALLOC_CHECK
/* ------------------------------------------------------------------------- 
 * Remove all files from the cache
 * ------------------------------------------------------------------------- */
void _kp_clear_cache(void)
{
#ifdef KPDEBUG
	printf("kp_clear_cache\n");
#endif
	while (kp_cachef != NULL)
		kp_remove_file_from_cache(kp_cachef);
}
#endif

/* ------------------------------------------------------------------------- 
 * Remove all old, non-dirty files from the cache
 * ------------------------------------------------------------------------- */
static void kp_clean_up_cache(time_t now)
{
	kp_fil **filp;
	kp_fil *fil;
	time_t expire;

	expire = now - 10;

	for (filp = &kp_cachef; *filp != NULL;) {
		fil = *filp;
		if (fil->use < expire && !fil->dirty) {
			*filp = fil->next;
#ifdef KPDEBUG
			printf("kp_clean_up_cache: %d < %d (Dirty: %d): %s\n", fil->use, expire, fil->dirty, fil->kpp.path);
#endif
			kp_free_file(fil);
		} else
			filp = &(*filp)->next;
	}
}

/* ------------------------------------------------------------------------- 
 * If the key is in the cached file, return it, otherwise return NULL
 * ------------------------------------------------------------------------- */
static kp_key *kp_get_cached_key(kp_fil * fil, const char *name)
{
	kp_key *key;

	for (key = fil->keys; key != NULL; key = key->next)
		if (strcmp(key->name, name) == 0) {
#ifdef KPDEBUG
			printf("kp_get_cached_file: %s-%s\n", fil->kpp.path, name);
#endif
			break;
		} 
	return key;
}

/* ------------------------------------------------------------------------- 
 * Find a key in a cached file. If it exists return it. If there exists a
 * subkey of it, or it is a subkey of one, then return an error
 * ------------------------------------------------------------------------- */
static int kp_check_insert(kp_fil * fil, char *name, kp_key ** keyretp)
{
	kp_key *key;

	for (key = fil->keys; key != NULL; key = key->next) {
		if (strcmp(key->name, name) == 0) {
#ifdef KPDEBUG
			printf("kp_check_insert: Got key %s-%s\n", fil->kpp.path, name);
#endif
			break;
		}
		if (kp_is_subkey(name, key->name)
		    || kp_is_subkey(key->name, name))
			return KPERR_BADKEY;
	}
#ifdef KPDEBUG
	printf("kp_check_insert: Got it\n");
#endif
	*keyretp = key;
	return 0;
}

/* -------------------------------------------------------------------------
 * This function returns a cached file.  If the file was not in the cache or
 * was outdated, then it reads it into the cache. If the path was refering
 * to a directory, then it returns this in the *isdirp flag.
 * ------------------------------------------------------------------------- */
static int kp_get_file(kp_path * kpp, kp_fil ** fp, int *isdirp)
{
	kp_fil *fil;
	int res;
	struct stat stbuf;
	time_t now;

	*fp = NULL;
	*isdirp = 0;

	fil = kp_get_cached_file(kpp);
	now = time(NULL);

	/* This check-magic is needed, because stat()-ing nonexistent
	   files on NFS seems to be very expensive on some (Solaris)
	   architectures */

	/* If the file was checked less then one second ago, then we
	   believe it hasn't changed. */

	/* FIXME: even if the file is dirty, the non-dirty keys should be
	   updated from a changed file. This is not very important
	   though. */

	if (fil != NULL && !fil->dirty && now != fil->check) {
		res = stat(kpp->path, &stbuf);

		if (fil->keys == NULL && res == -1 && errno == ENOENT) {
#ifdef KPDEBUG
			printf("kp_get_file: does not exist: %s\n",kpp->path);
#endif 
			fil->check = now;
		} else if (res == -1 || !S_ISREG(stbuf.st_mode) ||
			 stbuf.st_mtime != fil->modif) {
#ifdef KPDEBUG
			printf("kp_get_file: not regular file: %s\n", kpp->path);
#endif
			kp_remove_file_from_cache(fil);
			fil = NULL;
		} else {
#ifdef KPDEBUG
			printf("kp_get_file: ok %s\n", kpp->path);
#endif
			fil->check = now;
		}
	}

	if (fil != NULL) {
#ifdef KPDEBUG
		printf("kp_get_file: existing file\n");
#endif
		fil->use = now;
	} else {
		kp_key *keys = NULL;

		/* Clean up old cached files */
		kp_clean_up_cache(now);

		res = stat(kpp->path, &stbuf);
		if (res != 0) {
#ifdef KPDEBUG
			printf("kp_get_file: stat failed\n");
#endif
			res = _kp_errno_to_kperr(errno);
		} else if (!S_ISREG(stbuf.st_mode)) {
			res = KPERR_BADKEY;
			if (S_ISDIR(stbuf.st_mode))
				*isdirp = 1;
		} else {
#ifdef KPDEBUG
			printf("kp_get_file: kp_read_file called\n");
#endif
			res = _kp_read_file(kpp->path, &keys);
			if (res == 0) {
				res = stat(kpp->path, &stbuf);
				if (res == -1) {
					kp_free_keys(keys);
					res = _kp_errno_to_kperr(errno);
				}
			}
		}

		/* We cache nonexistent files also */
		if (res != 0 && res != KPERR_NOKEY)
			return res;

		fil = kp_new_cached_file(kpp);
		now = time(NULL);

		fil->use = now;
		fil->check = now;

		if (res == 0) {
			fil->keys = keys;
			fil->modif = stbuf.st_mtime;
		}
	}
	*fp = fil;

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Set a key in the cache. If it exists overwrite it, if not, then create
 * it. Removed (negative) keys are also inserted into the cache.
 * ------------------------------------------------------------------------- */
int _kp_cache_set(kp_path * kpp, kp_key * ck)
{
	kp_fil *fil;
	kp_key *key;
	int res;
	int isdir;

	res = kp_get_file(kpp, &fil, &isdir);
	if (res != 0)
		return res;

	if ((ck->flags & KPFL_REMOVED) != 0) {
#ifdef KPDEBUG
		printf("kp_cache_set: get cached key %s\n", kpp->path);
#endif
		key = kp_get_cached_key(fil, ck->name);

		if (key == NULL)
			return KPERR_NOKEY;
	}

	if (!(ck->flags & KPFL_REMOVED)) {
#ifdef KPDEBUG
		printf("Kp_cache_set: kp_check_insert %s\n", kpp->path);
#endif
		res = kp_check_insert(fil, ck->name, &key);
		if (res != 0)
			return res;
	}

	fil->dirty = 1;

	if (key != NULL)
		kp_value_destroy(key);
	else {
#ifdef KPDEBUG
		printf("kp_cache_set: create new %s\n", kpp->path);
#endif
		key = (kp_key *) malloc_check(sizeof(kp_key));
		key->next = fil->keys;
		fil->keys = key;
	}

	key->type = ck->type;
	key->len = ck->len;
	key->data = ck->data;
	key->name = ck->name;
	key->flags = ck->flags;
	key->flags |= KPFL_DIRTY;

	/* Data is MOVED to key, not COPIED */
	ck->data = NULL;
	ck->name = NULL;

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Get the value of a key from a given section
 * ------------------------------------------------------------------------- */
int _kp_cache_get(kp_path * kpp, const char *keyname, kpval_t type,
		  kp_key * ck)
{
	kp_fil *fil;
	kp_key *key;
	int res;
	int isdir;
#ifdef KPDEBUG
	printf("kp_cache_get\n");
#endif

	res = kp_get_file(kpp, &fil, &isdir);
	if (res != 0)
		return res;

	key = kp_get_cached_key(fil, keyname);

	if (key == NULL || key->flags & KPFL_REMOVED)
		return KPERR_NOKEY;

	if (key->flags & KPFL_BADDB)
		return KPERR_BADDB;

	if (type != KPVAL_UNKNOWN && key->type != type)
		return KPERR_BADTYPE;

	/* Data is COPIED */
	kp_value_new(ck, key->type, key->len, key->data);

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Get the type of a key from a given section
 * ------------------------------------------------------------------------- */
int _kp_cache_get_type(kp_path * kpp, char *keyname, int iskeyfile,
		       kpval_t * tp)
{
	kp_fil *fil;
	kp_key *key;
	int res;
	unsigned int keynamelen;
	int isdir;
#ifdef KPDEBUG
	printf("kp_cache_get_type\n");
#endif

	res = kp_get_file(kpp, &fil, &isdir);
	if (isdir) {
		if (iskeyfile)
			return KPERR_BADKEY;
		else {
			*tp = KPVAL_DIR;
			return 0;
		}
	}
	if (res != 0)
		return res;

	if (fil->keys == NULL)
		return KPERR_NOKEY;

	keynamelen = strlen(keyname);

	for (key = fil->keys; key != NULL; key = key->next) {
		if (strcmp(key->name, keyname) == 0) {

			if (key->flags & KPFL_REMOVED)
				return KPERR_NOKEY;

			if (key->flags & KPFL_BADDB)
				return KPERR_BADDB;

			if (iskeyfile && keynamelen == 0)
				return KPERR_BADKEY;

			*tp = key->type;
			return 0;
		}
		if (!(key->flags & KPFL_REMOVED)) {
			if (keynamelen == 0) {
				if (!iskeyfile)
					return KPERR_BADKEY;

				*tp = KPVAL_DIR;
				return 0;
			}
			if (strncmp(key->name, keyname, keynamelen) == 0 &&
			    key->name[keynamelen] == '/') {
				*tp = KPVAL_DIR;
				return 0;
			}
		}
	}

	return KPERR_NOKEY;
}

/* ------------------------------------------------------------------------- 
 * Collect the subkeys of a given key in a file
 * ------------------------------------------------------------------------- */
static int kp_get_subkeys_file(kp_path * kpp, const char *keyname,
			       struct key_array *keys)
{
	kp_fil *fil;
	kp_key *key;
	int res;
	unsigned int keynamelen;
	int found;
	char *s;
	char *ent;
	int isdir;
#ifdef KPDEBUG
	printf("kp_cache_get_subkeys_file\n");
#endif

	res = kp_get_file(kpp, &fil, &isdir);
	if (res != 0)
		return res;

	if (fil->keys == NULL)
		return KPERR_NOKEY;

	keynamelen = strlen(keyname);

	found = 0;
	for (key = fil->keys; key != NULL; key = key->next) {
		if ((key->flags & KPFL_REMOVED) != 0)
			continue;

		if (keynamelen == 0 ||
		    (strlen(key->name) > keynamelen
		     && key->name[keynamelen] == '/'
		     && strncmp(key->name, keyname, keynamelen) == 0)) {

			s = key->name + keynamelen;
			if (*s == '/')
				s++;

			ent = strdup_check(s);
			s = ent;
			while (*s && *s != '/')
				s++;

			*s = '\0';

			_kp_add_subkey_check(keys, ent);
			free(ent);

			found = 1;
		}
	}

	if (!found)
		return KPERR_NOKEY;

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Get the subkeys of a given key, from a directory or from a file
 * ------------------------------------------------------------------------- */
int _kp_cache_get_subkeys(kp_path * kpp, const char *keypath,
			  int iskeyfile, struct key_array *keys)
{
	int res;
#ifdef KPDEBUG
	printf("kp_cache_get_subkeys\n");
#endif

	if (!iskeyfile) {
		res = _kp_get_subkeys_dir(kpp->path, keys);
	} else
		res = kp_get_subkeys_file(kpp, keypath, keys);

	return res;
}

/* ------------------------------------------------------------------------- 
 * Flush the dirty files in the cache to the disk
 * ------------------------------------------------------------------------- */
int _kp_cache_flush()
{
	int res;
	int finalres = 0;
	kp_fil *fil;
#ifdef KPDEBUG
	printf("kp_cache_flush\n");
#endif
	for (fil = kp_cachef; fil != NULL; fil = fil->next) {
		if (fil->dirty) {
			res = _kp_write_file(&fil->kpp, fil->keys);

			/* The return value of the flush will be the value of the
			   last error that occured */

			if (res != 0)
				finalres = res;

			fil->dirty = 0;
		}
	}

	return finalres;
}

/* End of kp_cache.c */
