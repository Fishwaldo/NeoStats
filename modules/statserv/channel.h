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

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

typedef enum CHANNEL_SORT 
{
	CHANNEL_SORT_NONE = 0,
	CHANNEL_SORT_MEMBERS,
	CHANNEL_SORT_JOINS,
	CHANNEL_SORT_KICKS,
	CHANNEL_SORT_TOPICS,
}CHANNEL_SORT;

typedef struct channelstat 
{
	char name[MAXCHANLEN];
	Channel *c;
	time_t ts_start;
	time_t ts_lastseen;
	time_t lastsave;
	statistic users;
	statistic kicks;
	statistic topics;
	statistic joins;
}channelstat;

extern list_t *channelstatlist;

typedef void (*ChannelStatHandler) (channelstat *cs, void *v );

void GetChannelStats( ChannelStatHandler handler, CHANNEL_SORT sortstyle, int maxcount, int ignorehidden, void *v );
int topcurrentchannel( const void *key1, const void *key2 );
int topjoinrunningtotalchannel( const void *key1, const void *key2 );
int topkickrunningtotalchannel( const void *key1, const void *key2 );
int toptopicrunningtotalchannel( const void *key1, const void *key2 );
int ss_event_newchan( CmdParams *cmdparams );
int ss_event_delchan( CmdParams *cmdparams );
int ss_event_join( CmdParams *cmdparams );
int ss_event_part( CmdParams *cmdparams );
int ss_event_topic( CmdParams *cmdparams );
int ss_event_kick( CmdParams *cmdparams );
int ss_cmd_channel( CmdParams *cmdparams );
int DelOldChan( void );
void InitChannelStats( void );
void FiniChannelStats( void );
void SaveChanStats( void );
void ResetChannelStatistics( void );
void AverageChannelStatistics( void );

#endif /* _CHANNEL_H_ */
