/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
** $Id$
*/

#include "stats.h"
#include "dl.h"
#include "hash.h"
#include "log.h"
#include "users.h"
#include "chans.h"

/** @brief Chanmem structure
 *  
 */
typedef struct Chanmem {
	char nick[MAXNICK];
	time_t joint;
	long flags;
	void *moddata[NUM_MODULES];
} Chanmem;

hash_t *ch;

/** @brief initialize the channel data
 *
 * initializes the channel data and channel hash ch.
 *
 * @return Nothing
*/


int 
init_chan_hash ()
{
	ch = hash_create (C_TABLE_SIZE, 0, 0);
	if(!ch)	
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief Process the Channel TS Time 
 * 
 * Addes the channel TS time to the channel struct 
 *
 * @param c Channel Struct of channel who's ts is being changed
 * @param tstime ts time of the channel
 *
 * @returns Nothing
 *
 */
void
ChangeChanTS (Chans * c, time_t tstime)
{
	if (!c) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Called Change_Change_Ts with null channel");
		return;
	}
	c->tstime = tstime;
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

extern void
ChangeTopic (char *owner, Chans * c, time_t time, char *topic)
{
	char **av;
	int ac = 0;
	strlcpy (c->topic, topic, BUFSIZE);
	strlcpy (c->topicowner, owner, MAXHOST);
	c->topictime = time;
	AddStringToList (&av, c->name, &ac);
	AddStringToList (&av, owner, &ac);
	AddStringToList (&av, topic, &ac);
	ModuleEvent (EVENT_TOPICCHANGE, av, ac);
	free (av);
}

/** @brief Check if a mode is set on a Channel
 * 
 * used to check if a mode is set on a channel
 * 
 * @param c channel to check
 * @param mode is the mode to check, as a LONG
 *
 * @returns 1 on match, 0 on no match, -1 on error
 *
*/
extern int
CheckChanMode (Chans * c, long mode)
{
	ModesParm *m;
	lnode_t *mn;
	if (!c) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, CheckChanMode Called with empty channel");
		return -1;
	}
	if (c->modes & mode) {
		/* its a match */
		return 1;
	}
	/* if we get here, we have to check the modeparm list first */
	mn = list_first (c->modeparms);
	while (mn) {
		m = lnode_get (mn);
		if (m->mode & mode) {
			/* its a match */
			return 1;
		}
		mn = list_next (c->modeparms, mn);
	}
	return 0;
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

int
comparemode (const void *v, const void *mode)
{
	ModesParm *m = (void *) v;
	if (m->mode == (long) mode) {
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

int
ChanMode (char *origin, char **av, int ac)
{
	char *modes;
	int add = 0;
	int j = 2;
	int i;
	int modeexists;
	Chans *c;
	ModesParm *m;
	lnode_t *mn;
	c = findchan (av[0]);
	if (!c) {
		return 0;
	}
	modes = av[1];
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {

				if (*modes == cFlagTab[i].flag) {
					if (add) {
						if (cFlagTab[i].nickparam) {
							ChangeChanUserMode (c, finduser (av[j]), 1, cFlagTab[i].mode);
							j++;
						} else {
							if (cFlagTab[i].parameters) {
								mn = list_first (c->modeparms);
								modeexists = 0;
								while (mn) {
									m = lnode_get (mn);
									/* mode limit and mode key replace current values */
									if ((m->mode == MODE_LIMIT) && (cFlagTab[i].mode == MODE_LIMIT)) {
										strlcpy (m->param, av[j], PARAMSIZE);
										j++;
										modeexists = 1;
										break;
									} else if ((m->mode == MODE_KEY) && (cFlagTab[i].mode == MODE_KEY)) {
										strlcpy (m->param, av[j], PARAMSIZE);
										j++;
										modeexists = 1;
										break;
									} else if (((int *) m->mode == (int *) cFlagTab[i].mode) && !strcasecmp (m->param, av[j])) {
										nlog (LOG_INFO, LOG_CORE, "Mode %c (%s) already exists, not adding again", cFlagTab[i].flag, av[j]);
										j++;
										modeexists = 1;
										break;
									}
									mn = list_next (c->modeparms, mn);
								}
								if (modeexists != 1) {
									m = smalloc (sizeof (ModesParm));
									m->mode = cFlagTab[i].mode;
									strlcpy (m->param, av[j], PARAMSIZE);
									mn = lnode_create (m);
									if (list_isfull (c->modeparms)) {
										nlog (LOG_CRITICAL, LOG_CORE, "Eeek, Can't add additional Modes to Channel %s. Modelist is full", c->name);
										do_exit (NS_EXIT_ERROR, "List full - see log file");
									} else {
										list_append (c->modeparms, mn);
									}
									j++;
								}
							} else {
								c->modes |= cFlagTab[i].mode;
							}
						}
					} else {
						if (cFlagTab[i].nickparam) {
							ChangeChanUserMode (c, finduser (av[j]), 0, cFlagTab[i].mode);
							j++;
						} else {
							if (cFlagTab[i].parameters) {
								mn = list_find (c->modeparms, (int *) cFlagTab[i].mode, comparemode);
								if (!mn) {
									nlog (LOG_INFO, LOG_CORE, "Can't find Mode %c for Chan %s", *modes, c->name);
								} else {
									list_delete (c->modeparms, mn);
									m = lnode_get (mn);
									lnode_destroy (mn);
									free (m);

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

void
ChangeChanUserMode (Chans * c, User * u, int add, long mode)
{
	lnode_t *cmn;
	Chanmem *cm;
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Can't find user for ChangeChanUserMode");
		return;
	}
	if (!c) {
		nlog (LOG_WARNING, LOG_CORE, "Can't find channel for ChangeChanUserMode");
		return;
	}
	cmn = list_find (c->chanmembers, u->nick, comparef);
	if (!cmn) {
		if (me.debug_mode) {
			chanalert (s_Services, "ChangeChanUserMode() %s doesn't seem to be in the Chan %s", u->nick, c->name);
			ChanDump (c->name);
			UserDump (u->nick);
		}
		return;
	}
	cm = lnode_get (cmn);
	if (add) {
		nlog (LOG_DEBUG2, LOG_CORE, "Adding mode %ld to Channel %s User %s", mode, c->name, u->nick);
		cm->flags |= mode;
	} else {
		nlog (LOG_DEBUG2, LOG_CORE, "Deleting Mode %ld to Channel %s User %s", mode, c->name, u->nick);
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
Chans *
new_chan (char *chan)
{
	Chans *c;
	hnode_t *cn;

	SET_SEGV_LOCATION();
	c = smalloc (sizeof (Chans));
	strlcpy (c->name, chan, CHANLEN);
	cn = hnode_create (c);
	if (hash_isfull (ch)) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeek, Channel Hash is full");
	} else {
		hash_insert (ch, cn, c->name);
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

void
del_chan (Chans * c)
{
	hnode_t *cn;
	lnode_t *cm;

	SET_SEGV_LOCATION();
	cn = hash_lookup (ch, c->name);
	if (!cn) {
		nlog (LOG_WARNING, LOG_CORE, "Hu, Deleting a Non Existant Channel?");
		return;
	} else {
		nlog (LOG_DEBUG2, LOG_CORE, "Deleting Channel %s", c->name);
		cm = list_first (c->modeparms);
		while (cm) {
			free (lnode_get (cm));
			cm = list_next (c->modeparms, cm);
		}
		list_destroy_nodes (c->modeparms);
		list_destroy (c->modeparms);
		list_destroy (c->chanmembers);
		hash_delete (ch, cn);
		hnode_destroy (cn);
		free (c);
	}
}

/** @brief Process a kick from a channel. 
 *
 * In fact, this does nothing special apart from call part_chan, and processing a channel kick
 * @param User u, user structure of user getting kicked
 * @param channel name user being kicked from
 * @param User k, the user doing the kick
 * 
 *
 */

void
kick_chan (User * u, char *chan, User * k)
{
	char **av;
	int ac = 0;
	Chans *c;
	Chanmem *cm;
	lnode_t *un;

	SET_SEGV_LOCATION();
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "NULL user passed to kick_chan u=NULL, chan=%s: %s", chan, recbuf);
		if (me.debug_mode) {
			chanalert (s_Services, "NULL user passed to kick_chan u=NULL, chan=%s: %s", chan, recbuf);
			ChanDump (chan);
		}
		return;
	}
	if (!k) {
		nlog (LOG_WARNING, LOG_CORE, "NULL user passed to kick_chan k=NULL, chan=%s: %s", chan, recbuf);
		if (me.debug_mode) {
			chanalert (s_Services, "NULL user passed to kick_chan k=NULL, chan=%s: %s", chan, recbuf);
			ChanDump (chan);
		}
		return;
	}
	nlog (LOG_DEBUG2, LOG_CORE, "%s Kicking %s from %s", k->nick, u->nick, chan);
	c = findchan (chan);
	if (!c) {
		nlog (LOG_WARNING, LOG_CORE, "Hu, Kicking a Non existant Channel? %s", chan);
		return;
	} else {
		un = list_find (c->chanmembers, u->nick, comparef);
		if (!un) {
			nlog (LOG_WARNING, LOG_CORE, "Kick: hu, User %s isn't a member of this channel %s", u->nick, chan);
			if (me.debug_mode) {
				chanalert (s_Services, "Kick: hu, User %s isn't a member of this channel %s", u->nick, chan);
				ChanDump (c->name);
				UserDump (u->nick);
			}
		} else {
			cm = lnode_get (un);
			lnode_destroy (list_delete (c->chanmembers, un));
			free (cm);
			AddStringToList (&av, c->name, &ac);
			AddStringToList (&av, u->nick, &ac);
			AddStringToList (&av, k->nick, &ac);
			ModuleEvent (EVENT_KICK, av, ac);
			free (av);
			ac = 0;
			c->cur_users--;
		}
		if (findbot (u->nick)) {
			/* its one of our bots, so add it to the botchan list */
			del_bot_from_chan (u->nick, c->name);
			AddStringToList (&av, c->name, &ac);
			AddStringToList (&av, u->nick, &ac);
			AddStringToList (&av, k->nick, &ac);
			ModuleEvent (EVENT_KICKBOT, av, ac);
			free (av);
			ac = 0;

		}
		un = list_find (u->chans, c->name, comparef);
		if (!un) {
			nlog (LOG_WARNING, LOG_CORE, "Kick:Hu, User %s claims not to be part of Chan %s", u->nick, chan);
			if (me.debug_mode) {
				chanalert (s_Services, "Kick: Hu, User %s claims not to be part of Chan %s", u->nick, chan);
				ChanDump (c->name);
				UserDump (u->nick);
			}
		} else {
			lnode_destroy (list_delete (u->chans, un));
		}
		nlog (LOG_DEBUG3, LOG_CORE, "Cur Users %s %d (list %d)", c->name, c->cur_users, list_count (c->chanmembers));
		if (c->cur_users <= 0) {
			AddStringToList (&av, c->name, &ac);
			ModuleEvent (EVENT_DELCHAN, av, ac);
			free (av);
			ac = 0;
			del_chan (c);
		}
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


void
part_chan (User * u, char *chan)
{
	Chans *c;
	lnode_t *un;
	char **av;
	Chanmem *cm;
	int ac = 0;
	SET_SEGV_LOCATION();
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "NULL user passed to part_chan u=NULL, chan=%s: %s", chan, recbuf);
		if (me.debug_mode) {
			chanalert (s_Services, "NULL user passed to part_chan u=NULL, chan=%s: %s", chan, recbuf);
			ChanDump (chan);
		}
		return;
	}
	nlog (LOG_DEBUG2, LOG_CORE, "Parting %s from %s", u->nick, chan);
	c = findchan (chan);
	if (!c) {
		nlog (LOG_WARNING, LOG_CORE, "Hu, Parting a Non existant Channel? %s", chan);
		return;
	} else {
		un = list_find (c->chanmembers, u->nick, comparef);
		if (!un) {
			nlog (LOG_WARNING, LOG_CORE, "hu, User %s isn't a member of this channel %s", u->nick, chan);
			if (me.debug_mode) {
				chanalert (s_Services, "hu, User %s isn't a member of this channel %s", u->nick, chan);
				ChanDump (c->name);
				UserDump (u->nick);
			}
		} else {
			cm = lnode_get (un);
			lnode_destroy (list_delete (c->chanmembers, un));
			free (cm);
			AddStringToList (&av, c->name, &ac);
			AddStringToList (&av, u->nick, &ac);
			ModuleEvent (EVENT_PARTCHAN, av, ac);
			free (av);
			ac = 0;
			c->cur_users--;
		}
		if (findbot (u->nick)) {
			/* its one of our bots, so add it to the botchan list */
			del_bot_from_chan (u->nick, c->name);
			AddStringToList (&av, c->name, &ac);
			AddStringToList (&av, u->nick, &ac);
			ModuleEvent (EVENT_PARTBOT, av, ac);
			free (av);
			ac = 0;
		}
		un = list_find (u->chans, c->name, comparef);
		if (!un) {
			nlog (LOG_WARNING, LOG_CORE, "Hu, User %s claims not to be part of Chan %s", u->nick, chan);
			if (me.debug_mode) {
				chanalert (s_Services, "Hu, User %s claims not to be part of Chan %s", u->nick, chan);
				ChanDump (c->name);
				UserDump (u->nick);
			}
			return;
		} else {
			lnode_destroy (list_delete (u->chans, un));
		}
		nlog (LOG_DEBUG3, LOG_CORE, "Cur Users %s %d (list %d)", c->name, c->cur_users, list_count (c->chanmembers));
		if (c->cur_users <= 0) {
			AddStringToList (&av, c->name, &ac);
			ModuleEvent (EVENT_DELCHAN, av, ac);
			free (av);
			ac = 0;
			del_chan (c);
		}
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

void
change_user_nick (Chans * c, char *newnick, char *oldnick)
{
	lnode_t *cm;
	Chanmem *cml;
	SET_SEGV_LOCATION();
	cm = list_find (c->chanmembers, oldnick, comparef);
	if (!cm) {
		nlog (LOG_WARNING, LOG_CORE, "change_user_nick() %s isn't a member of %s", oldnick, c->name);
		if (me.debug_mode) {
			chanalert (s_Services, "change_user_nick() %s isn't a member of %s", oldnick, c->name);
			ChanDump (c->name);
			UserDump (oldnick);
		}
		return;
	} else {
		nlog (LOG_DEBUG3, LOG_CORE, "Change_User_Nick(): NewNick %s, OldNick %s", newnick, oldnick);
		cml = lnode_get (cm);
		strlcpy (cml->nick, newnick, MAXNICK);
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


void
join_chan (User * u, char *chan)
{
	Chans *c;
	lnode_t *un, *cn;
	Chanmem *cm;
	char **av;
	int ac = 0;
	SET_SEGV_LOCATION();
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "ehhh, Joining an Unknown user to %s: %s", chan, recbuf);
		return;
	}
	if (!strcasecmp ("0", chan)) {
		/* join 0 is actually part all chans */
		nlog (LOG_DEBUG2, LOG_CORE, "join_chan() -> Parting all chans %s", u->nick);
		list_process (u->chans, u, part_u_chan);
		return;
	}
	c = findchan (chan);
	if (!c) {
		/* its a new Channel */
		nlog (LOG_DEBUG2, LOG_CORE, "join_chan() -> New Channel %s", chan);
		c = new_chan (chan);
		c->chanmembers = list_create (CHAN_MEM_SIZE);
		c->modeparms = list_create (MAXMODES);
		c->cur_users = 0;
		c->topictime = 0;
		c->modes = 0;
		c->tstime = 0;
		AddStringToList (&av, c->name, &ac);
		ModuleEvent (EVENT_NEWCHAN, av, ac);
		free (av);
		ac = 0;
	}
	/* add this users details to the channel members hash */
	cm = smalloc (sizeof (Chanmem));
	strlcpy (cm->nick, u->nick, MAXNICK);
	cm->joint = me.now;
	cm->flags = 0;
	cn = lnode_create (cm);
	nlog (LOG_DEBUG2, LOG_CORE, "adding usernode %s to Channel %s", u->nick, chan);
	if (list_find (c->chanmembers, u->nick, comparef)) {
		nlog (LOG_WARNING, LOG_CORE, "Adding %s to Chan %s when he is already a member?", u->nick, chan);
		if (me.debug_mode) {
			chanalert (s_Services, "Adding %s to Chan %s when he is already a member?", u->nick, chan);
			ChanDump (c->name);
			UserDump (u->nick);
		}
		return;
	}
	if (list_isfull (c->chanmembers)) {
		nlog (LOG_CRITICAL, LOG_CORE, "ekk, Channel %s Members list is full", c->name);
		lnode_destroy (cn);
		free (cm);
		return;
	} else {
		list_append (c->chanmembers, cn);
	}
	c->cur_users++;
	un = lnode_create (c->name);
	if (list_isfull (u->chans)) {
		nlog (LOG_CRITICAL, LOG_CORE, "eek, User %s members list is full", u->nick);
		lnode_destroy (un);
	} else {
		list_append (u->chans, un);
	}
	AddStringToList (&av, c->name, &ac);
	AddStringToList (&av, u->nick, &ac);
	ModuleEvent (EVENT_JOINCHAN, av, ac);
	free (av);
	nlog (LOG_DEBUG3, LOG_CORE, "Cur Users %s %d (list %d)", c->name, c->cur_users, list_count (c->chanmembers));
	if (findbot (u->nick)) {
		nlog(LOG_DEBUG3, LOG_CORE, "Joining bot %s to Channel %s", u->nick, c->name);
		add_bot_to_chan (u->nick, c->name);
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


void
ChanDump (char *chan)
{
	hnode_t *cn;
	lnode_t *cmn;
	hscan_t sc;
	Chans *c;
	Chanmem *cm;
	char mode[10];
	int i;
	int j = 0;
	ModesParm *m;

	SET_SEGV_LOCATION();
	if (!chan) {
		debugtochannel("Channels %d", hash_count (ch));
		hash_scan_begin (&sc, ch);
		while ((cn = hash_scan_next (&sc)) != NULL) {
			c = hnode_get (cn);
			debugtochannel("====================");
			bzero (mode, 10);
			mode[0] = '+';
			for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
				if (c->modes & cFlagTab[i].mode) {
					mode[++j] = cFlagTab[i].flag;
				}
			}
			debugtochannel("Channel: %s Members: %d (List %d) Flags %s tstime %d", c->name, c->cur_users, list_count (c->chanmembers), mode, c->tstime);
			debugtochannel("       Topic Owner %s, TopicTime: %d, Topic %s", c->topicowner, c->topictime, c->topic);
			debugtochannel("PubChan?: %d", is_pub_chan (c));
			cmn = list_first (c->modeparms);
			while (cmn) {
				m = lnode_get (cmn);
				for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
					if (m->mode & cFlagTab[i].mode) {
						debugtochannel("        Modes: %c Parms %s", cFlagTab[i].flag, m->param);
					}
				}

				cmn = list_next (c->modeparms, cmn);
			}
			cmn = list_first (c->chanmembers);
			while (cmn) {
				cm = lnode_get (cmn);
				bzero (mode, 10);
				j = 0;
				mode[0] = '+';
				for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
					if (cm->flags & cFlagTab[i].mode) {
						mode[++j] = cFlagTab[i].flag;
					}
				}
				debugtochannel("Members: %s Modes %s Joined %d", cm->nick, mode, cm->joint);
				cmn = list_next (c->chanmembers, cmn);
			}
		}
	} else {
		c = findchan (chan);
		if (!c) {
			debugtochannel("Can't find Channel %s", chan);
		} else {
			bzero (mode, 10);
			j = 0;
			mode[0] = '+';
			for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
				if (c->modes & cFlagTab[i].mode) {
					mode[++j] = cFlagTab[i].flag;
				}
			}
			debugtochannel("Channel: %s Members: %d (List %d) Flags %s tstime %d", c->name, c->cur_users, list_count (c->chanmembers), mode, c->tstime);
			debugtochannel("       Topic Owner %s, TopicTime: %d Topic %s", c->topicowner, c->topictime, c->topic);
			debugtochannel("PubChan?: %d", is_pub_chan (c));
			cmn = list_first (c->modeparms);
			while (cmn) {
				m = lnode_get (cmn);
				for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
					if (m->mode & cFlagTab[i].mode) {
						debugtochannel("        Modes: %c Parms %s", cFlagTab[i].flag, m->param);
					}
				}
				cmn = list_next (c->modeparms, cmn);
			}
			cmn = list_first (c->chanmembers);
			while (cmn) {
				cm = lnode_get (cmn);
				bzero (mode, 10);
				mode[0] = '+';
				j = 0;
				for (i = 0; i < ((sizeof (cFlagTab) / sizeof (cFlagTab[0])) - 1); i++) {
					if (cm->flags & cFlagTab[i].mode) {
						mode[++j] = cFlagTab[i].flag;
					}
				}
				debugtochannel("Members: %s Modes %s Joined: %d", cm->nick, mode, cm->joint);
				cmn = list_next (c->chanmembers, cmn);
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


Chans *
findchan (char *chan)
{
	Chans *c;
	hnode_t *cn;
	SET_SEGV_LOCATION();
	cn = hash_lookup (ch, chan);
	if (cn) {
		c = hnode_get (cn);
		return c;
	} else {
		nlog (LOG_DEBUG3, LOG_CORE, "FindChan(%s) -> Not Found", chan);
		return NULL;
	}
}

/** @brief Returns if the nick is a member of the channel
 *
 * Returns 1 if nick is part of the channel, 0 if they are not
 *
 * @param chan the channel to check
 * @param u the user to check 
 *
 * @returns 1 if they are, 0 if they are not 
*/

int 
IsChanMember(Chans *c, User *u) 
{
	if (!u) {
		return 0;
	}
	if (!c) {
		return 0;
	}
	if (list_find (c->chanmembers, u->nick, comparef)) {
		return 1;
	}
	return 0;
}
