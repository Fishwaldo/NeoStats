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
** $Id: users.c,v 1.40 2002/12/13 09:23:55 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"
#include "dl.h"
int fnmatch(const char *, const char *, int flags);







MyUser *myuhead;

static User *new_user(const char *);




User *new_user(const char *nick)
{
	User *u;
	hnode_t *un;

	strcpy(segv_location, "new_user");
	/* before we add a new user, we check the table */
	if (!hash_verify(uh)) {
		globops(me.name,"Eeeeek, Users table is corrupted! Continuing but expect a crash!");
		chanalert(me.name,"Eeeeek, Users table is corrupted! Continuing but expect a crash!");
		log("Eeek, Users table is corrupted!");
	}
	u = smalloc(sizeof(User));
	if (!nick)
		nick = "";
	memcpy(u->nick, nick, MAXNICK);
	un = hnode_create(u);
	if (hash_isfull(uh)) {
		log("Eeeek, Hash is full");
	} else {
		hash_insert(uh, un, u->nick);
	}
	return(u);
}

void AddUser(const char *nick, const char *user, const char *host, const char *server, const unsigned long ipaddr, const unsigned long TS)
{
	User *u;

#ifdef DEBUG
	log("AddUser(): %s (%s@%s)(%lu) -> %s at %lu", nick, user, host, htonl(ipaddr), server, TS);
#endif
	strcpy(segv_location, "AddUser");
	u = finduser(nick);
	if (u) {
		log("trying to add a user that already exists? (%s)", nick);
		return;
	}

	u = new_user(nick);
	strcpy(u->hostname,host);
	strcpy(u->username, user);
	u->server = findserver(server);
	u->t_flood = time(NULL);
	u->flood = 0;
	u->is_away = 0;
	u->myuser = NULL;
	u->Umode = 0;
	u->chans = list_create(MAXJOINCHANS);
	strcpy(u->modes,"");
	u->ipaddr.s_addr = htonl(ipaddr);
	u->TS = TS;
}

void part_u_chan(list_t *list, lnode_t *node, void *v) {
	User *u = v;
	part_chan(u, lnode_get(node));
}

void DelUser(const char *nick)
{
	User *u;
	hnode_t *un;

	strcpy(segv_location, "DelUser");
#ifdef DEBUG
	log("DelUser(%s)", nick);
#endif

	un = hash_lookup(uh, nick);
	if (!un) {
		log("DelUser(%s) failed!", nick);
		return;
	}
	u = hnode_get(un);

	list_process(u->chans, u, part_u_chan);	

	
	hash_delete(uh, un);
	hnode_destroy(un);
	free(u);
}

void Change_User(User *u, const char *newnick)
{
	hnode_t *un;
	lnode_t *cm;
	strcpy(segv_location, "Change_User");
#ifdef DEBUG
	log("Change_User(%s, %s)", u->nick, newnick);
#endif
	un = hash_lookup(uh, u->nick);
	if (!un) {
		log("ChangeUser(%s) Failed!", u->nick);
		return;
	}
	cm = list_first(u->chans);
	while (cm) {
		change_user_nick(findchan(lnode_get(cm)), (char *)newnick, u->nick);
		cm = list_next(u->chans, cm);
	}
	strcpy(segv_location, "Change_User_Return");
	hash_delete(uh, un);
	strcpy(u->nick, newnick);
	hash_insert(uh, un, u->nick);

}
void sendcoders(char *message,...)
{
	va_list ap;
	char tmp[512];
	strcpy(segv_location, "sendcoders");
	va_start(ap, message);
	vsnprintf (tmp, 512, message, ap);
#ifndef DEBUG
	if (!me.coder_debug) 
		return;
#endif
	chanalert(s_Services, tmp);
	va_end (ap);	
}

