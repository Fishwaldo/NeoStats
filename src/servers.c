/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "ircd.h"
#include "exclude.h"
#include "modules.h"
#include "servers.h"
#include "services.h"
#include "users.h"

#define SERVER_TABLE_SIZE	-1

config nsconfig;
static hash_t *serverhash;

static unsigned int moddatacnt[NUM_MODULES];

static Client *
new_server (const char *name)
{
	Client *s;

	SET_SEGV_LOCATION();
	if (hash_isfull (serverhash)) {
		nlog (LOG_CRITICAL, "new_ban: server hash is full");
		return NULL;
	}
	dlog(DEBUG2, "new_server: %s", name);
	s = ns_calloc (sizeof (Client));
	strlcpy (s->name, name, MAXHOST);
	s->server = ns_calloc (sizeof (Server));
	hnode_create_insert (serverhash, s, s->name);
	me.servercount++;
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
		strlcpy (s->uplinkname, uplink, MAXHOST);
		s->uplink = FindServer (uplink);
	}
	if (infoline) {
		strlcpy (s->info, infoline, MAXINFO);
	}
	if (numeric) {
		s->server->numeric =  atoi(numeric);
	}
	s->tsconnect = me.now;
	if (!ircstrcasecmp(name, me.name)) {
		s->flags |= CLIENT_FLAG_ME;
	}
	/* check exclusions */
	ns_do_exclude_server(s);
	/* run the module event for a new server. */
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->source = s;
	SendAllModuleEvent (EVENT_SERVER, cmdparams);
	ns_free (cmdparams);
	return(s);
}

static void del_server_leaves (Client * hub)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	dlog(DEBUG1, "del_server_leaves: %s", hub->name);
	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if(ircstrcasecmp (hub->name, s->uplinkname) == 0) {
			dlog(DEBUG1, "del_server_leaves: server %s had uplink %s", s->name, hub->name);
			DelServer (s->name, hub->name);
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
	del_server_leaves (s);
	if(ircd_srv.protocol & PROTOCOL_NOQUIT) {
		QuitServerUsers (s);
	}
	me.servercount--;
	/* run the event for delete server */
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->source = s;
	if(reason) {
		cmdparams->param = (char*)reason;
	}
	SendAllModuleEvent (EVENT_SQUIT, cmdparams);
	ns_free (cmdparams);
	hash_delete (serverhash, sn);
	hnode_destroy (sn);
	ns_free (s->server);
	ns_free (s);
}

Client *
find_server_base64 (const char *num)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if(strncmp(s->name64, num, BASE64SERVERSIZE) == 0) {
			dlog(DEBUG1, "find_server_base64: %s -> %s", num, s->name);
			return s;
		}
	}
	dlog(DEBUG3, "find_server_base64: %s not found!", num);
	return NULL;
}

Client *
FindServer (const char *name)
{
	hnode_t *sn;

	sn = hash_lookup (serverhash, name);
	if (sn) {
		return (Client *) hnode_get (sn);
	}
	dlog(DEBUG3, "FindServer: %s not found!", name);
	return NULL;
}

static void 
dumpserver (Client *s)
{
	/* Calculate uptime as uptime from server plus uptime of NeoStats */
	time_t uptime = s->server->uptime  + (me.now - me.ts_boot);

	if(ircd_srv.protocol & PROTOCOL_B64SERVER) {
		irc_chanalert (ns_botptr, _("Server: %s (%s)"), s->name, s->name64);
	} else {
		irc_chanalert (ns_botptr, _("Server: %s"), s->name);
	}
	irc_chanalert (ns_botptr, _("Version: %s"), s->version);
	irc_chanalert (ns_botptr, _("Uptime:  %ld day%s, %02ld:%02ld:%02ld"), (uptime / 86400), (uptime / 86400 == 1) ? "" : "s", ((uptime / 3600) % 24), ((uptime / 60) % 60), (uptime % 60) );
	irc_chanalert (ns_botptr, _("Flags:   %x"), s->flags);
	irc_chanalert (ns_botptr, _("Uplink:  %s"), s->uplink->name);
	irc_chanalert (ns_botptr, "========================================");
}

