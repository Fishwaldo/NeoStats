/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: users.c,v 1.4 2000/02/23 05:39:24 fishwaldo Exp $
*/
#include <fnmatch.h>
 
#include "stats.h"

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
                                 {UMODE_NETADMIN, 'N',150},
				 {UMODE_TECHADMIN, 'T',190},
                                 {UMODE_CLIENT, 'c',0},
				 {UMODE_COADMIN, 'C',60},
                                 {UMODE_FLOOD, 'f',0},
                                 {UMODE_REGNICK, 'r',10},
                                 {UMODE_HIDE,    'x',0},
				 {UMODE_EYES,	'e',0},
                                 {UMODE_CHATOP, 'b',0},
				 {UMODE_WHOIS, 'W',0},
				 {UMODE_KIX, 'q',0},
				 {UMODE_BOT, 'B',10},
				 {UMODE_FCLIENT, 'F',0},
   				 {UMODE_HIDING,  'I',0},
             			 {UMODE_AGENT,   'Z',200},
				 {UMODE_CODER, '1',200},
	   			 {UMODE_DEAF,    'd',0},
                                 {0, 0, 0 }
};





static Server *new_server(char *);
Server *serverlist[S_TABLE_SIZE];
Chans *chanlist[C_TABLE_SIZE];

MyUser *myuhead;

static User *new_user(char *);
static void add_user_to_hash_table(char *, User *);
static void del_user_from_hash_table(char *, User *);
static void add_server_to_hash_table(char *, Server *);
static void del_server_from_hash_table(char *, Server *);
static void add_chan_to_hash_table(char *, Chans *);
static void del_chan_from_hash_table(char *, Chans *);

User *userlist[U_TABLE_SIZE];



static void add_chan_to_hash_table(char *name, Chans *c)
{
	c->hash = HASH(name, C_TABLE_SIZE);
	c->next = chanlist[c->hash];
	chanlist[c->hash] = (void *)c;
}

