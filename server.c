/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: server.c,v 1.1 2002/02/27 12:33:13 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"

int fnmatch(const char *, const char *, int flags);

static Server *new_server(char *);
Server *serverlist[S_TABLE_SIZE];

static void add_server_to_hash_table(char *, Server *);
static void del_server_from_hash_table(char *, Server *);

static void add_server_to_hash_table(char *name, Server *s)
{
	s->hash = HASH(name, S_TABLE_SIZE);
	s->next = serverlist[s->hash];
	serverlist[s->hash] = (void *)s;
}

static void del_server_from_hash_table(char *name, Server *s)
{
	Server *tmp, *prev = NULL;

	for (tmp = serverlist[s->hash]; tmp; tmp = tmp->next) {
		if (tmp == s) {
			if (prev)
				prev->next = tmp->next;
			else
				serverlist[s->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}


static Server *new_server(char *name)
{
	Server *s;

	s = calloc(sizeof(Server), 1);
	if (!name)
		name = "";
	memcpy(s->name, name, MAXHOST);
	add_server_to_hash_table(name, s);

	return s;
}

void AddServer(char *name,char *uplink, int hops)
{
	Server *s;

#ifdef DEBUG
	log("New Server: %s", name);
#endif
	strlower(name);
	s = new_server(name);
	s->hops = hops;
	s->connected_since = time(NULL);
	s->last_announce = time(NULL);
	memcpy(s->uplink,uplink, MAXHOST);
	s->ping = 0;
}

void DelServer(char *name)
{
	Server *s = findserver(name);

	if (!s || !name) {
		log("DelServer(): %s failed!", name);
		return;
	}

	del_server_from_hash_table(name, s);
	free(s);
}

Server *findserver(char *name)
{
	Server *s;

	strlower(name);
	s = serverlist[HASH(name, S_TABLE_SIZE)];
	while (s && strcmp(s->name, name) != 0)
		s = s->next;
	if (s)
		return s;

	s = serverlist[HASH(name, S_TABLE_SIZE)];
	while (s && fnmatch(name, s->name, 0) != 0)
		s = s->next;

#ifdef DEBUG
	log("findserver(%s) -> %s", name, (s) ? s->name : "NOT FOUND");
#endif

	return s;
}

void ServerDump()
{
	Server *s;
	register int j;

	sendcoders("Server Listing:");
	for (j = 0; j < S_TABLE_SIZE; j++) {
		for (s = serverlist[j]; s; s = s->next) {
			sendcoders("Server Entry: %s", 
				s->name);
		}
	}
	sendcoders("End of Listing.");
}

void init_server_hash()
{
	int i;
	Server *s, *b;

	for (i = 0; i < S_TABLE_SIZE; i++) {
		s = serverlist[i];
		while (s) {
			b = s->next;
			free(s);
			s = b;
		}
		serverlist[i] = NULL;
	}
	bzero((char *)serverlist, sizeof(serverlist));
	AddServer(me.name,me.name, 0);
}

