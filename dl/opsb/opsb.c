/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: opsb.c,v 1.13 2002/08/28 16:11:03 fishwaldo Exp $
*/


#include <stdio.h>
#include <fnmatch.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include "dl.h"
#include "stats.h"
#include "opsb.h"

const char opsbversion_date[] = __DATE__;
const char opsbversion_time[] = __TIME__;




void reportdns(char *data, adns_answer *a);
void dnsblscan(char *data, adns_answer *a);
static int ScanNick(char **av, int ac);
int startscan(scaninfo *scandata);
void savecache();
void loadcache();
void unconf();

extern const char *opsb_help[];
extern const char *opsb_help_oper[];
extern const char *opsb_help_lookup[];
extern const char *opsb_help_info[];
extern const char *opsb_help_check[];
extern const char *opsb_help_status[];
extern const char *opsb_help_set[];
extern const char *opsb_help_exclude[];

Module_Info my_info[] = { {
	"OPSB",
	"A Open Proxy Scanning Bot",
	"$Revision: 1.13 $"
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
	exemptinfo *exempts;
	int lookuptype, i;

	u = finduser(origin); 
	if (!u) { 
		log("Unable to find user %s (opsb)", origin); 
		return -1; 
	} 
	if (!strcasecmp(argv[1], "help")) {
		if (argc == 2) {
			privmsg_list(u->nick, s_opsb, opsb_help);
			if (UserLevel(u) >= 50)
				privmsg_list(u->nick, s_opsb, opsb_help_oper);
		} else if (!strcasecmp(argv[2], "lookup")) {
				privmsg_list(u->nick, s_opsb, opsb_help_lookup);
		} else if (!strcasecmp(argv[2], "info")) {
				privmsg_list(u->nick, s_opsb, opsb_help_info);
		} else if ((!strcasecmp(argv[2], "check") && UserLevel(u) >= 50)) {
				privmsg_list(u->nick, s_opsb, opsb_help_check);
		} else if ((!strcasecmp(argv[2], "status") && UserLevel(u) >= 50)) {
				privmsg_list(u->nick, s_opsb, opsb_help_status);
		} else if ((!strcasecmp(argv[2], "set") && UserLevel(u) >= 100)) {
				privmsg_list(u->nick, s_opsb, opsb_help_set);
		} else if ((!strcasecmp(argv[2], "exclude") && UserLevel(u) > 100)) {
				privmsg_list(u->nick, s_opsb, opsb_help_exclude);
		} else {
			prefmsg(u->nick, s_opsb, "Invalid Syntax. /msg %s help for more info", s_opsb);
		}
		return 1;
	} else if (!strcasecmp(argv[1], "info")) {
		privmsg_list(u->nick, s_opsb, opsb_help_info);
		return 1;
	} else if (!strcasecmp(argv[1], "status")) {
		if (UserLevel(u) < 50) {
			prefmsg(u->nick, s_opsb, "Access Denied");
			chanalert(s_opsb, "%s tried to view status, but is not a operator", u->nick);
			return 1;
		}
		send_status(u);
		return 1;
	} else if (!strcasecmp(argv[1], "lookup")) {
		if (argc < 3) {
			prefmsg(u->nick, s_opsb, "Invalid Syntax. /msg %s help lookup for more help", s_opsb);
			return 0;
		}
		scandata = malloc(sizeof(scaninfo));
		scandata->dnsstate = REPORT_DNS;
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
	} else if (!strcasecmp(argv[1], "check")) {
		if (UserLevel(u) < 50) {
			prefmsg(u->nick, s_opsb, "Access Denied");
			chanalert(s_opsb, "%s tried to use check, but does not have access", u->nick);
			return 0;
		}
		if (argc < 3) {
			prefmsg(u->nick, s_opsb, "Invalid Syntax. /msg %s help check for more info", s_opsb);
			return 0;
		}
		if ((list_find(opsbl, argv[2], findscan)) || (list_find(opsbq, argv[2], findscan))) {
			prefmsg(u->nick, s_opsb, "Already Scanning (or in queue) %s. Not Scanning again", argv[2]);
			return 0;
		}
		scandata = malloc(sizeof(scaninfo));
		scandata->u = u;
		if ((u2 = finduser(argv[2])) != NULL) {
			/* don't scan users from my server */
			if (!strcasecmp(u2->server->name, me.name)) {
				prefmsg(u->nick, s_opsb, "Error, Can not scan NeoStats Bots");
				return -1;
			}
			strncpy(scandata->who, u2->nick, MAXHOST);
			strncpy(scandata->lookup, u2->hostname, MAXHOST);
			strncpy(scandata->server, u2->server->name, MAXHOST);
			scandata->ipaddr.s_addr = u2->ipaddr.s_addr;
			if (scandata->ipaddr.s_addr > 0) {
				scandata->dnsstate = DO_OPM_LOOKUP;
			} else {
				if (inet_aton(u2->hostname, &scandata->ipaddr) > 0)
					scandata->dnsstate = DO_OPM_LOOKUP;
				else {
					scandata->dnsstate = GET_NICK_IP;
					scandata->ipaddr.s_addr = 0;
				}
			}
		} else {
			strncpy(scandata->who, argv[2], MAXHOST);
			strncpy(scandata->lookup, argv[2], MAXHOST);
			bzero(scandata->server, MAXHOST);
			if (inet_aton(argv[2], &scandata->ipaddr) > 0) {
				scandata->dnsstate = DO_OPM_LOOKUP;
			} else {
				scandata->dnsstate = GET_NICK_IP;
				scandata->ipaddr.s_addr = 0;
			}
		}
		prefmsg(u->nick, s_opsb, "Checking %s for open Proxies", argv[2]);
		if (!startscan(scandata)) 
			prefmsg(u->nick, s_opsb, "Check Failed");
		
		return 1;
	} else if (!strcasecmp(argv[1], "EXCLUDE")) {
		if (argc < 3) {
			prefmsg(u->nick, s_opsb, "Syntax Error. /msg %s help exclude", s_opsb);
			return 0;
		}
		if (!strcasecmp(argv[2], "LIST")) {
			lnode = list_first(exempt);
			i = 1;
			prefmsg(u->nick, s_opsb, "Exception List:");
			while (lnode) {
				exempts = lnode_get(lnode);
				prefmsg(u->nick, s_opsb, "%d) %s %s", i, exempts->host, (exempts->server ? "(Server)" : "(Client)"));
				++i;
				lnode = list_next(exempt, lnode);
			}
			prefmsg(u->nick, s_opsb, "End of List.");
			chanalert(s_opsb, "%s requested Exception List", u->nick);
		} else if (!strcasecmp(argv[2], "ADD")) {
			if (argc < 5) {
				prefmsg(u->nick, s_opsb, "Syntax Error. /msg %s help exclude", s_opsb);
				return 0;
			}
			if (list_isfull(exempt)) {
				prefmsg(u->nick, s_opsb, "Error, Exception list is full", s_opsb);
				return 0;
			}
			if (!index(argv[3], '.')) {
				prefmsg(u->nick, s_opsb, "Host field does not contain a vaild host");
				return 0;
			}
			exempts = malloc(sizeof(exemptinfo));
			snprintf(exempts->host, MAXHOST, "%s", argv[3]);
			if (atoi(argv[4]) > 0)
				exempts->server = 1;
			else 
				exempts->server = 0;
			lnode = lnode_create(exempts);
			list_append(exempt, lnode);
			prefmsg(u->nick, s_opsb, "Added %s (%s) exception to list", exempts->host, (exempts->server ? "(Server)" : "(Client)"));
			chanalert(s_opsb, "%s added %s (%s) exception to list", u->nick, exempts->host, (exempts->server ? "(Server)" : "(Client)"));
		} else if (!strcasecmp(argv[2], "DEL")) {
			if (argc < 3) {
				prefmsg(u->nick, s_opsb, "Syntax Error. /msg %s help exclude", s_opsb);
				return 0;
			}
			if (atoi(argv[3]) != 0) {
				lnode = list_first(exempt);
				i = 1;
				while (lnode) {
					if (i == atoi(argv[3])) {
						/* delete the entry */
						exempts = lnode_get(lnode);
						list_delete(exempt, lnode);
						prefmsg(u->nick, s_opsb, "Deleted %s %s out of exception list", exempts->host, (exempts->server ? "(Server)" : "(Client)"));
						chanalert(s_opsb, "%s deleted %s %s out of exception list", u->nick, exempts->host, (exempts->server ? "(Server)" : "(Client)"));
						free(exempts);
						return 1;
					}
					++i;
					lnode = list_next(exempt, lnode);
				}		
				/* if we get here, then we can't find the entry */
				prefmsg(u->nick, s_opsb, "Error, Can't find entry %d. /msg %s exclude list", atoi(argv[3]), s_opsb);
				return 0;
			} else {
				prefmsg(u->nick, s_opsb, "Error, Out of Range");
				return 0;
			}
		} else {
			prefmsg(u->nick, s_opsb, "Syntax Error. /msg %s help exclude", s_opsb);
			return 0;
		}
	} else if (!strcasecmp(argv[1], "SET")) {
		if (argc < 3) {
			prefmsg(u->nick, s_opsb, "Syntax Error. /msg %s help set", s_opsb);
			return 0;
		}
		do_set(u, argv, argc);
		if (opsb.confed == 1) 
			savecache();
	} else {
		prefmsg(u->nick, s_opsb, "Syntax Error. /msg %s help", s_opsb);
	}
	return 1;
}

int do_set(User *u, char **av, int ac) {
	char *buf;
	
	if (UserLevel(u) < 100) {
		prefmsg(u->nick, s_opsb, "Access Denied");
		chanalert(s_opsb, "%s tried to set, but doesn't have access");
		return 0;
	}

	if (!strcasecmp(av[2], "TARGETIP")) {
		if (!inet_addr(av[3])) {
			prefmsg(u->nick, s_opsb, "Invalid IP address (Can not be hostname) in TARGETIP");
			return 0;
		}
		snprintf(opsb.targethost, MAXHOST, "%s", av[3]);
		prefmsg(u->nick, s_opsb, "Target IP set to %s", av[3]);
		chanalert(s_opsb, "%s changed the target ip to %s", u->nick, av[3]);
		opsb.confed = 1;
		return 1;
	} else if (!strcasecmp(av[2], "TARGETPORT")) {
		if (!atoi(av[3])) {
			prefmsg(u->nick, s_opsb, "Invalid Port (Must be numeric) in TARGETPORT");
			return 0;
		}
		opsb.targetport = atoi(av[3]);
		prefmsg(u->nick, s_opsb, "Target PORT set to %d", opsb.targetport);
		chanalert(s_opsb, "%s changed the target port to %d", u->nick, opsb.targetport);
		opsb.confed = 1;
		return 1;
	} else if (!strcasecmp(av[2], "OPMDOMAIN")) {
		if (!index(av[3], '.')) {
			prefmsg(u->nick, s_opsb, "Invalid Domain name in OPMDOMAIN");
			return 0;
		}
		snprintf(opsb.opmdomain, MAXHOST, "%s", av[3]);
		prefmsg(u->nick, s_opsb, "OPM Domain changed to %s", opsb.opmdomain);
		chanalert(s_opsb, "%s changed the opm domain to %s", u->nick, opsb.opmdomain);
		opsb.confed = 1;
		return 1;
	} else if (!strcasecmp(av[2], "MAXBYTES")) {
		if (!atoi(av[3])) {
			prefmsg(u->nick, s_opsb, "Invalid setting (Must be numeric)");
			return 0;
		} 
		opsb.maxbytes = atoi(av[3]);
		prefmsg(u->nick, s_opsb, "Max Bytes set to %d", opsb.maxbytes);
		chanalert(s_opsb, "%s changed the Max Bytes setting to %d", u->nick, opsb.maxbytes);
		opsb.confed = 1;
		return 1;
	} else if (!strcasecmp(av[2], "TIMEOUT")) {
		if (!atoi(av[3]) || (atoi(av[3]) > 120)) {
			prefmsg(u->nick, s_opsb, "Setting must be numeric, and below 120");
			return 0;
		}
		opsb.timeout = atoi(av[3]);
		prefmsg(u->nick, s_opsb, "Timeout set to %d", opsb.timeout);
		chanalert(s_opsb, "%s changed the timeout to %d", u->nick, opsb.timeout);
		opsb.confed = 1;
		return 1;
	} else if (!strcasecmp(av[2], "OPENSTRING")) {
		buf = joinbuf(av, ac, 3);
		snprintf(opsb.lookforstring, 512, "%s", buf);
		free(buf);
		prefmsg(u->nick, s_opsb, "OPENSTRING changed to %s", opsb.lookforstring);
		chanalert(s_opsb, "%s changed OPENSTRING to %s", u->nick, opsb.lookforstring);
		opsb.confed = 1;
		return 0;
	} else if (!strcasecmp(av[2], "SPLITTIME")) {
		if (!atoi(av[3])) {
			prefmsg(u->nick, s_opsb, "Error, Setting must be numeric");
			return 0;
		}
		opsb.timedif = atoi(av[3]);
		prefmsg(u->nick, s_opsb, "SPLITTIME changed to %d", opsb.timedif);
		chanalert(s_opsb, "%s changed the split time to %d", u->nick, opsb.timedif);
		opsb.confed = 1;
		return 0;
	} else if (!strcasecmp(av[2], "SCANMSG")) {
		buf = joinbuf(av, ac, 3);
		snprintf(opsb.scanmsg, 512, "%s", buf);
		free(buf);
		prefmsg(u->nick, s_opsb, "ScanMessage changed to %s", opsb.scanmsg);
		chanalert(s_opsb, "%s changed the scan message to %s", u->nick, opsb.scanmsg);
		opsb.confed = 1;
		return 0;
	} else if (!strcasecmp(av[2], "BANTIME")) {
		if (!atoi(av[3])) {
			prefmsg(u->nick, s_opsb, "Error, Bantime must be numeric (in Seconds)");
			return 0;
		}
		opsb.bantime = atoi(av[3]);
		prefmsg(u->nick, s_opsb, "Ban time changed to %d", opsb.bantime);
		chanalert(s_opsb, "%s changed ban time to %d", u->nick, opsb.bantime);
		opsb.confed = 1;
		return 0;
	} else {
		prefmsg(u->nick, s_opsb, "TargetIP: %s", opsb.targethost);
		prefmsg(u->nick, s_opsb, "TargetPort: %d", opsb.targetport);
		prefmsg(u->nick, s_opsb, "OPM Domain: %s", opsb.opmdomain);
		prefmsg(u->nick, s_opsb, "Max Bytes: %d", opsb.maxbytes);
		prefmsg(u->nick, s_opsb, "TimeOut: %d", opsb.timeout);
		prefmsg(u->nick, s_opsb, "Target String: %s", opsb.lookforstring);
		prefmsg(u->nick, s_opsb, "Split Time: %d", opsb.timedif);
		prefmsg(u->nick, s_opsb, "ScanMessage: %s", opsb.scanmsg);
		prefmsg(u->nick, s_opsb, "Ban Time: %d", opsb.bantime);
		prefmsg(u->nick, s_opsb, "Configured: %s", (opsb.confed ? "Yes" : "No"));
		return 0;
	}
	
}

int Online(char **av, int ac) {
	struct sockaddr_in sa;
	socklen_t ulen = sizeof(struct sockaddr_in);

	if (init_bot(s_opsb,"opsb",me.name,"Proxy Scanning Bot", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_opsb = strcat(s_opsb, "_");
		init_bot(s_opsb,"opsb",me.name,"Proxy Scanning Bot", "+xd", my_info[0].module_name);
	}
	loadcache();
	if (opsb.confed == 0) add_mod_timer("unconf", "Un_configured_warn", "opsb", 60);
	unconf();
	if (opsb.confed == 0) {
		getpeername(servsock, (struct sockaddr *)&sa, (socklen_t*)&ulen);
		snprintf(opsb.targethost, MAXHOST, "%s", inet_ntoa(sa.sin_addr));
	}
	add_mod_timer("cleanlist", "CleanProxyList", "opsb", 1);
	add_mod_timer("savecache", "SaveProxyCache", "opsb", 600);
	chanalert(s_opsb, "Open Proxy Scanning bot has started (Concurrent Scans: %d Sockets %d)", opsb.socks, opsb.socks *7);
	return 1;
};


void unconf() {
	if (opsb.confed == 1) return;
	chanalert(s_opsb, "Warning, OPSB is configured with default Settings. Please Update this ASAP");
	globops(s_opsb, "Warning, OPSB is configred with default Settings, Please Update this ASAP");
}

void checkqueue() {
	lnode_t *scannode;
	scaninfo *scandata;
	
	/* exit, if the list is full */
	if (list_isfull(opsbl) || list_isempty(opsbq))
		return;
	
	scannode = list_first(opsbq);
	scandata = lnode_get(scannode);
	list_delete(opsbq, scannode);
	lnode_destroy(scannode);
	startscan(scandata);

}

void addtocache(unsigned long ipaddr) {
	lnode_t *cachenode;
	unsigned long *ip;
	/* pop off the oldest entry */
	if (list_isfull(cache)) {
		cachenode = list_del_last(cache);
		ip = lnode_get(cachenode);
		lnode_destroy(cachenode);
		free(ip);
	}
	cachenode = list_first(cache);
	while (cachenode) {
		ip = lnode_get(cachenode);
		if (*ip == ipaddr) {
#ifdef DEBUG
			log("OPSB: Not adding %ld to cache as it already exists", ipaddr);
#endif
			return;
		}
		cachenode = list_next(cache, cachenode);
	}
	
	ip = malloc(sizeof(unsigned long));
	*ip = ipaddr;
	cachenode = lnode_create(ip);
	list_prepend(cache, cachenode);
}

int checkcache(scaninfo *scandata) {
	lnode_t *node;
	unsigned long *ip;
	exemptinfo *exempts;
	node = list_first(exempt);
	while (node) {
		exempts = lnode_get(node);
		if ((exempts->server == 1) && (scandata->server)) {
			/* match a server */
			if (fnmatch(exempts->host, scandata->server, 0) == 0) {
#ifdef DEBUG
				log("OPSB: User %s exempt. Matched server entry %s in Exemptions", scandata->who, exempts->host);
#endif 
				if (scandata->u) prefmsg(scandata->u->nick, s_opsb,"%s Matches a Server Exception %s", scandata->who, exempts->host);
				return 1;
			}
		} else {
			if (fnmatch(exempts->host, scandata->lookup, 0) == 0) {
#ifdef DEBUG
				log("OPSB: User %s exempt. Matched host entry %s in exemptions", scandata->who, exempts->host);
#endif
				if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "%s Matches a Host Exception %s", scandata->who, exempts->host);
				return 2;
			}
		}
	node = list_next(exempt, node);
	}
	node = list_first(cache);
	while (node) {
		ip = lnode_get(node);
		if (*ip == scandata->ipaddr.s_addr) {
#ifdef DEBUG
			log("OPSB: user %s is already in Cache", scandata->who);
#endif
			if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "User %s is already in Cache", scandata->who);
			return 3;
		}
	node = list_next(cache, node);
	}
	return 0;
}

