/* NetStats - IRC Statistical Services Copyright (c) 1999 Adam Rutter,
** Justin Hammond http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: opsb.c,v 1.3 2002/08/20 04:30:47 fishwaldo Exp $
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



struct opsb {
	int dnscount;
	int dnsok;
	char dnsbldomain[125];
	list_t *dnsqueue;
} opsb;

adns_query adnsq;


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
	int i;
	adns_answer *adnsa;
	char *show;

	u = finduser(origin); 
	if (!u) { 
		log("Unable to find user %s (opsb)", origin); 
		return -1; 
	} 
	if (!strcasecmp(argv[1], "lookup")) {
		chanalert(s_opsb, "Looking up %s", argv[2]);
		i = adns_submit(ads, argv[2], adns_r_a, adns_qf_owner|adns_qf_usevc, NULL, &adnsq);
		chanalert(s_opsb, "query %d", i);
	}
	if (!strcasecmp(argv[1], "check")) {
		i = adns_check(ads, &adnsq, &adnsa, NULL);
		if (i == EAGAIN) {
			chanalert(s_opsb, "check again %d", i);
		} else if (i == ESRCH) {
			chanalert(s_opsb, "check esrch %d", i );
		} else {
			chanalert(s_opsb, "check %d", i);
		}
		if (!i) {
		adns_rr_info(adnsa->type, 0,0,0, adnsa->rrs.bytes, &show);
		chanalert(s_opsb, "Answer: %s", show);
		}
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

void _init() {
	s_opsb = "opsb";
	strcpy(opsb.dnsbldomain,"opm.blitzed.org\0");
	globops(me.name, "OPSB Module Loaded");
}


void _fini() {
	globops(me.name, "OPSB Module Unloaded");
};
