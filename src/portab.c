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

/* @file Portability wrapper functions
 */

#include "neostats.h"
#ifdef HAVE_SYS_POLL_H 
#include <poll.h>
#endif

int sys_mkdir (const char *filename, mode_t mode)
{
#ifdef WIN32
	return mkdir (filename);
#else
	return mkdir (filename, mode);
#endif
}

int sys_close_sock (SYS_SOCKET sock)
{
#ifdef WIN32
	return closesocket (sock);
#else
	return close (sock);
#endif      			
}

int sys_write_sock (SYS_SOCKET s, const char* buf, int len)
{
#ifdef WIN32
	return send (s, buf, len, 0);
#else	
	return write (s, buf, len);
#endif
}

int sys_read_sock (SYS_SOCKET s, char* buf, int len)
{
#ifdef WIN32
	return recv (s, buf, len, 0);
#else
	return read (s, buf, len);
#endif
}

int sys_set_nonblocking_sock (SYS_SOCKET s)
{
	int flags;
#ifdef WIN32

	flags = 1;
	return ioctlsocket(s, FIONBIO, &flags);
#else
	flags = fcntl(s, F_GETFL, 0);
	flags |= O_NONBLOCK;
	return fcntl (s, F_SETFL, flags);
#endif
}
