/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: timer.c,v 1.1 2000/02/03 23:45:56 fishwaldo Exp $
*/
 
#include "stats.h"
#include "dl.h"

static time_t last_stats_save;
static time_t last_lag_check;
static time_t last_cache_save;
static int midnight = 0;

void chk()
{
	Mod_Timer *mod_ptr = NULL;
	time_t current = time(NULL);

/* First, lets see if any modules have a function that is due to run..... */
	mod_ptr = module_timer_lists->next;
	while(mod_ptr != NULL) {
		if (current - mod_ptr->lastrun > mod_ptr->interval) {
			log("timer: Running Module %s Timers",mod_ptr->modname);
			mod_ptr->function();
			mod_ptr->lastrun = time(NULL);
		}
		mod_ptr = mod_ptr->next;
	}
	free(mod_ptr);

/*	if (current - last_stats_save > 600) {
		SaveStats();
		last_stats_save = time(NULL);
	}
*/
	if (current - ping.last_sent > 60) {
		TimerPings();
		ping.last_sent = time(NULL);
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


void TimerPings()
{
	register int i;
	Server *s;

#ifdef DEBUG
	log("Sendings pings...");
#endif
	ping.ulag = 0;

	for (i = 0; i < S_TABLE_SIZE; i++) {
		for (s = serverlist[i]; s; s = s->next) {
			if (!strcmp(me.name, s->name)) {
				s->ping = 0;
				continue;
			}
			sts(":%s PING %s :%s", me.name, me.name, s->name);
		}
	}
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
void WriteStats()
{
	Server *s;
	register int i;

#ifdef DEBUG
	log("Its midnight!!! -> %s", sctime(time(NULL)));
#endif
	for (i = 0; i < S_TABLE_SIZE; i++) {
		for (s = serverlist[i]; s; s = s->next) {
		}
	}
}
