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
int InitModuleHash (void);
void SendModuleEvent (char * event, char **av, int ac);
int load_module (char *path, User * u);
int unload_module (char *module_name, User * u);
int list_modules (User * u, char **av, int ac);
int list_bots (User * u, char **av, int ac);
ModUser* add_mod_user (char *nick, char *mod_name);
ModUser* add_neostats_mod_user (char *nick);
int del_mod_user (char *nick);

int list_timers (User * u, char **av, int ac);
void run_mod_timers (void);
int list_sockets (User * u, char **av, int ac);
int get_dl_handle (char *mod_name);

int list_bot_chans (User * u, char **av, int ac);
int get_mod_num (char *mod_name);
Module *get_mod_ptr (char *mod_name);
void unload_modules(void);
void verify_hashes(void);

int add_bot_cmd_list(ModUser *bot_ptr, bot_cmd *bot_cmd_list);
int del_bot_cmd_list(ModUser *bot_ptr, bot_cmd *bot_cmd_list);
int run_bot_cmd (ModUser *bot_ptr, User *u, char **av, int ac);
ModUser * init_mod_bot (char * nick, char * user, char * host, char * realname, const char *modes, unsigned int flags, bot_cmd *bot_cmd_list, bot_setting *bot_setting_list, char * modname);
int CloakHost (ModUser *bot_ptr);
int del_mod_bot (ModUser *bot_ptr, char * reason);
void finiModuleHash();
void ModulesVersion (const char* nick, const char *remoteserver);

void *ns_dlsym (void *handle, const char *name);
void *ns_dlopen (const char *file, int mode);
int ns_dlclose (void *handle);
char *ns_dlerror (void);


#endif /* !_dl_h_ */
