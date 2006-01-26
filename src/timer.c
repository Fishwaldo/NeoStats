/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2006 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "servers.h"
#include "services.h"
#include "modules.h"
#include "log.h"
#include "timer.h"
#include "event.h"
#include "main.h"

#define TIMER_TABLE_SIZE	300	/* Number of Timers */

/* @brief Module Timer hash list */
static hash_t *timerhash;
static struct event *timers;

static int midnight = 0;
static time_t lastservertimesync = 0;

/** @brief InitTimers
 *
 *  Init timer subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitTimers( void )
{
	struct timeval tv;

	timerhash = hash_create( TIMER_TABLE_SIZE, 0, 0 );
	if( !timerhash )
	{
		nlog( LOG_CRITICAL, "Unable to create timer hash" );
		return NS_FAILURE;
	}
	timers = os_malloc( sizeof( struct event ) );
	timerclear( &tv );
	tv.tv_sec = 1;
	event_set( timers, 0, EV_TIMEOUT|EV_PERSIST, CheckTimers_cb, NULL );
	event_add( timers, &tv );
	return NS_SUCCESS;
}

/** @brief FiniTimers
 *
 *  Fini timer subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniTimers( void )
{
	hscan_t tscan;
	hnode_t *tn;
	event_del( timers );
	os_free( timers );

	if (hash_count(timerhash) > 0) {
		hash_scan_begin( &tscan, timerhash );
		while( ( tn = hash_scan_next( &tscan ) ) != NULL )
		{
			Timer *timer = NULL;
			timer = hnode_get( tn );
			dlog( DEBUG3, "FiniTimers() BUG: Timer %s not deleted for module %s", timer->name, timer->moduleptr->info->name );
		}
	}
	hash_destroy( timerhash );
}

/** @brief is_midnight
 *
 *  Determine midnight
 *  Timer subsystem use only.
 *
 *  @param none
 *
 *  @return 1 if midnight else 0
 */

static int is_midnight( void )
{
	struct tm *ltm = localtime( &me.now );

	if( ltm->tm_hour == 0 && ltm->tm_min == 0 )
		return 1;
	return 0;
}

/** @brief run_mod_timers
 *
 *  NeoStats command to run pending timer functions
 *
 *  @param ismidnight whether we are at midnight
 * 
 *  @return none
 */

static void run_mod_timers( int ismidnight )
{
	struct tm *ts;
	Timer *timer = NULL;
	hscan_t tscan;
	hnode_t *tn;

	ts = gmtime( &me.now );
	/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin( &tscan, timerhash );
	while( ( tn = hash_scan_next( &tscan ) ) != NULL )
	{
		SET_SEGV_LOCATION();
		timer = hnode_get( tn );
		/* If a module is not yet synched, reset it's lastrun */
		if( !IsModuleSynched( timer->moduleptr ) )
		{
			timer->lastrun = ( int ) me.now;
		}
		else
		{
			switch( timer->type )
			{
				case TIMER_TYPE_DAILY:
					if( !ismidnight )
						continue;
					break;
				case TIMER_TYPE_WEEKLY:
					if( !ismidnight )
						continue;
					if( ts->tm_wday != 0 )
						continue;
					break;
				case TIMER_TYPE_MONTHLY:
					if( !ismidnight )
						continue;
					if( ts->tm_mday != 1 )
						continue;
					break;
				case TIMER_TYPE_INTERVAL:
					if( me.now - timer->lastrun < timer->interval ) 
						continue;
					break;
				case TIMER_TYPE_COUNTDOWN:
					if( me.now - timer->lastrun < timer->interval )
					{
						timer->interval -= ( me.now - timer->lastrun );
						timer->lastrun = me.now;
 						continue;
					}
					break;
			}
			if( setjmp( sigvbuf ) == 0 )
			{
				dlog( DEBUG3, "run_mod_timers: Running timer %s for module %s", timer->name, timer->moduleptr->info->name );
				SET_RUN_LEVEL( timer->moduleptr );
				if( timer->handler( timer->userptr ) < 0 )
				{
					dlog( DEBUG2, "run_mod_timers: Deleting Timer %s for Module %s as requested", timer->name, timer->moduleptr->info->name );
					hash_scan_delete_destroy_node( timerhash, tn );
					ns_free( timer );
				}
				else
				{
					timer->lastrun = ( int ) me.now;
				}
				RESET_RUN_LEVEL();
				if( timer->type == TIMER_TYPE_COUNTDOWN )
				{
					hash_scan_delete_destroy_node( timerhash, tn );
					ns_free( timer );
				}
			}
			else
			{
				nlog( LOG_CRITICAL, "run_mod_timers: setjmp() failed, can't call module %s", timer->moduleptr->info->name );
			}
		}
	}
}

