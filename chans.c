/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: chans.c,v 1.4 2002/03/08 09:01:00 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"


void init_chan_hash()
{
	ch = hash_create(C_TABLE_SIZE, 0, 0);
	if (usr_mds);	

	
}

Chans *new_chan(char *chan) {
	Chans *c;
	hnode_t *cn;

	c = malloc(sizeof(Chans));
	strcpy(c->name, chan);
	cn = hnode_create(c);
	if (hash_isfull(ch)) {
		log("Eeek, Channel Hash is full");
	} else {
		hash_insert(ch, cn, c->name);
	}
	return c;	
}
void del_chan(Chans *c) {
	hnode_t *cn;
	cn = hash_lookup(ch, c->name);
	if (!cn) {
		log("Hu, Deleting a Non Existand Channel?");
		return;
	} else {
		hash_delete(ch, cn);
		hnode_destroy(cn);
		free(c);
	}
}


void part_chan(User *u, char *chan) {
	Chans *c;
	hnode_t *un;

	if (!u) {
		log("Ehh, Parting a Unknown User from Chan %s: %s", chan, recbuf);
		return;
	}
	c = findchan(chan);
	if (!c) {
		log("Hu, Parting a Non existant Channel?");
		return;
	} else {
		un = hash_lookup(c->chanmembers, u->nick);
		if (!un) {
			log("hu, User %s isn't a member of this channel %s", u->nick, chan);
		} else {
			hash_delete(c->chanmembers, un);
			c->cur_users--;
		}
		if (c->cur_users <= 0) {
			del_chan(c);
		}
		un = hash_lookup(u->chans, chan);
		if (!un) {
			log("Hu, User %s claims not to be part of Chan %s", u->nick, chan);
			return;
		}
		hash_delete(u->chans, un);
	}
}			

void change_user_nick(Chans *c, char *newnick, char *oldnick) {
	hnode_t *cm;

	cm = hash_lookup(c->chanmembers, oldnick);
	if (!cm) {
		log("change_user_nick() %s isn't a member of %s", oldnick, c->name);
		return;
	} else {
		hash_delete(c->chanmembers, cm);
		hash_insert(c->chanmembers, cm, newnick);
	}		
}




void join_chan(User *u, char *chan) {
	Chans *c;
	hnode_t *un;
	Chanmem *cm;

	if (!u) {
		log("ehhh, Joining a Unknown user to %s: %s", chan, recbuf);
		return;
	} 
		
	c = findchan(chan);
	if (!c) {
		/* its a new Channel */
#ifdef DEBUG
		log("join_chan() -> New Channel %s", chan);
#endif
		c = new_chan(chan);
		c->chanmembers = hash_create(CHAN_MEM_SIZE, 0, 0);
	} 
	/* add this users details to the channel members hash */	
	cm = malloc(sizeof(Chanmem));
	cm->u = u;
	cm->joint = time(NULL);
	cm->flags = 0;
	un = hnode_create(cm);	
#ifdef DEBUG
	log("adding usernode %s to Channel %s", u->nick, chan);
#endif
	if (hash_lookup(c->chanmembers, u->nick)) {
		log("Adding %s to Chan %s when he is already a member?", u->nick, chan);
		return;
	}
	hash_insert(c->chanmembers, un, u->nick);
	c->cur_users++;
	un = hnode_create(c->name);
	hash_insert(u->chans, un, c->name); 
}

void chandump(User *u, char *chan) {
	hnode_t *cn, *cmn;
	hscan_t sc, scm;
	Chans *c;
	Chanmem *cm;

	if (!chan) {
		sendcoders("Channels %d", hash_count(ch));
		hash_scan_begin(&sc, ch);
		while ((cn = hash_scan_next(&sc)) != NULL) {
			c = hnode_get(cn);
			sendcoders("====================");
			sendcoders("Channel: %s Members: %d (Hash %d)", c->name, c->cur_users, hash_count(c->chanmembers));
			hash_scan_begin(&scm, c->chanmembers);
			while ((cmn = hash_scan_next(&scm)) != NULL) {
				cm = hnode_get(cmn);
				sendcoders("Members: %s Joined %d", cm->u->nick, cm->joint);
			}
		}
	} else {
		c = findchan(chan);
		if (!c) {
			sendcoders("Can't find Channel %s", chan);
		} else {
			sendcoders("Channel: %s Members: %d (Hash %d)", c->name, c->cur_users, hash_count(c->chanmembers));
			hash_scan_begin(&scm, c->chanmembers);
			while ((cmn = hash_scan_next(&scm)) != NULL) {
				cm = hnode_get(cmn);
				sendcoders("Members: %s %d", cm->u->nick, cm->joint);
			}
		}
	}
}

Chans *findchan(char *chan) {
	Chans *c;
	hnode_t *cn;

	cn = hash_lookup(ch, chan);
	if (cn) {
		c = hnode_get(cn);
		return c;
	} else {
#ifdef DEBUG
		log("FindChan(%s) -> Not Found", chan);
#endif
		return NULL;
	}
}
