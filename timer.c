/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: timer.c,v 1.7 2000/06/10 08:48:53 fishwaldo Exp $
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
	register int j;
	segv_loc("chk");
/* First, lets see if any modules have a function that is due to run..... */
	for (j = 0; j < T_TABLE_SIZE; j++) {
		for (mod_ptr = module_timer_lists[j]; mod_ptr; mod_ptr = mod_ptr->next) {
			if (current - mod_ptr->lastrun > mod_ptr->interval) {
				mod_ptr->function();
				mod_ptr->lastrun = time(NULL);
			}
		}
	}
	if (mod_ptr) free(mod_ptr);

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
	Server *s;
	DLL_Return ExitCode;

#ifdef DEBUG
	log("Sendings pings...");
#endif
	ping.ulag = 0;

	s = smalloc(sizeof(Server));
	ExitCode = DLL_CurrentPointerToHead(LL_Servers);
	if (ExitCode == DLL_NORMAL) {
		while (ExitCode == DLL_NORMAL) {
			ExitCode = DLL_GetCurrentRecord(LL_Servers, s);
			if (!strcmp(me.name, s->name)) {
				s->ping = 0;
			} else {
				sts(":%s PING %s :%s", me.name, me.name, s->name);
			}
			if ((ExitCode = DLL_IncrementCurrentPointer(LL_Servers)) == DLL_NOT_FOUND) break;
		}
	}
	if (s) free(s);
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