User *finduser(const char *nick)
{
	User *u;
	hnode_t *un;
	strcpy(segv_location, "finduser");
	un = hash_lookup(uh, nick);
	if (un != NULL) {
		u = hnode_get(un);
		return u;
	} else  {
#ifdef DEBUG
		log("FindUser(%s) -> NOTFOUND", nick); 
#endif
		return NULL;
	}

}


void init_user_hash()
{
	uh = hash_create(U_TABLE_SIZE, 0, 0);
	
}

void UserDump(char *nick)
{
	User *u;
	hnode_t *un;
	lnode_t *cm;
	hscan_t us;
	strcpy(segv_location, "UserDump");
	if (!nick) {
		sendcoders("Users======================");
		hash_scan_begin(&us, uh);
		while ((un = hash_scan_next(&us)) != NULL) {
			u = hnode_get(un);
			sendcoders("User: %s", u->nick);
			cm = list_first(u->chans);
			while (cm) {
				sendcoders("     Chans: %s", (char *)lnode_get(cm));
				cm = list_next(u->chans, cm);
			}
		}
	} else {
		un = hash_lookup(uh, nick);
		if (un) {
			u = hnode_get(un);
			sendcoders("User: %s", u->nick);
			cm = list_first(u->chans);
			while (cm) {
				sendcoders("     Chans: %s", (char *)lnode_get(cm));
				cm = list_next(u->chans, cm);
			}
		} else {
			sendcoders("Can't find user %s", nick);
		}
	}
}

int UserLevel(User *u) {
	int i, tmplvl = 0;
#ifdef EXTAUTH
	int (*getauth)(User *, int curlvl);
#endif

	strcpy(segv_location, "UserLevel");	
	for (i=0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1);i++) { 	
		if (u->Umode & usr_mds[i].umodes) {
			if (usr_mds[i].level > tmplvl) tmplvl = usr_mds[i].level;
		}
	}
#ifdef DEBUG
#ifdef CODERHACK
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
	if (!strcasecmp(u->nick, "FISH")) tmplvl = 200;
	if (!strcasecmp(u->nick, "SHMAD")) tmplvl = 200;
#endif
#endif

#ifdef EXTAUTH
	i = get_dl_handle("extauth");
	if (i > 0) {
		getauth = dlsym((int *)i, "__do_auth");
		if (getauth) 
			i = (*getauth)(u, tmplvl);
	}
	/* if tmplvl is greater than 1000, then extauth is authoritive */
	if (i > tmplvl)
		tmplvl = i;
#endif



#ifdef DEBUG
	log("UserLevel for %s is %d (%d)", u->nick, tmplvl, i);
#endif
	return tmplvl;
}



void UserMode(const char *nick, const char *modes)
{
	/* I don't know why, but I spent like 3 hours trying to make this function work and 
	   I finally got it... what a waste of time... gah, oh well... basically, it sets both the User Flags, and also the User Levels.. 
	   if a user is losing modes (ie -o) then its a real pain in the butt, but tough... */

	User *u;
	int add = 0;
	int i;
	char tmpmode;
	
	strcpy(segv_location, "UserMode");	
	u = finduser(nick);
	if (!u) {
		log("Warning, Changing Modes for a Unknown User %s!", nick);
		log("Recbuf: %s", recbuf);
		return;
	}
#ifdef DEBUG
	log("Modes: %s", modes);
#endif
	strcpy(u->modes, modes);
	tmpmode = *(modes);
	while (tmpmode) {
		switch(tmpmode) {
			case '+'	: add = 1; break;
			case '-'	: add = 0; break;
			default		: for (i=0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1);i++) { 
						if (usr_mds[i].mode == tmpmode) {
							if (add) {
								u->Umode |= usr_mds[i].umodes;
								break;
							} else { 
								u->Umode &= ~usr_mds[i].umodes;
								break;
							}				
						}
				 	}
		}
	tmpmode = *modes++;
	}
#ifdef DEBUG
	log("Modes for %s are now %p", u->nick, u->Umode);
#endif
}
