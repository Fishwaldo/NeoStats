/* NeoStats - IRC Statistical Services Copyright (c) 1999-2001 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      dns.c, 
** Version: 1.5
** Date:    17/11/2001
*/


/* this file does the dns checking for adns. it provides a callback mechinism for dns lookups
** so that DNS lookups will not block. It uses the adns libary (installed in the adns directory
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "stats.h"
#include <adns.h>

struct dnslookup_struct {
	adns_query q;
	adns_answer *a;
	char data[255];
	void (*callback)(char *data, adns_answer *a);
};

typedef struct dnslookup_struct DnsLookup;

list_t *dnslist;

/* dns_lookup
** inputs	- adns_query struct of what we are looking up
**		- callback function when we get a answer
**		- data string to pass to callback function to identify this lookup
** 
** outputs	- integer saying if the addition to lookup succeded. (1 = yes, 0 = no)
**
** side effects - Adds the query to the list dnslist for lookups
** 
** use this function to add a dns query to the list of ones that need to be performed.
*/

int dns_lookup(char *str, adns_rrtype type,  void (*callback)(char *data, adns_answer *a), char *data) {
	lnode_t *dnsnode;
	DnsLookup *dnsdata;
	int status;
	struct sockaddr_in sa;

	if (list_isfull(dnslist)) {
#ifdef DEBUG
		log("DNS: Lookup list is full");
#endif	
		return 0;
	}
	dnsdata = malloc(sizeof(DnsLookup));
	if (!dnsdata) {
		log("DNS: Out of Memory");
		return 0;
	}
	strncpy(dnsdata->data, data, 254);
	dnsdata->callback = callback;
	if (type == adns_r_ptr) {
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr(str);
		status = adns_submit_reverse(ads, (const struct sockaddr*)&sa, type, adns_qf_owner|adns_qf_cname_loose, NULL, &dnsdata->q);
	} else {
		status = adns_submit(ads, str, type, adns_qf_owner|adns_qf_cname_loose, NULL, &dnsdata->q);
	}
	if (status) {
#ifdef DEBUG
		log("DNS: adns_submit error: %s", strerror(status));
#endif
		return 0;
	}
	
#ifdef DEBUG
	log("DNS: Added dns query %s to list", data);
#endif
	/* if we get here, then the submit was successfull. Add it to the list of queryies */
	dnsnode = lnode_create(dnsdata);
	list_append(dnslist, dnsnode);

	return 1;
}


/* init_dns
** inputs	- Nothing
**
** outputs 	- integer saying if the dns init was successfull (1 = yes, 0 = no)
** 
** side effects - Initilizes the dns lookup list_t
**
** use this function to initilize the dns functions
*/

int init_dns() {
	int adnsstart;

	dnslist = list_create(DNS_QUEUE_SIZE);
	if (!dnslist)
		return 0;
#ifndef DEBUG
        adnsstart = adns_init(&ads, adns_if_noerrprint|adns_if_noautosys, 0);
#else 
        adnsstart = adns_init(&ads, adns_if_debug|adns_if_noautosys, 0);
#endif
        if (adnsstart) {
	        printf("ADNS init failed: %s\n", strerror(adnsstart));
		return 0;
	}
	return 1;

}

/* do_dns
** inputs 	- Nothing
** 
** outputs 	- Nothing
** 
** side effects - Goes through the dnslist of pending queries and calls the callback function for each lookup
** 		  with the adns_answer set. Always calls the callback function even if the lookup was unsuccessfull
** 		  its upto the callback function to make check the answer struct to see if it failed or not
** 
** This function is called each loop. 
*/

void do_dns() {
	lnode_t *dnsnode, *dnsnode1;
	int status;
	DnsLookup *dnsdata;

	/* if the list is empty, no use doing anything */
	if (list_isempty(dnslist))
		return;
	
	dnsnode = list_first(dnslist);
	while (dnsnode) {
		/* loop through the list */
		dnsdata = lnode_get(dnsnode);
		status = adns_check(ads, &dnsdata->q, &dnsdata->a, NULL);
		/* if status == eagain, the lookup hasn't completed yet */
		if (status == EAGAIN) {
#ifdef DEBUG
			log("DNS: Lookup hasn't completed for %s", &dnsdata->data);
#endif
			dnsnode = list_next(dnslist, dnsnode);
			break;
		}
		/* there was a error */
		if (status) {
			log("DNS: Baaaad error on adns_check: %s. Please report to NeoStats Group", strerror(status));
			chanalert(s_Services, "Bad Error on DNS lookup. Please check logfile");

			/* call the callback function with answer set to NULL */
			dnsdata->callback(dnsdata->data, NULL);

			/* delete from list */
			dnsnode1 = list_delete(dnslist, dnsnode); 
			dnsnode = list_next(dnslist, dnsnode1);
			free(dnsdata->a);
			free(dnsdata);
			lnode_destroy(dnsnode1);
			break;
		}
#ifdef DEBUG
		log("DNS: Calling callback function with data %s", dnsdata->data);
#endif		
		/* call the callback function */
		dnsdata->callback(dnsdata->data, dnsdata->a);

		/* delete from list */
		dnsnode1 = list_delete(dnslist, dnsnode);
		dnsnode = list_next(dnslist, dnsnode1);
		free(dnsdata->a);
		free(dnsdata);
		lnode_destroy(dnsnode1);
	}

}
