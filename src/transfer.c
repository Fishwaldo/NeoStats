/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2007 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* @file NeoStats interface to the curl library... 
 */

#include "neostats.h"
#include "transfer.h"
#include "curl.h"
#include "event.h"

#define MAXURL BUFSIZE	/* max URL size */
#define MAX_TRANSFERS	10	/* number of curl transfers */

/* this struct contains info for each transfer in progress */
typedef struct neo_transfer {
	/* the curl handle used to track downloads by curl */
	CURL *curleasyhandle;
	/* should we save to a file or a memory space */
	int savefileormem;
	/* if saving to memory, save here, */
	char *savemem;
	/* the allocated amount size of the savemem area */
	int savememsize;
	/* the current position of hte savemem area */
	int savemempos; 
	/* the file to save to */
	char filename[MAXPATH];
	/* if saving to a file, the file handle */
	FILE *savefile;
	/* the error, if any */
	char curlerror[CURL_ERROR_SIZE];
	/* user data, passed by modules, other functions to help reference this download */
	void *data;
	/* the url to retrive/upload/download etc etc etc */
	char url[MAXURL];
	/* the useragent */
	char useragent[MAXURL];
	/* the params, if any */
	char params[512];
	/* the callback function */
	transfer_callback *callback;
	/* the module that is using this function */
	Module *module_ptr;
} neo_transfer;

/* this is the curl multi handle we use */
static CURLM *curlmultihandle;
static list_t *activetransfers;
static int curl_socket_event_callback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);
static int curltimercallback(void *);
#undef CURL_TEST 
/* #define CURL_TEST 1 */
#ifdef CURL_TEST
void CurlTest( void *data, int status, char *ver, int versize ) {
	dlog(DEBUG1, "Download Ok: %d", status);
	return;
}
#endif

int InitCurl(void) 
{
	long timeout_ms;
    
	/* global curl init */
	switch (curl_global_init(CURL_GLOBAL_ALL)) {
		case CURLE_OK:
			break;
		case CURLE_FAILED_INIT:
			nlog(LOG_WARNING, "LibCurl Refused a global initilize request.");
			return NS_FAILURE;
		case CURLE_OUT_OF_MEMORY:
			nlog(LOG_WARNING, "LibCurl ran out of Memory. Damn");
			return NS_FAILURE;
		default:
			nlog(LOG_WARNING, "Got a weird LibCurl Error");
	}
	/* init the multi interface for curl */
	curlmultihandle = curl_multi_init();
	if (!curlmultihandle) {
		nlog(LOG_WARNING, "LibCurl Refused to initialize...  Could be problems");
		return NS_FAILURE;
	}
#ifndef CURLHACK
	if (curl_multi_setopt(curlmultihandle, CURLMOPT_SOCKETFUNCTION, curl_socket_event_callback)) {
		nlog(LOG_WARNING, "Libcurl failed to initialize the multisock interface");
		return NS_FAILURE;
	}
      	/* Since we need a global timeout to occur after a given time of inactivity,
        ** we use a single timeout-event. Get the timeout value from libcurl, and
        ** update it after every call to libcurl. */
        curl_multi_timeout(curlmultihandle, &timeout_ms);
	if ((timeout_ms/1000) < 1) timeout_ms = 1000;
	AddTimer(TIMER_TYPE_INTERVAL, curltimercallback, "CurlTimeOut", (timeout_ms/1000), curlmultihandle);
#endif
	/* init the internal list to track downloads */
	activetransfers = list_create(MAX_TRANSFERS);
	if( !activetransfers )
	{
		nlog( LOG_CRITICAL, "Unable to create transfer list" );
		return NS_FAILURE;
	}
	dlog(DEBUG1, "LibCurl Initialized successfully");
#ifdef CURL_TEST
	if(new_transfer( "http://10.1.1.1/Archive.zip", NULL, NS_FILE, "/tmp/test.txt", NULL, CurlTest ) == NS_FAILURE) {
		dlog(DEBUG1, "Curl Test Failed");
	}
#endif
	return NS_SUCCESS;
}

void FiniCurl(void) 
{
	DelTimer("CurlTimeOut");
	curl_multi_cleanup(curlmultihandle);
	list_destroy_auto (activetransfers);
}

