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
** $Id: m_stats.h,v 1.9 2003/04/18 06:53:38 fishwaldo Exp $
*/

#ifndef M_STATS_H
#define M_STATS_H

#define DecreaseOpers(x)	x->opers--;		stats_network.opers--;
#define DecreaseUsers(x)	x->users--;		stats_network.users--;
#define DecreaseServers()	stats_network.servers--;

#define IncreaseOpers(x)	x->opers++;		stats_network.opers++; 	
#define IncreaseUsers(x)	x->users++;		stats_network.users++;	x->totusers++;	stats_network.totusers++; daily.tot_users++;
#define IncreaseServers()	stats_network.servers++;

#define IncreaseKicks(x)	x->kicks++;	x->maxkickstoday++;
#define IncreaseTops(x)		x->topics++;	x->topicstoday++;
#define Increasemems(x)		x->members++;	x->totmem++;	x->lastseen = time(NULL); 	x->joinstoday++;
#define Decreasemems(x)		x->members--;	x->lastseen = time(NULL);
#define IncreaseChans()		stats_network.chans++;
#define DecreaseChans()		stats_network.chans--;
#endif
