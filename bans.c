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

#include "stats.h"
#include "log.h"

static hash_t *banshash;

void 
AddBan(const char* type, const char* user, const char* host, const char* mask,
	const char* reason, const char* setby, const char* tsset, const char* tsexpires)
{
	Ban* ban;
	ban.type = type;
	strlcpy(ban.user, user, MAXUSER];
	strlcpy(ban.host, host, MAXHOST];
	strlcpy(ban.mask, mask, MAXHOST];
	strlcpy(ban.reason, reason,,BUFSIZE];
	strlcpy(ban.setby ,setby, MAXHOST];
	ban.tsset;
	ban.tsexpires;

}

void 
DelBan()
{
}

int 
InitBans (void)
{
	banshash = hash_create (-1, 0, 0);
	if (!banshash) {
		nlog (LOG_CRITICAL, LOG_CORE, "Create bans hash failed\n");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

void 
FreeBans ()
{
	Ban *ban;
	hnode_t *bansnode;
	hscan_t hs;

	hash_scan_begin(&hs, banshash);
	while ((bansnode = hash_scan_next(&hs)) != NULL ) {
		ban = hnode_get (bansnode);
		hash_delete (banshash, bansnode);
		hnode_destroy (bansnode);
		free (ban);
	}
	hash_destroy(banshash);
}
