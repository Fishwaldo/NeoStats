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

#ifndef _SUPPORT_H_
#define _SUPPORT_H_

#ifndef HAVE_STRNLEN
EXPORTFUNC size_t strnlen( const char * src, size_t count );
#endif /* HAVE_STRNLEN */
#ifndef HAVE_STRLCPY
EXPORTFUNC size_t strlcpy( char *dst, const char *src, size_t size );
#endif /* HAVE_STRLCPY */
#ifndef HAVE_STRLCAT
EXPORTFUNC size_t strlcat( char *dst, const char *src, size_t size );
#endif /* HAVE_STRLCAT */
#ifndef HAVE_STRNDUP
EXPORTFUNC char *strndup( const char *src, size_t count );
#endif /* HAVE_STRNDUP */
#ifndef HAVE_INET_NTOP
EXPORTFUNC char *inet_ntop( int af, const unsigned char *src, char *dst, size_t size ); 
#endif /* HAVE_INET_NTOP */

#endif /* _SUPPORT_H_ */
