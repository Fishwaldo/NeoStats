/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: timer.c,v 1.18 2002/09/04 08:42:06 fishwaldo Exp $
*/
 
#include "stats.h"
#include "dl.h"

static time_t last_stats_save;
static time_t last_lag_check;
static time_t last_cache_save;
static int midnight = 0;

void init_timer() {
	if (usr_mds);
}


void chk()
{
	Mod_Timer *mod_ptr = NULL;
	time_t current = time(NULL);
	hscan_t ts;
	hnode_t *tn;

	strcpy(segv_location, "chk");
/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin(&ts, th);
	while ((tn = hash_scan_next(&ts)) != NULL) {
		mod_ptr = hnode_get(tn); 
		if (current - mod_ptr->lastrun > mod_ptr->interval) {
			strcpy(segv_location, mod_ptr->info->module_name);
			strcpy(segvinmodule, mod_ptr->info->module_name);
			if (setjmp(sigvbuf) == 0) {
				mod_ptr->function();
				mod_ptr->lastrun = time(NULL);
			} else {
				log("setjmp() Failed, Can't call Module %s\n", module_ptr->info->module_name);
			}
			strcpy(segvinmodule, "");
			strcpy(segv_location, "Module_Event_Return");
		}
	}

	if (current - ping.last_sent > 60) {
		TimerPings();
		ping.last_sent = time(NULL);
		if (hash_verify(sockh) == 0) {
			log("Eeeek, Corruption of the socket hash");
		}
		if (hash_verify(mh) == 0) {
			log("Eeeek, Corruption of the Module hash");
		}
		if (hash_verify(bh) == 0) {
			log("Eeeek, Corruption of the Bot hash");
		}
		if (hash_verify(th) == 0) {
			log("Eeeek, Corruption of the Timer hash");
		}
	

	}
	if (is_midnight() == 1 && midnight == 0) {
		TimerMidnight();
		midnight = 1;
	} else {
		if (midnight == 1 && is_midnight() == 0)
			midnight = 0;
	} 
}

void TimerReset()
{
	time_t current = time(NULL);
	last_stats_save = current;
	last_lag_check = current;
	last_cache_save = current;
}





void TimerMidnight()
{
#ifdef DEBUG
	log("Its midnight!!! -> %s", sctime(time(NULL)));
#endif
	ResetLogs();
}

int is_midnight()
{
	time_t current = time(NULL);
	struct tm *ltm = localtime(&current);

	if (ltm->tm_hour == 0)
		return 1;

	return 0;	
}
