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
	
	strlcpy(sqlbuf, mod_ptr->info->module_name, MAX_MOD_NAME);
	return sqlbuf;	
}

void *display_module_desc (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->module_description, BUFSIZE);
	return sqlbuf;
}

void *display_module_version (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->module_version, BUFSIZE);
	return sqlbuf;
}

void *display_module_builddate (void *tbl, char *col, char *sql, void *row) 
{
	Module *mod_ptr = row;
	
	ircsnprintf(sqlbuf, BUFSIZE, "%s - %s", mod_ptr->info->module_build_date, mod_ptr->info->module_build_time);
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
					nlog (LOG_DEBUG1, "Running Module %s for Command %s -> %s", module_ptr->info->module_name, event, ev_list->cmd_name);
					SET_SEGV_LOCATION();
					if (setjmp (sigvbuf) == 0) {
						SET_SEGV_INMODULE(module_ptr->info->module_name);
						ev_list->function (av, ac);
						CLEAR_SEGV_INMODULE();
					} else {
						nlog (LOG_CRITICAL, "setjmp() Failed, Can't call Module %s\n", module_ptr->info->module_name);
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
			module_ptr->info->module_name, module_ptr->info->module_version, 
			module_ptr->info->module_build_date, module_ptr->info->module_build_time);
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
	ModuleInfo *mod_info_ptr = NULL;
	ModuleEvent *event_fn_ptr = NULL;
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
			prefmsg (u->nick, s_Services, "Couldn't Load Module: %s %s", dl_error, path);
		}
		nlog (LOG_WARNING, "Couldn't Load Module: %s %s", dl_error, path);
		return NS_FAILURE;
	}

	mod_info_ptr = ns_dlsym (dl_handle, "module_info");
