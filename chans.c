/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: chans.c,v 1.1 2000/06/10 09:14:03 fishwaldo Exp $
*/

#include <fnmatch.h>
 
#include "linklist.h"
#include "stats.h"


Chans *chanlist[C_TABLE_SIZE];
static void add_chan_to_hash_table(char *, Chans *);
static void del_chan_from_hash_table(char *, Chans *);


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
#ifdef DEBUG
		log("Increasing Usercount for known Channel (%s)",c->name);
#endif
		c->cur_users++;
		return;
	} else {
		c = new_chan(name);
#ifdef DEBUG
		log("Increasing Usercount for a new Channel (%s)",c->name);
#endif
		c->cur_users = 1;
		strcpy(c->topic,"");
		strcpy(c->topicowner,"");
		strcpy(c->modes, "");
		
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
		strcpy(c->topic, topic);
		strcpy(c->topicowner, who);
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
