/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: opsb.c,v 1.2 2002/07/30 04:26:28 fishwaldo Exp $
*/


#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <ares.h>

const char opsbversion_date[] = __DATE__;
const char opsbversion_time[] = __TIME__;
char *s_opsb;

struct opsb {
	ares_channel channel;
	int dnscount;
	int dnsok;
	char dnsbldomain[125];
	list_t *dnsqueue;
} opsb;

static void dnscallback(void *arg, int status, unsigned char *abuf, int alen);
void dodnsqueue();


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


int __Bot_Message(char *origin, char **argv, int argc)
{
	User *u;
	u = finduser(origin); 
	if (!u) { 
		log("Unable to find user %s (opsb)", origin); 
		return -1; 
	} 
	return 1;
}



int StartScan(char **av, int ac) {
	User *u;
	char *dnsbl;
	lnode_t *dnsnode;
	if (!is_synced) {
		/* don't scan users when we are just hooking up to the net */
		return 0;
	}
	if (opsb.dnsok == 0) {
		chanalert(s_opsb, "Not Looking up ip address of %s", av[0]);
		goto scanproxy;
	}


	u = finduser(av[0]);
	if (!u) {
#ifdef DEBUG
		log("Cant Find user %s (OPSB)", av[0]);
#endif
		return 0;
	}
	if (u->ipaddr.s_addr == 0) {
		chanalert(s_opsb, "Not Scanning %s due to Invalid IP address", av[0]);
		return 0;
	}
	if (opsb.dnscount > 5) {
		chanalert(s_opsb, "Queuing %s for DNS Check due to queue full", av[0]);
		dnsnode = lnode_create(u->nick);
		if (list_isfull(opsb.dnsqueue)) {
			chanalert(s_opsb, "Warning DNSBL queue full. Not Checking %s", av[0]);
		} else {
			list_append(opsb.dnsqueue, dnsnode);
		}
	}
	dnsbl = malloc(18 + strlen(opsb.dnsbldomain));
	sprintf(dnsbl, "%s.%s", inet_ntoa(u->ipaddr), opsb.dnsbldomain);
	opsb.dnscount++;
#ifdef DEBUG
		log("OPSB: looking up %s in DNS Blacklist", dnsbl);
#endif
	ares_query(opsb.channel, dnsbl, C_IN, T_TXT, dnscallback, u->nick);
	free(dnsbl);

scanproxy:
	/* add the code to kick off the scan proxy stuff */
	return 1;
}

void checkares() {
	fd_set read_fds, write_fds;
  	struct timeval *tvp, tv;
	int nfsd;

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
//	nfsd = ares_fds(opsb.channel, &read_fds, &write_fds);
	if (nfsd == 0)
		/* nothing to check! */
		return;
	tvp = ares_timeout(opsb.channel, NULL, &tv);
	select(nfsd, &read_fds, &write_fds, NULL, tvp);
        ares_process(opsb.channel, &read_fds, &write_fds);			
}

static void dnscallback(void *arg, int status, unsigned char *abuf, int alen) {
  	char *mem;
	User *u;
  	opsb.dnscount--;
  	if (status == ARES_ENOTFOUND) {
  		dodnsqueue();
  		return;
  	}
  	if (status != ARES_SUCCESS)
    	 {
      		chanalert(s_opsb, "Warning (%s) %s", (char *) arg, ares_strerror(status, &mem));
      		log("OPSB Warning: %s", ares_strerror(status, &mem));
      		ares_free_errmem(mem);
      		dodnsqueue();
      		return;
    	 }
    	u = finduser((char *)arg);
	if (!u) {
		/* user has left network? */
		return;
	}
  	chanalert(s_opsb, "Alert, DNSBL contains %s!!! Banning from network", u->hostname);
  	/* doban */

	

	/* this checks the dnsqueue for any more lookups */
	dodnsqueue();
	return;
}

void dodnsqueue() {
	lnode_t *dnsnode;
	char *nick;
	char *dnsbl;
	User *u;
	if (list_count(opsb.dnsqueue) > 0) {
		dnsnode = list_del_first(opsb.dnsqueue);
		nick = lnode_get(dnsnode);
		u = finduser(nick);
		/* if the user doesn't exist, don't go on */
		if (!u) return;
		dnsbl = malloc(18 + strlen(opsb.dnsbldomain));
		sprintf(dnsbl, "%s.%s", inet_ntoa(u->ipaddr), opsb.dnsbldomain);

#ifdef DEBUG
		log("OPSB: Checking %s from dnsqueue", dnsbl);
#endif
		lnode_destroy(dnsnode);
	        ares_query(opsb.channel, dnsbl, C_IN, T_TXT, dnscallback, u->nick);
		opsb.dnscount++;
		free(dnsbl);
	}

}
int Online(char **av, int ac) {

	if (init_bot(s_opsb,"opsb",me.name,"Proxy Scanning Bot", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_opsb = strcat(s_opsb, "_");
		init_bot(s_opsb,"opsb",me.name,"Proxy Scanning Bot", "+xd", my_info[0].module_name);
	}
	add_mod_timer("checkares", "Check_OPSB_DNS", "opsb", 1);
	return 1;
};


EventFnList my_event_list[] = {
	{ "ONLINE", 	Online},
	{ "SIGNON", 	StartScan},
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

void _init() {
	int status;
	s_opsb = "opsb";
	status = ares_init(&opsb.channel);
	if (status != ARES_SUCCESS) {
		globops(me.name, "OPSB failed to initilize DNS");
		opsb.dnsok = 0;
	}
	opsb.dnsok = 1;
	opsb.dnscount = 0;
	opsb.dnsqueue = list_create(1000);
	strcpy(opsb.dnsbldomain,"opm.blitzed.org\0");
	globops(me.name, "OPSB Module Loaded");
}


void _fini() {
	ares_destroy(opsb.channel);
	globops(me.name, "OPSB Module Unloaded");

};