void savecache() {
	lnode_t *node;
	unsigned long *ip;
	exemptinfo *exempts;
	FILE *fp = fopen("data/opsb.db", "w");	

	if (!fp) {
		log("OPSB: warning, Can not open cache file for writting");
		chanalert(s_opsb, "Warning, Can not open cache file for writting");
		return;
	}
	fprintf(fp, "%s\n", opsb.opmdomain);
	fprintf(fp, "%s\n", opsb.targethost);
	fprintf(fp, "%s\n", opsb.lookforstring);
	fprintf(fp, "%d\n", opsb.targetport);
	fprintf(fp, "%d\n", opsb.maxbytes);
	fprintf(fp, "%d\n", opsb.timeout);
	fprintf(fp, "%d\n", opsb.timedif);
	fprintf(fp, "%s\n", opsb.scanmsg);
	fprintf(fp, "%d\n", opsb.bantime);
	fprintf(fp, "%d\n", opsb.confed);
	/* exempts next */
	node = list_first(exempt);
	while (node) {
		exempts = lnode_get(node);
		fprintf(fp, "%s %d\n", exempts->host, exempts->server);
		node = list_next(exempt, node);
	}
	fprintf(fp, "#CACHE\n");
	node = list_first(cache);
	while (node) {
		ip = lnode_get(node);
		if (*ip < 1) break;
		fprintf(fp, "%ld\n", *ip);
		node = list_next(cache, node);
	}
	fclose(fp);
}

