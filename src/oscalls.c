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

/* @file Portability wrapper functions
 */

/*  TODO:
 *  - port file functions from CRT to Win32 native calls (CreateFile etc)
 */

#include "neostats.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif

int os_errno;
static char tempbuf[BUFSIZE*2];

/*
 *  Wrapper function for mkdir
 */

int os_mkdir( const char *filename, mode_t mode )
{
	int retval;

#ifdef WIN32
	retval = mkdir( filename );
#else
	retval = mkdir( filename, mode );
#endif
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for check_create_dir
 */

int os_check_create_dir( const char *dirname )
{
	struct stat st;
	int res;

	/* first, make sure the logdir dir exists */
	res = stat( dirname, &st );
	if( res != 0 ) {
		/* hrm, error */
		if( errno == ENOENT ) {
			/* ok, it doesn't exist, create it */
			res = os_mkdir( dirname, 0700 );
			if( res != 0 ) {
				/* error */
				nlog( LOG_CRITICAL, "Couldn't create directory: %s", strerror( errno ) );
				return NS_FAILURE;
			}
			nlog( LOG_NOTICE, "Created directory: %s", dirname );
		} else {
			nlog( LOG_CRITICAL, "Stat returned error: %s", strerror( errno ) );
			return NS_FAILURE;
		}
	} else if( !S_ISDIR( st.st_mode ) )	{
		nlog( LOG_CRITICAL, "%s is not a Directory", dirname );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/*
 *  Wrapper function for fopen
 */

FILE* os_fopen( const char *filename, const char *filemode )
{
	FILE *retval;

	retval = fopen( filename, filemode );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fclose
 */

int os_fclose( FILE *handle )
{
	int retval;

	retval = fclose( handle );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fseek
 */

int os_fseek( FILE *handle, long offset, int origin )
{
	int retval;

	retval = fseek( handle, offset, origin );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for ftell
 */

long os_ftell( FILE *handle )
{
	int retval;

	retval = ftell( handle );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fprintf
 */

int os_fprintf( FILE *handle, char *fmt, ... )
{
	int retval;
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( tempbuf, BUFSIZE*2, fmt, ap );
	va_end( ap );
	retval = fprintf( handle, "%s", tempbuf );	
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fread
 */

int os_fread( void *buffer, size_t size, size_t count, FILE* handle )
{
	int retval;

	retval = fread( buffer, size, count, handle );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fgets
 */

char *os_fgets( char *string, int n, FILE* handle )
{
	char *retval;

	retval = fgets( string, n, handle );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fwrite
 */

int os_fwrite( const void *buffer, size_t size, size_t count, FILE* handle )
{
	int retval;

	retval = fwrite( buffer, size, count, handle );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fflush
 */

int os_fflush( FILE *handle )
{
	int retval;

	retval = fflush( handle );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for rename
 */

int os_rename( const char* oldname, const char* newname )
{
	int retval;
    
	/*  WIN32 does not allow rename if the file exists 
	 *  Behaviour is undefined on various systems so 
	 *  remove on all platforms
	 */
	remove( newname );
	retval = rename( oldname, newname );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for stat
 */

int os_stat( const char *path, struct stat *buffer )
{
	int retval;

	retval = stat( path, buffer );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for access
 */

int os_access( const char *path, int mode )
{
	int retval;

	retval = access( path, mode );
	os_errno = errno;
	return retval;
}

/*
 *  Wrapper function for strerror
 */

char *os_strerror( void )
{
	return strerror( errno );
}

/*
 *  Wrapper function for file_get_size
 */

int os_file_get_size( const char* filename )
{
	struct stat st;
	int res;

	res = stat( filename, &st );
	if( res != 0 ) {
		if( errno == ENOENT ) {
			nlog( LOG_CRITICAL, "No such file: %s", filename );
			return -1;
		} else {
			nlog( LOG_CRITICAL, "File error: %s", os_strerror(  ) );
			return -1;
		}
	}
	return st.st_size;
}

/*
 *  Wrapper function for sock_close
 */

int os_sock_close( OS_SOCKET sock )
{
#ifdef WIN32
	return closesocket( sock );
#else
	return close( sock );
#endif      			
}

/*
 *  Wrapper function for sock_write
 */

int os_sock_write( OS_SOCKET s, const char* buf, int len )
{
#ifdef WIN32
	return send( s, buf, len, 0 );
#else	
	return write( s, buf, len );
#endif
}

/*
 *  Wrapper function for sock_read
 */

int os_sock_read( OS_SOCKET s, char* buf, int len )
{
#ifdef WIN32
	return recv( s, buf, len, 0 );
#else
	return read( s, buf, len );
#endif
}

/*
 *  Wrapper function for sock_set_nonblocking
 */

int os_sock_set_nonblocking( OS_SOCKET s )
{
	int flags;

#ifdef WIN32
	flags = 1;
	return ioctlsocket( s, FIONBIO, &flags );
#else
	flags = fcntl( s, F_GETFL, 0 );
	flags |= O_NONBLOCK;
	return fcntl( s, F_SETFL, flags );
#endif
}

/*
 *  Wrapper function for strftime
 */

size_t os_strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr )
{
	return strftime( strDest, maxsize, format, timeptr );
}

/*
 *  Wrapper function for localtime
 */

struct tm *os_localtime( const time_t *timer )
{
	return localtime( timer );
}

/*
 *  Wrapper function for memset
 */

void *os_memset( void *dest, int c, size_t count )
{
	return memset( dest, c, count );
}

/*
 *  Wrapper function for memcpy
 */

void *os_memcpy( void *dest, const void *src, size_t count )
{
	return memcpy( dest, src, count );
}

/*
 *  Wrapper function for malloc
 */

void *os_malloc( size_t size )
{
	return malloc( size );
}

/*
 *  Wrapper function for free
 */

void os_free( void *ptr )
{
	free( ptr );
}
