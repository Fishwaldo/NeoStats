/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

#include "stats.h"
#include "dl.h"
#include "log.h"
#include "conf.h"

static int midnight = 0;

void
chk ()
{
	Mod_Timer *mod_ptr = NULL;
	time_t current = time (NULL);
	hscan_t ts;
	hnode_t *tn;

/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		SET_SEGV_LOCATION();
		mod_ptr = hnode_get (tn);
		if (current - mod_ptr->lastrun > mod_ptr->interval) {
			strncpy (segv_location, mod_ptr->modname, SEGV_LOCATION_BUFSIZE);
			SET_SEGV_INMODULE(mod_ptr->modname);
			if (setjmp (sigvbuf) == 0) {
				if (mod_ptr->function () < 0) {
					nlog(LOG_DEBUG2, LOG_CORE, "Deleting Timer %s for Module %s as requested", mod_ptr->timername, mod_ptr->modname);
					hash_scan_delete(th, tn);
					hnode_destroy(tn);
					free(mod_ptr);
				} else {
					mod_ptr->lastrun = (int) time (NULL);
				}
			} else {
				nlog (LOG_CRITICAL, LOG_CORE, "setjmp() Failed, Can't call Module %s\n", mod_ptr->modname);
			}
			CLEAR_SEGV_INMODULE();
		}
	}
	SET_SEGV_LOCATION();
	if (current - ping.last_sent > me.pingtime) {
		TimerPings ();
		flush_keeper();
		ping.last_sent = time (NULL);
		if (hash_verify (sockh) == 0) {
			nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the socket hash");
		}
		if (hash_verify (mh) == 0) {
			nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the Module hash");
		}
		if (hash_verify (bh) == 0) {
			nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the Bot hash");
		}
		if (hash_verify (th) == 0) {
			nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the Timer hash");
		}
		/* flush log files */
		fflush (NULL);
	}
	if (is_midnight () == 1 && midnight == 0) {
		TimerMidnight ();
		midnight = 1;
	} else {
		if (midnight == 1 && is_midnight () == 0)
			midnight = 0;
	}
}

void
TimerMidnight ()
{
	nlog (LOG_DEBUG1, LOG_CORE, "Its midnight!!! -> %s", sctime (time (NULL)));
	reset_logs ();
}

int
is_midnight ()
{
	time_t current = time (NULL);
	struct tm *ltm = localtime (&current);

	if (ltm->tm_hour == 0)
		return 1;

	return 0;
}
