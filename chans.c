/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
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
** $Id: chans.c,v 1.37 2002/12/26 15:15:04 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "stats.h"
#include "dl.h"
#include "hash.h"

/** @brief initilize the channel data
 *
 * Initilizes the channel data and channel hash ch.
 *
 * @return Nothing
*/


void init_chan_hash()
{
	ch = hash_create(C_TABLE_SIZE, 0, 0);
	
}
/** @brief Process a Topic Change
 *
 * Processes a Channel topic Change for particular channel and updates internals
 * Also triggers off a TOPICCHANGE event for modules
 *
 * @param owner Char of who changed the topic. Can't be a userstruct as the user might not be online anymore
 * @param c Channel Struct of channel who's topic is being changed
 * @param time when the topic was changed (might have been in the past 
 * @param topic the new topic
 *
 * @return Nothing
*/

extern void Change_Topic(char *owner, Chans *c, time_t time, char *topic) {
	char **av;
	int ac = 0;
	strncpy(c->topic, topic, BUFSIZE);
	strncpy(c->topicowner, owner, BUFSIZE);
	c->topictime = time;
	AddStringToList(&av, c->name, &ac);
	AddStringToList(&av, owner, &ac);
	AddStringToList(&av, topic, &ac);
	Module_Event("TOPICCHANGE",av, ac);
	FreeList(av, ac);
}
/** @brief Compare channel modes from the channel hash
 *
 * used in ChanMode to compare modes (list_find argument)
 *
 * @param v actually its a ModeParm struct
 * @param mode is the mode as a long
 *
 * @return 0 on match, 1 otherwise.
*/

int comparemode(const void *v, const void *mode) {
	ModesParm *m = (void *)v;
	if (m->mode == (long)mode) {
		return 0;
	} else {
		return 1;
	}
}

/** @brief Process a mode change on a channel
 *
 * process a mode change on a channel adding and deleting modes as required
 *
 * @param origin usually the server that sent the mode change. Not used
 * @param av array of variables to pass
 * @param ac number of variables n av
 *
 * @return 0 on error, number of modes processed on success.
*/

int ChanMode(char *origin, char **av, int ac) {
	char *modes;
	int add = 0;
	int j = 2; 
	int i;
	int modeexists;
	Chans *c;
	ModesParm *m;
	lnode_t *mn;
	c = findchan(av[0]);
	if (!c) {
		return 0;
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
									if (cFlagTab[i].parameters) {
										mn = list_first(c->modeparms);
										modeexists = 0;
										while (mn) {
											m = lnode_get(mn);
											/* mode limit and mode key replace current values */
											if ((m->mode == MODE_LIMIT) && (cFlagTab[i].mode == MODE_LIMIT)) {
												strcpy(m->param, av[j]);
												j++;
												modeexists = 1;
												break;
											} else if ((m->mode == MODE_KEY) && (cFlagTab[i].mode == MODE_KEY)) {
												strcpy(m->param, av[j]);
												j++;
												modeexists = 1;
												break;
											} else if (((int *)m->mode == (int *)cFlagTab[i].mode) && !strcasecmp(m->param, av[j])) {
#ifdef DEBUG
						 						log("Mode %c (%s) already exists, not adding again", cFlagTab[i].flag, av[j]);
#endif
												j++;
												modeexists = 1;
												break;
											}
											mn = list_next(c->modeparms, mn);
										}
										if (modeexists != 1) {
											m = smalloc(sizeof(ModesParm));
											m->mode = cFlagTab[i].mode;
											strcpy(m->param, av[j]);
											mn = lnode_create(m);
											if (list_isfull(c->modeparms)) {
												log("Eeek, Can't add additional Modes to Channel %s. Modelist is full", c->name);
												assert(0);
											} else {
												list_append(c->modeparms, mn);
											}
											j++;
										}
									} else {
										c->modes |= cFlagTab[i].mode;
									}
								}
							} else {
								if (cFlagTab[i].nickparam) {
									ChangeChanUserMode(c, finduser(av[j]), 0, cFlagTab[i].mode);
									j++;
								} else {	
									if (cFlagTab[i].parameters) {
										mn = list_find(c->modeparms, (int *)cFlagTab[i].mode, comparemode);
										if (!mn) {
#ifdef DEBUG
											log("Can't find Mode %c for Chan %s", *modes, c->name);
#endif
										} else {
											list_delete(c->modeparms, mn);
											m = lnode_get(mn);
											lnode_destroy(mn);
											free(m);
											if (!(cFlagTab[i].mode == MODE_LIMIT || cFlagTab[i].mode == MODE_KEY)) 
												j++;
										}
									} else {
										c->modes &= ~cFlagTab[i].mode;
									}
								}
							}
						}
					}
		}
	modes++;
	}
return j;		

}

