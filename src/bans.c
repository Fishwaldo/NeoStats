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
#include "dl.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

static hash_t *banshash;

static Ban *
new_ban (const char *mask)
{
	Ban *ban;
	hnode_t *bansnode;

	SET_SEGV_LOCATION();
	ban = calloc (sizeof (Ban), 1);
	bzero(ban, sizeof(Ban));
	strlcpy (ban->mask, mask, MAXHOST);
	bansnode = hnode_create (ban);
	if (!bansnode) {
		nlog (LOG_WARNING, LOG_CORE, "bans hash is broken\n");
	}
	if (hash_isfull (banshash)) {
		nlog (LOG_WARNING, LOG_CORE, "bans hash is full!\n");
	} else {
		hash_insert (banshash, bansnode, ban->mask);
	}
	return ban;
}

void AddBan(const char* type, const char* user, const char* host, const char* mask,
	const char* reason, const char* setby, const char* tsset, const char* tsexpires)
{
	char **av;
	int ac = 0;
	Ban* ban;

	ban = new_ban (mask);
	ban->type = type[0];
	strlcpy(ban->user, user, MAXUSER);
	strlcpy(ban->host, host, MAXHOST);
	strlcpy(ban->mask, mask, MAXHOST);
	strlcpy(ban->reason, reason,BUFSIZE);
	strlcpy(ban->setby ,setby, MAXHOST);
	ban->tsset = atol(tsset);
	ban->tsexpires = atol(tsexpires);

	/* run the module event for a new server. */
	AddStringToList (&av, (char*)type, &ac);
	AddStringToList (&av, (char*)user, &ac);
	AddStringToList (&av, (char*)host, &ac);
	AddStringToList (&av, (char*)mask, &ac);
	AddStringToList (&av, (char*)reason, &ac);
	AddStringToList (&av, (char*)setby, &ac);
	AddStringToList (&av, (char*)tsset, &ac);
	AddStringToList (&av, (char*)tsexpires, &ac);
	ModuleEvent (EVENT_ADDBAN, av, ac);
	free (av);
}

void 
DelBan(const char* type, const char* user, const char* host, const char* mask,
	const char* reason, const char* setby, const char* tsset, const char* tsexpires)
{
	Ban *ban;
	hnode_t *bansnode;
	char **av;
	int ac = 0;

	bansnode = hash_lookup (banshash, mask);
	if (!bansnode) {
		nlog (LOG_WARNING, LOG_CORE, "DelBan: unknown ban %s", mask);
		return;
	}
	ban = hnode_get (bansnode);

	/* run the module event for a new server. */
	AddStringToList (&av, (char*)type, &ac);
	AddStringToList (&av, (char*)user, &ac);
	AddStringToList (&av, (char*)host, &ac);
	AddStringToList (&av, (char*)mask, &ac);
	AddStringToList (&av, (char*)reason, &ac);
	AddStringToList (&av, (char*)setby, &ac);
	AddStringToList (&av, (char*)tsset, &ac);
	AddStringToList (&av, (char*)tsexpires, &ac);
	ModuleEvent (EVENT_DELBAN, av, ac);
	free (av);

	hash_delete (banshash, bansnode);
	hnode_destroy (bansnode);
	free (ban);
}

void
BanDump (void)
{
	Ban *ban;
	hscan_t ss;
	hnode_t *bansnode;

	debugtochannel("Server Listing:");
	hash_scan_begin (&ss, banshash);
	while ((bansnode = hash_scan_next (&ss)) != NULL) {
		ban = hnode_get (bansnode);
		debugtochannel("Ban: %s ", ban->mask);
	}
	debugtochannel("End of Listing.");
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


#ifdef SQLSRV
COLDEF neo_banscols[] = {
	{
		"bans",
		"type",
		RTA_STR,
		1,
		offsetof(struct Ban, type),
		RTA_READONLY,
		NULL,
		NULL,
		"type"
	},
	{
		"bans",
		"user",
		RTA_STR,
		MAXUSER,
		offsetof(struct Ban, user),
		RTA_READONLY,
		NULL,
		NULL,
		"user"
	},
	{
		"bans",
		"host",
		RTA_STR,
		MAXHOST,
		offsetof(struct Ban, host),
		RTA_READONLY,
		NULL,
		NULL,
		"host"
	},
	{
		"bans",
		"mask",
		RTA_STR,
		MAXHOST,
		offsetof(struct Ban, mask),
		RTA_READONLY,
		NULL,
		NULL,
		"mask"
	},
	{
		"bans",
		"reason",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Ban, reason),
		RTA_READONLY,
		NULL,
		NULL,
		"reason"
	},
	{
		"bans",
		"setby",
		RTA_STR,
		MAXHOST,
		offsetof(struct Ban, setby),
		RTA_READONLY,
		NULL,
		NULL,
		"setby"
	},
	{
		"bans",
		"tsset",
		RTA_INT,
		sizeof(int),
		offsetof(struct Ban, tsset),
		RTA_READONLY,
		NULL,
		NULL,
		"tsset"
	},
	{
		"bans",
		"tsexpire",
		RTA_INT,
		sizeof(int),
		offsetof(struct Ban, tsexpires),
		RTA_READONLY,
		NULL,
		NULL,
		"tsexpire"
	},
};

TBLDEF neo_bans = {
	"bans",
	NULL, 	/* for now */
	sizeof(struct Ban),
	0,
	TBL_HASH,
	neo_banscols,
	sizeof(neo_banscols) / sizeof(COLDEF),
	"",
	"The list of bans on the IRC network"
};
#endif /* SQLSRV */

int 
InitBans (void)
{
	banshash = hash_create (-1, 0, 0);
	if (!banshash) {
		nlog (LOG_CRITICAL, LOG_CORE, "Create bans hash failed\n");
		return NS_FAILURE;
	}
#ifdef SQLSRV
	/* add the server hash to the sql library */
	neo_bans.address = banshash;
	rta_add_table(&neo_bans);
#endif
	return NS_SUCCESS;
}

