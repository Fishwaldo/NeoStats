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

#include "neostats.h"
#include "servers.h"
#include "services.h"
#include "modules.h"
#include "log.h"

/* @brief Module Timer hash list */
static hash_t *th;

static int midnight = 0;
static int lastservertimesync = 0;

static int is_midnight (void);
static void TimerMidnight (void);
void run_mod_timers (void);

int InitTimers (void)
{
	th = hash_create (T_TABLE_SIZE, 0, 0);
	if(!th) {
		nlog (LOG_CRITICAL, "Unable to create timer hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

int FiniTimers (void)
{
	hash_destroy(th);
	return NS_SUCCESS;
}

void
CheckTimers (void)
{
	run_mod_timers();
	SET_SEGV_LOCATION();
	if (me.now - ping.last_sent > me.pingtime) {
		PingServers ();
		flush_keeper();
		ping.last_sent = me.now;
		/* flush log files */
		fflush (NULL);
	}
#ifdef GOTSVSTIME
	if (me.synced && me.setservertimes) {
		if((me.now - lastservertimesync) > me.setservertimes) {
			/* The above check does not need to be exact, but 
			   setting times ought to be so reset me.now */
			me.now = time(NULL);
			ssvstime_cmd (me.now);
			lastservertimesync = me.now;
		}
	}
#endif
	if (is_midnight () == 1 && midnight == 0) {
		TimerMidnight ();
		midnight = 1;
	} else {
		if (midnight == 1 && is_midnight () == 0)
			midnight = 0;
	}
}

static void
TimerMidnight (void)
{
	nlog (LOG_DEBUG1, "Its midnight!!! -> %s", sctime (me.now));
	ResetLogs ();
}

static int
is_midnight (void)
{
	struct tm *ltm = localtime (&me.now);

	if (ltm->tm_hour == 0 && ltm->tm_min == 0)
		return 1;

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
new_timer (char *name)
{
	Timer *timer;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, "new_timer: %s", name);
	timer = malloc (sizeof (Timer));
	strlcpy (timer->name, name, MAX_MOD_NAME);
	tn = hnode_create (timer);
	if (hash_isfull (th)) {
		nlog (LOG_WARNING, "new_timer: couldn't add new timer, hash is full!");
		return NULL;
	}
	hash_insert (th, tn, name);
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
findtimer (char *name)
{
	hnode_t *tn;

	tn = hash_lookup (th, name);
	if (tn)
		return (Timer *) hnode_get (tn);
	return NULL;
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
add_timer (timer_function func_name, char *name, int interval)
{
	Timer *timer;
	Module* moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if (func_name == NULL) {
		nlog (LOG_WARNING, "%s: Timer %s Function doesn't exist", moduleptr->info->name, name);
		return NS_FAILURE;
	}
	timer = new_timer (name);
	if (timer) {
		timer->interval = interval;
		timer->lastrun = me.now;
		timer->moduleptr = moduleptr;
		timer->function = func_name;
		nlog (LOG_DEBUG2, "add_timer: Registered Module %s with timer for Function %s", moduleptr->info->name, name);
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
del_timer (char *name)
{
	Timer *timer;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (th, name);
	if (tn) {
		timer = hnode_get (tn);
		nlog (LOG_DEBUG2, "del_timer: Unregistered Timer function %s from Module %s", name, timer->moduleptr->info->name);
		hash_delete (th, tn);
		hnode_destroy (tn);
		free (timer);
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
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, th);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		timer = hnode_get (modnode);
		if (timer->moduleptr == mod_ptr) {
			nlog (LOG_DEBUG1, "del_timers: deleting timer %s from module %s.", timer->name, mod_ptr->info->name);
			del_timer (timer->name);
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
set_timer_interval (char *name, int interval)
{
	Timer *timer;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (th, name);
	if (tn) {
		timer = hnode_get (tn);
		timer->interval = interval;
		nlog (LOG_DEBUG2, "set_timer_interval: timer interval for %s (%s) set to %d", name, timer->moduleptr->info->name, interval);
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
list_timers (CmdParams* cmdparams)
{
	Timer *timer = NULL;
	hscan_t ts;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Module timer List:");
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		timer = hnode_get (tn);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s:--------------------------------", timer->moduleptr->info->name);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Module Timer Name: %s", timer->name);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Module Interval: %d", timer->interval);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Time till next Run: %ld", (long)(timer->interval - (me.now - timer->lastrun)));
	}
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "End of Module timer List");
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
void
run_mod_timers (void)
{
	Timer *timer = NULL;
	hscan_t ts;
	hnode_t *tn;

/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		SET_SEGV_LOCATION();
		timer = hnode_get (tn);
		if (me.now - timer->lastrun > timer->interval) {
			if (setjmp (sigvbuf) == 0) {
				SET_RUN_LEVEL(timer->moduleptr);
				nlog(LOG_DEBUG3, "run_mod_timers: Running timer %s for module %s", timer->name, timer->moduleptr->info->name);
				if (timer->function () < 0) {
					nlog(LOG_DEBUG2, "run_mod_timers: Deleting Timer %s for Module %s as requested", timer->name, timer->moduleptr->info->name);
					hash_scan_delete(th, tn);
					hnode_destroy(tn);
					free(timer);
				} else {
					timer->lastrun = (int) me.now;
				}
				RESET_RUN_LEVEL();
			} else {
				nlog (LOG_CRITICAL, "run_mod_timers: setjmp() failed, can't call module %s\n", timer->moduleptr->info->name);
			}
		}
	}
}

