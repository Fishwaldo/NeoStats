/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000 - 2001 ^Enigma^
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

/** @file dl.c 
 *  @brief module functions
 */ 

#ifndef WIN32
#include <dlfcn.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "neostats.h"
#include "dl.h"

void *ns_dlsym (void *handle, const char *name)
{
#ifdef WIN32
	return NULL;
#else
#ifdef NEED_UNDERSCORE_PREFIX
	char sym[128];
	void* ret;
	ret = dlsym ((int *) handle, name);
	if (ret == NULL) {
		ircsnprintf(sym, 128, "_%s", name);
		return (dlsym ((int *) handle, sym));
	}
	return ret;
#else
	return (dlsym ((int *) handle, name));
#endif
#endif
}

void *ns_dlopen (const char *file, int mode)
{
#ifdef WIN32
	return NULL;
#else
	return (dlopen (file, mode));
#endif
}

int ns_dlclose (void *handle)
{
#ifdef WIN32
	return 0;
#else
	return (dlclose (handle));
#endif
}

char *ns_dlerror (void)
{
#ifdef WIN32
	return NULL;
#else
	return ((char *)dlerror ());
#endif
}

