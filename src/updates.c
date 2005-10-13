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

int32 MQSSendSock(const uint8 * buf, uint32 numBytes, void * arg) {
	return send_to_sock(mqs.Sockinfo, (char *)buf, numBytes); 
}

int32 MQSRecvSock(uint8 * buf, uint32 numBytes, void * arg) {
	return os_sock_read( mqs.sock, (char *)buf, numBytes );
}

int InitUpdate(void) 
{
#ifndef WIN32
	if (mqs.username[0]) {
		dns_lookup( mqs.hostname,  adns_r_a, GotUpdateAddress, NULL );
		dlog(DEBUG1, "Updates Initialized successfully");
	}
	mqs.state = MQS_DISCONNECTED;
#endif
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
				tv.tv_sec = 60;
				mqsgw = MGAllocMessageGateway();
				if (!mqsgw) {
					nlog(LOG_WARNING, "Couldn't allocate MiniMessageGateway Object");
					return;
				}
				mqs.Sockinfo = AddSock(SOCK_NATIVE, "MQS", mqs.sock, mqsread, mqswrite, EV_WRITE|EV_TIMEOUT|EV_READ|EV_PERSIST, NULL, &tv);
				mqs.state = MQS_CONNECTING;
			}
#if 0
			nlog (LOG_NORMAL, "Got DNS for MQ Pool Server: %s", inet_ntoa(a->rrs.addr->addr.inet.sin_addr));
#endif
	} else {
		nlog(LOG_WARNING, "DNS error Checking for MQ Server Pool: %s", adns_strerror(a->status));
	}
}

void ResetMQ() {
	if (mqs.state >= MQS_CONNECTING) {
		mqs.state = MQS_DISCONNECTED;
		mqs.Sockinfo = NULL;
		MGFreeMessageGateway(mqsgw);
	}
}

void CheckMQOut() {
	int i;
	if (mqs.state < MQS_SENTAUTH) {
			nlog(LOG_WARNING, "MQ Server Not Connected....");
			/* do something */
			return;
	}
	if (MGHasBytesToOutput(mqsgw)) {
		i = MGDoOutput(mqsgw, ~0, MQSSendSock, NULL);
		if (i < 0) {
			nlog(LOG_WARNING, "MQ Server Connection Lost.... ");
			/* we lost our connection... */
			ResetMQ();
		}
	}
}

void ProcessMessage(MMessage *msg) {
	unsigned long what;
	
	what = MMGetWhat(msg);
	switch (what) {
		case 1818718059:
			dlog(DEBUG1, "Login Ok");
			break;
		default:
			dlog(DEBUG1, "Got message type %d", what);
			break;
	}

}

int mqswrite(int fd, void *data) {
	switch (mqs.state) {
		case MQS_DISCONNECTED:
			nlog(LOG_CRITICAL, "Trying to send a message to MQ while disconnected?");
			break;
		/* we are connected */
		case MQS_CONNECTING:
			return mqs_login();
			break;
		case MQS_SENTAUTH:
		case MQS_OK:
			/* ask MiniMessageGateway to write any buffer out */
			CheckMQOut();
			break;
	}
	return NS_SUCCESS;
}

int mqsread(void *data, void *notused, size_t len) {
	MMessage *msg;
	
	if (len == -2 && mqs.state < MQS_OK) {
		/* timeout */
		nlog(LOG_WARNING, "Timeout Connecting to MQ Server");
		ResetMQ();
		return NS_FAILURE;
	}
	if (len <= 0) {
		/* EOF etc */
		nlog(LOG_WARNING, "Lost Connection to MQ Server %s", os_sock_getlasterrorstring());
		ResetMQ();
		return NS_SUCCESS;
	}
	MGDoInput(mqsgw, ~0, MQSRecvSock, NULL, &msg);
	if (msg) {
#ifdef DEBUG
		MMPrintToStream(msg);
#endif
		/* do something */
		ProcessMessage(msg);
		MMFreeMessage(msg);
	}
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
	nlog(LOG_INFO, "Connnected to MQ Server, Logging in");

	username = MMPutStringField(msg, false, "username", 1);
	username[0] = MBStrdupByteBuffer(mqs.username);
	password = MMPutStringField(msg, false, "password", 1);
	password[0] = MBStrdupByteBuffer(mqs.password);
	version = MMPutStringField(msg, false, "version", 1);
	version[0] = MBStrdupByteBuffer(me.version);
	MMSetWhat(msg, MAKETYPE("lgn"));
#ifdef DEBUG
	MMPrintToStream(msg);
#endif
	MGAddOutgoingMessage(mqsgw, msg);
	MMFreeMessage(msg);

	mqs.state = MQS_SENTAUTH;

	CheckMQOut();
		
	return NS_SUCCESS;
}
void sendtoMQ( MQ_MSG_TYPE type, void *data, size_t len) {
	char *buf;
	
	if (mqs.state == MQS_OK) {
		/* for now, we know that data is always a char string */
		buf = malloc(sizeof(int) + len);
		ircsnprintf(buf, (sizeof(int)+len+1), "%d\n%s", type, (char *)data);
		dlog(DEBUG1, "Send Update: %s", buf);
		os_sock_sendto(mqs.sock, buf, strlen(buf), 0,  (struct sockaddr *) &mqs.sendtomq, sizeof(mqs.sendtomq));
		free(buf);
	}
}
