/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: chans.c,v 1.2 2000/12/10 06:25:51 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"


void init_chan_hash() 
{
	DLL_Return ExitCode;
	
	if (DLL_CreateList(&LL_Chans) == NULL) {
		log("Error, Could not create Chanlist");
		exit(-1);
	}
	if ((ExitCode = DLL_InitializeList(LL_Chans, sizeof(Chans))) != DLL_NORMAL) {
		if (ExitCode == DLL_ZERO_INFO) log("Error, Chans Structure is 0");
		if (ExitCode == DLL_NULL_LIST) log("Error, Chans Structure is NULL");
		exit(-1);
	}
}
void ChanDump()
{
	Chans *c;
	DLL_Return ExitCode;
	DLL_Return ExitCode1;
	Channel_Members *cm;

	sendcoders("\2Channel Listing:\2");
	c = smalloc(sizeof(Chans));
	cm = smalloc(sizeof(Channel_Members));
	ExitCode = DLL_CurrentPointerToHead(LL_Chans);
	if (ExitCode == DLL_NORMAL) {
		while (ExitCode == DLL_NORMAL) {
			ExitCode = DLL_GetCurrentRecord(LL_Chans, c);
			sendcoders("\2Chan: %s\2, Users %d", c->name, c->cur_users);
			ExitCode1 = DLL_CurrentPointerToHead(c->chanmembers);
			if (ExitCode1 == DLL_NORMAL) {
				while (ExitCode1 == DLL_NORMAL) {
					ExitCode1 = DLL_GetCurrentRecord(c->chanmembers, cm);
					sendcoders("Chanmembers: %s", cm->nick);
					if ((ExitCode1 = DLL_IncrementCurrentPointer(c->chanmembers)) == DLL_NOT_FOUND) break;
				}
			}		 
			if ((ExitCode = DLL_IncrementCurrentPointer(LL_Chans)) == DLL_NOT_FOUND) break;
		}
	} else {
		sendcoders("Channel List is Empty");
	}
	sendcoders("\2End of List...\2");
	free(c);
	free(cm);
}
int match_chan(Chans *c, char *s) {
#ifdef DEBUGLL
	log("DEBUGLL: MatchChan: %s -> %s", c->name, s);
#endif
	return(strcasecmp(c->name, s));
}
Chans *findchan(char *chan)
{
	Chans *c;
	c = smalloc(sizeof(Chans));
#ifdef DEBUGLL
	log("DEBUGLL: FindChan: %s", chan);
#endif
	DLL_SetSearchModes(LL_Chans, DLL_HEAD, DLL_DOWN);
	switch(DLL_FindRecord(LL_Chans, c, chan, (int (*)()) match_chan)) {
		case DLL_NORMAL:
#ifdef DEBUGLL
			log("DEBUGLL: FindChan Found %s", c->name);
#endif
			return c;
			break;
		default:
			return NULL;
	}
}
int ChanJoin(char *chan, User *u) {
	DLL_Return ExitCode;
	Chans *c;
	Channel_Members *cm;
	User_Chans *uc;
	
	cm = smalloc(sizeof(Channel_Members));
	c = findchan(chan);
	if (!c) {
		/* This is a New Channel, create it first */
		AddChan(chan);
		c = findchan(chan);
	}
	/* add the user to the members list, and increment the counter */
	c->cur_users++;
log("%s", u->nick);

	memcpy(cm->nick,u->nick, sizeof(cm->nick));
log("%s", u->nick);
	if (DLL_AddRecord(c->chanmembers, cm, NULL) == DLL_MEM_ERROR) {
		log("Error, couldn't Chanjoin, Out of Memory");
		exit(-1);
	}
	/* k, update the channel list :) */
	ExitCode = DLL_UpdateCurrentRecord(LL_Chans, c);
	if (ExitCode != DLL_NORMAL) {
		log("Error, Couldn't update Channel Record for %s", chan);
	}
	if (c) free(c);
	if (cm) free(cm);
	
	/* now, update the users entry as well :) */
	/*u = finduser(u->nick); */
#ifdef DEBUG
	log("DEBUGLL: Updating User %s Record for new Chan %s", u->nick, chan);
#endif
	uc = smalloc(sizeof(User_Chans));
	memcpy(uc->chan, chan, sizeof(uc->chan));
	if (DLL_AddRecord(u->chans, uc, NULL) == DLL_MEM_ERROR) {
		log("Error, Couldn't update Users Chan, Out of Memory");
		exit(-1);
	}
	ExitCode = DLL_UpdateCurrentRecord(LL_Users, u);
	if (ExitCode != DLL_NORMAL) {
		log("Error, Couldn't update Users Record for %s", u->nick);
	}	
	if (uc) free(uc);
	return 1;
}

int AddChan(char *name)
{
	Chans *c;
	DLL_Return ExitCode;
#ifdef DEBUG
	log("AddChan(): %s", name);
#endif
	c = findchan(name);
	if (c) {
		log("Trying to add a Channel that already exists? (%s)", name);
		return -1;
	}
	c = smalloc(sizeof(Chans));
	strcpy(c->name, name);
	c->cur_users = 0;
	/* create the list of users on this channel */
	if (DLL_CreateList(&c->chanmembers) == NULL) {
		log("Error, Could not create ChanMembers List for Chan %s", name);
		exit(-1);
	}
	if ((ExitCode = DLL_InitializeList(c->chanmembers, sizeof(Channel_Members))) != DLL_NORMAL) {
		if (ExitCode == DLL_ZERO_INFO) log("Error, Channel Members structure is 0");
		if (ExitCode == DLL_NULL_LIST) log("Error, Channel Members structure is NULL");
		exit(-1);
	}
	if (DLL_AddRecord(LL_Chans, c, NULL) == DLL_MEM_ERROR) {
		log("Error, Couldn't AddChan, Out of Memory");
		exit(-1);
	}
#ifdef DEBUGLL
	log("DEBUGLL: Number of Channel Records %ld", DLL_GetNumberOfRecords(LL_Chans));
	log("DBEUGLL: Current User Indes %ld", DLL_GetCurrentIndex(LL_Chans));
#endif

	return 1;
}
void DelChan(char *name)
{
	Chans *c;
	
	c = findchan(name);
	if (DLL_IsListEmpty(c->chanmembers) != DLL_TRUE) {
		log("Hrm, Deleting a Channel that has %d Users still?", DLL_GetNumberOfRecords(c->chanmembers));
		return;
	}
	DLL_DestroyList(&c->chanmembers);
	if (DLL_DeleteCurrentRecord(LL_Chans) != DLL_NORMAL) {
		log("Problems Deleting Channel Record for %s", c->name);
	}
} 	