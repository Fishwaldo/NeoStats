/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _SERVER_H_
#define _SERVER_H_

typedef struct serverstat {
	char name[MAXHOST];
	Client *s;
	time_t ts_start;
	time_t ts_lastseen;
	statistic users;
	statistic opers;
	statistic operkills;
	statistic serverkills;
	statistic splits;
	long lowest_ping;
	time_t t_lowest_ping;
	long highest_ping;
	time_t t_highest_ping;
}serverstat;

extern hash_t *serverstathash;

int ss_event_server (CmdParams *cmdparams);
int ss_event_squit (CmdParams *cmdparams);
int ss_event_pong (CmdParams *cmdparams);
int ss_cmd_map (CmdParams *cmdparams);
int ss_cmd_server (CmdParams *cmdparams);
int ss_cmd_stats (CmdParams *cmdparams);
void InitServerStats (void);
void FiniServerStats (void);
void LoadServerStats (void);
void SaveServerStats (void);
void AddServerUser (Client *u);
void DelServerUser (Client *u);
void AddServerOper (Client *u);
void DelServerOper (Client *u);
void AverageServerStatistics (void);
void ResetServerStatistics (void);

#endif /* _SERVER_H_ */
