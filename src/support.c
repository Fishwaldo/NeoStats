/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* @file potentially missing C library functions
 */

#include "neostats.h"

#ifndef HAVE_STRNLEN
/* @brief find length of string up to count max
 *
 * @return length of string excluding NULL or count if longer
 *
 */
size_t strnlen( const char * src, size_t count )
{
	size_t len;

	/* Run through the string until we find NULL or reach count */
	for( len = 0; len < count; len++, src++ ) {
		if( *src == 0 ){
			return len;
		}
	}
	/* src is longer or equal to count so return count */
	return count;
}
#endif /* HAVE_STRNLEN */

#ifndef HAVE_STRLCPY
/* @brief copy up to size-1 characters from src to dst and NULL terminate
 *
 * @return total characters written to string.
 *
 */
size_t strlcpy( char *dst, const char *src, size_t size )
{
	size_t copycount;

    /* check size is safe */
	if ( size <= 0 )
		return 0;
	/* NULL pointer checks */
	if( !dst || !src )
		return 0;
	/* use strnlen so huge strings do not hold us up */
	for( copycount = 0; copycount < size-1 && *src!=0 ; copycount++ )
		*dst++=*src++;
    /* Always null terminate */
	*dst = 0;
    /* count of characters written excluding NULL terminator */
	return copycount;
}
#endif /* HAVE_STRLCPY */

#ifndef HAVE_STRLCAT
/* @brief append at most size-len( dst )-1 chars from src to dst and NULL terminate
 *
 * @return total characters written to string.
 *
 */
size_t strlcat( char *dst, const char *src, size_t size )
{
	size_t lendst;
	size_t copycount;

    /* check size is safe */
	if ( size <= 0 )
		return 0;
	/* NULL pointer checks */
	if( !dst || !src )
		return 0;
	/* if src contains NULL just NULL dst then quit to save a little CPU */
	if( *src == '\0' )
	{
		lendst = strnlen( dst, size );
		dst[lendst] = '\0';
		return 0;
	}
	/* use strnlen so huge strings do not hold us up */
	lendst = strnlen( dst, size );
	copycount = strnlen( src, size );
	/* bound copy size */
	if ( lendst + copycount >= size )
		copycount = size - lendst;
	/* memcpy the desired amount */
	if ( copycount > 0 ) 
	{
		os_memcpy( ( dst + lendst ), src, copycount );
		dst[lendst + copycount] = 0;
	}

    /* count of characters written excluding NULL terminator */
	return copycount;
}
#endif /* HAVE_STRLCAT */

#ifndef HAVE_STRNDUP
/* @brief allocate RAM and duplicate the passed string into the created buffer. 
 *  Always NULL terminates the new string.
 *  Suitable for partial string copies.
 *  Returned string will be count + 1 in length
 *
 * @return pointer to new string or NULL if failed to allocate
 *
 */
char *strndup( const char *src, size_t count )
{
	char *dup;
	
	/* validate inputs */
	if ( ( src == NULL ) || ( count < 0 ) )
		return NULL;
	/* Allocate count plus one for trailing NULL */
	dup = ( char* ) ns_malloc( count + 1 );
	/* Copy string into created buffer */
	os_memcpy( dup, src, count );
	dup[count] = 0;
	/* Return pointer to duplicated string */
	return dup;
}
#endif

#ifndef HAVE_STRDUP
/* @brief allocate RAM and duplicate the passed string into the created buffer. 
 *
 * @return pointer to new string or NULL if failed to allocate
 *
 */
char *strdup( const char *src )
{
	char *dup;
	
	/* validate inputs */
	if ( src == NULL )
		return NULL;
	/* Allocate count plus one for trailing NULL */
	dup = ( char* )ns_malloc( strlen( src ) + 1 );
	/* Copy string into created buffer */
	strcpy( dup, src );
	/* Return pointer to duplicated string */
	return dup;
}
#endif

#ifndef HAVE_INET_NTOP
char *inet_ntop( int af, const unsigned char *src, char *dst, size_t size )
{ 
	static const char *fmt = "%u.%u.%u.%u";
	char tmp[sizeof ( "255.255.255.255" )];

	if ( ( size_t ) sprintf( tmp, fmt, src[0], src[1], src[2], src[3] ) >= size )
		return NULL;
	strlcpy( dst, tmp, size );
	return dst;
}
#endif

#ifndef HAVE_INET_ATON
/* Convert from "a.b.c.d" IP address string into an in_addr structure.  
 * Return 0 on failure, 1 on success.
 */
int inet_aton( const char *name, struct in_addr *addr )
{
    addr->s_addr = inet_addr( name );
    return ( addr->s_addr == INADDR_NONE  ? 0 : 1 );
}
#endif
