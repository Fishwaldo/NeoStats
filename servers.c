/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: servers.c,v 1.1 2000/06/10 09:14:03 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "linklist.h"
#include "stats.h"

int fnmatch(const char *, const char *, int flags);


void AddServer(char *name,char *uplink, int hops)
{
	Server *s;


#ifdef DEBUG
	log("New Server: %s", name);
#endif
	strlower(name);
	s = smalloc(sizeof(Server));
	strcpy(s->name, name);
	s->hops = hops;
	s->connected_since = time(NULL);
	s->last_announce = time(NULL);
	memcpy(s->uplink,uplink, MAXHOST);
	s->ping = 0;
	if (DLL_AddRecord(LL_Servers, s, NULL) == DLL_MEM_ERROR) {
		log("Error, Couldn't AddServer, Out Of Memory");
		exit(-1);
	}
#ifdef DEBUGLL
	log("DEBUGLL: Number of Server Records %ld", DLL_GetNumberOfRecords(LL_Servers));
	log("DEBUGLL: Current Server Index %ld", DLL_GetCurrentIndex(LL_Servers));
#endif	
	if (s) free(s);
}

void DelServer(char *name)
{
	DLL_Return ExitCode;
	Server *s = findserver(name);

	if (!s || !name) {
		log("DelServer(): %s failed!", name);
		return;
	}
#ifdef DEBUG
	log("DelServer(%s)", name);
#endif
	ExitCode = DLL_DeleteCurrentRecord(LL_Servers);
	if (ExitCode != DLL_NORMAL) {
		log("Error, Couldn't Delete Current Server %s", name);
	}

	if (s) free(s);
}
int match_server(Server *s, char *name) {
#ifdef DEBUGLL
	log("DEBUGLL: MatchServer: %s -> %s", s->name, name);
#endif
	return(strcasecmp(s->name, name));
}

Server *findserver(char *name)
{

	strlower(name);
#ifdef DEBUGLL
	log("DEBUGLL: FindServer %s", name);
#endif
	DLL_SetSearchModes(LL_Servers, DLL_HEAD, DLL_DOWN);
	switch(DLL_FindRecord(LL_Servers, CurS, name, (int (*)()) match_server)) {
		case DLL_NORMAL:
#ifdef DEBUGLL
			log("DEBUGLL: FindServer Found %s", CurS->name);
#endif
			return CurS;
			break;
		default:
			return NULL;
	}
}

void ServerDump()
{
	Server *s;
	DLL_Return ExitCode;

	s = smalloc(sizeof(Server));
	sendcoders("\2Server Listing:\2");
	ExitCode = DLL_CurrentPointerToHead(LL_Servers);
	if (ExitCode == DLL_NORMAL) {
		while (ExitCode == DLL_NORMAL) {
			ExitCode = DLL_GetCurrentRecord(LL_Servers, s);
			sendcoders("Server: %s Uplink %s, Hops: %d, Ping %d", s->name, s->uplink, s->hops, s->ping);
			if ((ExitCode = DLL_IncrementCurrentPointer(LL_Servers)) == DLL_NOT_FOUND) break;
		}
	} else {
		sendcoders("Ehhh, Something is Wrong, ServerList is Empty");
	}
	sendcoders("\2End of List...\2");
	if (s) free(s);
}

void init_server_hash()
{
	DLL_Return ExitCode;
	
	if (DLL_CreateList(&LL_Servers) == NULL) {
		log("Error, Could not Create ServerList");
		exit(-1);
	}
	if ((ExitCode = DLL_InitializeList(LL_Servers, sizeof(Server))) != DLL_NORMAL) {
		if (ExitCode == DLL_ZERO_INFO) log("Error, Server Structure is 0");
		if (ExitCode == DLL_NULL_LIST) log("Error, Server Structure is Null");
		exit(-1);
	}
	CurS = smalloc(sizeof(Server));
	AddServer(me.name,me.name, 0);
}

int Server_Ping(Server *s) {
	DLL_Return ExitCode;
	
	s->ping = time(NULL) - ping.last_sent;
	if(ping.ulag > 1) {
		s->ping = (float) ping.ulag;
		if (!strcasecmp(me.s->name, s->name)) ping.ulag = me.s->ping;
	}
	ExitCode = DLL_UpdateCurrentRecord(LL_Servers, s);
	if (ExitCode != DLL_NORMAL) {
		log("Error, Couldn't update Record for %s", s->name);
		return -1;
	}
	return 1;
}	