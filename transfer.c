/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include <stdio.h>
#include "stats.h"
#include "log.h"
#include "transfer.h"
#include "curl.h"

void testcallback(void *data, int status, char *body, int bodysize) {
if (status == NS_FAILURE) {
	printf("gotcallback for %s\n", (char *)data);
	printf("Error: %s\n", body);
} else {
	printf("got good callback for %s\n", (char *)data);
	printf("Body:(%d - %d)\n%s\n", bodysize, strlen(body),body);
}
}


int init_curl() {
	/* global curl init */
	switch (curl_global_init(CURL_GLOBAL_ALL)) {
		case CURLE_OK:
			break;
		case CURLE_FAILED_INIT:
			nlog(LOG_WARNING, LOG_CORE, "LibCurl Refused a global initilize request.");
			return NS_FAILURE;
		case CURLE_OUT_OF_MEMORY:
			nlog(LOG_WARNING, LOG_CORE, "LibCurl ran out of Memory. Damn");
			return NS_FAILURE;
		default:
			nlog(LOG_WARNING, LOG_CORE, "Got a weird LibCurl Error");
	}
	/* init the multi interface for curl */
	curlmultihandle = curl_multi_init();
	if (!curlmultihandle) {
		nlog(LOG_WARNING, LOG_CORE, "LibCurl Refused to initilize...  Could be problems");
		return NS_FAILURE;
	}
	/* init the internal list to track downloads */
	activetransfers = list_create(MAX_TRANSFERS);
	nlog(LOG_DEBUG1, LOG_CORE, "LibCurl Initilized successfully");
	
/*	new_transfer("http://secure.irc-chat.net/", "", NS_MEMORY, "0", "Blah", testcallback); */
	return NS_SUCCESS;
}

static size_t neocurl_callback( void *transferptr, size_t size, size_t nmemb, void *stream) {
	size_t writesize;
	int rembuffer;
	char *newbuf;
	neo_transfer *neotrans = (neo_transfer *)stream;

	SET_SEGV_LOCATION();
	switch (neotrans->savefileormem) {
		case NS_FILE:
			/* we are saving to a file... :) */
			writesize = fwrite(transferptr, size, nmemb, neotrans->savefile);
			nlog(LOG_DEBUG1, LOG_CORE, "Write %d to file from transfer from URL %s", (int)size*nmemb, neotrans->url);
			break;
		case NS_MEMORY:
			size *= nmemb;
			rembuffer = neotrans->savememsize - neotrans->savemempos; /* the remianing buffer size */
			if (size > rembuffer) {
				newbuf = realloc(neotrans->savemem, neotrans->savememsize + (size - rembuffer));
				if (newbuf == NULL) {
					nlog(LOG_WARNING, LOG_CORE, "Ran out of Memory for transfer %s. %s", neotrans->url, strerror(errno));
					return -1;
				} else {
					neotrans->savememsize += size - rembuffer;
					neotrans->savemem = newbuf;
				}
			}
			memcpy((char *)&neotrans->savemem[neotrans->savemempos], transferptr, size);
			neotrans->savemempos += size;
			writesize = nmemb;
			break;
		default:
			nlog(LOG_WARNING, LOG_CORE, "Unknown SaveTo type. Failed download for URL %s", neotrans->url);
			writesize = -1;
			break;
	}
	return writesize;
}


int new_transfer(char *url, char *params, NS_TRANSFER savetofileormemory, char *filename, void *data, transfer_callback *callback) {
	int ret;
	neo_transfer *newtrans;
	

	/* sanity check the input first */
	if (url[0] == 0) {
		nlog(LOG_WARNING, LOG_CORE, "Undefined URL new_transfer , Aborting");
		return NS_FAILURE;
	}
	if (!callback) {
		nlog(LOG_WARNING, LOG_CORE, "Undefined Callback function for new_transfer URL %s, Aborting", url);
		return NS_FAILURE;
	}

	newtrans = malloc(sizeof(neo_transfer));
	bzero(newtrans, sizeof(neo_transfer));
	switch (savetofileormemory) {
		case NS_FILE:
			/* if we don't have a filename, bail out */
			if (filename[0] == 0) {
				nlog(LOG_WARNING, LOG_CORE, "Undefined Filename for new_transfer URL %s, Aborting", url);
				free(newtrans);
				return NS_FAILURE;
			}
			
			newtrans->savefile = fopen(filename, "w");
			if (!newtrans->savefile) {
				nlog(LOG_WARNING, LOG_CORE, "Error Opening file for writting in new_transfer: %s", strerror(errno));
				free(newtrans);
				return NS_FAILURE;
			}
			newtrans->savefileormem = NS_FILE;
			strlcpy(newtrans->filename, filename, MAXPATH);			
			nlog(LOG_DEBUG1, LOG_CORE, "Saving new download to %s from %s", filename, url);	
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
		nlog(LOG_WARNING, LOG_CORE, "Curl Easy Init Failed for url %s", url);
		free(newtrans);
		return NS_FAILURE;
	}
	/* setup some standard options we use globally */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_ERRORBUFFER, newtrans->curlerror)) != 0 ) {
		nlog(LOG_WARNING, LOG_CORE, "Curl set errorbuffer failed. Returned %d for url %s", ret, url);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}
