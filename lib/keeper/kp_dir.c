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

/* Structure containing the key path of the opened directory, and 
   the last returned subkey path. This structure is opaque to the user */
struct _KPDIR {
	char *basepath;
	char *retpath;
};

/* ------------------------------------------------------------------------- 
 * Open a key directory
 * ------------------------------------------------------------------------- */
KPDIR *kp_dir_open(const char *keypath)
{
	KPDIR *kpdir;

	kpdir = (KPDIR *) ns_malloc(sizeof(KPDIR));
	kpdir->basepath = sstrdup(keypath);
	kpdir->retpath = NULL;

	return kpdir;
}

/* ------------------------------------------------------------------------- 
 * Append the subkey name to the base diretory, and return it. Free
 * the previously used return string. 
 * ------------------------------------------------------------------------- */
const char *kp_dir_getpath(KPDIR * kpdir, const char *subkey)
{
	if (kpdir != NULL) {
		ns_free(kpdir->retpath);
		kpdir->retpath =
		    (char *) ns_malloc(strlen(kpdir->basepath) + 1 +
					  strlen(subkey) + 1);
		sprintf(kpdir->retpath, "%s/%s", kpdir->basepath, subkey);

		return kpdir->retpath;
	} else
		return subkey;
}

/* ------------------------------------------------------------------------- 
 * Free all allocated space in this KPDIR structure
 * ------------------------------------------------------------------------- */
void kp_dir_close(KPDIR * kpdir)
{
	ns_free(kpdir->retpath);
	ns_free(kpdir->basepath);
	ns_free(kpdir);
}

/* End of kp_dir.c */
