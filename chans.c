/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: chans.c,v 1.8 2002/03/11 08:02:40 fishwaldo Exp $
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
	lnode_t *cmn;
	Chanmem *cm;
	cmn = list_find(c->chanmembers, u->nick, comparef);
	if (!cmn) {
		if (me.coder_debug) {
			notice(s_Services, "ChangeChanUserMode() %s doesn't seem to be in the Chan %s", u->nick, c->name);
			chandump(c->name);
			UserDump(u->nick);
		}
		return;
	}
	cm = lnode_get(cmn);
	if (add) {
#ifdef DEBUG
		log("Adding mode %ld to Channel %s User %s", mode, c->name, u->nick);
#endif
		cm->flags |= mode;
	} else {
#ifdef DEBUG
		log("Deleting Mode %ld to Channel %s User %s", mode, c->name, u->nick);
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
	lnode_t *un;
	strcpy(segv_location, "part_chan");
#ifdef DEBUG
	log("Parting %s from %s", u->nick, chan);
#endif
	if (!u) {
		log("Ehh, Parting a Unknown User %s from Chan %s: %s", u->nick, chan, recbuf);
		if (me.coder_debug) {
			notice(s_Services, "Ehh, Parting a Unknown User %s from Chan %s: %s", u->nick, chan, recbuf);
			chandump(chan);
			UserDump(u->nick);
		}
		return;
	}
	c = findchan(chan);
	if (!c) {
		log("Hu, Parting a Non existant Channel? %s", chan);
		return;
	} else {
		un = list_find(c->chanmembers, u->nick, comparef);
		if (!un) {
			log("hu, User %s isn't a member of this channel %s", u->nick, chan);
			if (me.coder_debug) {
				notice(s_Services, "hu, User %s isn't a member of this channel %s", u->nick, chan);
				chandump(c->name);
				UserDump(u->nick);
			}
		} else {
			lnode_destroy(list_delete(c->chanmembers, un));
			c->cur_users--;
		}
		if (c->cur_users <= 0) {
			del_chan(c);
		}
		un = list_find(u->chans, c->name, comparef);
		if (!un) {
			log("Hu, User %s claims not to be part of Chan %s", u->nick, chan);
			if (me.coder_debug) {
				notice(s_Services, "Hu, User %s claims not to be part of Chan %s", u->nick, chan);
				chandump(c->name);
				UserDump(u->nick);
			}
			return;
		}
		lnode_destroy(list_delete(u->chans, un));
	}
}			

void change_user_nick(Chans *c, char *newnick, char *oldnick) {
	lnode_t *cm;
	strcpy(segv_location, "change_user_nick");
	cm = list_find(c->chanmembers, oldnick, comparef);
	if (!cm) {
		log("change_user_nick() %s isn't a member of %s", oldnick, c->name);
		if (me.coder_debug) {
			notice(s_Services, "change_user_nick() %s isn't a member of %s", oldnick, c->name);
			chandump(c->name);
			UserDump(oldnick);
		}
		return;
	} else {
		lnode_destroy(list_delete(c->chanmembers, cm));
		list_append(c->chanmembers, lnode_create(newnick));
	}		
}




void join_chan(User *u, char *chan) {
	Chans *c;
	lnode_t *un;
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
		c->chanmembers = list_create(CHAN_MEM_SIZE);
	} 
	/* add this users details to the channel members hash */	
	cm = malloc(sizeof(Chanmem));
	strcpy(cm->nick, u->nick);
	cm->joint = time(NULL);
	cm->flags = 0;
	un = lnode_create(cm);	
#ifdef DEBUG
	log("adding usernode %s to Channel %s", u->nick, chan);
#endif
	if (list_find(c->chanmembers, u->nick, comparef)) {
		log("Adding %s to Chan %s when he is already a member?", u->nick, chan);
		if (me.coder_debug) {
			notice(s_Services, "Adding %s to Chan %s when he is already a member?", u->nick, chan);
			chandump(c->name);
			UserDump(u->nick);
		}
		return;
	}
	if (list_isfull(c->chanmembers)) {
		log("ekk, Channel %s Members list is full", c->name);
	} else {
		list_append(c->chanmembers, un);
	}
	c->cur_users++;
	un = lnode_create(c->name);
	if (list_isfull(c->chanmembers)) {
		log("eek, User %s members list is full", u->nick);
	} else {
		list_append(u->chans, un); 
	}
}

void chandump(char *chan) {
	hnode_t *cn;
	lnode_t *cmn;
	hscan_t sc;
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
			sendcoders("Channel: %s Members: %d (List %d) Flags %s", c->name, c->cur_users, list_count(c->chanmembers), mode);
			cmn = list_first(c->chanmembers);
			while (cmn) {
				cm = lnode_get(cmn);
				strcpy(mode, "+");
				for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
					if (cm->flags & cFlagTab[i].mode) {
						sprintf(mode, "%s%c", mode, cFlagTab[i].flag);
					}
				}
				sendcoders("Members: %s Modes %s Joined %d", cm->nick, mode, cm->joint);
				cmn = list_next(c->chanmembers, cmn);
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
			sendcoders("Channel: %s Members: %d (List %d) Flags %s", c->name, c->cur_users, list_count(c->chanmembers), mode);
			cmn = list_first(c->chanmembers);
			while (cmn) {
				cm = lnode_get(cmn);
				strcpy(mode, "+");
				for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
					if (cm->flags & cFlagTab[i].mode) {
						sprintf(mode, "%s%c", mode, cFlagTab[i].flag);
					}
				}
				sendcoders("Members: %s Modes %s Joined: %d", cm->nick, mode, cm->joint);
				cmn = list_next(c->chanmembers, cmn);
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
