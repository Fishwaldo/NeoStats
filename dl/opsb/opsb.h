/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: opsb.h,v 1.1 2002/08/22 03:16:06 fishwaldo Exp $
*/


#ifndef OPSB_H
#define OPSB_H

const char opsbversion_date[] = __DATE__;
const char opsbversion_time[] = __TIME__;
char *s_opsb;


/* max scans in the max concurrent scans at any one time */
#define MAX_SCANS 100
/* max queue is the max amount of scans that may be concurrent and queued. */
#define MAX_QUEUE MAX_SCANS * 100



struct scanq {
	char who[MAXNICK];
	int state;
	char lookup[MAXHOST];
	struct in_addr ipaddr;
	User *u;
	int doreport;
};

typedef struct scanq scaninfo;

struct opsb {
	char opmdomain[MAXHOST];
	int init;
} opsb;


/* this is the list of items to be queued */
list_t *opsbq;
/* this is the list of currently active scans */
list_t *opsbl;



/* these are some status flags */
#define REPORT_DNS 	0x0001
#define GET_NICK_IP	0x0002
#define DO_OPM_LOOKUP	0x0004


#endif /* OPSB_H */