/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <sys/poll.h>
#include "neostats.h"
#include "hash.h"

/* 
 *   Ensure RTLD flags correctly defined
 */
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY	/* openbsd deficiency */
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif

/* @brief Module Socket List hash
 * 
 */
extern hash_t *sockh;

/* 
 * Prototypes
 */
int InitModules (void);
int FiniModules (void);
void SendModuleEvent (char * event, char **av, int ac);
int load_module (char *path, User * u);
int unload_module (char *module_name, User * u);
int list_modules (User * u, char **av, int ac);
int get_dl_handle (char *mod_name);
int get_mod_num (char *mod_name);
Module *get_mod_ptr (char *mod_name);
void unload_modules(void);

void ModulesVersion (const char* nick, const char *remoteserver);

void *ns_dlsym (void *handle, const char *name);
void *ns_dlopen (const char *file, int mode);
int ns_dlclose (void *handle);
char *ns_dlerror (void);


#endif /* !_dl_h_ */
