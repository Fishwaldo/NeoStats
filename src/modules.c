/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

#include "neostats.h"
#include "dl.h"
#include "timer.h"
#include "sock.h"
#include "services.h"

/** @brief Module list
 * 
 */
static Module *ModList[NUM_MODULES];

int del_all_bot_cmds(Bot* bot_ptr);

/* @brief Module hash list */
static hash_t *mh;

#ifdef SQLSRV

char sqlbuf[BUFSIZE];

void *display_module_name (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->name, MAX_MOD_NAME);
	return sqlbuf;	
}

void *display_module_desc (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->description, BUFSIZE);
	return sqlbuf;
}

void *display_module_version (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->version, BUFSIZE);
	return sqlbuf;
}

void *display_module_builddate (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	ircsnprintf(sqlbuf, BUFSIZE, "%s - %s", mod_ptr->info->build_date, mod_ptr->info->build_time);
	return sqlbuf;
}

void *display_core_info (void *tbl, char *col, char *sql, void *row) 
{
	ircsnprintf(sqlbuf, BUFSIZE, "%s", me.versionfull);
	return sqlbuf;	
}

COLDEF neo_modulecols[] = {
	{
		"modules",
		"name",
		RTA_STR,
		MAX_MOD_NAME,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_name,
		NULL,
		"The name of the Module"
	},
	{
		"modules",
		"description",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_desc, 
		NULL,
		"The Module Description"
	},
	{
		"modules",
		"version",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_version,
		NULL,
		"The module version"
	},
	{
		"modules",
		"builddate",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_builddate,
		NULL,
		"The module build date"
	},
	{
		"modules",
		"coreinfo",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_core_info,
		NULL,
		"The NeoStats core Version"
	},
};

TBLDEF neo_modules = {
	"modules",
	NULL, 	/* for now */
	sizeof(struct Module),
	0,
	TBL_HASH,
	neo_modulecols,
	sizeof(neo_modulecols) / sizeof(COLDEF),
	"",
	"The list of Modules loaded by NeoStats"
};
#endif /* SQLSRV */


/** @brief Initialise module list hashes
 *
 * For core use only, initialises module list hashes
 *
 * @param none
 * 
 * @return none
*/
int 
InitModules ()
{
	SET_SEGV_LOCATION();
	mh = hash_create (NUM_MODULES, 0, 0);
	if(!mh)
		return NS_FAILURE;

#ifdef SQLSRV
        /* add the module hash to the sql library */
	neo_modules.address = mh;
	rta_add_table(&neo_modules);
#endif                      

	return NS_SUCCESS;
}

int FiniModules (void) 
{
	hash_destroy(mh);
}

/** @brief SendModuleEvent
 *
 * 
 *
 * @return none
 */
