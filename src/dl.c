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

#ifndef HAVE_LIBDL
char *ns_dlerrormsg;
#else /* HAVE_LIBDL */
char *ns_dlerrormsg;
#endif /* HAVE_LIBDL */

void *ns_dlsym (void *handle, const char *name)
{
#ifdef WIN32
	void* ret;

	ret = (void*)GetProcAddress((HMODULE)handle, name);
	return ret;
#else
#ifdef NEED_UNDERSCORE_PREFIX
	static char sym[128];
	void* ret;

	/* reset error */
	ns_dlerrormsg = 0;
	ret = dlsym ((int *) handle, name);
	/* Check with underscore prefix */
	if (ret == NULL) {
		ircsnprintf(sym, 128, "_%s", name);
		ret = dlsym ((int *) handle, sym);
	}
	/* Check for error */
#ifndef HAVE_LIBDL
	if(ret == NULL) {
		ns_dlerrormsg = ns_dlerror ();
#else /* HAVE_LIBDL */
	if ((ns_dlerrormsg = ns_dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		return NULL;
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
	void* ret;

	/* reset error */
	ns_dlerrormsg = 0;
	ret = (void*)LoadLibrary(file);
	/* Check for error */
	if(ret == NULL)
	{
	}
	return ret;
#else
	void* ret;

	/* reset error */
	ns_dlerrormsg = 0;
	ret = dlopen (file, mode);
	/* Check for error */
#ifndef HAVE_LIBDL
	if(ret == NULL) {
		ns_dlerrormsg = ns_dlerror ();
#else /* HAVE_LIBDL */
	if ((ns_dlerrormsg = ns_dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		return NULL;
	}
	return ret;
#endif
}

int ns_dlclose (void *handle)
{
#ifdef WIN32
	FreeLibrary((HMODULE)handle);
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

