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

static int midnight = 0;
#ifdef GOTSVSTIME
static int lastservertimesync = 0;
#endif

static int is_midnight (void);
static void TimerMidnight (void);

void
CheckTimers (void)
{
	run_mod_timers();
	SET_SEGV_LOCATION();
	if (me.now - ping.last_sent > me.pingtime) {
		PingServers ();
		flush_keeper();
		ping.last_sent = me.now;
#ifdef DEBUG
		verify_hashes();
#endif
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