void loadcache() {
	lnode_t *node;
	unsigned long ip;
	exemptinfo *exempts = NULL;
	char buf[512];
	int gotcache = 0;
	FILE *fp = fopen("data/opsb.db", "r");
	char *tmp;

	if (!fp) {
		log("OPSB: Warning, Can not open Cache file for Reading");
		chanalert(s_opsb, "Warning, Can not open Cache file for Reading");
		return;
	}
	fgets(buf, 512, fp);
	snprintf(opsb.opmdomain, MAXHOST, "%s", buf);
	fgets(buf, 512, fp);
	snprintf(opsb.targethost, MAXHOST, "%s", buf);
	fgets(buf, 512, fp);
	snprintf(opsb.lookforstring, 512, "%s", buf);
	fgets(buf, 512, fp);
	opsb.targetport = atoi(buf);
	fgets(buf, 512, fp);
	opsb.maxbytes = atoi(buf);
	fgets(buf, 512, fp);
	opsb.timeout = atoi(buf);
	fgets(buf, 512, fp);
	opsb.timedif = atoi(buf);
	fgets(buf, 512, fp);
	snprintf(opsb.scanmsg, 512, "%s", buf);
	fgets(buf, 512, fp);
	opsb.bantime = atoi(buf);
	fgets(buf, 512, fp);
	opsb.confed = atoi(buf);
	while (fgets(buf, 512, fp)) {
		if (!strcasecmp("#CACHE\n", buf)) {
			gotcache = 1;	
		}
		if (gotcache == 0) {
			if (list_isfull(exempt))
				break;
			exempts = malloc(sizeof(exemptinfo));
			snprintf(exempts->host, MAXHOST, "%s", strtok(buf, " "));
			exempts->server = atoi(strtok(NULL, " "));
			node = lnode_create(exempts);
			list_prepend(exempt, node);			
		} else {
			if (list_isfull(cache))
				break;
			tmp = strtok(buf, "\n");
			ip = strtol(tmp, (char **)NULL, 10);
			if (ip > 0) addtocache(ip);
		}
	}
	fclose(fp);
}


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
	{ "SIGNON", 	ScanNick},
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


