/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
** $Id$
*/

#ifndef _dl_h_
#define _dl_h_

/*
 * dl.h
 * dynamic runtime library loading routines
 */

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

/* 
 *   Ensure RTLD flags correctly defined
 */
#ifndef RTLD_NOW
#define RTLD_NOW  1 
#endif
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY	/* openbsd deficiency */
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif

#ifdef WIN32
#ifndef NDEBUG
/* The extra d in debug mode uses debug versions of DLLs */
#define MOD_EXT	"d.dll"
#else
#define MOD_EXT	".dll"
#endif
#else
#define MOD_EXT	".so"
#endif

/* 
 * Prototypes
 */
void *ns_dlsym( void *handle, const char *name );
void *ns_dlopen( const char *file, int mode );
int ns_dlclose( void *handle );
char *ns_dlerror( void );

char *ns_dlerrormsg;

#endif /* !_dl_h_ */
