/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: users.c,v 1.10 2002/02/27 11:15:16 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"

int fnmatch(const char *, const char *, int flags);

struct Oper_Modes usr_mds[]      = { 
				 {UMODE_OPER, 'o', 50},
                                 {UMODE_LOCOP, 'O', 40},
                                 {UMODE_INVISIBLE, 'i', 0},
                                 {UMODE_WALLOP, 'w', 0},
                                 {UMODE_FAILOP, 'g', 0},
                                 {UMODE_HELPOP, 'h', 30},
                                 {UMODE_SERVNOTICE, 's',0},
                                 {UMODE_KILLS, 'k',0},
                                 {UMODE_SERVICES, 'S',200},
                                 {UMODE_SADMIN, 'a',100},
                                 {UMODE_ADMIN, 'A',70},
                                 {UMODE_NETADMIN, 'N',185},
				 {UMODE_TECHADMIN, 'T',190},
                                 {UMODE_CLIENT, 'c',0},
				 {UMODE_COADMIN, 'C',60},
                                 {UMODE_FLOOD, 'f',0},
                                 {UMODE_REGNICK, 'r',0},
                                 {UMODE_HIDE,    'x',0},
				 {UMODE_EYES,	'e',0},
                                 {UMODE_CHATOP, 'b',0},
				 {UMODE_WHOIS, 'W',0},
				 {UMODE_KIX, 'q',0},
				 {UMODE_BOT, 'B',0},
				 {UMODE_FCLIENT, 'F',0},
   				 {UMODE_HIDING,  'I',0},
/*             			 {UMODE_AGENT,   'Z',200},
				 {UMODE_CODER, '1',200}, */
	   			 {UMODE_DEAF,    'd',0},
                                 {0, 0, 0 }
};





static Server *new_server(char *);
Server *serverlist[S_TABLE_SIZE];
Chans *chanlist[C_TABLE_SIZE];

MyUser *myuhead;

static User *new_user(char *);
static void add_server_to_hash_table(char *, Server *);
static void del_server_from_hash_table(char *, Server *);


hash_t *uh;


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

User *new_user(char *nick)
{
	User *u;
	hnode_t *un;

	/* before we add a new user, we check the table */
	if (hash_verify(uh)) {
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

void AddUser(char *nick, char *user, char *host, char *server)
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

void DelUser(char *nick)
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
	User *u;
	hscan_t us;
	hnode_t *un;

	va_start(ap, message);
	vsnprintf (tmp, 512, message, ap);
	if (!me.coder_debug) 
		return;
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
	va_end (ap);	
}

User *finduser(char *nick)
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



void UserMode(char *nick, char *modes)
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
	while (*modes++) {
	tmpmode = *(modes);
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
	}
#ifdef DEBUG
	log("Modes for %s are now %p", u->nick, u->Umode);
#endif
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

