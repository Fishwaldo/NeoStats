/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: m_stats.h,v 1.1 2000/02/05 04:54:00 fishwaldo Exp $
*/

#ifndef M_STATS_H
#define M_STATS_H

#define DecreaseOpers(x)	x->opers--;		me.opers--;
#define DecreaseUsers(x)	x->users--;		me.users--;
#define DecreaseServers()	me.servers--;

#endif
