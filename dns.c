/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id: dns.c,v 1.8 2002/12/26 14:15:07 fishwaldo Exp $
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


/** @brief starts a DNS lookup
 *
 * starts a DNS lookup for str of type type can callback the function
 * when complete. Data is a identifier that is not modified to identify this lookup to the callback function
 *
 * @param str the record to lookup 
 * @param type The type of record to lookup. See adns.h for more details
 * @param callback the function to callback when we are complete
 * @param data a string to pass unmodified to the callback function to help identifing this lookup
 * 
 * @return returns 1 on success, 0 on failure (to add the lookup, not a successfull lookup
*/

int dns_lookup(char *str, adns_rrtype type,  void (*callback)(char *data, adns_answer *a), char *data) {
	lnode_t *dnsnode;
	DnsLookup *dnsdata;
	int status;
	struct sockaddr_in sa;

	strcpy(segv_location, "dns_lookup");

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
		log("DNS: adns_submit error: %s", strerror(status));
		free(dnsdata);
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

	strcpy(segv_location, "init_dns");

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

	strcpy(segv_location, "do_dns");

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