/* this function kicks of a scan of a user that just signed on the network */
static int ScanNick(char **av, int ac) {
	User *u;
	scaninfo *scandata;
	lnode_t *scannode;
	
	u = finduser(av[0]);
	if (!u) {
		log("OPSB: Ehhh, Can't find user %s", av[0]);
		return -1;
	}
	
	/* don't scan users from my own server */
	if (!strcasecmp(u->server->name, me.name)) {
		return -1;
	}

	if (time(NULL) - u->TS > opsb.timedif) {
#ifdef DEBUG
		log("Netsplit Nick %s, Not Scanning", av[0]);
#endif
		return -1;
	}

	scannode = list_find(opsbl, av[0], findscan);
	if (!scannode) scannode = list_find(opsbq, av[0], findscan);
	if (scannode) {
		log("ScanNick(): Not scanning %s as we are already scanning them", av[0]);
		return -1;
	}
	prefmsg(u->nick, s_opsb, "%s", opsb.scanmsg);
	scandata = malloc(sizeof(scaninfo));
	scandata->u = NULL;
	strncpy(scandata->who, u->nick, MAXHOST);
	strncpy(scandata->lookup, u->hostname, MAXHOST);
	strncpy(scandata->server, u->server->name, MAXHOST);
	scandata->ipaddr.s_addr = u->ipaddr.s_addr;
	if (scandata->ipaddr.s_addr > 0) {
		scandata->dnsstate = DO_OPM_LOOKUP;
	} else {
		if (inet_aton(u->hostname, &scandata->ipaddr) > 0)
			scandata->dnsstate = DO_OPM_LOOKUP;
		else {
			scandata->dnsstate = GET_NICK_IP;
			scandata->ipaddr.s_addr = 0;
		}
	}
	if (!startscan(scandata)) 
		chanalert(s_opsb, "Warning Can't scan %s", u->nick);

	return 1;


}