#ifndef HAVE_LIBDL
	if(mod_info_ptr == NULL) {
		dl_error = ns_dlerror ();
#else /* HAVE_LIBDL */
	if ((dl_error = ns_dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		if (do_msg) {
			prefmsg (u->nick, s_Services, "Couldn't Load Module: %s %s", dl_error, path);
		}
		nlog (LOG_WARNING, "Couldn't Load Module: %s %s", dl_error, path);
		ns_dlclose (dl_handle);
		return NS_FAILURE;
	}

	event_fn_ptr = ns_dlsym (dl_handle, "module_events");

	/* Check that the Module hasn't already been loaded */
	if (hash_lookup (mh, mod_info_ptr->module_name)) {
		ns_dlclose (dl_handle);
		if (do_msg)
			prefmsg (u->nick, s_Services, "Module %s already Loaded, Can't Load 2 Copies", mod_info_ptr->module_name);
		return NS_FAILURE;
	}

	mod_ptr = (Module *) smalloc (sizeof (Module));

	mn = hnode_create (mod_ptr);
	if (hash_isfull (mh)) {
		if (do_msg) {
			chanalert (s_Services, "Module List is Full. Can't Load any more modules");
			prefmsg (u->nick, s_Services, "Module List is Full, Can't Load any more Modules");
		}
		ns_dlclose (dl_handle);
		free (mod_ptr);
		return NS_FAILURE;
	} else {
		hash_insert (mh, mn, mod_info_ptr->module_name);
	}
	nlog (LOG_DEBUG1, "Module Internal name: %s", mod_info_ptr->module_name);
	nlog (LOG_DEBUG1, "Module description: %s", mod_info_ptr->module_description);

	mod_ptr->info = mod_info_ptr;
	mod_ptr->dl_handle = dl_handle;
	mod_ptr->event_list = event_fn_ptr;

	/* assign a module number to this module */
	i = 0;
	while (ModList[i] != NULL)
		i++;
	/* no need to check for overflow of NUM_MODULES, as its done above */
	ModList[i] = mod_ptr;
	nlog (LOG_DEBUG1, "Assigned %d to Module %s for ModuleNum", i, mod_ptr->info->module_name);

	/* Module side user authentication for SecureServ helpers */
	mod_ptr->mod_auth_cb = ns_dlsym ((int *) dl_handle, "ModuleAuth");

	/* call ModInit (replacement for library __init() call */
	ModInit = ns_dlsym ((int *) dl_handle, "ModInit");
	if (!ModInit) {
		if (do_msg) {
			chanalert (s_Services, "Unable to load %s, missing ModInit.", mod_ptr->info->module_name);
			prefmsg (u->nick, s_Services, "Unable to load %s, missing ModInit.", mod_ptr->info->module_name);
		}
		ns_dlclose (dl_handle);
		free (mod_ptr);
		return NS_FAILURE;
	} else {
		int err;
		SET_SEGV_LOCATION();
		SET_SEGV_INMODULE(mod_ptr->info->module_name);
		err = (*ModInit) (mod_ptr);
		if (err == NS_ERR_VERSION ) {
			nlog (LOG_NORMAL, "Module %s was built with an old version of NeoStats. Please recompile %s.", mod_ptr->info->module_name, mod_ptr->info->module_name);
			unload_module(mod_ptr->info->module_name, NULL);
			return NS_FAILURE;
		}
		else if (err < 1) {
			nlog (LOG_NORMAL, "Unable to load module %s. See %s.log for further information.", mod_ptr->info->module_name, mod_ptr->info->module_name);
			unload_module(mod_ptr->info->module_name, NULL);
			return NS_FAILURE;
		}
		CLEAR_SEGV_INMODULE();
		SET_SEGV_LOCATION();
	}

	/* Let this module know we are online if we are! */
	if (me.onchan == 1) {
		while (event_fn_ptr->cmd_name != NULL) {
			if (!ircstrcasecmp (event_fn_ptr->cmd_name, EVENT_ONLINE)) {
				AddStringToList (&av, me.s->name, &ac);
				SET_SEGV_LOCATION();
				SET_SEGV_INMODULE(mod_ptr->info->module_name);
				event_fn_ptr->function (av, ac);
				CLEAR_SEGV_INMODULE();
				SET_SEGV_LOCATION();
				free (av);
				break;
			}
			event_fn_ptr++;
		}
	}
	if (do_msg) {
		prefmsg (u->nick, s_Services, "Module %s loaded, %s", mod_info_ptr->module_name, mod_info_ptr->module_description);
		globops (me.name, "Module %s loaded", mod_info_ptr->module_name);
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
get_dl_handle (char *mod_name)
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
get_mod_num (char *mod_name)
{
	int i;

	for (i = 0; i < NUM_MODULES; i++) {
		if (ModList[i] != NULL) {
			if (!ircstrcasecmp (ModList[i]->info->module_name, mod_name)) {
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
get_mod_ptr (char *mod_name)
{
	int i;

	for (i = 0; i < NUM_MODULES; i++) {
		if (ModList[i] != NULL) {
			if (!ircstrcasecmp (ModList[i]->info->module_name, mod_name)) {
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
		prefmsg (u->nick, s_Services, "Module: %s (%s)", mod_ptr->info->module_name, mod_ptr->info->module_version);
		prefmsg (u->nick, s_Services, "Module Description: %s", mod_ptr->info->module_description);
		prefmsg (u->nick, s_Services, "Module Number: %d", get_mod_num (mod_ptr->info->module_name));
	}
	prefmsg (u->nick, s_Services, "End of Module List");
	return 0;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
unload_module (char *module_name, User * u)
{
	Module *mod_ptr;
	hnode_t *modnode;
	int i;
	void (*ModFini) ();

	SET_SEGV_LOCATION();
	/* Check to see if this Module is loaded....  */
	modnode = hash_lookup (mh, module_name);
	if (!modnode) {
		if (u) {
			prefmsg (u->nick, s_Services, "Module %s not loaded, Try /msg %s modlist", module_name, s_Services);
			chanalert (s_Services, "%s tried to unload %s but its not loaded", u->nick, module_name);
		}
		return NS_FAILURE;
	}
	chanalert (s_Services, "Unloading module %s", module_name);

	/* canx any DNS queries running */
	canx_dns(module_name);
	/* Check to see if this Module has any timers registered....  */
	del_timers (module_name);
	/* check if the module had a socket registered... */
	del_sockets (module_name);
	/* Remove module....  */
	modnode = hash_lookup (mh, module_name);
	if (modnode) {
		nlog (LOG_DEBUG1, "Deleting Module %s from Hash", module_name);
		globops (me.name, "%s Module Unloaded", module_name);
		i = get_mod_num (module_name);
		mod_ptr = hnode_get (modnode);
		/* Remove from the module hash so we dont call events for this module during signoff*/
		hash_delete (mh, modnode);		
		/* call ModFini (replacement for library __fini() call */
		ModFini = ns_dlsym ((int *) mod_ptr->dl_handle, "ModFini");
		if (ModFini) {
			SET_SEGV_INMODULE(module_name);
			(*ModFini) ();
			CLEAR_SEGV_INMODULE();
		}
		/* now, see if this Module has any bots with it 
		 * we delete the modules *after* we call ModFini, so the bot 
		 * can still send messages generated from ModFini calls */
		del_bots(module_name);
		hnode_destroy (modnode);
		/* Close module */
		SET_SEGV_INMODULE(module_name);
#ifndef VALGRIND
		ns_dlclose (mod_ptr->dl_handle);
#endif
		CLEAR_SEGV_INMODULE();
		if (i >= 0) {
			/* free the module number */
			nlog (LOG_DEBUG1, "Free %d from Module Numbers", i);
			ModList[i] = NULL;
		}
		free (mod_ptr);
		return NS_SUCCESS;
	}
	chanalert (s_Services, "Unload module %s failed", module_name);
	return NS_FAILURE;
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
		unload_module (mod_ptr->info->module_name, NULL);
	}
}
