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

#ifndef _KEEPER_H
#define _KEEPER_H

/* define this to debug some functions in keeper */
#undef KPDEBUG

typedef enum {
	KPVAL_UNKNOWN = -1,
	KPVAL_DIR,
	KPVAL_DATA,
	KPVAL_STRING,
	KPVAL_INT,
	KPVAL_FLOAT
} kpval_t;

typedef enum {
	KPERR_OK,		/* OK                     */
	KPERR_NOKEY,		/* Key does not exist     */
	KPERR_NOACCES,		/* Access denied          */
	KPERR_BADKEY,		/* Bad key                */
	KPERR_BADTYPE,		/* Bad type               */
	KPERR_BADDB,		/* Bad database           */
	KPERR_NOSPACE		/* Not enough resources   */
} kperr_t;

#ifdef __cplusplus
extern "C" {
#endif

/* ==== Basic functions ==== */

/* Get functions */
	int kp_get_string(const char *keypath, char **stringp);
	int kp_get_int(const char *keypath, int *intp);
	int kp_get_float(const char *keypath, double *floatp);
	int kp_get_data(const char *keypath, void **datap,
			unsigned int *lenp);
	int kp_get_dir(const char *keypath, char ***keysp,
		       unsigned int *nump);

/* Set functions */
	int kp_set_string(const char *keypath, const char *string);
	int kp_set_int(const char *keypath, int intval);
	int kp_set_float(const char *keypath, double floatval);
	int kp_set_data(const char *keypath, const void *data,
			unsigned int len);

/* Misc functions */
	int kp_remove(const char *keypath);
	int kp_flush(void);
	int kp_get_type(const char *keypath, kpval_t * typep);
	char *kp_strerror(int kperr);
	void kp_exit();


/* ==== Utility functions ==== */

/* Directory utils */
	typedef struct _KPDIR KPDIR;
#define KP_PATH(kpdir, subkey) kp_dir_getpath(kpdir, subkey)

	KPDIR *kp_dir_open(const char *keypath);
	const char *kp_dir_getpath(KPDIR * kpdir, const char *subkey);
	void kp_dir_close(KPDIR * kpdir);

/* Enum utils */
	int kp_get_enum(const char *keypath, const char *names[],
			int *valp);
	int kp_set_enum(const char *keypath, const char *names[], int val);

	int kp_get_bool(const char *keypath, int *boolp);
	int kp_set_bool(const char *keypath, int boolval);

/* Sort keys in alphabetical order */
	void kp_sort_keys(char **keys, unsigned int numkeys);

/* Do a function on all subkeys*/
	typedef int (*kp_func) (const char *key, void *user_data);
	int kp_recursive_do(const char *key, kp_func func, int stop_on_err,
			    void *user_data);

#ifdef __cplusplus
}
#endif
#endif				/* _KEEPER_H */
