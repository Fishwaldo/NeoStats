/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond, Mark Hetherington
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

int init_curl() {
	curlmultihandle = curl_multi_init();
	if (!curlmultihandle) {
		nlog(LOG_WARNING, LOG_CORE, "LibCurl Refused to initilize...  Could be problems");
		return NS_FAILURE;
	}
	nlog(LOG_DEBUG1, LOG_CORE, "LibCurl Initilized successfully");
	return NS_SUCCESS;
}
