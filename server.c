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
** $Id: server.c,v 1.13 2002/09/04 08:40:27 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"

int fnmatch(const char *, const char *, int flags);
Server *new_server(char *);


void init_server() {
	if (usr_mds);
}

Server *new_server(char *name)
{
	Server *s;
	hnode_t *sn;

	if (!hash_verify(sh)) {
		globops(me.name, "Eeek, Server Table is corrupted! Continuing, but expect a crash");
		chanalert(me.name, "Eeek, Server Table is corrupted! Continuing, but expect a crash");
		log("Eeek, Server table is corrupted");
	}	
	s = calloc(sizeof(Server), 1);
	if (!name)
		name = "";
	memcpy(s->name, name, MAXHOST);
	sn = hnode_create(s);
	if (!sn) {
		log("Eeek, Hash is broken\n");
	}
	if (hash_isfull(sh)) {
		log("Eeek, Server Hash is full!\n");
	} else {
		hash_insert(sh, sn, s->name);
	}
	return s;
}

void AddServer(char *name,char *uplink, int hops)
{
	Server *s;

#ifdef DEBUG
	log("New Server: %s", name);
#endif
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
		log("DelServer(): %s failed!", name);
		return;
	}
	sn = hash_lookup(sh, name);
	if (!sn) {
		log("DelServer(): %s not found!", name);
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
#ifdef DEBUG
		log("FindServer(): %s not found!", name);
#endif
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
		log("Create Server Hash Failed\n");
		exit(-1);
	}
	AddServer(me.name,NULL, 0);
}

void TimerPings()
{
	Server *s;
	hscan_t ss;
	hnode_t *sn;

#ifdef DEBUG
	log("Sendings pings...");
#endif
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
