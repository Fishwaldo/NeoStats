/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: chans.c,v 1.6 2002/03/08 14:18:08 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "hash.h"

#include "UnrealModes.h"

void ChangeChanUserMode(Chans *c, User *u, int add, long mode);

void init_chan_hash()
{
	ch = hash_create(C_TABLE_SIZE, 0, 0);
	if (usr_mds);	

	
}

void ChanMode(char *origin, char **av, int ac) {
	char *modes;
	int add = 0;
	int j = 2; 
	int i;
	Chans *c;

	c = findchan(av[0]);
	if (!c) {
		return;
	}
	modes = av[1];
	while (*modes) {
		switch (*modes) {
			case '+'	: add = 1; break;
			case '-'	: add = 0; break;
			default		: for (i=0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) -1);i++) {

						if (*modes == cFlagTab[i].flag) {
							if (add) {
								if (cFlagTab[i].nickparam) {
									ChangeChanUserMode(c, finduser(av[j]), 1, cFlagTab[i].mode);
									j++;
								} else {	
									c->modes |= cFlagTab[i].mode;
									if (cFlagTab[i].parameters) {
										printf("Mode Param: %s\n", av[j]);
										j++;
									}
								}
							} else {
								if (cFlagTab[i].nickparam) {
									ChangeChanUserMode(c, finduser(av[j]), 0, cFlagTab[i].mode);
									j++;
								} else {	
									c->modes &= ~cFlagTab[i].mode;
									if (cFlagTab[i].parameters) {
										printf("removeparam\n");
									}
								}
							}
						}
					}
		}
	modes++;
	}
		

}

void ChangeChanUserMode(Chans *c, User *u, int add, long mode) {
	hnode_t *cmn;
	Chanmem *cm;
	cmn = hash_lookup(c->chanmembers, u->nick);
	if (!cmn) {
		log("ChangeChanUserMode() %s doesn't seem to be in the Chan %s", u->nick, c->name);
		return;
	}
	cm = hnode_get(cmn);
	if (add) {
#ifdef DEBUG
		log("Adding mode %ld to Channel %s User %s\n", mode, c->name, u->nick);
#endif
		cm->flags |= mode;
	} else {
#ifdef DEBUG
		log("Deleting Mode %ld to Channel %s User %s\n", mode, c->name, u->nick);
#endif
		cm->flags &= ~mode;
	}
}
Chans *new_chan(char *chan) {
	Chans *c;
	hnode_t *cn;

	strcpy(segv_location, "new_chan");
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
	strcpy(segv_location, "del_chan");
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
	strcpy(segv_location, "part_chan");
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
	strcpy(segv_location, "change_user_nick");
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
	strcpy(segv_location, "join_chan");
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
	char mode[10];
	int i;

	strcpy(segv_location, "chandump");
	if (!chan) {
		sendcoders("Channels %d", hash_count(ch));
		hash_scan_begin(&sc, ch);
		while ((cn = hash_scan_next(&sc)) != NULL) {
			c = hnode_get(cn);
			sendcoders("====================");
			strcpy(mode, "+");
			for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
				if (c->modes & cFlagTab[i].mode) {
					sprintf(mode, "%s%c", mode, cFlagTab[i].flag);
				}
			}
			sendcoders("Channel: %s Members: %d (Hash %d) Flags %s", c->name, c->cur_users, hash_count(c->chanmembers), mode);
			hash_scan_begin(&scm, c->chanmembers);
			while ((cmn = hash_scan_next(&scm)) != NULL) {
				cm = hnode_get(cmn);
				strcpy(mode, "+");
				for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
					if (cm->flags & cFlagTab[i].mode) {
						sprintf(mode, "%s%c", mode, cFlagTab[i].flag);
					}
				}
				sendcoders("Members: %s Modes %s Joined %d", cm->u->nick, mode, cm->joint);
			}
		}
	} else {
		c = findchan(chan);
		if (!c) {
			sendcoders("Can't find Channel %s", chan);
		} else {
			strcpy(mode, "+");
			for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
				if (c->modes & cFlagTab[i].mode) {
					sprintf(mode, "%s%c", mode, cFlagTab[i].flag);
				}
			}
			sendcoders("Channel: %s Members: %d (Hash %d) Flags %s", c->name, c->cur_users, hash_count(c->chanmembers), mode);
			hash_scan_begin(&scm, c->chanmembers);
			while ((cmn = hash_scan_next(&scm)) != NULL) {
				cm = hnode_get(cmn);
				strcpy(mode, "+");
				for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
					if (cm->flags & cFlagTab[i].mode) {
						sprintf(mode, "%s%c", mode, cFlagTab[i].flag);
					}
				}
				sendcoders("Members: %s Modes %s Joined: %d", cm->u->nick, mode, cm->joint);
			}
		}
	}
}

Chans *findchan(char *chan) {
	Chans *c;
	hnode_t *cn;
	strcpy(segv_location, "findchan");
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