/** @brief CheckTimers_cb
 *
 *  CheckTimers_cb
 *  NeoStats core use only.
 *
 *  @param notused Justin????
 *  @param event Justin????
 *  @param arg Justin????
 *
 *  @return none
 */

void CheckTimers_cb( int notused, short event, void *arg )
{
	struct timeval tv;

	SET_SEGV_LOCATION();
	timerclear( &tv );
	tv.tv_sec = 1;
	if( ( me.now - me.tslastping ) > nsconfig.pingtime )
	{
		PingServers();
		me.tslastping = me.now;
		/* flush log files */
		fflush( NULL );
	}
	if( IsNeoStatsSynched() && nsconfig.setservertimes )
	{
		if( ( me.now - lastservertimesync ) > nsconfig.setservertimes )
		{
			/* The above check does not need to be exact, but 
			   setting times ought to be so reset me.now */
			update_time_now();
			irc_svstime( ns_botptr, NULL, me.now );
			lastservertimesync = me.now;
		}
	}
	if( is_midnight() && midnight == 0 )
	{
		dlog( DEBUG1, "Midnight (%s)", sctime( me.now ) );
		run_mod_timers( 1 );
		ResetLogs();
		midnight = 1;
	}
	else
	{
		run_mod_timers( 0 );
		if( midnight == 1 && is_midnight() == 0 )
			midnight = 0;
	}	
	/* re-add this timeout */
	event_add( timers, &tv );
}

/** @brief create new new_timer
 *
 *  For core use only, creates a timer
 *
 *  @param name of new timer
 * 
 *  @return pointer to new timer on success, NULL on error
 */

static Timer *new_timer( const char *name )
{
	Timer *timer;

	SET_SEGV_LOCATION();
	if( hash_isfull( timerhash ) )
	{
		nlog( LOG_WARNING, "new_timer: timer hash is full" );
		return NULL;
	}
	dlog( DEBUG2, "new_timer: %s", name );
	timer = ns_calloc( sizeof( Timer ) );
	strlcpy( timer->name, name, MAX_MOD_NAME );
	hnode_create_insert( timerhash, timer, name );
	return timer;
}

/** @brief FindTimer
 *
 *  Finds a timer in the current list of timers
 *
 *  @param name the name of timer to find
 * 
 *  @return pointer to timer if found, NULL if not found
 */

Timer *FindTimer( const char *name )
{
	Timer *t;

	t = ( Timer * )hnode_find( timerhash, name );
	if( !t )
		dlog( DEBUG3, "FindTimer: %s not found", name );
	return t;
}

/** @brief AddTimer
 *
 *  For module use. Adds a timer with the given function to the timer list
 *
 *  @param type of timer
 *  @param handler for timer
 *  @param name of timer
 *  @param interval the interval at which the timer triggers in seconds
 *  @param userptr Justin????
 * 
 *  @return NS_SUCCESS if added, NS_FAILURE if not 
 */

