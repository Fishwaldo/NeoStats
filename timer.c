/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      timer.c, 
** Version: 1.5
** Date:    17/11/2001
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

	segv_location = sstrdup("chk");
/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin(&ts, th);
	while ((tn = hash_scan_next(&ts)) != NULL) {
		mod_ptr = hnode_get(tn); 
		if (current - mod_ptr->lastrun > mod_ptr->interval) {
			mod_ptr->function();
			mod_ptr->lastrun = time(NULL);
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
