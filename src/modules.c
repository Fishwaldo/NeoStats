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

#include "neostats.h"
#include "dl.h"
#include "timer.h"
#include "sock.h"
#include "services.h"
#include "modules.h"
#include "bots.h"
#include "dns.h"
#include "ircd.h"
#include "auth.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

/** @brief Module list
 * 
 */
static Module *ModList[NUM_MODULES];
Module* RunModule[10];
int RunLevel = 0;

int del_all_bot_cmds(Bot* bot_ptr);

/* @brief Module hash list */
static hash_t *modulehash;

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
	ircsnprintf(sqlbuf, BUFSIZE, "%s", me.version);
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
	modulehash = hash_create (NUM_MODULES, 0, 0);
	if(!modulehash) {
		nlog (LOG_CRITICAL, "Unable to create module hash");
		return NS_FAILURE;
	}

#ifdef SQLSRV
        /* add the module hash to the sql library */
	neo_modules.address = modulehash;
	rta_add_table(&neo_modules);
#endif                      

	return NS_SUCCESS;
}

int FiniModules (void) 
{
	hash_destroy(modulehash);
	return NS_SUCCESS;
}

/** @brief SendModuleEvent
 *
 * 
 *
 * @return none
 */
void
SendModuleEvent (Event event, CmdParams* cmdparams, Module* module_ptr)
{
	SET_SEGV_LOCATION();
	if (event == EVENT_ONLINE) {
		module_ptr->synched = 1;
	}
	if (module_ptr->event_list) {
		if (module_ptr->event_list[event].function) {
			/* If we are not yet synched, check that the module supports 
				* the event before we are synched. */
			if (!module_ptr->synched && !(module_ptr->event_list[event].flags & EVENT_FLAG_IGNORE_SYNCH)) {
				dlog(DEBUG1, "Skipping module %s for event %d since module is not yet synched", module_ptr->info->name, event);
				return;
			}
			dlog(DEBUG1, "Running module %s with event %d", module_ptr->info->name, event);
			SET_SEGV_LOCATION();
			if (setjmp (sigvbuf) == 0) {
				SET_RUN_LEVEL(module_ptr);
				module_ptr->event_list[event].function (cmdparams);
				RESET_RUN_LEVEL();
			} else {
				nlog (LOG_CRITICAL, "setjmp() Failed, Can't call Module %s\n", module_ptr->info->name);
			}
			SET_SEGV_LOCATION();
		}
	}
}

/** @brief SendAllModuleEvent
 *
 * 
 *
 * @return none
 */
