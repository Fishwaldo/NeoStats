/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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
** $Id$
*/


/* this file does the dns checking for adns. it provides a callback mechinism for dns lookups
** so that DNS lookups will not block. It uses the adns libary (installed in the adns directory
*/
#include "neostats.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "dns.h"
#include "services.h"

#define DNS_QUEUE_SIZE  300	/* number on concurrent DNS lookups */
#define DNS_DATA_SIZE	255

typedef struct DnsLookup {
	adns_query q;	/**< the ADNS query */
	adns_answer *a;	/**< the ADNS result if we have completed */
	adns_rrtype type; /**< the type we are looking for, only populated if we add to a queue */
	char data[DNS_DATA_SIZE];	/**< the User data based to the callback */
	char lookupdata[255]; /**< the look up data, only populated if we add to a queue */
	void (*callback) (char *data, adns_answer * a);
						      /**< a function pointer to call when we have a result */
	Module* modptr;
} DnsLookup;

adns_state ads;

struct DNSStats {
	int totalq;
	int maxqueued;
	int totalqueued;
	int success;
	int failure;
} DNSStats;

/** @brief List of DNS queryies
 *  Contains DnsLookup entries 
 */
static list_t *dnslist;

/** @brief list of DNS queries that are queued up
 * 
 */
list_t *dnsqueue;

void dns_check_queue();

