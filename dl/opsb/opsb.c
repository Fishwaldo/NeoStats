

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "opsb.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>





void reportdns(char *data, adns_answer *a);
void dnsblscan(char *data, adns_answer *a);

void do_ban(scaninfo *scandata);
void start_proxy_scan(lnode_t *scannode);

Module_Info my_info[] = { {
	"OPSB",
	"A Open Proxy Scanning Bot",
	"$Revision: 1.7 $"
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
	User *u, *u2;
	lnode_t *lnode;
	scaninfo *scandata;
	int lookuptype;

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
		if (inet_aton(scandata->lookup, NULL) > 0) {
			lookuptype = adns_r_ptr;
		} else {
			if (argc == 4) {
				if (!strcasecmp(argv[3], "txt"))
					lookuptype = adns_r_txt;
				else if (!strcasecmp(argv[3], "rp"))
					lookuptype = adns_r_rp;
				else if (!strcasecmp(argv[3], "ns"))
					lookuptype = adns_r_ns;
				else if (!strcasecmp(argv[3], "soa"))
					lookuptype = adns_r_soa;
				else 
					lookuptype = adns_r_a;
			} else {
				lookuptype = adns_r_a;
			}
		}
		if (dns_lookup(scandata->lookup, lookuptype, reportdns, scandata->who) != 1) {
			prefmsg(u->nick, s_opsb, "DnsLookup Failed.");
			free(scandata);
			return 0;
		} 
		lnode = lnode_create(scandata);
		list_append(opsbl, lnode);
	}
	if (!strcasecmp(argv[1], "check")) {

		if (argc < 3) {
			prefmsg(u->nick, s_opsb, "Invalid Syntax. /msg %s help check for more info", s_opsb);
			return 0;
		}
		scandata = malloc(sizeof(scaninfo));
		scandata->u = u;
		if ((u2 = finduser(argv[2])) != NULL) {
			strncpy(scandata->who, u->nick, MAXNICK);
			strncpy(scandata->lookup, u->hostname, MAXHOST);
			scandata->ipaddr.s_addr = u2->ipaddr.s_addr;
			if (scandata->ipaddr.s_addr == 0)
				if (inet_aton(u->hostname, &scandata->ipaddr) > 0)
					scandata->state = DO_OPM_LOOKUP;
				else 
					scandata->state = GET_NICK_IP;
			else
				scandata->state = DO_OPM_LOOKUP;
		} else {
			strncpy(scandata->who, argv[2], MAXNICK);
			strncpy(scandata->lookup, argv[2], MAXNICK);
			if (inet_aton(argv[2], &scandata->ipaddr) > 0) {
				scandata->state = DO_OPM_LOOKUP;
			} else {
				scandata->state = GET_NICK_IP;
				scandata->ipaddr.s_addr = 0;
			}
		}
		if (!startscan(scandata)) 
			prefmsg(u->nick, s_opsb, "Check Failed");
		
		return 1;
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


/* this function is the entry point for all scans. Any scan you want to kick off is started with this function. */
/* this includes moving scans from the queue to the active list */

int startscan(scaninfo *scandata) {
	lnode_t *scannode;
	unsigned char a, b, c, d;
	char *buf;
	int buflen;

	switch(scandata->state) {
		case GET_NICK_IP:
				if (list_isfull(opsbl)) {
					if (list_isfull(opsbq)) {
						chanalert(s_opsb, "Warning, Both Current and queue lists are full. Not Adding additional scans");
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "To Busy. Try again latter");
						free(scandata);
						return 0;
					}
					scannode = lnode_create(scandata);
					list_append(opsbq, scannode);
#ifdef DEBUG
					log("DNS: Added %s to dns queue", scandata->who);
#endif
					return 1;
				}
				if (dns_lookup(scandata->lookup, adns_r_a, dnsblscan, scandata->who) != 1) {
					log("DNS: startscan() GET_NICK_IP dns_lookup() failed");
					free(scandata);
					return 0;
				}
				scannode = lnode_create(scandata);
				list_append(opsbl, scannode);
#ifdef DEBUG
				log("DNS: Added getnickip to DNS active list");
#endif
				return 1;		
				break;
		case DO_OPM_LOOKUP:
				if (list_isfull(opsbl)) {
					if(list_isfull(opsbq)) {
						chanalert(s_opsb, "Warning, Both Current and Queue lists are full, Not adding Scan");
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "Too Busy. Try again Later");
						free(scandata);
						return 0;
					}
					scannode = lnode_create(scandata);
					list_append(opsbq, scannode);
#ifdef DEBUG
					log("DNS: Added OPM lookup to queue", scandata->who);
#endif
					return 1;
				}
        			d = (unsigned char) (scandata->ipaddr.s_addr >> 24) & 0xFF;
                		c = (unsigned char) (scandata->ipaddr.s_addr >> 16) & 0xFF;
                        	b = (unsigned char) (scandata->ipaddr.s_addr >> 8) & 0xFF;
                                a = (unsigned char) scandata->ipaddr.s_addr & 0xFF;
                                
                                /* Enough for a reversed IP and the zone. */
                                buflen = 18 + strlen(opsb.opmdomain);
                                buf = malloc(buflen * sizeof(*buf));
                                                     
                                snprintf(buf, buflen, "%d.%d.%d.%d.%s", d, c, b, a, opsb.opmdomain);
				if (dns_lookup(buf, adns_r_a, dnsblscan, scandata->who) != 1) {
					log("DNS: startscan() DO_OPM_LOOKUP dns_lookup() failed");
					free(scandata);
					return 0;
				}
				scannode = lnode_create(scandata);
				list_append(opsbl, scannode);
#ifdef DEBUG
				log("DNS: Added OPM %s lookup to DNS active list", buf);
#endif
				free(buf);
				start_proxy_scan(scannode);
				return 1;
				break;
		default:
				log("Warning, Unknown Status in startscan()");
				return 0;
	}
}

