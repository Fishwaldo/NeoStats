/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: opsb.c,v 1.4 2002/08/20 09:29:20 fishwaldo Exp $
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

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
};

typedef struct scanq scaninfo;

/* this is the list of items to be queued */
list_t *opsbq;
/* this is the list of currently active scans */
list_t *opsbl;



/* these are some status flags */
#define REPORT_DNS 1


void reportdns(char *data, adns_answer *a);


Module_Info my_info[] = { {
	"OPSB",
	"A Open Proxy Scanning Bot",
	"1.0"
} };


int new_m_version(char *origin, char **av, int ac) {
	snumeric_cmd(351,origin, "Module OPSB Loaded, Version: %s %s %s",my_info[0].module_version,opsbversion_date,opsbversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ MSG_VERSION,	new_m_version,	1 },
	{ TOK_VERSION,	new_m_version,	1 },
	{ NULL,		NULL,		0 }
};


int findscan(const void *key1, const void *key2) {
        const scaninfo *chan1 = key1;
        return (strcasecmp(chan1->who, key2));
}




int __Bot_Message(char *origin, char **argv, int argc)
{
	User *u;
	lnode_t *lnode;
	scaninfo *scandata;

	u = finduser(origin); 
	if (!u) { 
		log("Unable to find user %s (opsb)", origin); 
		return -1; 
	} 
	if (!strcasecmp(argv[1], "lookup")) {
		if (argc < 3) {
			prefmsg(u->nick, s_opsb, "Invalid Syntax. /msg %s help lookup for more help", s_opsb);
			return 0;
		}
		scandata = malloc(sizeof(scaninfo));
		scandata->state = REPORT_DNS;
		strncpy(scandata->who, u->nick, MAXNICK);
		strncpy(scandata->lookup, argv[2], MAXHOST);
		/* if the lists are full, don't add it, and alert the user */
		if (list_isfull(opsbl)) {
			if (list_isfull(opsbq)) {
				prefmsg(u->nick, s_opsb, "Too Busy. Try again Later");
				free(scandata);
				return 0;
			}
			prefmsg(u->nick, s_opsb, "OPSB list is full, queuing your request");
			lnode = lnode_create(scandata);
			list_append(opsbq, lnode);
		}
	
		if (dns_lookup(scandata->lookup, adns_r_a, reportdns, scandata->who) != 1) {
			prefmsg(u->nick, s_opsb, "DnsLookup Failed.");
			free(scandata);
			return 0;
		} 
		lnode = lnode_create(scandata);
		list_append(opsbl, lnode);
	}
	if (!strcasecmp(argv[1], "check")) {
	}
	return 1;
}



int Online(char **av, int ac) {

	if (init_bot(s_opsb,"opsb",me.name,"Proxy Scanning Bot", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_opsb = strcat(s_opsb, "_");
		init_bot(s_opsb,"opsb",me.name,"Proxy Scanning Bot", "+xd", my_info[0].module_name);
	}
	return 1;
};


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
//	{ "SIGNON", 	StartScan},
	{ NULL, 	NULL}
};



Module_Info *__module_get_info() {
	return my_info;
};

Functions *__module_get_functions() {
	return my_fn_list;
};

EventFnList *__module_get_events() {
	return my_event_list;
};


void reportdns(char *data, adns_answer *a) {
	lnode_t *dnslookup;
	scaninfo *dnsinfo;
	char *show;
	int i, len, ri;
	
	dnslookup = list_find(opsbl, data, findscan);
	if (!dnslookup) {
		log("reportdns(): Ehhh, something wrong here");
		return;
	}
	dnsinfo = lnode_get(dnslookup);
	if (a) {
		adns_rr_info(a->type, 0, 0, &len, 0, 0);
		for(i = 0; i < a->nrrs;  i++) {
			ri = adns_rr_info(a->type, 0, 0, 0, a->rrs.bytes +i*len, &show);
			if (!ri) {
				prefmsg(data, s_opsb, "%s resolves to %s", dnsinfo->lookup, show);
			} else {
				prefmsg(data, s_opsb, "DNS error %s", adns_strerror(ri));
			}
			free(show);
		}
		if (a->nrrs < 1) {
			prefmsg(data, s_opsb, "%s Does not resolve", dnsinfo->lookup);
		}
	} else {
		prefmsg(data, s_opsb, "A unknown error occured");
	}	
	
	list_delete(opsbl, dnslookup);
	lnode_destroy(dnslookup);
	free(dnsinfo);
}



void _init() {
	s_opsb = "opsb";
	globops(me.name, "OPSB Module Loaded");

	opsbl = list_create(MAX_SCANS);
	opsbq = list_create(MAX_QUEUE);

}


void _fini() {
	globops(me.name, "OPSB Module Unloaded");
};
