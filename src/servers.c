/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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

#include "neostats.h"
#include "hash.h"
#include "ircd.h"
#include "exclude.h"
#include "modules.h"
#include "servers.h"
#include "services.h"
#include "users.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

tconfig config;
static hash_t *serverhash;

static Client *
new_server (const char *name)
{
	Client *s;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	if (hash_isfull (serverhash)) {
		nlog (LOG_CRITICAL, "new_ban: server hash is full");
		return NULL;
	}
	s = scalloc (sizeof (Client));
	strlcpy (s->name, name, MAXHOST);
	s->server = scalloc (sizeof (Server));
	sn = hnode_create (s);
	if (!sn) {
		nlog (LOG_WARNING, "Server hash broken");
		sfree (s->server);
		sfree (s);
		return NULL;
	}
	hash_insert (serverhash, sn, s->name);
	return s;
}

Client *
AddServer (const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline)
{
	CmdParams * cmdparams;
	Client *s;

	dlog(DEBUG1, "AddServer: %s", name);
	s = new_server (name);
	if(hops) {
		s->server->hops = atoi (hops);
	}
	if (uplink) {
		strlcpy (s->uplink, uplink, MAXHOST);
	}
	if (infoline) {
		strlcpy (s->info, infoline, MAXINFO);
	}
	if (numeric) {
		s->server->numeric =  atoi(numeric);
	}
	s->tsconnect = me.now;
	if (!ircstrcasecmp(name, me.name)) {
		s->flags |= NS_FLAGS_ME;
	}
	/* check exclusions */
	ns_do_exclude_server(s);
	/* run the module event for a new server. */
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = s;
	SendAllModuleEvent (EVENT_SERVER, cmdparams);
	sfree (cmdparams);
	return(s);
}

static void del_server_leaves(Client * hub)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	dlog(DEBUG1, "del_server_leaves: %s", hub->name);
	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if(ircstrcasecmp(hub->name, s->uplink) == 0) {
			dlog(DEBUG1, "del_server_leaves: del child %s", s->name);
			DelServer(s->name, hub->name);
		}
	}
}

void 
DelServer (const char *name, const char* reason)
{
	CmdParams * cmdparams;
	Client *s;
	hnode_t *sn;

	dlog(DEBUG1, "DelServer: %s", name);
	sn = hash_lookup (serverhash, name);
	if (!sn) {
		nlog (LOG_WARNING, "DelServer: squit from unknown server %s", name);
		return;
	}
	s = hnode_get (sn);
	del_server_leaves(s);
	if(ircd_srv.protocol & PROTOCOL_NOQUIT) {
		QuitServerUsers (s);
	}
	/* run the event for delete server */
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = s;
	if(reason) {
		cmdparams->param = (char*)reason;
	}
	SendAllModuleEvent (EVENT_SQUIT, cmdparams);
	sfree (cmdparams);
	hash_delete (serverhash, sn);
	hnode_destroy (sn);
	sfree (s->server);
	sfree (s);
}

Client *
findserverbase64 (const char *num)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if(strncmp(s->name64, num, BASE64SERVERSIZE) == 0) {
			dlog(DEBUG1, "findserverbase64: %s -> %s", num, s->name);
			return s;
		}
	}
	dlog(DEBUG3, "findserverbase64: %s not found!", num);
	return NULL;
}

Client *
find_server (const char *name)
{
	hnode_t *sn;

	sn = hash_lookup (serverhash, name);
	if (sn) {
		return (Client *) hnode_get (sn);
	}
	dlog(DEBUG3, "find_server: %s not found!", name);
	return NULL;
}

static void 
dumpserver (Client *s)
{
	/* Calculate uptime as uptime from server plus uptime of NeoStats */
	int uptime = s->server->uptime  + (me.now - me.t_start);

	if(ircd_srv.protocol & PROTOCOL_B64SERVER) {
		irc_chanalert (ns_botptr, "Server: %s (%s)", s->name, s->name64);
	} else {
		irc_chanalert (ns_botptr, "Server: %s", s->name);
	}
	irc_chanalert (ns_botptr, "Version: %s", s->version);
	irc_chanalert (ns_botptr, "Uptime:  %d day%s, %02d:%02d:%02d", (uptime / 86400), (uptime / 86400 == 1) ? "" : "s", ((uptime / 3600) % 24), ((uptime / 60) % 60), (uptime % 60) );
	irc_chanalert (ns_botptr, "Flags:   %lx", s->flags);
	irc_chanalert (ns_botptr, "Uplink:  %s", s->uplink);
	irc_chanalert (ns_botptr, "========================================");
}

void
ServerDump (const char *name)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	irc_chanalert (ns_botptr, "===============SERVERDUMP===============");
	if (!name) {
		hash_scan_begin (&ss, serverhash);
		while ((sn = hash_scan_next (&ss)) != NULL) {
			s = hnode_get (sn);
			dumpserver (s);
		}
	} else {
		s = find_server (name);
		if (s) {
			dumpserver (s);
		} else {
			irc_chanalert (ns_botptr, "ServerDump: can't find server %s", name);
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
		offsetof(struct Server, tsconnect),
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
InitServers (void)
{
	serverhash = hash_create (S_TABLE_SIZE, 0, 0);
	if (!serverhash) {
		nlog (LOG_CRITICAL, "Unable to create server hash");
		return NS_FAILURE;
	}
	AddServer (me.name, NULL, 0, NULL, me.infoline);
#ifdef SQLSRV
	/* add the server hash to the sql library */
	neo_servers.address = serverhash;
	rta_add_table(&neo_servers);
#endif
	return NS_SUCCESS;
}


void
PingServers (void)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	if(!me.synched)
		return;
	dlog(DEBUG3, "Sending pings...");
	ping.ulag = 0;
	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if (!strcmp (me.name, s->name)) {
			s->server->ping = 0;
			continue;
		}
		irc_send_ping (me.name, me.name, s->name);
	}
}

void 
FiniServers (void)
{
	Client *s;
	hnode_t *sn;
	hscan_t hs;

	hash_scan_begin(&hs, serverhash);
	while ((sn = hash_scan_next(&hs)) != NULL ) {
		s = hnode_get (sn);
		hash_delete (serverhash, sn);
		hnode_destroy (sn);
		sfree (s->server);
		sfree (s);
	}
	hash_destroy(serverhash);
}

void GetServerList(ServerListHandler handler)
{
	hnode_t *node;
	hscan_t scan;
	Client *ss;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, serverhash);
	while ((node = hash_scan_next(&scan)) != NULL) {
		ss = hnode_get(node);
		handler(ss);
	}
}

void RequestServerUptimes (void)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if (strcmp (me.name, s->name)) {
			send_cmd(":%s STATS u %s", ns_botptr->u->name, s->name);
		}
	}
}