static size_t neocurl_callback( void *transferptr, size_t size, size_t nmemb, void *stream) 
{
	size_t writesize;
	size_t rembuffer;
	char *newbuf;
	neo_transfer *neotrans = (neo_transfer *)stream;

	SET_SEGV_LOCATION();
	switch (neotrans->savefileormem) {
		case NS_FILE:
			/* we are saving to a file... :) */
			writesize = fwrite(transferptr, size, nmemb, neotrans->savefile);
			dlog(DEBUG1, "Write %d to file from transfer from URL %s", (int)size*(int)nmemb, neotrans->url);
			break;
		case NS_MEMORY:
			size *= nmemb;
			rembuffer = neotrans->savememsize - neotrans->savemempos; /* the remaining buffer size */
			if (size > rembuffer) {
				newbuf = ns_realloc(neotrans->savemem, neotrans->savememsize + (size - rembuffer));
				if (newbuf == NULL) {
					nlog(LOG_WARNING, "Ran out of Memory for transfer %s. %s", neotrans->url, strerror(errno));
					return -1;
				} else {
					neotrans->savememsize += size - rembuffer;
					neotrans->savemem = newbuf;
				}
			}
			os_memcpy( (char *)&neotrans->savemem[neotrans->savemempos], transferptr, size );
			neotrans->savemempos += size;
			writesize = nmemb;
			break;
		default:
			nlog(LOG_WARNING, "Unknown SaveTo type. Failed download for URL %s", neotrans->url);
			writesize = -1;
			break;
	}
	return writesize;
}


int new_transfer(char *url, char *params, NS_TRANSFER savetofileormemory, char *filename, void *data, transfer_callback *callback) 
{
	int ret;
	neo_transfer *newtrans;

	/* sanity check the input first */
	if (url[0] == 0) {
		nlog(LOG_WARNING, "Undefined URL new_transfer , Aborting");
		return NS_FAILURE;
	}
	if (!callback) {
		nlog(LOG_WARNING, "Undefined Callback function for new_transfer URL %s, Aborting", url);
		return NS_FAILURE;
	}

	newtrans = ns_calloc(sizeof(neo_transfer));
	newtrans->module_ptr=GET_CUR_MODULE();
	switch (savetofileormemory) {
		case NS_FILE:
			/* if we don't have a filename, bail out */
			if (filename[0] == 0) {
				nlog(LOG_WARNING, "Undefined Filename for new_transfer URL %s, Aborting", url);
				ns_free(newtrans);
				return NS_FAILURE;
			}
			
			newtrans->savefile = fopen(filename, "wt");
			if (!newtrans->savefile) {
				nlog(LOG_WARNING, "Error Opening file for writting in new_transfer: %s", strerror(errno));
				ns_free(newtrans);
				return NS_FAILURE;
			}
			newtrans->savefileormem = NS_FILE;
			strlcpy(newtrans->filename, filename, MAXPATH);			
			dlog(DEBUG1, "Saving new download to %s from %s", filename, url);	
			break;
		case NS_MEMORY:
			newtrans->savefileormem = NS_MEMORY;
			break;
		default:
			break;
	}
	/* save the callback function */
	newtrans->callback = callback;
	
	/* save the data function */
	newtrans->data = data;

	/* init the easy handle */
	newtrans->curleasyhandle = curl_easy_init();
	if (!newtrans->curleasyhandle) {
		nlog(LOG_WARNING, "Curl Easy Init Failed for url %s", url);
		ns_free(newtrans);
		return NS_FAILURE;
	}
	/* setup some standard options we use globally */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_ERRORBUFFER, newtrans->curlerror)) != 0 ) {
		nlog(LOG_WARNING, "Curl set errorbuffer failed. Returned %d for url %s", ret, url);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}
