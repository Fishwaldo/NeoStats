/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
#include "dl.h"
#include "hash.h"
#include "log.h"
#include "ircd.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

hash_t *sh;

static Server *
new_server (const char *name)
{
	Server *s;
	hnode_t *sn;

	s = calloc (sizeof (Server), 1);
	strlcpy (s->name, name, MAXHOST);
	sn = hnode_create (s);
	if (!sn) {
		nlog (LOG_WARNING, LOG_CORE, "Eeek, Hash is broken\n");
	}
	if (hash_isfull (sh)) {
		nlog (LOG_WARNING, LOG_CORE, "Eeek, Server Hash is full!\n");
	} else {
		hash_insert (sh, sn, s->name);
	}
	return s;
}

Server *
AddServer (const char *name, const char *uplink, const char* hops, const char *infoline)
{
	Server *s;
	char **av;
	int ac = 0;
	int nhops;

	nlog (LOG_DEBUG1, LOG_CORE, "New Server: %s", name);
	s = new_server (name);
	s->hops = atoi (hops);
	s->connected_since = me.now;
	if (uplink) {
		strlcpy (s->uplink, uplink, MAXHOST);
	} else {
		strsetnull (s->uplink);
	}
	s->ping = 0;
	s->flags = 0;
	if(infoline) {
		strlcpy (s->infoline, infoline, MAXINFO);
	} else {
		strsetnull (s->infoline);
	}

	/* run the module event for a new server. */
	AddStringToList (&av, s->name, &ac);
	AddStringToList (&av, (char*)uplink, &ac);
    AddStringToList (&av, (char*)hops, &ac);
	AddStringToList (&av, s->infoline, &ac);
	ModuleEvent (EVENT_SERVER, av, ac);
	free (av);
	return(s);
}

void 
SquitServer (const char *name, const char* reason)
{
	Server *s;
	hnode_t *sn;
	char **av;
	int ac = 0;

	if (!name) {
		return;
	}
	sn = hash_lookup (sh, name);
	if (!sn) {
		nlog (LOG_WARNING, LOG_CORE, "SquitServer: squit from unknown server %s", name);
		return;
	}
	s = hnode_get (sn);

	/* run the event for delete server */
	AddStringToList (&av, s->name, &ac);
	if(reason) {
		AddStringToList (&av, (char*)reason, &ac);
	}
	ModuleEvent (EVENT_SQUIT, av, ac);
	free (av);

	hash_delete (sh, sn);
	hnode_destroy (sn);
	free (s);
}

Server *
findserver (const char *name)
{
	Server *s;
	hnode_t *sn;

	sn = hash_lookup (sh, name);
	if (sn) {
		s = hnode_get (sn);
		return s;
	} else {
		nlog (LOG_DEBUG2, LOG_CORE, "FindServer: %s not found!", name);
		return NULL;
	}
}

void
ServerDump (void)
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	debugtochannel("Server Listing:");

	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		debugtochannel("Server Entry: %s", s->name);
	}
	debugtochannel("End of Listing.");
}

#ifdef SQLSRV
COLDEF neo_serverscols[] = {
	{
		"servers",
		"name",
		RTA_STR,
		MAXHOST,
		offsetof(struct Server, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the server linked to the IRC network"
	},
	{
		"servers",
		"hops",
		RTA_INT,
		sizeof(int),
		offsetof(struct Server, hops),
		RTA_READONLY,
		NULL, 
		NULL,
		"The Number of hops away from the NeoStats Server"
	},
	{
		"servers",
		"connected",
		RTA_INT,
		sizeof(int),
		offsetof(struct Server, connected_since),
		RTA_READONLY,
		NULL,
		NULL,
		"The time the server connected to the IRC network"
	},
	{
		"servers",
		"last_ping",
		RTA_INT,
		sizeof(int),
		offsetof(struct Server, ping),
		RTA_READONLY,
		NULL,
		NULL,
		"The last ping time to this server from the NeoStats Server"
	},
	{
		"servers",
		"flags",
		RTA_INT,
		sizeof(int),
		offsetof(struct Server, flags),
		RTA_READONLY,
		NULL,
		NULL,
		"Flags that specify special functions for this Server"
	},
	{	
		"servers",
		"uplink",
		RTA_STR,
		MAXHOST,
		offsetof(struct Server, uplink),
		RTA_READONLY,
		NULL,
		NULL,
		"The uplink Server this server is connected to. if it = self, means the NeoStats Server"
	},
	{	
		"servers",
		"infoline",
		RTA_STR,
		MAXINFO,
		offsetof(struct Server, infoline),
		RTA_READONLY,
		NULL,
		NULL,
		"The description of this server"
	},
};

TBLDEF neo_servers = {
	"servers",
	NULL, 	/* for now */
	sizeof(struct Server),
	0,
	TBL_HASH,
	neo_serverscols,
	sizeof(neo_serverscols) / sizeof(COLDEF),
	"",
	"The list of Servers connected to the IRC network"
};
#endif /* SQLSRV */


int 
init_server_hash (void)
{
	sh = hash_create (S_TABLE_SIZE, 0, 0);
	if (!sh) {
		nlog (LOG_CRITICAL, LOG_CORE, "Create Server Hash Failed\n");
		return NS_FAILURE;
	}
	AddServer (me.name, NULL, 0, NULL);
#ifdef SQLSRV
	/* add the server hash to the sql library */
	neo_servers.address = sh;
	rta_add_table(&neo_servers);
#endif
	return NS_SUCCESS;
}

void
PingServers (void)
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	nlog (LOG_DEBUG3, LOG_CORE, "Sendings pings...");
	ping.ulag = 0;

	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if (!strcmp (me.name, s->name)) {
			s->ping = 0;
			continue;
		}
		send_ping (me.name, me.name, s->name);
	}
}