/* this function is the entry point for all scans. Any scan you want to kick off is started with this function. */
/* this includes moving scans from the queue to the active list */

int startscan(scaninfo *scandata) {
	lnode_t *scannode;
	unsigned char a, b, c, d;
	char *buf;
	int buflen;
	int i;
	i = checkcache(scandata);
	if ((i > 0) && (!scandata->u)) {
		free(scandata);
		return 1;
	}

	switch(scandata->dnsstate) {
		case GET_NICK_IP:
				if (list_isfull(opsbl)) {
					if (list_isfull(opsbq)) {
						chanalert(s_opsb, "Warning, Both Current and queue lists are full. Not Adding additional scans");
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "To Busy. Try again later");
						free(scandata);
						return 0;
					}
					scannode = lnode_create(scandata);
					list_append(opsbq, scannode);
#ifdef DEBUG
					log("DNS: Added %s to dns queue", scandata->who);
#endif
					if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "Your Request has been added to the Queue");
					return 1;
				}
				if (dns_lookup(scandata->lookup, adns_r_a, dnsblscan, scandata->who) != 1) {
					log("DNS: startscan() GET_NICK_IP dns_lookup() failed");
					free(scandata);
					checkqueue();
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
					checkqueue();
					return 0;
				}
				scannode = lnode_create(scandata);
				list_append(opsbl, scannode);