int AddTimer( TIMER_TYPE type, timer_handler handler, const char *name, int interval, void *userptr )
{
	Timer *timer;
	Module *moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if( handler == NULL )
	{
		nlog( LOG_WARNING, "Module %s timer %s does not exist", moduleptr->info->name, name );
		return NS_FAILURE;
	}
	if( FindTimer( name ) )
	{
		nlog( LOG_WARNING, "Module %s timer %s already exists. Not adding.", moduleptr->info->name, name );
		return NS_FAILURE;
	}
	timer = new_timer( name );
	if( timer )
	{
		timer->type = type;
		timer->interval = interval;
		timer->lastrun = me.now;
		timer->moduleptr = moduleptr;
		timer->handler = handler;
		timer->userptr = userptr;
		dlog( DEBUG2, "AddTimer: Module %s added timer %s", moduleptr->info->name, name );
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief DelTimer
 *
 *  Deletes a timer with the given name from the timer list
 *  For module use.
 *
 *  @param name the name of timer to delete
 * 
 *  @return NS_SUCCESS if deleted, NS_FAILURE if not found
 */

int DelTimer( const char *name )
{
	Timer *timer;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup( timerhash, name );
	if( tn )
	{
		timer = hnode_get( tn );
		dlog( DEBUG2, "DelTimer: removed timer %s for module %s", name, timer->moduleptr->info->name );
		hash_delete_destroy_node( timerhash, tn );
#ifdef USE_PERL
		if( IS_PERL_MOD( timer->moduleptr ) )
			ns_free( timer->userptr );
#endif /* USE_PERL */
		ns_free( timer );
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief del_timers
 *
 *  delete all timers from the timer list for given module
 *  For core use. 
 *
 *  @param mod_ptr pointer to module to delete timers from
 * 
 *  @return NS_SUCCESS if deleted, NS_FAILURE if not found
 */

int del_timers( const Module *mod_ptr )
{
	Timer *timer;
	hnode_t *tn;
	hscan_t hscan;

	hash_scan_begin( &hscan, timerhash );
	while( ( tn = hash_scan_next( &hscan ) ) != NULL )
	{
		timer = hnode_get( tn );
		if( timer->moduleptr == mod_ptr )
		{
			dlog( DEBUG1, "del_timers: deleting timer %s from module %s.", timer->name, mod_ptr->info->name );
			hash_scan_delete_destroy_node( timerhash, tn );
			ns_free( timer );
		}
	}
	return NS_SUCCESS;
}

/** @brief SetTimerInterval
 *
 *  For module use. Sets interval for timer
 *
 *  @param name of timer to set
 *  @param interval to set
 * 
 *  @return NS_SUCCESS if deleted, NS_FAILURE if not found
 */

int SetTimerInterval( const char *name, int interval )
{
	Timer *timer;

	SET_SEGV_LOCATION();
	timer = FindTimer( name );
	if( timer )
	{
		timer->interval = interval;
		dlog( DEBUG2, "SetTimerInterval: timer interval for %s (%s) set to %d", name, timer->moduleptr->info->name, interval );
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief ns_cmd_timerlist
 *
 *  NeoStats command to list the current timers from IRC
 *
 *  @param cmdparams
 * 
 *  @return none
*/

int ns_cmd_timerlist( const CmdParams *cmdparams )
{
	Timer *timer = NULL;
	hscan_t ts;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Timer List:", cmdparams->source ) );
	hash_scan_begin( &ts, timerhash );
	while( ( tn = hash_scan_next( &ts ) ) != NULL )
	{
		timer = hnode_get( tn );
		irc_prefmsg( ns_botptr, cmdparams->source, "%s:", timer->moduleptr->info->name );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Timer: %s", cmdparams->source ), timer->name );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Interval: %ld", cmdparams->source ), ( long )timer->interval );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Next run in: %ld", cmdparams->source ), ( long )( timer->interval -( me.now - timer->lastrun ) ) );
	}
	irc_prefmsg( ns_botptr, cmdparams->source, __( "End of list.", cmdparams->source ) );
	return 0;
}
