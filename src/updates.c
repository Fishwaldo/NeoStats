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
#include "event.h"
#include "MiniMessage.h"
#include "MiniMessageGateway.h"


void GotUpdateAddress(void *data, adns_answer *a);
static int mqswrite(int fd, void *data);
static int mqsread(void *data, void *notused, size_t len);
int mqs_login();

updateserver mqs;
MMessageGateway *mqsgw;

int MQSSendSock(const char * buf, uint32 numBytes, void * arg) {
	return os_sock_write(mqs.sock, buf, numBytes); 
}


int InitUpdate(void) 
{
	mqs.state = MQS_DISCONNECTED;
	mqsgw = MGAllocMessageGateway();
	if (!mqsgw) {
		nlog(LOG_WARNING, "Couldn't allocate MiniMessageGateway Object");
		return NS_FAILURE;
	}
	dns_lookup( mqs.hostname,  adns_r_a, GotUpdateAddress, NULL );
	dlog(DEBUG1, "Updates Initialized successfully");
	return NS_SUCCESS;
}

void FiniUpdate(void) 
{
}

void GotUpdateAddress(void *data, adns_answer *a) 
{
	struct timeval tv;
	SET_SEGV_LOCATION();
	if( a && a->nrrs > 0 && a->status == adns_s_ok ) {
			mqs.sock = sock_connect(SOCK_STREAM, a->rrs.addr->addr.inet.sin_addr, mqs.port);
			if (mqs.sock > 0) {
				tv.tv_sec = 30;
				AddSock(SOCK_NATIVE, "MQS", mqs.sock, mqsread, mqswrite, EV_WRITE|EV_TIMEOUT, NULL, &tv);
				mqs.state = MQS_CONNECTING;
			}
			nlog (LOG_NORMAL, "Got DNS for MQ Pool Server: %s", inet_ntoa(a->rrs.addr->addr.inet.sin_addr));
	} else {
		nlog(LOG_WARNING, "DNS error Checking for MQ Server Pool: %s", adns_strerror(a->status));
	}
}

int mqswrite(int fd, void *data) {
	switch (mqs.state) {
		case MQS_DISCONNECTED:
			break;
		/* we are connected */
		case MQS_CONNECTING:
			return mqs_login();
			break;
		case MQS_SENTAUTH:
		case MQS_OK:
			/* ask MiniMessageGateway to write any buffer out */
			break;
	}
	return NS_FAILURE;
}

int mqsread(void *data, void *notused, size_t len) {

	return NS_SUCCESS;
}

int mqs_login() 
{
	MMessage *msg = MMAllocMessage(0);
	MByteBuffer **username;
	MByteBuffer **password;
	MByteBuffer **version;
	if (!msg) {
		nlog(LOG_WARNING, "Warning, Couldn't create MiniMessage");
		return NS_FAILURE;
	}
	username = MMPutStringField(msg, false, "username", 1);
	username[0] = MBStrdupByteBuffer(mqs.username);
	password = MMPutStringField(msg, false, "password", 1);
	password[0] = MBStrdupByteBuffer(mqs.password);
	version = MMPutStringField(msg, false, "version", 1);
	version[0] = MBStrdupByteBuffer(me.version);
	MMSetWhat(msg, MAKETYPE("login\0"));
#ifdef DEBUG
	MMPrintToStream(msg);
#endif
	MGAddOutgoingMessage(mqsgw, msg);
	MMFreeMessage(msg);

	MGDoOutput(mqsgw, ~0, MQSSendSock, NULL);
	
	mqs.state = MQS_SENTAUTH;
	
	return NS_SUCCESS;
}
void sendtoMQ( MQ_MSG_TYPE type, void *data, size_t len) {
	char *buf;
	
	if (mqs.state == MQS_OK) {
		/* for now, we know that data is always a char string */
		buf = malloc(sizeof(int) + len);
		ircsnprintf(buf, (sizeof(int)+len+1), "%d\n%s", type, (char *)data);
		dlog(DEBUG1, "Send Update: %s", buf);
		sendto(mqs.sock, buf, strlen(buf), 0,  (struct sockaddr *) &mqs.sendtomq, sizeof(mqs.sendtomq));
		free(buf);
	}
}