#ifdef DEBUG
	/* setup verbose mode in debug compile... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_VERBOSE, 1)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}	
#endif
	/* setup no signals... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_NOSIGNAL, 1)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}	

	/* fail for any return above 300 (HTTP)... */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_FAILONERROR, 1)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}	

	/* setup the user agent */
	ircsnprintf(newtrans->useragent, MAXURL, "NeoStats %d.%d.%d(%s) (%s)",MAJOR, MINOR, REV, ircd_version, segvinmodule); 
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_USERAGENT, newtrans->useragent)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set useragent failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}	

	/* setup the url to retrive.*/
	strlcpy(newtrans->url, url, MAXURL);
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_URL, newtrans->url)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}	

	/* setup the private data id.*/
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_PRIVATE, newtrans)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}	

	/* setup any params we must post to the server */
	if (params[0] != 0) {
		strlcpy(newtrans->params, params, MAXURL);
		if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_POSTFIELDS, newtrans->params)) != 0) {
			nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
			nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
			curl_easy_cleanup(newtrans->curleasyhandle);
			free(newtrans);
			return NS_FAILURE;
		}
	}	

	/* the callback function for data recieved from the server */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_WRITEFUNCTION, neocurl_callback)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}
	
	
	/* copy our struct to curl so we can reference it when we return */
	if ((ret = curl_easy_setopt(newtrans->curleasyhandle, CURLOPT_WRITEDATA, newtrans)) != 0) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Set nosignal failed. Returned %d for url %s", ret, url);
		nlog(LOG_WARNING, LOG_CORE, "Error Was: %s", newtrans->curlerror);
		curl_easy_cleanup(newtrans->curleasyhandle);
		free(newtrans);
		return NS_FAILURE;
	}
	
	
	if (!curl_multi_add_handle(curlmultihandle, newtrans->curleasyhandle)) {
		nlog(LOG_WARNING, LOG_CORE, "Curl Init Failed: %s", newtrans->curlerror);
		free(newtrans);
		return NS_FAILURE;
	}
	/* we have to do this at least once to get things going */
	while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curlmultihandle, &ret)) {
	}

	return NS_SUCCESS;
}


void transfer_status() {
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
					nlog(LOG_DEBUG1, LOG_CORE, "Write Finished to file from transfer from URL %s",  neotrans->url);
					break;
				case NS_MEMORY:
					break;
			}

			/* check if it failed etc */
			if (msg->data.result != 0) {
				/* it failed for whatever reason */
				/* XXX TODO... Callback with a failure */
				nlog(LOG_NOTICE, LOG_CORE, "Transfer %s Failed. Error was: %s", neotrans->url, neotrans->curlerror);
				neotrans->callback(neotrans->data, NS_FAILURE, neotrans->curlerror, strlen(neotrans->curlerror));
			} else {
				nlog(LOG_DEBUG1, LOG_CORE, "Transfer %s succeded.", neotrans->url);
				/* success, so we must callback with success */
				neotrans->callback(neotrans->data, NS_SUCCESS, neotrans->savefileormem == NS_MEMORY ? neotrans->savemem : NULL, neotrans->savememsize);
			}
			CLEAR_SEGV_INMODULE();
			/* regardless, clean up the transfer */
			curl_multi_remove_handle(curlmultihandle, neotrans->curleasyhandle);
			curl_easy_cleanup(neotrans->curleasyhandle);
			/* XXX remove from list */
			if (neotrans->savefileormem == NS_MEMORY)
				free(neotrans->savemem);
			free(neotrans);
		}
	}
}