void
SendAllModuleEvent (Event event, CmdParams* cmdparams)
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, modulehash);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		if (module_ptr->event_list) {
			SendModuleEvent(event, cmdparams, module_ptr);
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
	hash_scan_begin (&ms, modulehash);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		irc_numeric (RPL_VERSION, nick,
			"Module %s version: %s %s %s",
			module_ptr->info->name, module_ptr->info->version, 
			module_ptr->info->build_date, module_ptr->info->build_time);
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void RegisterEventList (Module* mod_ptr, ModuleEvent *event_ptr)
{
	mod_ptr->event_list = scalloc ( sizeof(ModuleEvent) * EVENT_COUNT );
	while (event_ptr->event != EVENT_NULL) {
		memcpy (&mod_ptr->event_list[event_ptr->event], event_ptr, sizeof(ModuleEvent));
		event_ptr ++;
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void RegisterEvent (Event event, event_function function, unsigned int flags)
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if (!mod_ptr->event_list) {
		mod_ptr->event_list = scalloc ( sizeof(ModuleEvent) * EVENT_COUNT );
	}
	mod_ptr->event_list[event].function = function;
	mod_ptr->event_list[event].flags = flags;
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void DeleteEvent (Event event)
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if (mod_ptr->event_list) {
		mod_ptr->event_list[event].function = NULL;
		mod_ptr->event_list[event].flags = 0;
	}
}

static void load_module_error(const Client *target, const char *fmt, ...)
{
	static char buf[BUFSIZE];
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (buf, BUFSIZE, fmt, ap);
	va_end (ap);
	if (target) {
		irc_prefmsg (ns_botptr, target, buf);
	}
	nlog (LOG_WARNING, buf);
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
Module *
load_module (const char *modfilename, Client * u)
{
	int err;
	void *dl_handle;
	char path[255];
	char loadmodname[255];
	int moduleindex = 0;
	ModuleInfo *info_ptr = NULL;
	ModuleEvent *event_ptr = NULL;
	Module *mod_ptr = NULL;
	int (*ModInit) (Module * module_ptr);

	SET_SEGV_LOCATION();
	if (hash_isfull (modulehash)) {
		load_module_error (u, "Unable to load module: module list is full");
		return NULL;
	} 
	strlcpy (loadmodname, modfilename, 255);
	strlwr (loadmodname);
	ircsnprintf (path, 255, "%s/%s%s", MOD_PATH, loadmodname, MOD_EXT);
	dl_handle = ns_dlopen (path, RTLD_NOW || RTLD_GLOBAL);
	if (!dl_handle) {
		load_module_error (u, "Unable to load module: %s %s", ns_dlerrormsg, path);
		return NULL;
	}
	info_ptr = ns_dlsym (dl_handle, "module_info");
	if(info_ptr == NULL) {
		load_module_error (u, "Unable to load module: %s missing module_info", path);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Check module was built for this version of NeoStats */
	if(	ircstrncasecmp (NEOSTATS_VERSION, info_ptr->neostats_version, VERSIONSIZE) !=0 ) {
		load_module_error (u, "Unable to load module: %s was built with an old version of NeoStats and must be rebuilt.", modfilename);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Check that the Module hasn't already been loaded */
	if (hash_lookup (modulehash, info_ptr->name)) {
		ns_dlclose (dl_handle);
		load_module_error (u, "Unable to load module: %s already loaded", info_ptr->name);
		return NULL;
	}
	/* Check we have require PROTOCOL/FEATURE support for module */
	if((info_ptr->features & ircd_srv.features) != info_ptr->features) {
		load_module_error (u, "Unable to load module: %s Required module features not available on this IRCd.", modfilename);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Lookup ModInit (replacement for library __init() call */
	ModInit = ns_dlsym ((int *) dl_handle, "ModInit");
	if (!ModInit) {
		load_module_error (u, "Unable to load module: %s missing ModInit.", mod_ptr->info->name);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Allocate module */
	mod_ptr = (Module *) scalloc (sizeof (Module));
	hnode_create_insert (modulehash, mod_ptr, info_ptr->name);
	dlog(DEBUG1, "Module internal name: %s", info_ptr->name);
	dlog(DEBUG1, "Module description: %s", info_ptr->description);
	mod_ptr->info = info_ptr;
	mod_ptr->dl_handle = dl_handle;
	/* Extract pointer to event list */
	event_ptr = ns_dlsym (dl_handle, "module_events");
	if(event_ptr) {
		RegisterEventList (mod_ptr, event_ptr);
	}
	if (mod_ptr->info->flags & MODULE_FLAG_AUTH) {
		if (init_auth_module (mod_ptr) != NS_SUCCESS) {
			load_module_error (u, "Unable to load auth module: %s missing ModAuthUser function", mod_ptr->info->name);
			unload_module(mod_ptr->info->name, NULL);
			return NULL;
		}
	} else {
		/* Module side user authentication for e.g. SecureServ helpers */
		mod_ptr->mod_auth_cb = ns_dlsym ((int *) dl_handle, "ModAuth");
	}
	/* assign a module number to this module */
	while (ModList[moduleindex] != NULL)
		moduleindex++;
	ModList[moduleindex] = mod_ptr;
	mod_ptr->modnum = moduleindex;
	dlog(DEBUG1, "Assigned %d to module %s for modulenum", moduleindex, mod_ptr->info->name);

	SET_SEGV_LOCATION();
	SET_RUN_LEVEL(mod_ptr);
	err = (*ModInit) (mod_ptr); 
	RESET_RUN_LEVEL();
	if (err < 1) {
		load_module_error (u, "Unable to load module: %s. See %s.log for further information.", mod_ptr->info->name, mod_ptr->info->name);
		unload_module(mod_ptr->info->name, NULL);
		return NULL;
	}
	SET_SEGV_LOCATION();

	/* Let this module know we are online if we are! */
	if (is_synched) {
		SendModuleEvent (EVENT_ONLINE, NULL, mod_ptr);
	}
	if (u) {
		irc_prefmsg (ns_botptr, u, "Module %s loaded, %s", info_ptr->name, info_ptr->description);
		irc_globops (NULL, "Module %s loaded", info_ptr->name);
	}
	return mod_ptr;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
list_modules (CmdParams* cmdparams)
{
	Module *mod_ptr = NULL;
	hnode_t *mn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, modulehash);
	while ((mn = hash_scan_next (&hs)) != NULL) {
		mod_ptr = hnode_get (mn);
		irc_prefmsg (ns_botptr, cmdparams->source, "Module: %d %s (%s)", mod_ptr->modnum, mod_ptr->info->name, mod_ptr->info->version);
		irc_prefmsg (ns_botptr, cmdparams->source, "      : %s", mod_ptr->info->description);
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "End of Module List");
	return 0;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
unload_module (const char *modname, Client * u)
{
	Module *mod_ptr;
	hnode_t *modnode;
	int moduleindex;
	void (*ModFini) ();

	SET_SEGV_LOCATION();
	/* Check to see if module is loaded */
	modnode = hash_lookup (modulehash, modname);
	if (!modnode) {
		if (u) {
			irc_prefmsg (ns_botptr, u, "Module %s not loaded, try /msg %s modlist", modname, ns_botptr->name);
			irc_chanalert (ns_botptr, "%s tried to unload %s but its not loaded", u->name, modname);
		}
		return NS_FAILURE;
	}
	mod_ptr = hnode_get (modnode);
	if (mod_ptr->info->flags & MODULE_FLAG_AUTH)
	{
		delete_auth_module (mod_ptr);
	}
	moduleindex = mod_ptr->modnum;
	irc_chanalert (ns_botptr, "Unloading module %s", modname);
	/* canx any DNS queries used by this module */
	canx_dns (mod_ptr);
	/* Delete any timers used by this module */
	del_timers (mod_ptr);
	/* Delete any sockets used by this module */
	del_sockets (mod_ptr);
	dlog(DEBUG1, "Deleting Module %s from Hash", modname);
	irc_globops (NULL, "%s Module Unloaded", modname);
	/* Delete any associated event list */
	if (mod_ptr->event_list) {
		sfree (mod_ptr->event_list);
		mod_ptr->event_list = NULL;
	}
	/* Remove from the module hash so we dont call events for this module 
	 * during signoff 
	 */
	hash_delete (modulehash, modnode);		
	/* call ModFini (replacement for library __fini() call */
	ModFini = ns_dlsym ((int *) mod_ptr->dl_handle, "ModFini");
	if (ModFini) {
		SET_RUN_LEVEL(mod_ptr);
		(*ModFini) ();
		RESET_RUN_LEVEL();
	}
	/* Delete any bots used by this module. Done after ModFini, so the bot 
	 * can still send messages during ModFini 
	 */
	del_module_bots (mod_ptr);
	hnode_destroy (modnode);
	/* Close module */
	SET_RUN_LEVEL(mod_ptr);
#ifndef VALGRIND
	ns_dlclose (mod_ptr->dl_handle);
#endif
	RESET_RUN_LEVEL();
	/* free the module number */
	if (moduleindex >= 0) {
		dlog(DEBUG1, "Free %d from Module Numbers", moduleindex);
		ModList[moduleindex] = NULL;
	}
	sfree (mod_ptr);
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
	hash_scan_begin (&ms, modulehash);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		mod_ptr = hnode_get (mn);
		unload_module (mod_ptr->info->name, NULL);
	}
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
ModuleConfig(bot_setting* set_ptr)
{
	char *temp = NULL;

	SET_SEGV_LOCATION();
	while(set_ptr->option)
	{
		if(set_ptr->confitem) {
			switch(set_ptr->type) {
				case SET_TYPE_BOOLEAN:
					if (GetConf((void *)set_ptr->varptr, CFGBOOL, set_ptr->confitem) <= 0) {
						*(int *)set_ptr->varptr = (int)set_ptr->defaultval;
					}
					break;
				case SET_TYPE_INT:
					if (GetConf((void *)set_ptr->varptr, CFGINT, set_ptr->confitem) <= 0) {
						*(int *)set_ptr->varptr = (int)set_ptr->defaultval;
					}
					break;
				case SET_TYPE_STRING:
				case SET_TYPE_CHANNEL:							
				case SET_TYPE_MSG:
				case SET_TYPE_NICK:
				case SET_TYPE_USER:
				case SET_TYPE_HOST:
				case SET_TYPE_REALNAME:
				case SET_TYPE_IPV4:
					if(GetConf((void *) &temp, CFGSTR, set_ptr->confitem) > 0) {
						strlcpy(set_ptr->varptr, temp, set_ptr->max);
						sfree(temp);
					} else {
						strlcpy(set_ptr->varptr, set_ptr->defaultval, set_ptr->max);
						
					}
					break;			
				case SET_TYPE_CUSTOM:
					if(set_ptr->handler) {
						set_ptr->handler(NULL);
					}
					break;
				default:
					nlog(LOG_WARNING, "Unsupported SET type %d in ModuleConfig %s", 
						set_ptr->type, set_ptr->option);
					break;
			}
		}
		set_ptr++;
	}
	return NS_SUCCESS;
}