void
SendModuleEvent (char *event, char **av, int ac)
{
	Module *module_ptr;
	ModuleEvent *ev_list;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		ev_list = module_ptr->event_list;
		if (ev_list) {
			while (ev_list->cmd_name != NULL) {
				/* This goes through each Command */
				if (!ircstrcasecmp (ev_list->cmd_name, event)) {
					nlog (LOG_DEBUG1, "Running Module %s for Command %s -> %s", module_ptr->info->name, event, ev_list->cmd_name);
					SET_SEGV_LOCATION();
					if (setjmp (sigvbuf) == 0) {
						SET_SEGV_INMODULE(module_ptr->info->name);
						ev_list->function (av, ac);
						CLEAR_SEGV_INMODULE();
					} else {
						nlog (LOG_CRITICAL, "setjmp() Failed, Can't call Module %s\n", module_ptr->info->name);
					}
					SET_SEGV_LOCATION();
#ifndef VALGRIND
					break;
#endif
				}
				ev_list++;
			}
		}
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void
ModulesVersion (const char* nick, const char *remoteserver)
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		numeric(RPL_VERSION, nick,
			"Module %s version: %s %s %s",
			module_ptr->info->name, module_ptr->info->version, 
			module_ptr->info->build_date, module_ptr->info->build_time);
	}
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
load_module (char *modfilename, User * u)
{
#ifndef HAVE_LIBDL
	const char *dl_error;
#else /* HAVE_LIBDL */
	char *dl_error;
#endif /* HAVE_LIBDL */
	void *dl_handle;
	int do_msg;
	char path[255];
	char loadmodname[255];
	char **av;
	int ac = 0;
	int i = 0;
	ModuleInfo *info_ptr = NULL;
	ModuleEvent *event_ptr = NULL;
	Module *mod_ptr = NULL;
	hnode_t *mn;
	int (*ModInit) (Module * module_ptr);

	SET_SEGV_LOCATION();
	if (u == NULL) {
		do_msg = 0;
	} else {
		do_msg = 1;
	}
	strlcpy (loadmodname, modfilename, 255);
	strlwr (loadmodname);
	ircsnprintf (path, 255, "%s/%s.so", MOD_PATH, loadmodname);
	dl_handle = ns_dlopen (path, RTLD_NOW || RTLD_GLOBAL);
	if (!dl_handle) {
		dl_error = ns_dlerror ();
		if (do_msg) {
			prefmsg (u->nick, ns_botptr->nick, "Unable to load module: %s %s", dl_error, path);
		}
		nlog (LOG_WARNING, "Unable to load module: %s %s", dl_error, path);
		return NS_FAILURE;
	}

	info_ptr = ns_dlsym (dl_handle, "module_info");
#ifndef HAVE_LIBDL
	if(info_ptr == NULL) {
		dl_error = ns_dlerror ();
#else /* HAVE_LIBDL */
	if ((dl_error = ns_dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		if (do_msg) {
			prefmsg (u->nick, ns_botptr->nick, "Unable to load module: %s %s", dl_error, path);
		}
		nlog (LOG_WARNING, "Unable to load module: %s %s", dl_error, path);
		ns_dlclose (dl_handle);
		return NS_FAILURE;
	}
	/* Check module was built for this version of NeoStats */
	if(	ircstrncasecmp (NEOSTATS_VERSION, info_ptr->neostats_version, VERSIONSIZE) !=0 ) {
		nlog (LOG_WARNING, "Unable to load module: %s was built with an old version of NeoStats and must be rebuilt.", mod_ptr->info->name);
		ns_dlclose (dl_handle);
		return NS_FAILURE;
	}
	/* Check that the Module hasn't already been loaded */
	if (hash_lookup (mh, info_ptr->name)) {
		ns_dlclose (dl_handle);
		if (do_msg) {
			prefmsg (u->nick, ns_botptr->nick, "Unable to load module: %s already loaded", info_ptr->name);
		}
		nlog (LOG_WARNING, "Unable to load module: %s already loaded", info_ptr->name);
		return NS_FAILURE;
	}
	/* Extract pointer to event list */
	event_ptr = ns_dlsym (dl_handle, "module_events");
	/* Allocate module */
	mod_ptr = (Module *) smalloc (sizeof (Module));
	mn = hnode_create (mod_ptr);
	if (hash_isfull (mh)) {
		if (do_msg) {
			chanalert (ns_botptr->nick, "Unable to load module: module list is full");
			prefmsg (u->nick, ns_botptr->nick, "Unable to load module: module list is full");
			nlog (LOG_WARNING, "Unable to load module: module list is full");
		}
		ns_dlclose (dl_handle);
		free (mod_ptr);
		return NS_FAILURE;
	} 
	hash_insert (mh, mn, info_ptr->name);
	nlog (LOG_DEBUG1, "Module Internal name: %s", info_ptr->name);
	nlog (LOG_DEBUG1, "Module description: %s", info_ptr->description);
	mod_ptr->info = info_ptr;
	mod_ptr->dl_handle = dl_handle;
	mod_ptr->event_list = event_ptr;
	/* Module side user authentication for SecureServ helpers */
	mod_ptr->mod_auth_cb = ns_dlsym ((int *) dl_handle, "ModAuth");
	/* assign a module number to this module */
	i = 0;
	while (ModList[i] != NULL)
		i++;
	ModList[i] = mod_ptr;
	mod_ptr->modnum = i;
	nlog (LOG_DEBUG1, "Assigned %d to module %s for modulenum", i, mod_ptr->info->name);

	/* call ModInit (replacement for library __init() call */
	ModInit = ns_dlsym ((int *) dl_handle, "ModInit");
	if (!ModInit) {
		if (do_msg) {
			chanalert (ns_botptr->nick, "Unable to load module: %s missing ModInit.", mod_ptr->info->name);
			prefmsg (u->nick, ns_botptr->nick, "Unable to load module: %s missing ModInit.", mod_ptr->info->name);
		}
		ns_dlclose (dl_handle);
		free (mod_ptr);
		return NS_FAILURE;
	} else {
		int err;
		SET_SEGV_LOCATION();
		SET_SEGV_INMODULE(mod_ptr->info->name);
		err = (*ModInit) (mod_ptr);
		if (err < 1) {
			nlog (LOG_NORMAL, "Unable to load module: %s. See %s.log for further information.", mod_ptr->info->name, mod_ptr->info->name);
			unload_module(mod_ptr->info->name, NULL);
			return NS_FAILURE;
		}
		CLEAR_SEGV_INMODULE();
		SET_SEGV_LOCATION();
	}

	/* Let this module know we are online if we are! */
	if (me.onchan == 1) {
		while (event_ptr->cmd_name != NULL) {
			if (!ircstrcasecmp (event_ptr->cmd_name, EVENT_ONLINE)) {
				AddStringToList (&av, me.s->name, &ac);
				SET_SEGV_LOCATION();
				SET_SEGV_INMODULE(mod_ptr->info->name);
				event_ptr->function (av, ac);
				CLEAR_SEGV_INMODULE();
				SET_SEGV_LOCATION();
				free (av);
				break;
			}
			event_ptr++;
		}
	}
	if (do_msg) {
		prefmsg (u->nick, ns_botptr->nick, "Module %s loaded, %s", info_ptr->name, info_ptr->description);
		globops (me.name, "Module %s loaded", info_ptr->name);
	}
	return NS_SUCCESS;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
get_dl_handle (const char *mod_name)
{
	Module *mod_ptr;
	hnode_t *mn;

	mn = hash_lookup (mh, mod_name);
	if (mn) {
		mod_ptr = hnode_get (mn);
		return (int) mod_ptr->dl_handle;
	}
	return 0;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
get_mod_num (const char *mod_name)
{
	int i;

	for (i = 0; i < NUM_MODULES; i++) {
		if (ModList[i] != NULL) {
			if (!ircstrcasecmp (ModList[i]->info->name, mod_name)) {
				return i;
			}
		}
	}
	/* if we get here, it wasn't found */
	nlog (LOG_DEBUG1, "get_mod_num: can't find %s in module number list", mod_name);
	return NS_FAILURE;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
Module *
get_mod_ptr (const char *mod_name)
{
	int i;

	for (i = 0; i < NUM_MODULES; i++) {
		if (ModList[i] != NULL) {
			if (!ircstrcasecmp (ModList[i]->info->name, mod_name)) {
				return ModList[i];
			}
		}
	}
	/* if we get here, it wasn't found */
	nlog (LOG_DEBUG1, "get_mod_ptr: can't find %s in module number list", mod_name);
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
list_modules (User * u, char **av, int ac)
{
	Module *mod_ptr = NULL;
	hnode_t *mn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, mh);
	while ((mn = hash_scan_next (&hs)) != NULL) {
		mod_ptr = hnode_get (mn);
		prefmsg (u->nick, ns_botptr->nick, "Module: %s (%s)", mod_ptr->info->name, mod_ptr->info->version);
		prefmsg (u->nick, ns_botptr->nick, "Module Description: %s", mod_ptr->info->description);
		prefmsg (u->nick, ns_botptr->nick, "Module Number: %d", get_mod_num (mod_ptr->info->name));
	}
	prefmsg (u->nick, ns_botptr->nick, "End of Module List");
	return 0;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
unload_module (const char *modname, User * u)
{
	Module *mod_ptr;
	hnode_t *modnode;
	int i;
	void (*ModFini) ();

	SET_SEGV_LOCATION();
	/* Check to see if module is loaded */
	modnode = hash_lookup (mh, modname);
	if (!modnode) {
		if (u) {
			prefmsg (u->nick, ns_botptr->nick, "Module %s not loaded, try /msg %s modlist", modname, ns_botptr->nick);
			chanalert (ns_botptr->nick, "%s tried to unload %s but its not loaded", u->nick, modname);
		}
		return NS_FAILURE;
	}
	i = get_mod_num (modname);
	mod_ptr = hnode_get (modnode);
	chanalert (ns_botptr->nick, "Unloading module %s", modname);
	/* canx any DNS queries used by this module */
	canx_dns (modname);
	/* Delete any timers used by this module */
	del_timers (mod_ptr);
	/* Delete any sockets used by this module */
	del_sockets (mod_ptr);
	nlog (LOG_DEBUG1, "Deleting Module %s from Hash", modname);
	globops (me.name, "%s Module Unloaded", modname);
	/* Remove from the module hash so we dont call events for this module 
	 * during signoff 
	 */
	hash_delete (mh, modnode);		
	/* call ModFini (replacement for library __fini() call */
	ModFini = ns_dlsym ((int *) mod_ptr->dl_handle, "ModFini");
	if (ModFini) {
		SET_SEGV_INMODULE(modname);
		(*ModFini) ();
		CLEAR_SEGV_INMODULE();
	}
	/* Delete any bots used by this module. Done after ModFini, so the bot 
	 * can still send messages during ModFini 
	 */
	del_bots (mod_ptr);
	hnode_destroy (modnode);
	/* Close module */
	SET_SEGV_INMODULE(modname);
#ifndef VALGRIND
	ns_dlclose (mod_ptr->dl_handle);
#endif
	CLEAR_SEGV_INMODULE();
	/* free the module number */
	if (i >= 0) {
		nlog (LOG_DEBUG1, "Free %d from Module Numbers", i);
		ModList[i] = NULL;
	}
	free (mod_ptr);
	return NS_SUCCESS;
}

/** @brief unload all loaded modules
 *
 * Unloads all loaded modules
 *
 * @param none
 * 
 * @return none
*/
void unload_modules(void)
{
	Module *mod_ptr;
	hscan_t ms;
	hnode_t *mn;

	/* Walk through hash list unloading each module */
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		mod_ptr = hnode_get (mn);
		unload_module (mod_ptr->info->name, NULL);
	}
}
