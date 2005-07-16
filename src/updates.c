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

/* @file NeoStats interface to send updates back to NeoStats/Secure.irc-chat.net
 */

#include "neostats.h"

void GotUpdateAddress(void *data, adns_answer *a);

struct updateserver {
	int ok;
	struct sockaddr_in sendtomq;
	OS_SOCKET sock;
} mqs;

int InitUpdate(void) 
{
	mqs.ok = 0;
	dns_lookup( "mqpool.neostats.net",  adns_r_a, GotUpdateAddress, NULL );
	dlog(DEBUG1, "Updates Initialized successfully");
	return NS_SUCCESS;
}

void FiniUpdate(void) 
{
}

void GotUpdateAddress(void *data, adns_answer *a) 
{
	char *url;
	int i, len, ri;

	SET_SEGV_LOCATION();
	adns_rr_info(a->type, 0, 0, &len, 0, 0);
	for(i = 0; i < a->nrrs;  i++) {
		ri = adns_rr_info(a->type, 0, 0, 0, a->rrs.bytes +i*len, &url);
		if (!ri) {
			/* ok, we got a valid answer, lets maybe kick of the update check.*/
			mqs.sendtomq.sin_addr.s_addr = inet_addr(url);
			mqs.sendtomq.sin_port = htons(2335);
			mqs.sendtomq.sin_family = AF_INET;
			mqs.sock = socket(AF_INET, SOCK_DGRAM, 0);
			mqs.ok = 1;
			nlog (LOG_NORMAL, "Got DNS for MQ Pool Server: %s", url);
		} else {
			nlog(LOG_WARNING, "DNS error Checking for MQ Server Pool: %s", adns_strerror(ri));
		}
		ns_free (url);
	}
	if (a->nrrs < 1) {
		nlog(LOG_WARNING, "DNS Error checking for MQ Server Pool");
	}
}
void sendtoMQ( MQ_MSG_TYPE type, void *data, size_t len) {
	char *buf;
	
	if (mqs.ok == 1) {
		/* for now, we know that data is always a char string */
		buf = malloc(sizeof(int) + len);
		ircsnprintf(buf, (sizeof(int)+len+1), "%d\n%s", type, (char *)data);
		dlog(DEBUG1, "Send Update: %s", buf);
		sendto(mqs.sock, buf, strlen(buf), 0,  (struct sockaddr *) &mqs.sendtomq, sizeof(mqs.sendtomq));
		free(buf);
	}
}
