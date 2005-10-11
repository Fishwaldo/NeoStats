/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
** $Id: misc.c 2019 2004-08-13 20:19:20Z Mark $
*/

#include "neostats.h"
#include "main.h"

/** @brief ns_malloc
 *
 *  NeoStats wrapper for malloc.
 *  Allocates memory for internal variables. 
 *  If enough memory can't be allocated, exit the program 
 *
 *  @param size of memory to alloc
 *
 *  @return pointer to allocated buffer
 */

void *ns_malloc( size_t size )
{
	void *buf;

	if( !size )
	{
		dlog( DEBUG2, "ns_malloc: illegal attempt to allocate 0 bytes!" );
		size = 1;
	}
	buf = malloc( size );
	if( !buf )
	{
		nlog( LOG_CRITICAL, "ns_malloc: out of memory." );
		do_exit( NS_EXIT_ERROR, "Out of memory" );
	}
	return buf;
}

/** @brief ns_calloc
 *
 *	NeoStats wrapper for calloc.
 *	Allocates memory for internal variables. 
 *	If enough memory can't be allocated, exit the program 
 *
 *	@param size of memory to alloc
 *
 *	@return pointer to allocated buffer
 */

void *ns_calloc( size_t size )
{
	void *buf;

	if( !size )
	{
		dlog( DEBUG2, "ns_calloc: illegal attempt to allocate 0 bytes!" );
		size = 1;
	}
	buf = calloc( 1, size );
	if( !buf )
	{
		nlog( LOG_CRITICAL, "ns_calloc: out of memory." );
		do_exit( NS_EXIT_ERROR, "Out of memory" );
	}
	return buf;
}

/** @brief ns_realloc
 *
 *  NeoStats wrapper for realloc.
 *  Reallocates memory
 *  If enough memory can't be allocated, exit the program 
 *
 *  @param ptr to existing allocation
 *  @param size of memory to realloc
 *
 *  @return pointer to allocated buffer
 */

void *ns_realloc( void *ptr, size_t size )
{
	void *newptr;

	newptr = realloc( ptr, size );
	if( !newptr )
	{
		nlog( LOG_CRITICAL, "ns_realloc: out of memory." );
		do_exit( NS_EXIT_ERROR, "Out of memory" );
	}
	return newptr;
}

/** @brief _ns_free
 *
 *  NeoStats wrapper for free.
 *  Free memory associated with pointer.
 *  If NULL pointer log error and ignore free
 *
 *  @param ptr to buffer to free
 *
 *  @returns none
 */

void _ns_free( void **ptr )
{
	if( !*ptr )
	{
		dlog( DEBUG2, "ns_free: illegal attempt to free NULL pointer" );
		return;
	}
	free( *ptr );
	*ptr = 0;
}
