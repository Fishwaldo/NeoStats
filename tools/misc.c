/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include <stdlib.h>
#include <string.h>

/** @brief NeoStats wrapper for malloc.
 *
 * Allocates memory for internal variables. 
 * If enough memory can't be allocated, exit the program 
 *
 * @param size The amount of memory to alloc
 *
 * @returns pointer to allocated buffer
 */

void * smalloc ( const int size )
{
	unsigned int allocsize;
	void *buf;

	allocsize = size;
	if (!allocsize) {
		allocsize = 1;
	}
	buf = malloc (allocsize);
	if (!buf) {
		abort();
	}
	return buf;
}

/** @brief NeoStats wrapper for calloc.
 *
 * Allocates memory for internal variables. 
 * If enough memory can't be allocated, exit the program 
 *
 * @param size The amount of memory to alloc
 *
 * @returns pointer to allocated buffer
 */

void * scalloc ( const int size )
{
	void *buf;
	unsigned int allocsize;

	allocsize = size;
	if (!allocsize) {
		allocsize = 1;
	}
	buf = calloc (1, allocsize);
	if (!buf) {
		abort();
	}
	return buf;
}

/** @brief NeoStats wrapper for realloc.
 *
 * Reallocates memory
 * If enough memory can't be allocated, exit the program 
 *
 * @param size The amount of memory to realloc
 *
 * @returns pointer to allocated buffer
 */

void * srealloc ( void* ptr, const int size )
{
	void* newptr;

	newptr = realloc (ptr, size);
	if (!newptr) {
		abort();
	}
	return newptr;
}

/** @brief NeoStats wrapper for free.
 *
 * Free memory associated with pointer.
 * If NULL pointer log error and ignore free
 *
 * @param size Pointer to buffer to free
 *
 * @returns none
 */

void sfree ( void *buf )
{
	if (!buf) {
		return;
	}
	free (buf);
	buf = 0;
}

/** @brief Duplicate a string
 *
 * make a copy of a string, with memory allocated for the new string
 *
 * @param s a pointer to the string to duplicate
 *
 * @returns a pointer to the new string
 *
 * @deprecated  Try not to use this function, it will go away one day
 *
 */

char *
sstrdup (const char *s)
{
	char *t = strdup (s);
	if (!t) {
		abort();
	}
	return t;
}

