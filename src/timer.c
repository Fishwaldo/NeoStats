/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#define TIMER_TABLE_SIZE	300	/* Number of Timers */

/* @brief Module Timer hash list */
static hash_t *timerhash;

static int midnight = 0;
static time_t lastservertimesync = 0;

static int is_midnight (void);
static void run_mod_timers (int ismidnight);

int InitTimers (void)
{
	timerhash = hash_create (TIMER_TABLE_SIZE, 0, 0);
	if(!timerhash) {
		nlog (LOG_CRITICAL, "Unable to create timer hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

void FiniTimers (void)
{
	hash_destroy (timerhash);
}

void
CheckTimers (void)
{
	SET_SEGV_LOCATION();
	if (me.now - ping.last_sent > nsconfig.pingtime) {
		PingServers ();
		flush_keeper ();
		ping.last_sent = me.now;
		/* flush log files */
		fflush (NULL);
	}
	if (is_synched && nsconfig.setservertimes) {
		if((me.now - lastservertimesync) > nsconfig.setservertimes) {
			/* The above check does not need to be exact, but 
			   setting times ought to be so reset me.now */
			me.now = time (NULL);
			irc_svstime (ns_botptr, NULL, me.now);
			lastservertimesync = me.now;
		}
	}
	if (is_midnight () == 1 && midnight == 0) {
		dlog (DEBUG1, "Its midnight!!! -> %s", sctime (me.now));
		run_mod_timers (1);
		ResetLogs ();
		midnight = 1;
	} else {
		run_mod_timers (0);
		if (midnight == 1 && is_midnight () == 0)
			midnight = 0;
	}
}

static int
is_midnight (void)
{
	struct tm *ltm = localtime (&me.now);

	if (ltm->tm_hour == 0 && ltm->tm_min == 0) {
		return 1;
	}
	return 0;
}

/** @brief create new timer
 *
 * For core use only, creates a timer
 *
 * @param name the name of new timer
 * 
 * @return pointer to new timer on success, NULL on error
*/
static Timer *
new_timer (const char *name)
{
	Timer *timer;

	SET_SEGV_LOCATION();
	if (hash_isfull (timerhash)) {
		nlog (LOG_WARNING, "new_timer: timer hash is full");
		return NULL;
	}
	dlog (DEBUG2, "new_timer: %s", name);
	timer = ns_calloc (sizeof (Timer));
	strlcpy (timer->name, name, MAX_MOD_NAME);
	hnode_create_insert (timerhash, timer, name);
	return timer;
}

/** @brief find timer
 *
 * For core use only, finds a timer in the current list of timers
 *
 * @param name the name of timer to find
 * 
 * @return pointer to timer if found, NULL if not found
*/
Timer *
find_timer (const char *name)
{
	Timer *t;

	t = (Timer *)hnode_find (timerhash, name);
	if (!t) {
		dlog (DEBUG3, "find_timer: %s not found", name);
	}
	return t;
}

/** @brief add a timer to the timer list
 *
 * For module use. Adds a timer with the given function to the timer list
 *
 * @param func_name the name of function to register with this timer
 * @param name the name of timer to register
 * @param mod_name the name of module registering the timer
 * @param interval the interval at which the timer triggers in seconds
 * 
 * @return NS_SUCCESS if added, NS_FAILURE if not 
*/
int
add_timer (TIMER_TYPE type, timer_function func_name, const char *name, int interval)
{
	Timer *timer;
	Module* moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if (func_name == NULL) {
		nlog (LOG_WARNING, "Module %s timer %s does not exist", moduleptr->info->name, name);
		return NS_FAILURE;
	}
	if (find_timer (name)) {
		nlog (LOG_WARNING, "Module %s timer %s already exists. Not adding.", moduleptr->info->name, name);
		return NS_FAILURE;
	}
	timer = new_timer (name);
	if (timer) {
		timer->type = type;
		timer->interval = interval;
		timer->lastrun = me.now;
		timer->moduleptr = moduleptr;
		timer->function = func_name;
		dlog (DEBUG2, "add_timer: Module %s added timer %s", moduleptr->info->name, name);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete a timer from the timer list
 *
 * For module use. Deletes a timer with the given name from the timer list
 *
 * @param name the name of timer to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_timer (const char *name)
{
	Timer *timer;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (timerhash, name);
	if (tn) {
		timer = hnode_get (tn);
		dlog(DEBUG2, "del_timer: removed timer %s for module %s", name, timer->moduleptr->info->name);
		hash_delete (timerhash, tn);
		hnode_destroy (tn);
		ns_free (timer);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete all timers from the timer list for given module
 *
 * For core use. 
 *
 * @param 
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_timers (Module *mod_ptr)
{
	Timer *timer;
	hnode_t *tn;
	hscan_t hscan;

	hash_scan_begin (&hscan, timerhash);
	while ((tn = hash_scan_next (&hscan)) != NULL) {
		timer = hnode_get (tn);
		if (timer->moduleptr == mod_ptr) {
			dlog(DEBUG1, "del_timers: deleting timer %s from module %s.", timer->name, mod_ptr->info->name);
			hash_delete (timerhash, tn);
			hnode_destroy (tn);
			ns_free (timer);
		}
	}
	return NS_SUCCESS;
}

/** @brief delete a timer from the timer list
 *
 * For module use. Deletes a timer with the given name from the timer list
 *
 * @param name the name of timer to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
set_timer_interval (const char *name, int interval)
{
	Timer *timer;

	SET_SEGV_LOCATION();
	timer = (Timer *)hnode_find (timerhash, name);
	if (timer) {
		timer->interval = interval;
		dlog (DEBUG2, "set_timer_interval: timer interval for %s (%s) set to %d", name, timer->moduleptr->info->name, interval);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief list timers in use
 *
 * NeoStats command to list the current timers from IRC
 *
 * @param u pointer to user structure of the user issuing the request
 * 
 * @return none
*/
int
ns_cmd_timerlist (CmdParams* cmdparams)
{
	Timer *timer = NULL;
	hscan_t ts;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("Timer List:", cmdparams->source));
	hash_scan_begin (&ts, timerhash);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		timer = hnode_get (tn);
		irc_prefmsg (ns_botptr, cmdparams->source, "%s:", timer->moduleptr->info->name);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Timer: %s", cmdparams->source), timer->name);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Interval: %d", cmdparams->source), timer->interval);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Next run in: %ld", cmdparams->source), (long)(timer->interval - (me.now - timer->lastrun)));
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("End of list.", cmdparams->source));
	return 0;
}

/** @brief run pending module timer functions
 *
 * NeoStats command to run pending timer functions
 *
 * @param none
 * 
 * @return none
*/
static void
run_mod_timers (int ismidnight)
{
	int deleteme = 0;
	Timer *timer = NULL;
	hscan_t ts;
	hnode_t *tn;

	/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin (&ts, timerhash);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		SET_SEGV_LOCATION();
		timer = hnode_get (tn);
		/* If a module is not yet synched, reset it's lastrun */
		if (!timer->moduleptr->synched) {
			timer->lastrun = (int) me.now;
		} else {
			switch (timer->type) {
				case TIMER_TYPE_MIDNIGHT:
					if (!ismidnight)
						continue;
					break;
				case TIMER_TYPE_INTERVAL:
					if (me.now - timer->lastrun < timer->interval) 
						continue;
					break;
				case TIMER_TYPE_COUNTDOWN:
					deleteme = 1;
					break;
			}
			if (setjmp (sigvbuf) == 0) {
				dlog (DEBUG3, "run_mod_timers: Running timer %s for module %s", timer->name, timer->moduleptr->info->name);
				SET_RUN_LEVEL (timer->moduleptr);
				if (timer->function () < 0) {
					dlog (DEBUG2, "run_mod_timers: Deleting Timer %s for Module %s as requested", timer->name, timer->moduleptr->info->name);
					hash_scan_delete (timerhash, tn);
					hnode_destroy (tn);
					ns_free (timer);
				} else {
					timer->lastrun = (int) me.now;
				}
				RESET_RUN_LEVEL();
				if (deleteme) {
					hash_scan_delete (timerhash, tn);
					hnode_destroy (tn);
					ns_free (timer);
				}
			} else {
				nlog (LOG_CRITICAL, "run_mod_timers: setjmp() failed, can't call module %s\n", timer->moduleptr->info->name);
			}
		}
	}
}