#ifdef DEBUG
	/* setup verbose mode in debug compile... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_VERBOSE, 1)) != 0) {
		nlog(LOG_WARNING, "Curl Set verbose failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	
#endif
	/* setup no signals... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_NOSIGNAL, 1)) != 0) {
		nlog(LOG_WARNING, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* fail for any return above 300 (HTTP)... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_FAILONERROR, 1)) != 0) {
		nlog(LOG_WARNING, "Curl Set failonerror failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* fail for any return above 300 (HTTP)... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_FOLLOWLOCATION, 1)) != 0) {
		nlog(LOG_WARNING, "Curl Set followlocation failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* fail for any return above 300 (HTTP)... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_FOLLOWLOCATION, 1)) != 0) {
		nlog(LOG_WARNING, "Curl Set followlocation failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* setup the user agent */
	ircsnprintf(newtrans->useragent, MAXURL, "NeoStats %s (%s)", me.version, GET_CUR_MODNAME()); 
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_USERAGENT, newtrans->useragent)) != 0) {
		nlog(LOG_WARNING, "Curl Set useragent failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* setup the url to retrive.*/
	strlcpy(newtrans->url, url, MAXURL);
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_URL, newtrans->url)) != 0) {
		nlog(LOG_WARNING, "Curl Set url failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* setup the private data id.*/
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_PRIVATE, newtrans)) != 0) {
		nlog(LOG_WARNING, "Curl Set private failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}	

	/* setup any params we must post to the server */
#if 0
	if (params[0] != 0) {
#endif
    if (params) {
		strlcpy(newtrans->params, params, MAXURL);
		if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_POSTFIELDS, newtrans->params)) != 0) {
			nlog(LOG_WARNING, "Curl Set postfields failed. Returned %d for url %s", ret, url);
			nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
			curl_easy_cleanup(newtrans->curleasyhandle);
			ns_free(newtrans);
			return NS_FAILURE;
		}
	}	

	/* the callback function for data received from the server */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_WRITEFUNCTION, neocurl_callback)) != 0) {
		nlog(LOG_WARNING, "Curl Set writefunc failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}
	
	
	/* copy our struct to curl so we can reference it when we return */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_WRITEDATA, newtrans)) != 0) {
		nlog(LOG_WARNING, "Curl Set writedata failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		ns_free(newtrans);
		return NS_FAILURE;
	}
	
	
	if (curl_multi_add_handle(curlmultihandle, newtrans->curleasyhandle)) {
		nlog(LOG_WARNING, "Curl Init Failed: %s", newtrans->curlerror);
		ns_free(newtrans);
		return NS_FAILURE;
	}
	/* we have to do this at least once to get things going */
	while(CURLM_CALL_MULTI_PERFORM == curl_multi_socket_all(curlmultihandle, &ret)) {
	}
	return NS_SUCCESS;
}

#ifdef CURLHACK
static void transfer_status( void ) 
{
	CURLMsg *msg;
	int msg_left;
	neo_transfer *neotrans;
	SET_SEGV_LOCATION();
	while ((msg = curl_multi_info_read(curlmultihandle, &msg_left))) {
		if (msg->msg == CURLMSG_DONE) {
			/* find the handle here */
			curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &neotrans);

			/* close up any handles */
			switch (neotrans->savefileormem) {
				case NS_FILE:
					/* we are saving to a file... :) */
					fclose(neotrans->savefile);
					dlog(DEBUG1, "Write Finished to file from transfer from URL %s",  neotrans->url);
					break;
				case NS_MEMORY:
					break;
			}
			/* check if it failed etc */
			if (msg->data.result != 0) {
				/* it failed for whatever reason */
				nlog(LOG_NOTICE, "Transfer %s Failed. Error was: %s", neotrans->url, neotrans->curlerror);
                                SET_RUN_LEVEL(neotrans->module_ptr);
				neotrans->callback(neotrans->data, NS_FAILURE, neotrans->curlerror, strlen(neotrans->curlerror));
                                RESET_RUN_LEVEL();
			} else {
				dlog(DEBUG1, "Transfer %s succeded.", neotrans->url);
				/* success, so we must callback with success */
                                SET_RUN_LEVEL(neotrans->module_ptr);
				neotrans->callback(neotrans->data, NS_SUCCESS, neotrans->savefileormem == NS_MEMORY ? neotrans->savemem : NULL, neotrans->savememsize);
                                RESET_RUN_LEVEL();
			}
			/* regardless, clean up the transfer */
			curl_multi_remove_handle(curlmultihandle, neotrans->curleasyhandle);
			curl_easy_cleanup(neotrans->curleasyhandle);
			/* XXX remove from list */
			if (neotrans->savefileormem == NS_MEMORY)
				ns_free(neotrans->savemem);
			ns_free(neotrans);
		}
	}
}

