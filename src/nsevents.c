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

#include "neostats.h"
#include "nsevents.h"
#include "modules.h"
#ifdef USE_PERL
#undef _
#define PERLDEFINES
#include "perlmod.h"
#endif /* USE_PERL */

/** @brief SendModuleEvent
 *
 *	Call event handler for a specific module
 *  NeoStats core use only
 *
 *  @param event to send
 *  @param cmdparams
 *  @param module_ptr pointer to module to raise event for
 *
 *  @return none
 */

void SendModuleEvent( Event event, CmdParams* cmdparams, Module* module_ptr )
{
	SET_SEGV_LOCATION();
	dlog( DEBUG5, "SendModuleEvent: event %d to module %s", event, module_ptr->info->name );
	if( !module_ptr->event_list )
	{
		dlog( DEBUG5, "SendModuleEvent: module %s has no events associated with it", module_ptr->info->name );
		return;
	}
	if ((IS_STD_MOD(module_ptr) && module_ptr->event_list[event] && module_ptr->event_list[event]->handler)
#ifdef USE_PERL
		|| (IS_PERL_MOD(module_ptr) && module_ptr->event_list[event])
#endif
		){
		/* If we are not yet synched, check that the module supports 
			* the event before we are synched. */
		if (!module_ptr->synched && !(module_ptr->event_list[event]->flags & EVENT_FLAG_IGNORE_SYNCH)) {
			dlog( DEBUG5, "Skipping module %s for event %d since module is not yet synched", module_ptr->info->name, event );
			return;
		}
		if ((module_ptr->event_list[event]->flags & EVENT_FLAG_DISABLED)) {
			dlog( DEBUG5, "Skipping module %s for event %d since it is disabled", module_ptr->info->name, event );
			return;
		}
		if ((module_ptr->event_list[event]->flags & EVENT_FLAG_EXCLUDE_ME) && IsMe (cmdparams->source) ) {
			dlog( DEBUG5, "Skipping module %s for event %d since %s is excluded as a NeoStats client", module_ptr->info->name, event, cmdparams->source->name );
			return;
		}
		if (module_ptr->event_list[event]->flags & EVENT_FLAG_EXCLUDE_MODME) {
			if (cmdparams->source && cmdparams->source->user && cmdparams->source->user->bot && cmdparams->source->user->bot->moduleptr == module_ptr) {
				dlog( DEBUG5, "Skipping module %s for event %d since %s is excluded as a Module client", module_ptr->info->name, event, cmdparams->source->name );
				return;
			}
		}			
		if ((module_ptr->event_list[event]->flags & EVENT_FLAG_USE_EXCLUDE) && IsExcluded (cmdparams->source)) {
			dlog( DEBUG5, "Skipping module %s for event %d since %s is excluded", module_ptr->info->name, event, cmdparams->source->name );
			return;
		}			
		dlog(DEBUG1, "Running module %s with event %d", module_ptr->info->name, event);
		SET_SEGV_LOCATION();
		if (IS_STD_MOD(module_ptr)) {
			if (setjmp (sigvbuf) == 0) {
				SET_RUN_LEVEL(module_ptr);
				module_ptr->event_list[event]->handler (cmdparams);
				RESET_RUN_LEVEL();
			} else {
				nlog (LOG_CRITICAL, "setjmp() failed, not calling module %s", module_ptr->info->name);
			}
			return;
#if USE_PERL
		} else if (IS_PERL_MOD(module_ptr)) {
			perl_event_cb(event, cmdparams, module_ptr);
			return;
#endif
		}			
	}
	dlog( DEBUG5, "Module %s has no event handler for %d", module_ptr->info->name, event );
}

/** @brief SendAllModuleEvent
 *
 *	Call event handler for all modules
 *  NeoStats core use only
 *
 *  @param event to send
 *  @param cmdparams
 *
 *  @return none
 */

void SendAllModuleEvent( Event event, CmdParams* cmdparams )
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, GetModuleHash());
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
void AddEvent( ModuleEvent* eventptr )
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if( !mod_ptr->event_list )
		mod_ptr->event_list = ns_calloc( sizeof( ModuleEvent * ) * EVENT_COUNT );
	dlog( DEBUG5, "AddEvent: adding event %d to %s", eventptr->event, mod_ptr->info->name );
	mod_ptr->event_list[eventptr->event] = eventptr;
	if( eventptr->event == EVENT_NICKIP )
		me.want_nickip = 1; 		
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void AddEventList( ModuleEvent *eventlistptr )
{
	while( eventlistptr->event != EVENT_NULL )
	{
		AddEvent( eventlistptr );
		eventlistptr ++;
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void DeleteEvent( Event event )
{
	Module* mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if( mod_ptr->event_list )
		mod_ptr->event_list[event] = NULL;
	dlog( DEBUG5, "DeleteEvent: deleting event %d from %s", event, mod_ptr->info->name );
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void DeleteEventList( ModuleEvent *eventlistptr )
{
	while( eventlistptr->event )
	{
		DeleteEvent( eventlistptr->event );
		eventlistptr++;
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void FreeEventList( Module* mod_ptr )
{
	if( mod_ptr->event_list )
		ns_free( mod_ptr->event_list );
	mod_ptr->event_list = NULL;
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
	ModuleEvent** eventlistptr;

	eventlistptr = GET_CUR_MODULE()->event_list;
	if (eventlistptr) {
		for (i = 0; i < EVENT_COUNT; i++) {
			if (eventlistptr[i]) {
				if (enable)
					eventlistptr[i]->flags |= flag;
				else
					eventlistptr[i]->flags &= ~flag;
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
	ModuleEvent** eventlistptr;

	eventlistptr = GET_CUR_MODULE()->event_list;
	if (eventlistptr)
	{
		if (enable)
			eventlistptr[event]->flags |= flag;
		else
			eventlistptr[event]->flags &= ~flag;
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
