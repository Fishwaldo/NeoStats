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
#include "exclude.h"
#include "server.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

hash_t *sh;

static Server *
new_server (const char *name)
{
	Server *s;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	s = calloc (sizeof (Server), 1);
	bzero(s, sizeof(Server));
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
AddServer (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline)
{
	Server *s;
	char **av;
	int ac = 0;

	nlog (LOG_DEBUG1, LOG_CORE, "New Server: %s", name);
	s = new_server (name);
	if(hops) {
		s->hops = atoi (hops);
	}
	if (uplink) {
		strlcpy (s->uplink, uplink, MAXHOST);
	}
	if(infoline) {
		strlcpy (s->infoline, infoline, MAXINFO);
	}
	if(numeric) {
		s->numeric =  atoi(numeric);
	}
	s->connected_since = me.now;

	if (!ircstrcasecmp(name, me.name)) {
		s->flags |= NS_FLAGS_ME;
	}

	/* check exclusions */
	ns_do_exclude_server(s);

	/* run the module event for a new server. */
	AddStringToList (&av, (char*)name, &ac);
	AddStringToList (&av, (char*)uplink, &ac);
	AddStringToList (&av, (char*)hops, &ac);
	AddStringToList (&av, (char*)numeric, &ac);
	AddStringToList (&av, (char*)infoline, &ac);
	ModuleEvent (EVENT_SERVER, av, ac);
	free (av);
	return(s);
}

static void del_server_leaves(Server* hub)
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	nlog (LOG_DEBUG1, LOG_CORE, "del_server_leaves: %s", hub->name);
	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if(ircstrcasecmp(hub->name, s->uplink) == 0) {
			nlog (LOG_DEBUG1, LOG_CORE, "del_server_leaves: del child %s", s->name);
			DelServer(s->name, hub->name);
		}
	}
}

void 
DelServer (const char *name, const char* reason)
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
		nlog (LOG_WARNING, LOG_CORE, "DelServer: squit from unknown server %s", name);
		return;
	}
	s = hnode_get (sn);
	del_server_leaves(s);
	if(ircd_srv.noquit) {
		QuitServerUsers (s);
	}
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

#ifdef BASE64SERVERNAME
Server *
findserverbase64 (const char *num)
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	hash_scan_begin (&ss, sh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if(strncmp(s->name64, num, BASE64SERVERSIZE) == 0) {
			nlog (LOG_DEBUG1, LOG_CORE, "findserverbase64: %s -> %s", num, s->name);
			return s;
		}
	}
	nlog (LOG_DEBUG3, LOG_CORE, "findserverbase64: %s not found!", num);
	return NULL;
}
#endif

Server *
findserver (const char *name)
{
	Server *s;
	hnode_t *sn;

	sn = hash_lookup (sh, name);
	if (sn) {
		s = hnode_get (sn);
		return s;
	}
	nlog (LOG_DEBUG3, LOG_CORE, "findserver: %s not found!", name);
	return NULL;
}

static void 
dumpserver (Server *s)
{
#ifdef BASE64SERVERNAME
	debugtochannel("Server: %s (%s)", s->name, s->name64);
#else
	debugtochannel("Server: %s", s->name);
#endif
	debugtochannel("Flags:  %lx", s->flags);
	debugtochannel("Uplink: %s", s->uplink);
	debugtochannel("========================================");
}

void
ServerDump (const char *name)
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	debugtochannel("================SERVDUMP================");
	if (!name) {
		hash_scan_begin (&ss, sh);
		while ((sn = hash_scan_next (&ss)) != NULL) {
			s = hnode_get (sn);
			dumpserver (s);
		}
	} else {
		s = findserver (name);
		if (s) {
			dumpserver (s);
		} else {
			debugtochannel("ServerDump: can't find server %s", name);
		}
	}
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
	AddServer (me.name, NULL, 0, NULL, me.infoline);
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

	if(!me.synced)
		return;
	nlog (LOG_DEBUG3, LOG_CORE, "Sending pings...");
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

void 
FreeServers ()
{
	Server *s;
	hnode_t *sn;
	hscan_t hs;

	hash_scan_begin(&hs, sh);
	while ((sn = hash_scan_next(&hs)) != NULL ) {
		s = hnode_get (sn);
		hash_delete (sh, sn);
		hnode_destroy (sn);
		free (s);
	}
	hash_destroy(sh);

}
