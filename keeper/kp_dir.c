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

/* Structure containing the key path of the opened directory, and 
   the last returned subkey path. This structure is opaque to the user */
struct _KPDIR {
    char *basepath;
    char *retpath;
};

/* ------------------------------------------------------------------------- 
 * Open a key directory
 * ------------------------------------------------------------------------- */
KPDIR * kp_dir_open    (const char *keypath)
{
    KPDIR *kpdir;

    kpdir = (KPDIR *) malloc_check(sizeof(KPDIR));
    kpdir->basepath = strdup_check(keypath);
    kpdir->retpath = NULL;

    return kpdir;
}

/* ------------------------------------------------------------------------- 
 * Append the subkey name to the base diretory, and return it. Free
 * the previously used return string. 
 * ------------------------------------------------------------------------- */
const char * kp_dir_getpath (KPDIR *kpdir, const char *subkey)
{
    if(kpdir != NULL) {
        free(kpdir->retpath);
        kpdir->retpath = (char *) malloc_check(strlen(kpdir->basepath)+1
                                               +strlen(subkey)+1);
        sprintf(kpdir->retpath, "%s/%s", kpdir->basepath, subkey);

        return kpdir->retpath;
    }
    else
        return subkey;
}

/* ------------------------------------------------------------------------- 
 * Free all allocated space in this KPDIR structure
 * ------------------------------------------------------------------------- */
void    kp_dir_close   (KPDIR *kpdir)
{
    free(kpdir->retpath);
    free(kpdir->basepath);
    free(kpdir);
}

/* End of kp_dir.c */
