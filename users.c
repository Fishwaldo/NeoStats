/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: users.c,v 1.16 2002/03/05 12:59:58 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"

int fnmatch(const char *, const char *, int flags);







MyUser *myuhead;

static User *new_user(const char *);


hash_t *uh;


User *new_user(const char *nick)
{
	User *u;
	hnode_t *un;

	/* before we add a new user, we check the table */
	if (!hash_verify(uh)) {
		globops(me.name,"Eeeeek, Users table is corrupted! Continuing but expect a crash!");
		notice(me.name,"Eeeeek, Users table is corrupted! Continuing but expect a crash!");
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

void AddUser(const char *nick, const char *user, const char *host, const char *server)
{
	User *u;

#ifdef DEBUG
	log("AddUser(): %s (%s@%s) -> %s", nick, user, host, server);
#endif
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
	strcpy(u->modes,"");

}

void DelUser(const char *nick)
{
	User *u;
	hnode_t *un;

#ifdef DEBUG
	log("DelUser(%s)", nick);
#endif

	un = hash_lookup(uh, nick);
	if (!un) {
		log("DelUser(%s) failed!", nick);
		return;
	}
	hash_delete(uh, un);
	u = hnode_get(un);
	hnode_destroy(un);
	free(u);
}

void Change_User(User *u, char *newnick)
{
	hnode_t *un;
#ifdef DEBUG
	log("Change_User(%s, %s)", u->nick, newnick);
#endif

	DelUser(u->nick);
	u->nick[1] = '\0';
	memcpy(u->nick, newnick, MAXNICK);
	un = hnode_create(u);
	hash_insert(uh, un, u->nick);
}
void sendcoders(char *message,...)
{
	va_list ap;
	char tmp[512];
#ifdef UNREAL
	User *u;
	hscan_t us;
	hnode_t *un;
#endif
	va_start(ap, message);
	vsnprintf (tmp, 512, message, ap);
	if (!me.coder_debug) 
		return;
#ifdef UNREAL
	if (!me.usesmo) {
		hash_scan_begin(&us, uh);
		while ((un = hash_scan_next(&us)) != NULL) {
			u = hnode_get(un);
				if (u->Umode & UMODE_CODER)	
				privmsg(u->nick, s_Debug, "Debug: %s",tmp);
		}
	} else {		
		sts(":%s SMO 1 :%s Debuging: %s ",me.name,s_Services, tmp);
	}
#elif ULTIMATE
	notice(s_Services, "Debuging: %s", tmp);
#endif
	va_end (ap);	
}

User *finduser(const char *nick)
{
	User *u;
	hnode_t *un;
		
	un = hash_lookup(uh, nick);
	if (un != NULL) {
		u = hnode_get(un);
		return u;
	} else  {
		log("FindUser(%s) -> NOTFOUND", nick); 
		return NULL;
	}

}


void init_user_hash()
{
	uh = hash_create(U_TABLE_SIZE, 0, 0);
	
}

void UserDump()
{
	User *u;
	hnode_t *un;
	hscan_t us;
	hash_scan_begin(&us, uh);
	while ((un = hash_scan_next(&us)) != NULL) {
		u = hnode_get(un);
		sendcoders("User: %s", u->nick);
	}
}

int UserLevel(User *u) {
	int i, tmplvl = 0;
	
	for (i=0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) -1);i++) { 	
		if (u->Umode & usr_mds[i].umodes) {
			if (usr_mds[i].level > tmplvl) tmplvl = usr_mds[i].level;
		}
	}
#ifdef DEBUG
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
/*	if (!strcasecmp(u->nick, "FISH")) tmplvl = 200; */
	log("UserLevel for %s is %d", u->nick, tmplvl);
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
	
	u = finduser(nick);
	if (!u) {
		log("Warning, Changing Modes for a Unknown User %s!", nick);
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