/** @brief starts a DNS lookup
 *
 * starts a DNS lookup for str of type type can callback the function
 * when complete. Data is an identifier that is not modified to identify this lookup to the callback function
 *
 * @param str the record to lookup 
 * @param type The type of record to lookup. See adns.h for more details
 * @param callback the function to callback when we are complete
 * @param data a string to pass unmodified to the callback function to help identifing this lookup
 * 
 * @return returns 1 on success, 0 on failure (to add the lookup, not a successful lookup
*/
int dns_lookup (char *str, adns_rrtype type, void (*callback) (char *data, adns_answer * a), char *data)
{
	DnsLookup *dnsdata;
	int status;
	struct sockaddr_in sa;

	SET_SEGV_LOCATION();
	dnsdata = ns_calloc (sizeof (DnsLookup));
	DNSStats.totalq++;
	if (!dnsdata) {
		nlog (LOG_CRITICAL, "DNS: Out of Memory");
		DNSStats.failure++;
		return 0;
	}
	/* set the module name */
	dnsdata->modptr = GET_CUR_MODULE();
	strlcpy (dnsdata->data, data, DNS_DATA_SIZE);
	dnsdata->callback = callback;
	dnsdata->type = type;
	if (list_isfull (dnslist)) {
		dlog(DEBUG1, "DNS: Lookup list is full, adding to queue");
		strlcpy(dnsdata->lookupdata, str, 254);
		lnode_create_append (dnslist, dnsdata);
		DNSStats.totalqueued++;
		if (list_count(dnsqueue) > DNSStats.maxqueued) {
			DNSStats.maxqueued = list_count(dnsqueue);
		}
		return NS_SUCCESS;
	}
	if (type == adns_r_ptr) {
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr (str);
		status = adns_submit_reverse (ads, (const struct sockaddr *) &sa, type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
	} else {
		status = adns_submit (ads, str, type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
	}
	if (status) {
		nlog (LOG_WARNING, "DNS: adns_submit error: %s", strerror (status));
		ns_free (dnsdata);
		DNSStats.failure++;
		return 0;
	}
	dlog(DEBUG1, "DNS: Added dns query %s to list", data);
	/* if we get here, then the submit was successful. Add it to the list of queryies */
	lnode_create_append (dnslist, dnsdata);
	return 1;
}

/** @brief sets up DNS subsystem
 *
 * configures ADNS for use with NeoStats.
 *
 * @return returns 1 on success, 0 on failure
*/
int InitDns (void)
{
	int adnsstart;

	SET_SEGV_LOCATION();
	dnslist = list_create (DNS_QUEUE_SIZE);
	if (!dnslist) {
		nlog (LOG_CRITICAL, "Unable to create DNS list");
		return NS_FAILURE;
	}
	/* dnsqueue is unlimited. */
	dnsqueue = list_create(-1);
	if (!dnsqueue) 
		return NS_FAILURE;
#ifndef DEBUG
	adnsstart = adns_init (&ads, adns_if_noerrprint | adns_if_noautosys, 0);
#else
	adnsstart = adns_init (&ads, adns_if_debug | adns_if_noautosys, 0);
#endif
	if (adnsstart) {
		nlog (LOG_CRITICAL, "ADNS init failed: %s", strerror (adnsstart));
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/* @brief Clean up ADNS data when we shutdown 
 *
 */
void FiniDns (void) 
{
	lnode_t *dnsnode;
	DnsLookup *dnsdata;

	SET_SEGV_LOCATION();
	dnsnode = list_first (dnslist);
	while (dnsnode) {
		dnsdata = lnode_get(dnsnode);
		adns_cancel(dnsdata->q);
		ns_free (dnsdata->a);
		ns_free (dnsdata);
	}
	list_destroy_nodes (dnslist);
	list_destroy (dnslist);
	dnsnode = list_first(dnsqueue);
	while (dnsnode) {
		dnsdata = lnode_get(dnsnode);
		ns_free(dnsdata);
	}
	list_destroy_nodes (dnsqueue);
	list_destroy (dnsqueue);
	free(ads);
}
/** @brief Canx any DNS queries for modules we might be unloading
 * 
 * @param module name
 * @return Nothing
 */
void canx_dns(Module* modptr) 
{
	lnode_t *dnsnode, *lnode2;
	DnsLookup *dnsdata;

	SET_SEGV_LOCATION();
	dnsnode = list_first (dnslist);
	while (dnsnode) {
		dnsdata = lnode_get(dnsnode);
		if (dnsdata->modptr == modptr) {
			adns_cancel(dnsdata->q);
			ns_free (dnsdata->a);
			ns_free (dnsdata);
			lnode2 = list_next(dnslist, dnsnode);
			list_delete(dnslist, dnsnode);
			lnode_destroy(dnsnode);
			dnsnode = lnode2;
		}
	}
	dnsnode = list_first(dnsqueue);
	while (dnsnode) {
		dnsdata = lnode_get(dnsnode);
		if (dnsdata->modptr == modptr) {
			ns_free(dnsdata);
			lnode2 = list_next(dnsqueue, dnsnode);
			list_delete(dnsqueue, dnsnode);
			lnode_destroy(dnsnode);
			dnsnode = lnode2;
		}
	}
	dns_check_queue();
}

/** @brief Checks for Completed DNS queries
 *
 *  Goes through the dnslist of pending queries and calls the callback function for each lookup
 *  with the adns_answer set. Always calls the callback function even if the lookup was unsuccessful
*  its upto the callback function to make check the answer struct to see if it failed or not
 *
 * @return Nothing
*/
void do_dns (void)
{
	lnode_t *dnsnode, *dnsnode1;
	int status;
	DnsLookup *dnsdata;

	SET_SEGV_LOCATION();
	/* if the list is empty, no use doing anything */
	if (list_isempty (dnslist)) {
		dns_check_queue();
		return;
	}
	dnsnode = list_first (dnslist);
	while (dnsnode) {
		/* loop through the list */
		dnsdata = lnode_get (dnsnode);
		status = adns_check (ads, &dnsdata->q, &dnsdata->a, NULL);
		/* if status == eagain, the lookup hasn't completed yet */
		if (status == EAGAIN) {
			dlog(DEBUG2, "DNS: Lookup hasn't completed for %s",(char *) &dnsdata->data);
			dnsnode = list_next (dnslist, dnsnode);
			break;
		}
		/* there was an error */
		if (status) {
			nlog (LOG_CRITICAL, "DNS: Baaaad error on adns_check: %s. Please report to NeoStats Group", strerror (status));
			irc_chanalert (ns_botptr, "Bad Error on DNS lookup. Please check logfile");
			DNSStats.failure++;
			/* call the callback function with answer set to NULL */
			SET_RUN_LEVEL(dnsdata->modptr);
			dnsdata->callback (dnsdata->data, NULL);
			RESET_RUN_LEVEL();
			/* delete from list */
			dnsnode1 = list_delete (dnslist, dnsnode);
			dnsnode = list_next (dnslist, dnsnode1);
			ns_free (dnsdata->a);
			ns_free (dnsdata);
			lnode_destroy (dnsnode1);
			break;
		}
		dlog(DEBUG1, "DNS: Calling callback function with data %s for module %s", dnsdata->data, dnsdata->modptr->info->name);
		DNSStats.success++;
		/* call the callback function */
		SET_RUN_LEVEL(dnsdata->modptr);
		dnsdata->callback (dnsdata->data, dnsdata->a);
		RESET_RUN_LEVEL();
		/* delete from list */
		dnsnode1 = list_delete (dnslist, dnsnode);
		dnsnode = list_next (dnslist, dnsnode1);
		ns_free (dnsdata->a);
		ns_free (dnsdata);
		lnode_destroy (dnsnode1);
	}
	dns_check_queue();
}

/** @brief Checks the DNS queue and if we can
 * add new queries to the active DNS queries and remove from Queue 
*/
void dns_check_queue() 
{
	lnode_t *dnsnode, *dnsnode2;
	DnsLookup *dnsdata;
	struct sockaddr_in sa;
	int status;
	
	/* first, if the DNSLIST is full, just exit straight away */
	if (list_isfull(dnslist)) {
		dlog(DEBUG2, "DNS list is still full. Can't work on queue");
		return;
	}
	/* if the dnsqueue isn't empty, then lets process some more till we are full again */
	if (!list_isempty(dnsqueue)) {
		dnsnode = list_first(dnsqueue);
		while ((dnsnode) && (!list_isfull(dnslist))) {
			dnsdata = lnode_get(dnsnode);	
			dlog(DEBUG2, "Moving DNS query %s from queue to active", dnsdata->data);
			if (dnsdata->type == adns_r_ptr) {
				sa.sin_family = AF_INET;
				sa.sin_addr.s_addr = inet_addr (dnsdata->lookupdata);
				status = adns_submit_reverse (ads, (const struct sockaddr *) &sa, dnsdata->type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
			} else {
				status = adns_submit (ads, dnsdata->lookupdata, dnsdata->type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
			}
			if (status) {
				/* delete from queue and delete node */
				nlog (LOG_WARNING, "DNS: adns_submit error: %s", strerror (status));
				ns_free (dnsdata);
				dnsnode2 = dnsnode;
				dnsnode = list_next(dnsqueue, dnsnode);
				list_delete(dnsqueue, dnsnode2);
				lnode_destroy(dnsnode2);
				continue;
			}
			/* move from queue to active list */
			dnsnode2 = dnsnode;
			dnsnode = list_next(dnsqueue, dnsnode);
			list_delete(dnsqueue, dnsnode2);
			list_append(dnslist, dnsnode2);
			dlog(DEBUG1, "DNS: Added dns query %s to list", dnsdata->data);
		/* while loop */
		}
	/* isempty */
	}
}

void do_dns_stats_Z(Client *u) 
{
	irc_numeric (RPL_MEMSTATS, u->name, "Active DNS queries: %d", (int) list_count(dnslist));
	irc_numeric (RPL_MEMSTATS, u->name, "Queued DNS Queries: %d", (int) list_count(dnsqueue));
	irc_numeric (RPL_MEMSTATS, u->name, "Max Queued Queries: %d", DNSStats.maxqueued);
	irc_numeric (RPL_MEMSTATS, u->name, "Total DNS Questions: %d", DNSStats.totalq);
	irc_numeric (RPL_MEMSTATS, u->name, "Successful Lookups: %d", DNSStats.success);
	irc_numeric (RPL_MEMSTATS, u->name, "Unsuccessful Lookups: %d", DNSStats.failure);
}