#ifdef DEBUG
				log("DNS: Added OPM %s lookup to DNS active list", buf);
#endif
				free(buf);
				start_proxy_scan(scannode);
				++opsb.scanned;
				return 1;
				break;
		default:
				log("Warning, Unknown Status in startscan()");
				free(scandata);
				return -1;
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
		switch(scandata->dnsstate) {
			case GET_NICK_IP:
					if (a->nrrs < 1) {
						chanalert(s_opsb, "No Record for %s. Aborting Scan", scandata->lookup);
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "No A record for %s. Aborting Scan", scandata->lookup);
						list_delete(opsbl, scannode);
						lnode_destroy(scannode);
						free(scandata);
						checkqueue();
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
							scandata->dnsstate = DO_OPM_LOOKUP;
							list_delete(opsbl, scannode);
							lnode_destroy(scannode);
							startscan(scandata);
							break;
						} else {
							log("DNS: dnsblscan() GETNICKIP failed-> %s", show);
							checkqueue();
						}

					}
					log("DNS: dnsblscan GETNICKIP rr_info failed");
					list_delete(opsbl, scannode);
					lnode_destroy(scannode);
					free(scandata);
					checkqueue();
					break;
			case DO_OPM_LOOKUP:
					if (a->nrrs > 0) {
						/* TODO: print out what type of open proxy it is based on IP address returned */
						if (scandata->u) prefmsg(scandata->u->nick, s_opsb, "%s apears in DNS blacklist", scandata->lookup);
						scandata->dnsstate = OPMLIST;
						do_ban(scandata);
						checkqueue();
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
	checkqueue();
}



