/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      m_stats.h, 
** Version: 1.3
** Date:    29/03/2000
*/

#ifndef M_STATS_H
#define M_STATS_H

#define DecreaseOpers(x)	x->opers--;		stats_network.opers--;
#define DecreaseUsers(x)	x->users--;		stats_network.users--;
#define DecreaseServers()	stats_network.servers--;

#define IncreaseOpers(x)	x->opers++;		stats_network.opers++; 	
#define IncreaseUsers(x)	x->users++;		stats_network.users++;	x->totusers++;	stats_network.totusers++;
#define IncreaseServers()	stats_network.servers++;

#define IncreaseKicks(x)	x->kicks++;	x->members--;	x->maxkickstoday++;
#define IncreaseTops(x)		x->topics++;	x->topicstoday++;
#define Increasemems(x)		x->members++;	x->totmem++;	x->lastseen = time(NULL); 	x->joinstoday++;
#define Decreasemems(x)		x->members--;	x->lastseen = time(NULL);
#define IncreaseChans()		stats_network.chans++;
#define DecreaseChans()		stats_network.chans--;
#endif
