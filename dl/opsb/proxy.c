/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: proxy.c,v 1.1 2002/08/22 03:16:06 fishwaldo Exp $
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

void start_proxy_scan(lnode_t *scannode) {
	chanalert(s_opsb, "Starting proxy scan on");
}
