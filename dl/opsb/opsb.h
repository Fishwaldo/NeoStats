/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: opsb.h,v 1.2 2002/08/22 07:57:37 fishwaldo Exp $
*/


#ifndef OPSB_H
#define OPSB_H

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
	list_t *socks;
	time_t started;
};

typedef struct scanq scaninfo;

struct opsb {
	char opmdomain[MAXHOST];
	int init;
	char targethost[MAXHOST];
	char lookforstring[512];
	int targetport;
	int maxbytes;
} opsb;

struct sockinfo {
	int sock;
	int (*function)(int sock);
	int flags;
	int type;
	int bytes;
};

typedef struct sockinfo socklist;


/* this is the list of items to be queued */
list_t *opsbq;
/* this is the list of currently active scans */
list_t *opsbl;



/* these are some state flags */
#define REPORT_DNS 	0x0001
#define GET_NICK_IP	0x0002
#define DO_OPM_LOOKUP	0x0004
#define DOING_SCAN	0x0008
#define GOTOPENPROXY	0x0010

/* this is some socklist flags */
#define CONNECTING	0x0001
#define SOCKCONNECTED	0x0002
#define UNCONNECTED	0x0004
#define OPENPROXY	0x0008

/* opsb.c */
int findscan(const void *key1, const void *key2);
void do_ban(scaninfo *scandata);


/* proxy.c */
void start_proxy_scan(lnode_t *scannode);



#endif /* OPSB_H */