void _init() {

	s_opsb = "opsb";
	globops(me.name, "OPSB Module Loaded");
	

	/* we have to be carefull here. Currently, we have 7 sockets that get opened per connection. Soooo.
	*  we check that MAX_SCANS is not greater than the maxsockets available / 7
	*  this way, we *shouldn't* get problems with running out of sockets 
	*/
	if (MAX_SCANS > me.maxsocks / 7) {
		opsbl = list_create(me.maxsocks /7);
		opsb.socks = me.maxsocks /7;
	} else {
		opsbl = list_create(MAX_SCANS);
		opsb.socks = MAX_SCANS;
	}
	/* queue can be anything we want */
	opsbq = list_create(MAX_QUEUE);

	
	/* scan cache is MAX_QUEUE size (why not?) */
	cache = list_create(MAX_QUEUE);

	exempt = list_create(MAX_EXEMPTS);

					
	sprintf(opsb.opmdomain, "%s", "opm.blitzed.org");
	sprintf(opsb.targethost, "%s", me.uplink);
	opsb.targetport = me.port;
	opsb.maxbytes = 500;
	opsb.timeout = 30;
	opsb.timedif = 30;
	opsb.open = 0;
	opsb.scanned = 0;
	opsb.confed = 0;
	snprintf(opsb.lookforstring, 512, "*** Looking up your hostname...");
	snprintf(opsb.scanmsg, 512, "Your Host is being Scanned for Open Proxies");
}


void _fini() {
	globops(me.name, "OPSB Module Unloaded");
};


void do_ban(scaninfo *scandata) {
	++opsb.open;
	chanalert(s_opsb, "Banning %s", scandata->who);
}
