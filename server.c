/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

hash_t *sh;

static Server *new_server (char * name);

static Server *
new_server (char *name)
{
	Server *s;
	hnode_t *sn;

	s = calloc (sizeof (Server), 1);
	if (!name)
		strsetnull (s->name);
	else
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
AddServer (char *name, char *uplink, int hops)
{
	Server *s;
	char **av;
	int ac = 0;

	nlog (LOG_DEBUG1, LOG_CORE, "New Server: %s", name);
	s = new_server (name);
	s->hops = hops;
	s->connected_since = me.now;
	if (uplink) {
		strlcpy (s->uplink, uplink, MAXHOST);
	} else {
		strsetnull (s->uplink);
	}
	s->ping = 0;

	/* run the module event for a new server. */
	AddStringToList (&av, s->name, &ac);
	ModuleEvent (EVENT_NEWSERVER, av, ac);
	free (av);
	return(s);
}

void
DelServer (char *name)
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
		nlog (LOG_DEBUG1, LOG_CORE, "DelServer(): %s not found!", name);
		return;
	}
	s = hnode_get (sn);

	/* run the event for delete server */
	AddStringToList (&av, s->name, &ac);
	ModuleEvent (EVENT_SQUIT, av, ac);
	free (av);

	hash_delete (sh, sn);
	hnode_destroy (sn);
	free (s);
}

void 
SquitServer(char* name)
{
	Server *s;
	s = findserver (name);
	if (s) {
		DelServer (name);
	} else {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Squit from Unknown Server %s", name);
	}
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
		nlog (LOG_DEBUG2, LOG_CORE, "FindServer(): %s not found!", name);
		return NULL;
	}
}

void
ServerDump ()
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
		"uplink",
		RTA_STR,
		MAXHOST,
		offsetof(struct Server, uplink),
		RTA_READONLY,
		NULL,
		NULL,
		"The uplink Server this server is connected to. if it = self, means the NeoStats Server"
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
init_server_hash ()
{


	sh = hash_create (S_TABLE_SIZE, 0, 0);
	if (!sh) {
		nlog (LOG_CRITICAL, LOG_CORE, "Create Server Hash Failed\n");
		return NS_FAILURE;
	}
	AddServer (me.name, NULL, 0);
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
		sping_cmd (me.name, me.name, s->name);
	}
}