/** @brief Process a mode change that affects a user on a channel
 *
 * process a mode change on a channel that affects a user
 *
 * @param c Channel Struct of channel mode being changed
 * @param u User struct of user that mode is affecting
 * @param add 1 means add, 0 means remove mode
 * @param mode is the long int of the mode
 *
 * @return Nothing
*/

void ChangeChanUserMode(Chans *c, User *u, int add, long mode) {
	lnode_t *cmn;
	Chanmem *cm;
	cmn = list_find(c->chanmembers, u->nick, comparef);
	if (!cmn) {
		if (me.coder_debug) {
			chanalert(s_Services, "ChangeChanUserMode() %s doesn't seem to be in the Chan %s", u->nick, c->name);
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

/** @brief Create a new channel record
 *
 * And insert it into the hash, mallocing required memory for it and so on
 * also check that the channel hash is not full
 *
 * @param chan name of the channel to create
 *
 * @returns c the newly created channel record
 * @todo Dynamically resizable channel hashes
*/
Chans *new_chan(char *chan) {
	Chans *c;
	hnode_t *cn;

	strcpy(segv_location, "new_chan");
	c = smalloc(sizeof(Chans));
	strcpy(c->name, chan);
	cn = hnode_create(c);
	if (hash_isfull(ch)) {
		log("Eeek, Channel Hash is full");
	} else {
		hash_insert(ch, cn, c->name);
	}
	return c;	
}

/** @brief Deletes a channel record
 *
 * frees any memory associated with the record and removes it from the channel hash
 *
 * @param c the corrosponding channel structure you wish to delete
 *
 * @returns Nothing
*/

void del_chan(Chans *c) {
	hnode_t *cn;
	lnode_t *cm;
	
	strcpy(segv_location, "del_chan");
	cn = hash_lookup(ch, c->name);
	if (!cn) {
		log("Hu, Deleting a Non Existand Channel?");
		return;
	} else {
#ifdef DEBUG
		log("Deleting Channel %s", c->name);
#endif
		cm = list_first(c->modeparms);
		while (cm) {
			free(lnode_get(cm));
			cm = list_next(c->modeparms, cm);
		}
		list_destroy_nodes(c->modeparms);
		list_destroy(c->modeparms);		
		hash_delete(ch, cn);
		hnode_destroy(cn);
		free(c);
	}
}

/** @brief Parts a user from a channel
 *
 * Parts a user from a channel and raises events if required
 * Events raised are PARTCHAN and DELCHAN
 * if its one of our bots, also update bot channel lists
 *
 * @param u the User structure corrosponding to the user that left the channel
 * @param chan the channel to part them from
 *
 * @returns Nothing
*/


void part_chan(User *u, char *chan) {
	Chans *c;
	lnode_t *un;
	char **av;
	int ac = 0;
	strcpy(segv_location, "part_chan");
#ifdef DEBUG
	log("Parting %s from %s", u->nick, chan);
#endif
	if (!u) {
		log("Ehh, Parting a Unknown User %s from Chan %s: %s", u->nick, chan, recbuf);
		if (me.coder_debug) {
			chanalert(s_Services, "Ehh, Parting a Unknown User %s from Chan %s: %s", u->nick, chan, recbuf);
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
				chanalert(s_Services, "hu, User %s isn't a member of this channel %s", u->nick, chan);
				chandump(c->name);
				UserDump(u->nick);
			}
		} else {
			lnode_destroy(list_delete(c->chanmembers, un));
			AddStringToList(&av, c->name, &ac);
			AddStringToList(&av, u->nick, &ac);
			Module_Event("PARTCHAN", av, ac);
			FreeList(av, ac);
			c->cur_users--;
		}
#ifdef DEBUG
		log("Cur Users %s %d (list %d)", c->name, c->cur_users, list_count(c->chanmembers));
#endif
		if (c->cur_users <= 0) {
			AddStringToList(&av, c->name, &ac);
			Module_Event("DELCHAN", av, ac);
			FreeList(av, ac);
			del_chan(c);
		}
		un = list_find(u->chans, c->name, comparef);
		if (!un) {
			log("Hu, User %s claims not to be part of Chan %s", u->nick, chan);
			if (me.coder_debug) {
				chanalert(s_Services, "Hu, User %s claims not to be part of Chan %s", u->nick, chan);
				chandump(c->name);
				UserDump(u->nick);
			}
			return;
		}
		if (findbot(u->nick)) {
			/* its one of our bots, so add it to the botchan list */
			del_bot_from_chan(u->nick, c->name);
		}
		lnode_destroy(list_delete(u->chans, un));
	}
}			

/** @brief Change channel records when a nick change occurs
 *
 * goes through the channel members list, changing users nicks after a nickname change occurs
 *
 * @param c the channel to check (as called by the user functions)
 * @param newnick the New Nickname of the client
 * @param oldnick the old nickname of the client
 *
 * @returns Nothing
 * @todo What happens if one of our bots change their nick?
*/

void change_user_nick(Chans *c, char *newnick, char *oldnick) {
	lnode_t *cm;
	Chanmem *cml;
	strcpy(segv_location, "change_user_nick");
	cm = list_find(c->chanmembers, oldnick, comparef);
	if (!cm) {
		log("change_user_nick() %s isn't a member of %s", oldnick, c->name);
		if (me.coder_debug) {
			chanalert(s_Services, "change_user_nick() %s isn't a member of %s", oldnick, c->name);
			chandump(c->name);
			UserDump(oldnick);
		}
		return;
	} else {
#ifdef DEBUG
		log("Change_User_Nick(): NewNick %s, OldNick %s", newnick, oldnick);
#endif
		cml = lnode_get(cm);
		strcpy(cml->nick, newnick);
	}		
}



/** @brief Process a user joining a channel
 *
 * joins a user to a channel and raises JOINCHAN event and if required NEWCHAN events
 * if the channel is new, a new channel record is requested and defaults are set
 * if its one of our bots, also update the botchanlist
 *
 * @param u The User structure of the user joining the channel
 * @param chan the channel name
 *
 * @returns Nothing
*/


void join_chan(User *u, char *chan) {
	Chans *c;
	lnode_t *un, *cn;
	Chanmem *cm;
	char **av;
	int ac = 0;
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
		c->modeparms = list_create(MAXMODES);
		c->cur_users =0;
		c->topictime = 0;
		c->modes = 0;
		AddStringToList(&av, c->name, &ac);
		Module_Event("NEWCHAN", av, ac);
		FreeList(av, ac);
	}
	/* add this users details to the channel members hash */	
	cm = smalloc(sizeof(Chanmem));
	strcpy(cm->nick, u->nick);
	cm->joint = time(NULL);
	cm->flags = 0;
	cn = lnode_create(cm);	
#ifdef DEBUG
	log("adding usernode %s to Channel %s", u->nick, chan);
#endif
	if (list_find(c->chanmembers, u->nick, comparef)) {
		log("Adding %s to Chan %s when he is already a member?", u->nick, chan);
		if (me.coder_debug) {
			chanalert(s_Services, "Adding %s to Chan %s when he is already a member?", u->nick, chan);
			chandump(c->name);
			UserDump(u->nick);
		}
		return;
	}
	if (list_isfull(c->chanmembers)) {
		log("ekk, Channel %s Members list is full", c->name);
	} else {
		list_append(c->chanmembers, cn);
	}
	c->cur_users++;
	un = lnode_create(c->name);
	if (list_isfull(u->chans)) {
		log("eek, User %s members list is full", u->nick);
	} else {
		list_append(u->chans, un); 
	}
	AddStringToList(&av, c->name, &ac);
	AddStringToList(&av, u->nick, &ac);
	Module_Event("JOINCHAN", av, ac);
	FreeList(av, ac);
#ifdef DEBUG
	log("Cur Users %s %d (list %d)", c->name, c->cur_users, list_count(c->chanmembers));
#endif
	if (findbot(u->nick)) {
		add_bot_to_chan(u->nick, c->name);
	}
}


/** @brief Dump Channel information
 *
 * dump either the entire channel list, or a single channel detail. Used for debugging
 * sends the output to the services channel
 *
 * @param chan the channel name to dump, or NULL for all channels
 *
 * @returns Nothing
*/


void chandump(char *chan) {
	hnode_t *cn;
	lnode_t *cmn;
	hscan_t sc;
	Chans *c;
	Chanmem *cm;
	char mode[10];
	int i;
	ModesParm *m;

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
			sendcoders("       Topic Owner %s, TopicTime: %d, Topic %s", c->topicowner, c->topictime, c->topic);
			cmn = list_first(c->modeparms);
			while (cmn) {
				m = lnode_get(cmn);
				for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
					if (m->mode & cFlagTab[i].mode) {
						sendcoders("        Modes: %c Parms %s", cFlagTab[i].flag, m->param);
					}
				}

				cmn = list_next(c->modeparms, cmn);
			}
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
			sendcoders("       Topic Owner %s, TopicTime: %d Topic %s", c->topicowner, c->topictime, c->topic);
			cmn = list_first(c->modeparms);
			while (cmn) {
				m = lnode_get(cmn);
				for (i = 0; i < ((sizeof(cFlagTab) / sizeof(cFlagTab[0])) - 1); i++) {
					if (m->mode & cFlagTab[i].mode) {
						sendcoders("        Modes: %c Parms %s", cFlagTab[i].flag, m->param);
					}
				}
				cmn = list_next(c->modeparms, cmn);
			}
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


/** @brief Find a channel
 *
 * Finds the channel structure for the channel named chan, or NULL if it can't be found
 *
 * @param chan the channel name to find
 *
 * @returns The Channel structure for chan, or NULL if it can't be found.
*/


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
