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

#ifndef _TRANSFER_H_
#define _TRANSFER_H_
#include "neostats.h"
#include "curl.h"

/* max URL size */
#define MAXURL 64



/* this is the curl multi handle we use */
CURLM *curlmultihandle;



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
} neo_transfer;

list_t *activetransfers;


int InitCurl(void);
void FiniCurl(void);

#endif /* _TRANSFER_H_ */
