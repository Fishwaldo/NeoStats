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

#include "keeper.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREAD_MUTEX
#define _REENTRANT
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef ALLOC_CHECK
extern void *_iamalloc(size_t size, int id);
extern void *_iarealloc(void *ptr, size_t size, int id);
extern void  _iafree(void *ptr, int id);
#define malloc(size)        _iamalloc(size, 4)
#define free(ptr)           _iafree(ptr, 4)
#define realloc(ptr, size)  _iarealloc(ptr, size, 4)
#endif

#define SEP1 ':'
#define SEP2 '='

#define ROUND_TO(x, r) (((x) + ((r) - 1)) & ~((r) - 1))

#ifdef HAVE_PTHREAD_MUTEX
#define KP_MUTEX(name) static pthread_mutex_t name = PTHREAD_MUTEX_INITIALIZER
#define KP_LOCK(name)   pthread_mutex_lock(&name)
#define KP_UNLOCK(name) pthread_mutex_unlock(&name)
#else
/* ------------------------------------------------------------------------- 
 * Pseudo locking function that can (sometimes) detect lock violation in
 * case the proper locking was not compied into keeper
 * ------------------------------------------------------------------------- */
static inline int kp_check_lock(int l)
{
    if(l != 0) {
        fprintf(stderr,
                "keeper: Error: Locking violation. Please recompile keeper\n"
                "               library with pthread support enabled\n");
        abort();
    }
    return 1;
}

#define KP_MUTEX(name)  static volatile int name = 0
#define KP_LOCK(name)   name = kp_check_lock(name)
#define KP_UNLOCK(name) name = 0
#endif

/* Key list element used to store keys in the cache */
typedef struct _kp_key {
    kpval_t type;
    unsigned int len;
    void *data;

    char *name;
    int  flags;
    struct _kp_key *next;
} kp_key;

/* Path of a database file, and the section that it belongs to */
typedef struct {
    char *path;
    int dbindex;
} kp_path;

/* Key name array used to collect subkeys in a key directory */
struct key_array {
    char **array;
    unsigned int num;
    unsigned int strsize;
};

/* Flags for cached keys */
#define KPFL_DIRTY   (1 << 0)  /* key was modified */
#define KPFL_REMOVED (1 << 1)  /* key was removed (negative) */
#define KPFL_BADDB   (1 << 2)  /* database contained an error for this key */

/* ------------------------------------------------------------------------- 
 * This function check if an allocation was successful. If not it aborts
 * the program. I think this is OK, because keeper only allocates very
 * small amounts of memory, and if that fails there will probably be
 * greater problems in the program itself. If you don't like this
 * don't use keeper :)
 * ------------------------------------------------------------------------- */
static inline void *check_ptr(void *ptr)
{
    if(ptr == NULL) {
        fprintf(stderr, "keeper: Out of Memory!\n");
        abort();
    }
    return ptr;
}

/* ------------------------------------------------------------------------- 
 * Malloc memory and check if successful
 * ------------------------------------------------------------------------- */
static inline void *malloc_check(size_t size)
{
    return check_ptr(malloc(size ? size : 1));
}

/* ------------------------------------------------------------------------- 
 * Strdup a string and check if successful
 * ------------------------------------------------------------------------- */
static inline char *strdup_check(const char *s)
{
    return strcpy((char *) check_ptr(malloc(strlen(s) + 1)), s);
}

/* ------------------------------------------------------------------------- 
 * Create a new key list element, and allocate space for the value data
 * ------------------------------------------------------------------------- */
static inline void kp_value_new(kp_key *ck, kpval_t type,
                                unsigned int len, const void *data)
{
    ck->type = type;
    ck->len  = len;
    if(type != KPVAL_UNKNOWN)
        ck->data = malloc_check(ck->len+1);
    else
        ck->data = NULL;

    ck->flags = 0;
    ck->name = NULL;
    ck->next = NULL;

    if(data != NULL)
        memcpy(ck->data, data, ck->len);
}

/* ------------------------------------------------------------------------- 
 * Free allocated data in the key list element. Doesn't free the element
 * itself
 * ------------------------------------------------------------------------- */
static inline void kp_value_destroy(kp_key *ck)
{
    if(ck->data != NULL)
        free(ck->data);
    if(ck->name != NULL)
        free(ck->name);
}

/* ------------------------------------------------------------------------- 
 * Check if the second argument is a subkey of the first (i.e if it begins
 * with the same path components)
 * ------------------------------------------------------------------------- */
static inline int kp_is_subkey(char *key, char *subkey)
{
    unsigned int keylen = strlen(key);

    if(keylen == 0)
        return 1;

    if(strlen(subkey) > keylen &&
       strncmp(subkey, key, keylen) == 0 &&
       subkey[keylen] == '/')
        return 1;

    return 0;
}

/* Internal functions that are used across modules */
extern kpval_t _kp_type_from_code(int c);
extern int   _kp_get_path(const char *keypath, kp_path *kpp, char **keynamep,
                         int *iskeyfile);
extern int   _kp_errno_to_kperr(int en);
extern char *_kp_get_line(FILE *fp, char **valuep);
extern int   _kp_lock_file(int dbindex, int iswrite);
extern void  _kp_unlock_file(int lockfd);
extern int   _kp_get_ibeg(int dbindex);
extern char *_kp_get_tmpfile(int dbindex);
extern void  _kp_add_subkey_check(struct key_array *keys, char *name);
extern int   _kp_read_file(char *path, kp_key **ksp);
extern int   _kp_get_subkeys_dir(char *path, struct key_array *keys);
extern int   _kp_write_file(kp_path *kpp, kp_key *keys);
extern int   _kp_cache_get(kp_path *kpp, const char *keyname, kpval_t type,
                           kp_key *ck);
extern int   _kp_cache_get_type(kp_path *kpp, char *keyname, int iskeyfile,
                                kpval_t *tp);
extern int   _kp_cache_get_subkeys(kp_path *kpp, const char *keypath,
                                   int iskeyfile, struct key_array *keys);
extern int   _kp_cache_set(kp_path *kpp, kp_key *ck);
extern int   _kp_cache_flush(void);

/* End of kp_util.h */
