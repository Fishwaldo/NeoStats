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
#include "users.h"
#include "channels.h"
#include "servers.h"
#include "exclude.h"

/** @brief Module list
 * 
 */
static Module *ModList[NUM_MODULES];
Module* RunModule[10];
int RunLevel = 0;
unsigned int fusermoddata = 0;
unsigned int fservermoddata = 0;
unsigned int fchannelmoddata = 0;

/* @brief Module hash list */
static hash_t *modulehash;

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
	return NS_SUCCESS;
}

void FiniModules (void) 
{
	hash_destroy(modulehash);
}

/** @brief SynchModule 
 *
 * 
 *
 * @return none
 */
int SynchModule (Module* module_ptr)
{
	int err = NS_SUCCESS; /*FAILURE;*/
	int (*ModSynch) (void);

	module_ptr->insynch = 1;
	ModSynch = ns_dlsym ((int *) module_ptr->dl_handle, "ModSynch");
	if (ModSynch) {
		SET_RUN_LEVEL(module_ptr);
		err = (*ModSynch) (); 
		RESET_RUN_LEVEL();
	}
	module_ptr->synched = 1;
	return err;
}

/** @brief SynchModule 
 *
 * 
 *
 * @return none
 */
int SynchAllModules (void)
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, modulehash);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		if (SynchModule (module_ptr) != NS_SUCCESS) {
			unload_module (module_ptr->info->name, NULL);
		}
	}
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
	if (module_ptr->event_list) {
		if (module_ptr->event_list[event] && module_ptr->event_list[event]->function) {
			/* If we are not yet synched, check that the module supports 
				* the event before we are synched. */
			if (!module_ptr->synched && !(module_ptr->event_list[event]->flags & EVENT_FLAG_IGNORE_SYNCH)) {
				dlog(DEBUG1, "Skipping module %s for event %d since module is not yet synched", module_ptr->info->name, event);
				return;
			}
			if ((module_ptr->event_list[event]->flags & EVENT_FLAG_DISABLED)) {
				dlog(DEBUG1, "Skipping module %s for event %d since it is disabled", module_ptr->info->name, event);
				return;
			}
			if ((module_ptr->event_list[event]->flags & EVENT_FLAG_EXCLUDE_ME) && IsMe (cmdparams->source) ) {
				dlog(DEBUG1, "Skipping module %s for event %d since %s is excluded as a NeoStats client", module_ptr->info->name, event, cmdparams->source->name);
				return;
			}
			if (module_ptr->event_list[event]->flags & EVENT_FLAG_EXCLUDE_MODME) {
				if (cmdparams->source->user && cmdparams->source->user->bot && cmdparams->source->user->bot->moduleptr == module_ptr) {
					dlog(DEBUG1, "Skipping module %s for event %d since %s is excluded as a Module client", module_ptr->info->name, event, cmdparams->source->name);
					return;
				}
			}			
			if ((module_ptr->event_list[event]->flags & EVENT_FLAG_USE_EXCLUDE) && IsExcluded (cmdparams->source)) {
				dlog(DEBUG1, "Skipping module %s for event %d since %s is excluded", module_ptr->info->name, event, cmdparams->source->name);
				return;
			}			
			dlog(DEBUG1, "Running module %s with event %d", module_ptr->info->name, event);
			SET_SEGV_LOCATION();
			if (setjmp (sigvbuf) == 0) {
				SET_RUN_LEVEL(module_ptr);
				module_ptr->event_list[event]->function (cmdparams);
				RESET_RUN_LEVEL();
			} else {
				nlog (LOG_CRITICAL, "setjmp() Failed, Can't call Module %s", module_ptr->info->name);
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
			_("Module %s version: %s %s %s"),
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
	int (*ModInit) (Module *module_ptr);
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	if (hash_isfull (modulehash)) {
		load_module_error (u, __("Unable to load module: module list is full", u));
		return NULL;
	} 
	strlcpy (loadmodname, modfilename, 255);
	strlwr (loadmodname);
	ircsnprintf (path, 255, "%s/%s%s", MOD_PATH, loadmodname, MOD_EXT);
	dl_handle = ns_dlopen (path, RTLD_NOW || RTLD_GLOBAL);
	if (!dl_handle) {
		load_module_error (u, __("Unable to load module: %s %s", u), ns_dlerrormsg, path);
		return NULL;
	}
	info_ptr = ns_dlsym (dl_handle, "module_info");
	if(info_ptr == NULL) {
		load_module_error (u, __("Unable to load module: %s missing module_info", u), path);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Check module was built for this version of NeoStats */
	if( ircstrncasecmp (NEOSTATS_VERSION, info_ptr->neostats_version, VERSIONSIZE) !=0 ) {
		load_module_error (u, __("Unable to load module: %s was built with an old version of NeoStats and must be rebuilt.", u), modfilename);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Check that the Module hasn't already been loaded */
	if (hash_lookup (modulehash, info_ptr->name)) {
		ns_dlclose (dl_handle);
		load_module_error (u, __("Unable to load module: %s already loaded", u), info_ptr->name);
		return NULL;
	}
	/* Check we have require PROTOCOL/FEATURE support for module */
	if((info_ptr->features & ircd_srv.features) != info_ptr->features) {
		load_module_error (u, __("Unable to load module: %s Required module features not available on this IRCd.", u), modfilename);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Lookup ModInit (replacement for library __init() call */
	ModInit = ns_dlsym ((int *) dl_handle, "ModInit");
	if (!ModInit) {
		load_module_error (u, __("Unable to load module: %s missing ModInit.", u), mod_ptr->info->name);
		ns_dlclose (dl_handle);
		return NULL;
	}
	/* Allocate module */
	mod_ptr = (Module *) ns_calloc (sizeof (Module));
	hnode_create_insert (modulehash, mod_ptr, info_ptr->name);
	dlog(DEBUG1, "Module internal name: %s", info_ptr->name);
	dlog(DEBUG1, "Module description: %s", info_ptr->description);
	mod_ptr->info = info_ptr;
	mod_ptr->dl_handle = dl_handle;
	/* Extract pointer to event list */
	event_ptr = ns_dlsym (dl_handle, "module_events");
	if(event_ptr) {
		SET_RUN_LEVEL(mod_ptr);
		AddEventList (event_ptr);
		RESET_RUN_LEVEL();
	}
    /* For Auth modules, register auth function */
	if (info_ptr->flags & MODULE_FLAG_AUTH) {
		if (AddAuthModule (mod_ptr) != NS_SUCCESS) {
			load_module_error (u, __("Unable to load auth module: %s missing ModAuthUser function",u), info_ptr->name);
			unload_module(mod_ptr->info->name, NULL);
			return NULL;
		}
	}
    /* Module side user authentication for e.g. SecureServ helpers 
     * Not available on auth modules */
	if (!(info_ptr->flags & MODULE_FLAG_AUTH)) {
		mod_ptr->mod_auth_cb = ns_dlsym ((int *) dl_handle, "ModAuthUser");
	}
	/* assign a module number to this module */
	while (ModList[moduleindex] != NULL)
		moduleindex++;
	ModList[moduleindex] = mod_ptr;
	mod_ptr->modnum = moduleindex;
	dlog(DEBUG1, "Assigned %d to module %s for modulenum", moduleindex, mod_ptr->info->name);

	SET_SEGV_LOCATION();
	SET_RUN_LEVEL(mod_ptr);
	DBAOpenDatabase ();
	err = (*ModInit) (mod_ptr); 
	RESET_RUN_LEVEL();
	if (err < 1 || mod_ptr->error) {
		load_module_error (u, __("Unable to load module: %s. See %s.log for further information.",u), mod_ptr->info->name, mod_ptr->info->name);
		unload_module(mod_ptr->info->name, NULL);
		return NULL;
	}
	SET_RUN_LEVEL(mod_ptr);
	if (info_ptr->flags & MODULE_FLAG_LOCAL_EXCLUDES) 
	{
		InitModExcludes(mod_ptr);
	}
	RESET_RUN_LEVEL();
	SET_SEGV_LOCATION();

	/* Let this module know we are online if we are! */
	if (is_synched) {
		if (SynchModule (mod_ptr) != NS_SUCCESS || mod_ptr->error)
		{
			load_module_error (u, __("Unable to load module: %s. See %s.log for further information.", u), mod_ptr->info->name, mod_ptr->info->name);
			unload_module(mod_ptr->info->name, NULL);
			return NULL;
		}
	}
	cmdparams = ns_calloc (sizeof(CmdParams));
	cmdparams->param = (char*)info_ptr->name;
	SendAllModuleEvent(EVENT_MODULELOAD, cmdparams);
	ns_free(cmdparams);
	if (u) {
		irc_prefmsg (ns_botptr, u, __("Module %s loaded, %s",u), info_ptr->name, info_ptr->description);
		irc_globops (NULL, _("Module %s loaded"), info_ptr->name);
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
ns_cmd_modlist (CmdParams* cmdparams)
{
	Module *mod_ptr = NULL;
	hnode_t *mn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, modulehash);
	while ((mn = hash_scan_next (&hs)) != NULL) {
		mod_ptr = hnode_get (mn);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Module: %d %s (%s)", cmdparams->source), mod_ptr->modnum, mod_ptr->info->name, mod_ptr->info->version);
		irc_prefmsg (ns_botptr, cmdparams->source, "      : %s", mod_ptr->info->description);
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("End of Module List", cmdparams->source));
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
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	/* Check to see if module is loaded */
	modnode = hash_lookup (modulehash, modname);
	if (!modnode) {
		if (u) {
			irc_prefmsg (ns_botptr, u, __("Module %s not loaded, try /msg %s modlist", u), modname, ns_botptr->name);
			irc_chanalert (ns_botptr, _("%s tried to unload %s but its not loaded"), u->name, modname);
		}
		return NS_FAILURE;
	}
	mod_ptr = hnode_get (modnode);
	irc_chanalert (ns_botptr, _("Unloading module %s"), modname);
	if (mod_ptr->info->flags & MODULE_FLAG_AUTH)
	{
		DelAuthModule (mod_ptr);
	}
	SET_RUN_LEVEL(mod_ptr);
	if (mod_ptr->info->flags & MODULE_FLAG_LOCAL_EXCLUDES) 
	{
		FiniModExcludes(mod_ptr);
	}
	RESET_RUN_LEVEL();
	moduleindex = mod_ptr->modnum;
	/* canx any DNS queries used by this module */
	canx_dns (mod_ptr);
	/* Delete any timers used by this module */
	del_timers (mod_ptr);
	/* Delete any sockets used by this module */
	del_sockets (mod_ptr);
	/* Delete any associated event list */
	if (mod_ptr->event_list) {
		ns_free (mod_ptr->event_list);
		mod_ptr->event_list = NULL;
	}
	/* Remove from the module hash so we dont call events for this module 
	 * during signoff 
	 */
	dlog(DEBUG1, "Deleting Module %s from Hash", modname);
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
	DelModuleBots (mod_ptr);
	hnode_destroy (modnode);
	/* Close module */
	irc_globops (NULL, _("%s Module Unloaded"), modname);
#ifndef VALGRIND
	SET_RUN_LEVEL(mod_ptr);
	DBACloseDatabase ();
	ns_dlclose (mod_ptr->dl_handle);
	RESET_RUN_LEVEL();
#endif
	ns_free (mod_ptr);
	/* free the module number */
	if (moduleindex >= 0) {
		dlog(DEBUG1, "Free %d from Module Numbers", moduleindex);
		ModList[moduleindex] = NULL;
	}
	/* Cleanup moddata */
	if (fusermoddata & (1 << moduleindex)) {
		CleanupUserModdata (moduleindex);
	}
	if (fservermoddata & (1 << moduleindex)) {
		CleanupServerModdata (moduleindex);
	}
	if (fchannelmoddata & (1 << moduleindex)) {
		CleanupChannelModdata (moduleindex);
	}
	cmdparams = ns_calloc (sizeof(CmdParams));
	cmdparams->param = (char*)modname;
	SendAllModuleEvent(EVENT_MODULEUNLOAD, cmdparams);
	ns_free(cmdparams);
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
	SET_SEGV_LOCATION();
	while(set_ptr->option)
	{
		if(set_ptr->confitem) {
			switch(set_ptr->type) {
				case SET_TYPE_BOOLEAN:
					if (DBAFetchConfigBool (set_ptr->confitem, set_ptr->varptr) != NS_SUCCESS) {
						if( set_ptr->defaultval ) {
							*(int *)set_ptr->varptr = (int)set_ptr->defaultval;
						}
						DBAStoreConfigBool (set_ptr->confitem, set_ptr->varptr);
					}
					if(set_ptr->handler) {
						set_ptr->handler(NULL, SET_LOAD);
					}
					break;
				case SET_TYPE_INT:
					if (DBAFetchConfigInt (set_ptr->confitem, set_ptr->varptr) != NS_SUCCESS) {
						if( set_ptr->defaultval ) {
							*(int *)set_ptr->varptr = (int)set_ptr->defaultval;
						}
						DBAStoreConfigInt(set_ptr->confitem, set_ptr->varptr);
					}
					if(set_ptr->handler) {
						set_ptr->handler(NULL, SET_LOAD);
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
					if(	DBAFetchConfigStr (set_ptr->confitem, set_ptr->varptr, set_ptr->max) != NS_SUCCESS) {
						if( set_ptr->defaultval ) {
							strlcpy(set_ptr->varptr, set_ptr->defaultval, set_ptr->max);
						}
						DBAStoreConfigStr (set_ptr->confitem, set_ptr->varptr, set_ptr->max);
					}
					if(set_ptr->handler) {
						set_ptr->handler(NULL, SET_LOAD);
					}
					break;			
				case SET_TYPE_CUSTOM:
					if(set_ptr->handler) {
						set_ptr->handler(NULL, SET_LOAD);
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

/** @brief 
 *
 * 
 *
 * @return none
 */
void AddEvent (ModuleEvent* event)
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if (!mod_ptr->event_list) {
		mod_ptr->event_list = ns_calloc ( sizeof(ModuleEvent) * EVENT_COUNT );
	}
	if (event->event == EVENT_NICKIP)
	{
		me.want_nickip = 1; 		
	}
	mod_ptr->event_list[event->event] = event;
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void AddEventList (ModuleEvent *event_ptr)
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if (!mod_ptr->event_list) {
		mod_ptr->event_list = ns_calloc ( sizeof(ModuleEvent*) * EVENT_COUNT );
	}
	while (event_ptr->event != EVENT_NULL) {
		mod_ptr->event_list[event_ptr->event] = event_ptr;
		event_ptr ++;
	}
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
		mod_ptr->event_list[event] = NULL;
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void DeleteEventList (ModuleEvent *event_ptr)
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	while (event_ptr->event) {
		if (mod_ptr->event_list[event_ptr->event]) {
			mod_ptr->event_list[event_ptr->event] = NULL;
		}
		event_ptr++;
	}
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
void SetAllEventFlags (unsigned int flag, unsigned int enable)
{
	int i;
	ModuleEvent** eventptr;

	eventptr = GET_CUR_MODULE()->event_list;
	if (eventptr) {
		for (i = 0; i < EVENT_COUNT; i++) {
			if (eventptr[i]) {
				if (enable) {
					eventptr[i]->flags |= flag;
				} else {
					eventptr[i]->flags &= ~flag;
				}
			}
		}
	}
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
void SetEventFlags (Event event, unsigned int flag, unsigned int enable)
{
	ModuleEvent** eventptr;

	eventptr = GET_CUR_MODULE()->event_list;
	if (eventptr) {
		if (enable) {
			eventptr[event]->flags |= flag;
		} else {
			eventptr[event]->flags &= ~flag;
		}
	}
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
void EnableEvent (Event event)
{
	GET_CUR_MODULE()->event_list[event]->flags &= ~EVENT_FLAG_DISABLED;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
void DisableEvent (Event event)
{
	GET_CUR_MODULE()->event_list[event]->flags |= EVENT_FLAG_DISABLED;
}

hash_t *GetModuleHash (void)
{
	return modulehash;
}
