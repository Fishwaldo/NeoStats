/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: m_stats.h,v 1.3 2000/03/29 13:05:57 fishwaldo Exp $
*/

#ifndef M_STATS_H
#define M_STATS_H

#define DecreaseOpers(x)	x->opers--;		stats_network.opers--; 	
#define DecreaseUsers(x)	x->users--;		stats_network.users--; 
#define DecreaseServers()	stats_network.servers--;

#define IncreaseOpers(x)	x->opers++;		stats_network.opers++; 	daily.opers++;
#define IncreaseUsers(x)	x->users++;		stats_network.users++;	x->totusers++;	x->daily_totusers++; 	stats_network.totusers++; 	daily.users++;
#define IncreaseServers()	stats_network.servers++;	daily.servers++;

#endif
