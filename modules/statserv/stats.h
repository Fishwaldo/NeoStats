/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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

#ifndef _STATS_H_
#define _STATS_H_

typedef struct statisticentry {
	unsigned int runningtotal;
	unsigned int average;
	unsigned int max;
	time_t ts_max;
} statisticentry;

typedef struct statistic {
	unsigned int day;
	unsigned int week;
	unsigned int month;
	unsigned int current;
	statisticentry alltime;
	statisticentry daily;
	statisticentry weekly;
	statisticentry monthly;
} statistic;

void AverageStatisticEntry (statisticentry *stat, unsigned int current);
void AverageStatistic (statistic *stat);
int AverageStatistics (void *);
void ResetStatisticEntry (statisticentry *stat, unsigned int current);
void ResetStatistic (statistic *stat);
int ResetStatistics (void *);
int IncStatisticEntry (statisticentry *stat, unsigned int current);
int IncStatistic (statistic *stat);
void DecStatisticEntry (statisticentry *stat, unsigned int current);
void DecStatistic (statistic *stat);
int SetStatisticEntry (statisticentry *stat, unsigned int current, int diff);
int SetStatistic (statistic *stat, int current);
void PreSaveStatistic (statistic *stat);
void PostLoadStatistic (statistic *stat);

int GetAllTimePercent (statistic *stat);
int GetDailyPercent (statistic *stat);
int GetWeeklyPercent (statistic *stat);
int GetMonthlyPercent (statistic *stat);

#endif /* _STATS_H_ */
