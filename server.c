/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: server.c,v 1.15 2003/04/11 09:26:30 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"
#include "log.h"

int fnmatch(const char *, const char *, int flags);
Server *new_server(char *);


void init_server() {
	if (usr_mds);
}

Server *new_server(char *name)
{
	Server *s;
	hnode_t *sn;

	s = calloc(sizeof(Server), 1);
	if (!name)
		name = "";
	memcpy(s->name, name, MAXHOST);
	sn = hnode_create(s);
	if (!sn) {
		nlog(LOG_WARNING, LOG_CORE, "Eeek, Hash is broken\n");
	}
	if (hash_isfull(sh)) {
		nlog(LOG_WARNING, LOG_CORE, "Eeek, Server Hash is full!\n");
	} else {
		hash_insert(sh, sn, s->name);
	}
	return s;
}

void AddServer(char *name,char *uplink, int hops)
{
	Server *s;

	nlog(LOG_DEBUG1, LOG_CORE, "New Server: %s", name);
	s = new_server(name);
	s->hops = hops;
	s->connected_since = time(NULL);
	s->last_announce = time(NULL);
	if (uplink) {
		memcpy(s->uplink,uplink, MAXHOST);
	} else {
		strcpy(s->uplink, "\0");
	}
	s->ping = 0;
}

void DelServer(char *name)
{
	Server *s;
	hnode_t *sn;

	if (!name) {
		return;
	}
	sn = hash_lookup(sh, name);
	if (!sn) {
		nlog(LOG_DEBUG1, LOG_CORE, "DelServer(): %s not found!", name);
		return;
	}
	hash_delete(sh, sn);
	s = hnode_get(sn);
	hnode_destroy(sn);
	free(s);
}

Server *findserver(const char *name)
{
	Server *s;
	hnode_t *sn;

	sn = hash_lookup(sh, name);
	if (sn) {
		s = hnode_get(sn);
		return s;
	} else {
		nlog(LOG_DEBUG2, LOG_CORE, "FindServer(): %s not found!", name);
		return NULL;
	}	
}

void ServerDump()
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	sendcoders("Server Listing:");

	hash_scan_begin(&ss, sh);
	while ((sn = hash_scan_next(&ss)) != NULL) {
		s = hnode_get(sn);
		sendcoders("Server Entry: %s", s->name);
	}
	sendcoders("End of Listing.");
}

void init_server_hash()
{
	sh = hash_create(S_TABLE_SIZE, 0, 0);	
	if (!sh) {
		nlog(LOG_CRITICAL, LOG_CORE, "Create Server Hash Failed\n");
		do_exit(1);
	}
	AddServer(me.name,NULL, 0);
}

void TimerPings()
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

	nlog(LOG_DEBUG3, LOG_CORE, "Sendings pings...");
	ping.ulag = 0;

	hash_scan_begin(&ss, sh);
	while ((sn = hash_scan_next(&ss)) != NULL) {
		s = hnode_get(sn);
		if (!strcmp(me.name, s->name)) {
			s->ping = 0;
			continue;
		}
		sping_cmd(me.name, me.name, s->name);
	}
}