void CurlHackReadLoop( void )
{
	struct timeval TimeOut;
	int SelectResult;
	fd_set readfds, writefds, errfds;
	int maxfdsunused;
#if 0
         /* XXX our activetransfers list isn't maintained, we should so we do cycle this curl hack when no transfers in progress */
         if (list_count(activetransfers) > 0) {	
#endif
         if (1) {
             /* don't wait */
      	    TimeOut.tv_sec = 0;
      	    TimeOut.tv_usec = 0;
      	    FD_ZERO (&readfds);
      	    FD_ZERO (&writefds);
      	    FD_ZERO (&errfds);
      	    curl_multi_fdset(curlmultihandle, &readfds, &writefds, &errfds, &maxfdsunused);
      	    SelectResult = select (maxfdsunused+1, &readfds, &writefds, &errfds, &TimeOut);
      	    if (SelectResult > 0)
      	    {
		/* check CURL fds */
		while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curlmultihandle, &maxfdsunused))
		{
		}
		SET_SEGV_LOCATION();
		transfer_status();
             }
        }
}
#else 
int curl_read(void *userp, void *data, int size) {
	int ret;
	curl_socket_t sock = (int)userp;
	while(CURLM_CALL_MULTI_PERFORM == curl_multi_socket(curlmultihandle, sock, &ret))
	{
	}
	return NS_SUCCESS;
}
int curl_write(int socket, void *userp) {
	int ret;
	while(CURLM_CALL_MULTI_PERFORM == curl_multi_socket(curlmultihandle, socket, &ret))
	{
	}
	return NS_SUCCESS;
}
static void update_timeout(CURLM *multi_handle)
{
	long timeout_ms;
    
      	/* Since we need a global timeout to occur after a given time of inactivity,
        ** we use a single timeout-event. Get the timeout value from libcurl, and
        ** update it after every call to libcurl. */
        curl_multi_timeout(multi_handle, &timeout_ms);
                  
	if ((timeout_ms/1000) < 1) timeout_ms=1000;
	SetTimerInterval("CurlTimeOut", (timeout_ms/1000));
}

/* called from libevent when our timer event expires */
static int curltimercallback(void *userp)
{
      	CURLM *multi_handle = (CURLM *)userp;
	int running_handles;
        CURLMcode rc;
          
        /* tell libcurl to deal with the transfer associated with this socket */
        do {
        	rc = curl_multi_socket(multi_handle, CURL_SOCKET_TIMEOUT,
        		&running_handles);
        } while (rc == CURLM_CALL_MULTI_PERFORM);
        if(running_handles)
        	/* Get the current timeout value from libcurl and set a new timeout */
        	update_timeout(multi_handle);
	return NS_SUCCESS;
}

int curl_socket_event_callback(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) {
	Sock *sock;
	char buf[BUFSIZE];
	/* if socketp is NULL, this is the first time we have seen this socket from CURL, so create it in our 
	 * routines */
	if (socketp == NULL) {
		ircsnprintf(buf, BUFSIZE, "CURL-%d", s);
		sock = AddSock(SOCK_NATIVE, buf, s, curl_read, curl_write, 0, (void *)s, NULL);
		if (sock) curl_multi_assign(curlmultihandle, s, sock);
	} else {
		sock = socketp;
	}
	switch (action) {
		case CURL_POLL_NONE:
		case CURL_POLL_IN:
			UpdateSock(sock, EV_READ|EV_PERSIST, 1, NULL);
			update_timeout(curlmultihandle);
			break;
		case CURL_POLL_OUT:
			UpdateSock(sock, EV_WRITE|EV_PERSIST, 1, NULL);	
			update_timeout(curlmultihandle);
			break;
		case CURL_POLL_INOUT:
			UpdateSock(sock, EV_READ|EV_WRITE|EV_PERSIST, 1, NULL);	
			update_timeout(curlmultihandle);
			break;	
		case CURL_POLL_REMOVE:
			DelSock(sock);
			break;
	}

	return 0;
}

#endif /* CURLHACK */

