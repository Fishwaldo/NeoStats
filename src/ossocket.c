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
** $Id: oscalls.c 2366 2005-03-18 12:29:37Z Fish $
*/

/* @file Portability wrapper functions
 */

/*
 *  Wrapper function for sock_close
 */

#include "neostats.h"

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