static void del_chan_from_hash_table(char *name, Chans *c)
{
	Chans *tmp, *prev = NULL;

	for (tmp = chanlist[c->hash]; tmp; tmp = tmp->next) {
		if (tmp == c) {
			if (prev)
				prev->next = tmp->next;
			else
				chanlist[c->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

void init_chan_hash()
{
	int i;
	Chans *c, *prev;

	for (i = 0; i < C_TABLE_SIZE; i++) {
		c = chanlist[i];
		while (c) {
			prev = c->next;
			free(c);

			c = prev;
		}
		chanlist[i] = NULL;
	}
	bzero((char *)chanlist, sizeof(chanlist));
}
void ChanDump()
{
	Chans *c;
	register int j;

	sendcoders("Channel Listing:");
	for (j = 0; j < C_TABLE_SIZE; j++) {
		for (c = chanlist[j]; c; c = c->next) {
			sendcoders("Channel Entry: %s, Users: %d, Modes: %s, Topic %s",c->name,c->cur_users,c->modes,c->topic);
		}
	}
	sendcoders("End of Listing.");
}

static Chans *new_chan(char *name)
{
	Chans *c;

	c = smalloc(sizeof(Chans));
	if (!name)
		name = "";
	memcpy(c->name, name, CHANLEN);
	add_chan_to_hash_table(name, c);

	return c;
}
void Addchan(char *name)
{
	Chans *c;

#ifdef DEBUG
	log("Addchan(): %s", name);
#endif
	c = findchan(name);
	if (c) {
		log("Increasing Usercount for known Channel (%s)",c->name);
		c->cur_users++;
		return;
	} else {
		c = new_chan(name);
		log("Increasing Usercount for a new Channel (%s)",c->name);
		c->cur_users = 1;
		c->topic = "";
		c->topicowner = "";
		c->modes = "";
		
	}		
}
Chans *findchan(char *name)
{
	Chans *c;


	c = chanlist[HASH(name, C_TABLE_SIZE)];
	while (c && strcasecmp(c->name, name) != 0)
		c = c->next;

#ifdef DEBUG
	log("findchan(%s) -> %s", name, (c) ? c->name : "NOTFOUND");
#endif

	return c;
}
void ChanMode(char *chan, char *modes)
{
	int add = 0;

	Chans *c = findchan(chan);
#ifdef DEBUG
	log("Chanmode(%s): %s", chan,modes);
#endif

	if (!c) {
		log("Chanmode(%s) Failed!", chan);
		return;
	} else {
		switch(*modes) {
			case '+': add = 1;	break;
			case '-': add = 0;	break;
			case 'p':
				if (add) {
					c->is_priv = 1;
				} else {
					c->is_priv = 0;
				}
				break;
			case 's':
				if (add) {
					c->is_secret = 1;
				} else {
					c->is_secret = 0;
				}
				break;
			case 'i':
				if (add) {
					c->is_invite = 1;
				} else {
					c->is_invite = 0;
				}
				break;
			case 'm':
				if (add) {
					c->is_mod = 1;
				} else {
					c->is_mod = 0;
				}
				break;
			case 'n':
				if (add) {
					c->is_outside = 1;
				} else {
					c->is_outside = 0;
				}
				break;
			case 't':
				if (add) {
					c->is_optopic = 1;
				} else {
					c->is_optopic = 0;
				}
				break;
			case 'r':
				if (add) {
					c->is_regchan = 1;
				} else {
					c->is_regchan = 0;
				}
				break;
			case 'R':
				if (add) {
					c->is_regnick = 1;
				} else {
					c->is_regnick = 0;
				}
				break;
			case 'x':
				if (add) {
					c->is_nocolor = 1;
				} else {
					c->is_nocolor = 0;
				}
				break;
			case 'Q':
				if (add) {
					c->is_nokick = 1;
				} else {
					c->is_nokick = 0;
				}
				break;
			case 'O':
				if (add) {
					c->is_ircoponly = 1;
				} else {
					c->is_ircoponly = 0;
				}
				break;
			case 'A':
				if (add) {
					c->is_Svrmode = 1;
				} else {
					c->is_Svrmode = 0;
				}
				break;
			case 'K':
				if (add) {
					c->is_noknock = 1;
				} else {
					c->is_noknock = 0;
				}
				break;
			case 'I':
				if (add) {
					c->is_noinvite = 1;
				} else {
					c->is_noinvite = 0;
				}
				break;
			case 'S':
				if (add) {
					c->is_stripcolor = 1;
				} else {
					c->is_stripcolor = 0;
				}
				break;
			default:
				break;
		}
	}
}
void ChanTopic(char *chan, char *topic, char *who)
{
	Chans *c = findchan(chan);

	if (!c) {
		log("Chantopic(%s) Failed!", chan);
		return;
	} else {
		c->topic = topic;
		c->topicowner = who;
		/* c->topictime = when; */
	}
#ifdef DEBUG
	log("Chantopic(%s): %s Set By: %s", c->name,c->topic,c->topicowner);
#endif
	
}
void DelChan(char *name)
{
	Chans *c = findchan(name);

#ifdef DEBUG
	log("DelChan(%s)", name);
#endif

	if (!c) {
		log("DelChan(%s) failed!", name);
		return;
	}

	if (c->cur_users <= 0) {
		del_chan_from_hash_table(name, c);
		free(c);
	}
}

static void add_user_to_hash_table(char *nick, User *u)
{
	u->hash = HASH(nick, U_TABLE_SIZE);
	u->next = userlist[u->hash];
	userlist[u->hash] = (void *)u;
}

static void del_user_from_hash_table(char *nick, User *u)
{
	User *tmp, *prev = NULL;

	for (tmp = userlist[u->hash]; tmp; tmp = tmp->next) {
		if (tmp == u) {
			if (prev)
				prev->next = tmp->next;
			else
				userlist[u->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

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

static User *new_user(char *nick)
{
	User *u;

	u = smalloc(sizeof(User));
	if (!nick)
		nick = "";
	memcpy(u->nick, nick, MAXNICK);
	add_user_to_hash_table(nick, u);

	return u;
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
	u->hostname = sstrdup(host);
	u->username = sstrdup(user);
	u->server = findserver(server);
	u->t_flood = time(NULL);
	u->flood = 0;
	u->is_away = 0;
	u->myuser = NULL;
	u->Umode = 0;
}

void DelUser(char *nick)
{
	User *u = finduser(nick);

#ifdef DEBUG
	log("DelUser(%s)", nick);
#endif

	if (!u) {
		log("DelUser(%s) failed!", nick);
		return;
	}

	del_user_from_hash_table(nick, u);

		if (u->myuser)
		u->myuser->ison = 0;

	free(u->hostname);
	free(u->username);
	free(u);
}

void Change_User(User *u, char *newnick)
{
#ifdef DEBUG
	log("Change_User(%s, %s)", u->nick, newnick);
#endif

	del_user_from_hash_table(u->nick, u);

	u->nick[1] = 0;
	memcpy(u->nick, newnick, MAXNICK);

	add_user_to_hash_table(u->nick, u);
}
void sendcoders(char *message,...)
{
	va_list ap;
	char tmp[512];
	User *u;
        int i;

	va_start(ap, message);
	vsnprintf (tmp, 512, message, ap);
	if (!me.coder_debug) 
		return;
	if (!me.usesmo) {
	        for (i = 0; i < U_TABLE_SIZE; i++) {
        		for (u = userlist[i]; u; u = u->next)
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

	u = userlist[HASH(nick, U_TABLE_SIZE)];
	while (u && (strcasecmp(u->nick, nick) != 0))
			u = u->next;
#ifdef DEBUG
	log("finduser(%s) -> %s", nick, (u) ? u->nick : "NOTFOUND");
#endif

	return u;
}

void init_user_hash()
{
	int i;
	User *u, *prev;

	for (i = 0; i < U_TABLE_SIZE; i++) {
		u = userlist[i];
		while (u) {
			prev = u->next;
			free(u->hostname);
			free(u->username);
			free(u);

			u = prev;
		}
		userlist[i] = NULL;
	}
	bzero((char *)userlist, sizeof(userlist));
}

void UserDump()
{
	User *u;
	int i;
	for (i = 0; i < U_TABLE_SIZE; i++) {
		for (u = userlist[i]; u; u = u->next)
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
#endif

	log("UserLevel for %s is %d", u->nick, tmplvl);
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