void
ListServers (const char *name)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	irc_chanalert (ns_botptr, _("===============SERVERLIST==============="));
	if (!name) {
		hash_scan_begin (&ss, serverhash);
		while ((sn = hash_scan_next (&ss)) != NULL) {
			s = hnode_get (sn);
			dumpserver (s);
		}
	} else {
		s = FindServer (name);
		if (s) {
			dumpserver (s);
		} else {
			irc_chanalert (ns_botptr, _("ListServers: can't find server %s"), name);
		}
	}
}



int 
InitServers (void)
{
	serverhash = hash_create (SERVER_TABLE_SIZE, 0, 0);
	if (!serverhash) {
		nlog (LOG_CRITICAL, "Unable to create server hash");
		return NS_FAILURE;
	}
	AddServer (me.name, NULL, 0, NULL, me.infoline);
	return NS_SUCCESS;
}


void
PingServers (void)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	if(!is_synched)
		return;
	dlog(DEBUG3, "Sending pings...");
	me.ulag = 0;
	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if( IsMe( s ) ) {
			s->server->ping = 0;
			continue;
		}
		irc_ping (me.name, me.name, s->name);
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
		ns_free (s->server);
		ns_free (s);
	}
	hash_destroy(serverhash);
}

int GetServerList (ServerListHandler handler, void *v)
{
	hnode_t *node;
	hscan_t scan;
	Client *ss;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, serverhash);
	while ((node = hash_scan_next(&scan)) != NULL) {
		ss = hnode_get(node);
		if (handler (ss, v) == NS_TRUE)
			break;
	}
	return NS_SUCCESS;
}

void RequestServerUptimes (void)
{
	Client *s;
	hscan_t ss;
	hnode_t *sn;

	hash_scan_begin (&ss, serverhash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		s = hnode_get (sn);
		if( !IsMe( s ) ) {
			send_cmd(":%s STATS u %s", ns_botptr->u->name, s->name);
		}
	}
}

hash_t *GetServerHash (void)
{
	return serverhash;
}

void *AllocServerModPtr (Client* s, int size)
{
	void *ptr;
	ptr = ns_calloc (size);
	s->modptr[GET_CUR_MODNUM()] = ptr;
	fservermoddata |= (1 << GET_CUR_MODNUM());
	moddatacnt[GET_CUR_MODNUM()]++;
	return ptr;
}

void FreeServerModPtr (Client* s)
{
	ns_free (s->modptr[GET_CUR_MODNUM()]);
	moddatacnt[GET_CUR_MODNUM()]--;
	if (moddatacnt[GET_CUR_MODNUM()] == 0)
	{
		fservermoddata &= ~(1 << GET_CUR_MODNUM());
	}
}

void* GetServerModPtr (Client* s)
{
	return s->modptr[GET_CUR_MODNUM()];
}

void ClearServerModValue (Client* s)
{
	if (s)
	{
		s->modvalue[GET_CUR_MODNUM()] = NULL;
		moddatacnt[GET_CUR_MODNUM()]--;
	}
	if (moddatacnt[GET_CUR_MODNUM()] == 0)
	{
		fservermoddata &= ~(1 << GET_CUR_MODNUM());
	}
}

void SetServerModValue (Client* s, void *data)
{
	if (s)
	{
		s->modvalue[GET_CUR_MODNUM()] = data;
		fservermoddata |= (1 << GET_CUR_MODNUM());
		moddatacnt[GET_CUR_MODNUM()]++;
	}
}

void *GetServerModValue (Client* s)
{
	if (s)
	{
		return s->modvalue[GET_CUR_MODNUM()];
	}
	return NULL;	
}

void CleanupServerModdata (int index)
{
	hnode_t *node;
	hscan_t scan;
	Client *s;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, serverhash);
	if (moddatacnt[index] > 0) {
		nlog (LOG_WARNING, "Cleaning up servers after dirty module!");
		while ((node = hash_scan_next(&scan)) != NULL) {
			s = hnode_get(node);
			if (s->modptr[index]) {
				ns_free (s->modptr[index]);		
			}
			s->modvalue[index] = NULL;
		}
	}
	fservermoddata &= ~(1 << index);	
	moddatacnt[index] = 0;
}
