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
#include "conf.h"
#include "server.h"

/* @brief Module Timer hash list */
static hash_t *th;

static int midnight = 0;
#ifdef GOTSVSTIME
static int lastservertimesync = 0;
#endif

static int is_midnight (void);
static void TimerMidnight (void);
void run_mod_timers (void);

int InitTimers (void)
{
	th = hash_create (T_TABLE_SIZE, 0, 0);
	if(!th)
		return NS_FAILURE;
	return NS_SUCCESS;
}

int FiniTimers (void)
{
	hash_destroy(th);
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
	nlog (LOG_DEBUG1, LOG_CORE, "Its midnight!!! -> %s", sctime (me.now));
	reset_logs ();
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
 * @param timer_name the name of new timer
 * 
 * @return pointer to new timer on success, NULL on error
*/
static ModTimer *
new_timer (char *timer_name)
{
	ModTimer *mod_tmr;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "new_timer: %s", timer_name);
	mod_tmr = malloc (sizeof (ModTimer));
	strlcpy (mod_tmr->timername, timer_name, MAX_MOD_NAME);
	tn = hnode_create (mod_tmr);
	if (hash_isfull (th)) {
		nlog (LOG_WARNING, LOG_CORE, "new_timer: couldn't add new timer, hash is full!");
		return NULL;
	}
	hash_insert (th, tn, timer_name);
	return mod_tmr;
}

/** @brief find timer
 *
 * For core use only, finds a timer in the current list of timers
 *
 * @param timer_name the name of timer to find
 * 
 * @return pointer to timer if found, NULL if not found
*/
ModTimer *
findtimer (char *timer_name)
{
	hnode_t *tn;

	tn = hash_lookup (th, timer_name);
	if (tn)
		return (ModTimer *) hnode_get (tn);
	return NULL;
}

/** @brief add a timer to the timer list
 *
 * For module use. Adds a timer with the given function to the timer list
 *
 * @param func_name the name of function to register with this timer
 * @param timer_name the name of timer to register
 * @param mod_name the name of module registering the timer
 * @param interval the interval at which the timer triggers in seconds
 * 
 * @return NS_SUCCESS if added, NS_FAILURE if not 
*/
int
add_mod_timer (timer_function func_name, char *timer_name, char *mod_name, int interval)
{
	ModTimer *mod_tmr;

	SET_SEGV_LOCATION();
	if (func_name == NULL) {
		nlog (LOG_WARNING, LOG_CORE, "%s: Timer %s Function %s doesn't exist", mod_name, timer_name, func_name);
		return NS_FAILURE;
	}
	mod_tmr = new_timer (timer_name);
	if (mod_tmr) {
		mod_tmr->interval = interval;
		mod_tmr->lastrun = me.now;
		strlcpy (mod_tmr->modname, mod_name, MAX_MOD_NAME);
		mod_tmr->function = func_name;
		nlog (LOG_DEBUG2, LOG_CORE, "add_mod_timer: Registered Module %s with timer for Function %s", mod_name, func_name);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete a timer from the timer list
 *
 * For module use. Deletes a timer with the given name from the timer list
 *
 * @param timer_name the name of timer to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_mod_timer (char *timer_name)
{
	ModTimer *mod_tmr;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (th, timer_name);
	if (tn) {
		mod_tmr = hnode_get (tn);
		nlog (LOG_DEBUG2, LOG_CORE, "del_mod_timer: Unregistered Timer function %s from Module %s", timer_name, mod_tmr->modname);
		hash_delete (th, tn);
		hnode_destroy (tn);
		free (mod_tmr);
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
del_mod_timers (char *module_name)
{
	ModTimer *mod_tmr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, th);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_tmr = hnode_get (modnode);
		if (!ircstrcasecmp (mod_tmr->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "del_mod_timers: Module %s has timer %s Registered. Deleting..", module_name, mod_tmr->timername);
			del_mod_timer (mod_tmr->timername);
		}
	}
	return NS_SUCCESS;
}

/** @brief delete a timer from the timer list
 *
 * For module use. Deletes a timer with the given name from the timer list
 *
 * @param timer_name the name of timer to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
change_mod_timer_interval (char *timer_name, int interval)
{
	ModTimer *mod_tmr;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (th, timer_name);
	if (tn) {
		mod_tmr = hnode_get (tn);
		mod_tmr->interval = interval;
		nlog (LOG_DEBUG2, LOG_CORE, "change_mod_timer_interval: Changing timer interval for %s from Module %s", timer_name, mod_tmr->modname);
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
list_timers (User * u, char **av, int ac)
{
	ModTimer *mod_tmr = NULL;
	hscan_t ts;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Module timer List:");
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		mod_tmr = hnode_get (tn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_tmr->modname);
		prefmsg (u->nick, s_Services, "Module Timer Name: %s", mod_tmr->timername);
		prefmsg (u->nick, s_Services, "Module Interval: %d", mod_tmr->interval);
		prefmsg (u->nick, s_Services, "Time till next Run: %ld", (long)(mod_tmr->interval - (me.now - mod_tmr->lastrun)));
	}
	prefmsg (u->nick, s_Services, "End of Module timer List");
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
	ModTimer *mod_tmr = NULL;
	hscan_t ts;
	hnode_t *tn;

/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		SET_SEGV_LOCATION();
		mod_tmr = hnode_get (tn);
		if (me.now - mod_tmr->lastrun > mod_tmr->interval) {
			strlcpy (segv_location, mod_tmr->modname, SEGV_LOCATION_BUFSIZE);
			if (setjmp (sigvbuf) == 0) {
				SET_SEGV_INMODULE(mod_tmr->modname);
				nlog(LOG_DEBUG3, LOG_CORE, "run_mod_timers: Running timer %s for module %s", mod_tmr->timername, mod_tmr->modname);
				if (mod_tmr->function () < 0) {
					nlog(LOG_DEBUG2, LOG_CORE, "run_mod_timers: Deleting Timer %s for Module %s as requested", mod_tmr->timername, mod_tmr->modname);
					hash_scan_delete(th, tn);
					hnode_destroy(tn);
					free(mod_tmr);
				} else {
					mod_tmr->lastrun = (int) me.now;
				}
				CLEAR_SEGV_INMODULE();
			} else {
				nlog (LOG_CRITICAL, LOG_CORE, "run_mod_timers: setjmp() failed, can't call module %s\n", mod_tmr->modname);
			}
		}
	}
}