/* this function is called when either checking the opm list, or when we are trying to resolve the hostname */

void dnsblscan(char *data, adns_answer *a) {
	lnode_t *scannode;
	scaninfo *scandata;
	char *show;
	int len, ri;

	scannode = list_find(opsbl, data, findscan);
	if (!scannode) {
		log("dnsblscan(): Ehhh, Something is wrong here");
		return;
	}
	scandata = lnode_get(scannode);
	if (a) {
		switch(scandata->state) {
			case GET_NICK_IP:
					if (a->nrrs < 1) {
						chanalert(s_opsb, "No Record for %s. Aborting Scan", scandata->lookup);
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "No A record for %s. Aborting Scan", scandata->lookup);
						list_delete(opsbl, scannode);
						lnode_destroy(scannode);
						free(scandata);
						break;
					}
					adns_rr_info(a->type, 0, 0, &len, 0, 0);
					ri = adns_rr_info(a->type, 0, 0, 0, a->rrs.bytes, &show);
					if (!ri) {
#ifdef DEBUG
						log("DNS: Got IP for %s -> %s", scandata->who, show);
#endif
						if (a->nrrs > 1) {
							chanalert(s_opsb, "Warning, More than one IP address for %s. Using %s only", scandata->lookup, show);
							if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "Warning, More than one IP address for %s. Using %s only", scandata->lookup, show);
						}
						if (inet_aton(show, &scandata->ipaddr) > 0) {
							scandata->state = DO_OPM_LOOKUP;
							list_delete(opsbl, scannode);
							lnode_destroy(scannode);
							startscan(scandata);
							break;
						} else {
							log("DNS: dnsblscan() GETNICKIP failed-> %s", show);
						}

					}
					log("DNS: dnsblscan GETNICKIP rr_info failed");
					list_delete(opsbl, scannode);
					lnode_destroy(scannode);
					free(scandata);
 /* TODO: Queue */
					break;
			case DO_OPM_LOOKUP:
					if (a->nrrs > 0) {
						/* TODO: print out what type of open proxy it is based on IP address returned */
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "%s apears in DNS blacklist", scandata->lookup);
						do_ban(scandata);
						list_delete(opsbl, scannode);
						lnode_destroy(scannode);
						free(scandata);
					} else 
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "%s does not appear in DNS black list", scandata->lookup);
					break;
			default:
					log("Warning, Unknown Status in dnsblscan()");
					return;
		}
		return;
			
	}
}

/* this function is to send the results to the user after a lookup command */

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


	/* we don't want to use more than half the available socks */
	if (MAX_SCANS > me.maxsocks / 2)
		opsbl = list_create(maxsocks /2);
	else 
		opsbl = list_create(MAX_SCANS);
	
	/* queue can be anything we want */
	opsbq = list_create(MAX_QUEUE);
	sprintf(opsb.opmdomain, "%s", "opm.blitzed.org");

}


void _fini() {
	globops(me.name, "OPSB Module Unloaded");
};


void do_ban(scaninfo *scandata) {
	chanalert(s_opsb, "Banning %s", scandata->who);
}
void start_proxy_scan(lnode_t *scannode) {
	chanalert(s_opsb, "Starting proxy scan on");
}
