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
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "channel.h"
#include "server.h"
#include "version.h"
#include "tld.h"

static char announce_buf[BUFSIZE];

#define STATNAMESIZE 64

void SaveStatisticEntry (statisticentry *stat, char *tablename, char *entryname, char *statname, char* entrytype)
{
	static char fullstatname[STATNAMESIZE];

	ircsnprintf (fullstatname, STATNAMESIZE, "%s%srunningtotal", statname, entrytype);
	SetData ((void *)stat->runningtotal, CFGINT, tablename, entryname, fullstatname);
	ircsnprintf (fullstatname, STATNAMESIZE, "%s%saverage", statname, entrytype);
	SetData ((void *)stat->average, CFGINT, tablename, entryname, fullstatname);
	ircsnprintf (fullstatname, STATNAMESIZE, "%s%smax", statname, entrytype);
	SetData ((void *)stat->max, CFGINT, tablename, entryname, fullstatname);
	ircsnprintf (fullstatname, STATNAMESIZE, "%s%sts_max", statname, entrytype);
	SetData ((void *)stat->ts_max, CFGINT, tablename, entryname, fullstatname);
}

void SaveStatistic (statistic *stat, char *tablename, char *entryname, char *statname)
{
	SaveStatisticEntry (&stat->daily, tablename, entryname, statname, "daily");
	SaveStatisticEntry (&stat->weekly, tablename, entryname, statname, "weekly");
	SaveStatisticEntry (&stat->monthly, tablename, entryname, statname, "monthly");
	SaveStatisticEntry (&stat->alltime, tablename, entryname, statname, "alltime");
}

void LoadStatisticEntry (statisticentry *stat, char *tablename, char *entryname, char *statname, char* entrytype)
{
	static char fullstatname[STATNAMESIZE];

	ircsnprintf (fullstatname, STATNAMESIZE, "%s%srunningtotal", statname, entrytype);
	GetData ((void *)&stat->runningtotal, CFGINT, tablename, entryname, fullstatname);
	ircsnprintf (fullstatname, STATNAMESIZE, "%s%saverage", statname, entrytype);
	GetData ((void *)&stat->average, CFGINT, tablename, entryname, fullstatname);
	ircsnprintf (fullstatname, STATNAMESIZE, "%s%smax", statname, entrytype);
	GetData ((void *)&stat->max, CFGINT, tablename, entryname, fullstatname);
	ircsnprintf (fullstatname, STATNAMESIZE, "%s%sts_max", statname, entrytype);
	GetData ((void *)&stat->ts_max, CFGINT, tablename, entryname, fullstatname);
}

void LoadStatistic (statistic *stat, char *tablename, char *entryname, char *statname)
{
	LoadStatisticEntry (&stat->daily, tablename, entryname, statname, "daily");
	LoadStatisticEntry (&stat->weekly, tablename, entryname, statname, "weekly");
	LoadStatisticEntry (&stat->monthly, tablename, entryname, statname, "monthly");
	LoadStatisticEntry (&stat->alltime, tablename, entryname, statname, "alltime");
}

void AverageStatisticEntry (statisticentry *stat, unsigned int current)
{
	stat->average = (stat->max + current) / 2;
}

void AverageStatistic (statistic *stat)
{
	AverageStatisticEntry (&stat->daily, stat->current);
	AverageStatisticEntry (&stat->weekly, stat->current);
	AverageStatisticEntry (&stat->monthly, stat->current);
	AverageStatisticEntry (&stat->alltime, stat->current);
}

void ResetStatisticEntry (statisticentry *stat, unsigned int current)
{
	stat->max = current;
	stat->ts_max = me.now;
}

void ResetStatistic (statistic *stat)
{
	struct tm *ts = gmtime(&me.now);

	/* Reset daily on first hour of day (i.e. midnight) */
	if (ts->tm_hour == 0) {
		ResetStatisticEntry (&stat->daily, stat->current);
	}
	/* Reset weekly on first day of week */
	if (ts->tm_wday == 0) {
		ResetStatisticEntry (&stat->weekly, stat->current);
	}
	/* Reset monthly on first day of month */
	if (ts->tm_mday == 1) {
		ResetStatisticEntry (&stat->monthly, stat->current);
	}
}

int IncStatisticEntry (statisticentry *stat, unsigned int current)
{
	int isrecord = 0;

	if (current > stat->max)
	{
		stat->max = current;
		stat->ts_max = me.now;
		isrecord = 1;
	}
	else if (current == stat->max)
	{
		stat->ts_max = me.now;
	}
	stat->runningtotal++;
	return isrecord;
}

int IncStatistic (statistic *stat)
{
	stat->current++;
	IncStatisticEntry (&stat->daily, stat->current);
	IncStatisticEntry (&stat->weekly, stat->current);
	IncStatisticEntry (&stat->monthly, stat->current);
	return IncStatisticEntry (&stat->alltime, stat->current);
}

void DecStatistic (statistic *stat)
{
	stat->current--;
}

static int check_interval()
{
	static time_t lasttime;
	static int count;

	if (!i_am_synched) {
		return -1;
	}
	if ((me.now - lasttime) < StatServ.msginterval ) {
		if (++count > StatServ.msglimit)
			return -1;
	} else {
		lasttime = me.now;
		count = 0;
	}
	return NS_SUCCESS;
}

static void
announce(int announcetype, const char *msg)
{
	switch(announcetype) {
		case 3:
			irc_wallops (ss_bot, "%s", msg);
			break;
		case 2:
			irc_globops (ss_bot, "%s", msg);
			break;
		case 1:
		default:
			irc_chanalert (ss_bot, "%s", msg);
			break;
	}
}

void
announce_record (const char *msg, ...)
{
	va_list ap;

	if(StatServ.recordalert <= 0 || check_interval() < 0) {
		return;
	}
	va_start (ap, msg);
	ircvsnprintf (announce_buf, BUFSIZE, msg, ap);
	va_end (ap);
	announce(StatServ.recordalert, announce_buf);
}

void
announce_lag(const char *msg, ...)
{
	va_list ap;

	if(StatServ.lagalert <= 0 || check_interval() < 0) {
		return;
	}
	va_start (ap, msg);
	ircvsnprintf (announce_buf, BUFSIZE, msg, ap);
	va_end (ap);
	announce(StatServ.lagalert, announce_buf);
}

int ResetStatistics (void)
{
	SET_SEGV_LOCATION();
	dlog (DEBUG1, "Reset Statistics");
	ResetNetworkStatistics ();
	ResetServerStatistics ();
	ResetChannelStatistics ();
	ResetTLDStatistics ();
	return NS_SUCCESS;
}

int AverageStatistics (void)
{
	SET_SEGV_LOCATION();
	dlog (DEBUG1, "Average Statistics");
	AverageNetworkStatistics ();
	AverageServerStatistics ();
	AverageChannelStatistics ();
	AverageTLDStatistics ();
	return NS_SUCCESS;
}

int GetAllTimePercent (statistic *stat)
{
	return (int)(((float) stat->current / (float) stat->alltime.max) * 100);
}

int GetDailyPercent (statistic *stat)
{
	return (int)(((float) stat->current / (float) stat->daily.max) * 100);
}

int GetWeeklyPercent (statistic *stat)
{
	return (int)(((float) stat->current / (float) stat->weekly.max) * 100);
}

int GetMonthlyPercent (statistic *stat)
{
	return (int)(((float) stat->current / (float) stat->monthly.max) * 100);
}
