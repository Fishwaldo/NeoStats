/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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

#include "neostats.h"
#include "bots.h"
#include "ircd.h"
#include "hash.h"
#include "users.h"
#include "channels.h"
#include "exclude.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif
#include "services.h"
#include "modules.h"

/** @brief Chanmem structure
 *  
 */
typedef struct Chanmem {
	char nick[MAXNICK];
	time_t tsjoin;
	long flags;
	void *moddata[NUM_MODULES];
} Chanmem;

hash_t *ch;


/** @brief Process the Channel TS Time 
 * 
 * Addes the channel TS time to the channel struct 
 *
 * @param c Channel Struct of channel who's ts is being changed
 * @param creationtime ts time of the channel
 *
 * @returns Nothing
 *
 */
void
SetChanTS (Channel * c, const time_t creationtime)
{
	c->creationtime = creationtime;
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

void
ChanTopic (const char* chan, const char *owner, const char* ts, const char *topic)
{
	CmdParams * cmdparams;
	Channel *c;
	time_t time;

	c = findchan (chan);
	if (!c) {
		nlog (LOG_WARNING, "ChanTopic: can't find channel %s", chan);
		return;
	}
	if(ts) {
		time = atoi (ts);
	} else {
		time = me.now;
	}
	if(topic) {
		strlcpy (c->topic, topic, BUFSIZE);
	} else {
		c->topic[0] = 0;
	}
	strlcpy (c->topicowner, owner, MAXHOST);
	c->topictime = time;
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->channel = c;
	AddStringToList (&cmdparams->av, (char*)owner, &cmdparams->ac);
	if(topic) {
		AddStringToList (&cmdparams->av, (char*)topic, &cmdparams->ac);
	} else {
		AddStringToList (&cmdparams->av, "", &cmdparams->ac);
	}
	SendAllModuleEvent (EVENT_TOPIC, cmdparams);
	free (cmdparams->av);
	free (cmdparams);
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
int
CheckChanMode (Channel * c, long mode)
{
	ModesParm *m;
	lnode_t *mn;
	if (!c) {
		nlog (LOG_WARNING, "CheckChanMode: tied to check modes of empty channel");
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
 * used in ChanModes to compare modes (list_find argument)
 *
 * @param v actually its a ModeParm struct
 * @param mode is the mode as a long
 *
 * @return 0 on match, 1 otherwise.
*/

static int
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
	CmdParams * cmdparams;
	char *modes;
	int add = 0;
	int j = 2;
	int i;
	int modeexists;
	Channel *c;
	ModesParm *m;
	lnode_t *mn;

	c = findchan (av[0]);
	if (!c) {
		return 0;
	}
	
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	AddStringToList(&cmdparams->av, origin, &cmdparams->ac);
	for (i = 0; i < ac; i++) {
		AddStringToList(&cmdparams->av, av[i], &cmdparams->ac);	
	}
	SendAllModuleEvent(EVENT_CHANMODE, cmdparams);
	free(cmdparams);	

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
			for (i = 0; i < ircd_cmodecount; i++) {

				if (*modes == chan_modes[i].flag) {
					if (add) {
						if (chan_modes[i].nickparam) {
							ChanUserMode (av[0], av[j], 1, chan_modes[i].mode);
							j++;
						} else {
							if (chan_modes[i].parameters) {
								mn = list_first (c->modeparms);
								modeexists = 0;
								while (mn) {
									m = lnode_get (mn);
									/* mode limit and mode key replace current values */
									if ((m->mode == CMODE_LIMIT) && (chan_modes[i].mode == CMODE_LIMIT)) {
										strlcpy (m->param, av[j], PARAMSIZE);
										j++;
										modeexists = 1;
										break;
									} else if ((m->mode == CMODE_KEY) && (chan_modes[i].mode == CMODE_KEY)) {
										strlcpy (m->param, av[j], PARAMSIZE);
										j++;
										modeexists = 1;
										break;
									} else if (((int *) m->mode == (int *) chan_modes[i].mode) && !ircstrcasecmp (m->param, av[j])) {
										nlog (LOG_INFO, "ChanMode: Mode %c (%s) already exists, not adding again", chan_modes[i].flag, av[j]);
										j++;
										modeexists = 1;
										break;
									}
									mn = list_next (c->modeparms, mn);
								}
								if (modeexists != 1) {
									m = smalloc (sizeof (ModesParm));
									m->mode = chan_modes[i].mode;
									strlcpy (m->param, av[j], PARAMSIZE);
									mn = lnode_create (m);
									if (list_isfull (c->modeparms)) {
										nlog (LOG_CRITICAL, "ChanMode: modelist is full adding to channel %s", c->name);
										do_exit (NS_EXIT_ERROR, "List full - see log file");
									} else {
										list_append (c->modeparms, mn);
									}
									j++;
								}
							} else {
								c->modes |= chan_modes[i].mode;
							}
						}
					} else {
						if (chan_modes[i].nickparam) {
							ChanUserMode (av[0], av[j], 0, chan_modes[i].mode);
							j++;
						} else {
							if (chan_modes[i].parameters) {
								mn = list_find (c->modeparms, (int *) chan_modes[i].mode, comparemode);
								if (!mn) {
									nlog (LOG_INFO, "ChanMode: can't find mode %c for channel %s", *modes, c->name);
								} else {
									list_delete (c->modeparms, mn);
									m = lnode_get (mn);
									lnode_destroy (mn);
									free (m);

									if (!(chan_modes[i].mode == CMODE_LIMIT))
										j++;
								}
							} else {
								c->modes &= ~chan_modes[i].mode;
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
ChanUserMode (const char* chan, const char* nick, int add, long mode)
{
	lnode_t *cmn;
	Chanmem *cm;
	Channel * c;
	User * u;

	u = finduser(nick);
	if (!u) {
		nlog (LOG_WARNING, "ChanUserMode: can't find user %s", nick);
		return;
	}
	c = findchan(chan);
	if (!c) {
		nlog (LOG_WARNING, "ChanUserMode: can't find channel %s", chan);
		return;
	}
	cmn = list_find (c->chanmembers, u->nick, comparef);
	if (!cmn) {
		if (me.debug_mode) {
			chanalert (ns_botptr->nick, "ChanUserMode: %s is not a member of channel %s", u->nick, c->name);
			ChanDump (c->name);
			UserDump (u->nick);
		}
		return;
	}
	cm = lnode_get (cmn);
	if (add) {
		nlog (LOG_DEBUG2, "ChanUserMode: Adding mode %ld to Channel %s User %s", mode, c->name, u->nick);
		cm->flags |= mode;
	} else {
		nlog (LOG_DEBUG2, "ChanUserMode: Deleting Mode %ld to Channel %s User %s", mode, c->name, u->nick);
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
static Channel *
new_chan (const char *chan)
{
	CmdParams * cmdparams;
	Channel *c;
	hnode_t *cn;

	SET_SEGV_LOCATION();
	c = calloc (sizeof (Channel), 1);
	strlcpy (c->name, chan, CHANLEN);
	cn = hnode_create (c);
	if (hash_isfull (ch)) {
		nlog (LOG_CRITICAL, "new_chan: channel hash is full");
		free (c);
		return NULL;
	}
	hash_insert (ch, cn, c->name);
	c->chanmembers = list_create (CHAN_MEM_SIZE);
	c->modeparms = list_create (MAXMODES);
	c->users = 0;
	c->topictime = 0;
	c->modes = 0;
	c->creationtime = me.now;
	c->flags = 0;
	/* check exclusions */
	ns_do_exclude_chan(c);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->channel = c;
	SendAllModuleEvent (EVENT_NEWCHAN, cmdparams);
	free (cmdparams);
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
del_chan (Channel * c)
{
	CmdParams * cmdparams;
	hnode_t *cn;
	lnode_t *cm;

	SET_SEGV_LOCATION();
	cn = hash_lookup (ch, c->name);
	if (!cn) {
		nlog (LOG_WARNING, "del_chan: channel %s not found.", c->name);
		return;
	} else {
		nlog (LOG_DEBUG2, "del_chan: deleting channel %s", c->name);
		cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
		cmdparams->channel = c;
		SendAllModuleEvent (EVENT_DELCHAN, cmdparams);
		free (cmdparams);

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

/** @brief Deletes a channel user record
 *
 * frees any memory associated with the record and removes it from the hash
 *
 * @param c the corrosponding channel structure you wish to delete
 *
 * @returns Nothing
*/
void del_chan_user(Channel *c, User *u)
{
	lnode_t *un;

	un = list_find (u->chans, c->name, comparef);
	if (!un) {
		nlog (LOG_WARNING, "del_chan_user: %s not found in channel %s", u->nick, c->name);
		if (me.debug_mode) {
			chanalert (ns_botptr->nick, "del_chan_user: %s not found in channel %s", u->nick, c->name);
			ChanDump (c->name);
			UserDump (u->nick);
		}
	} else {
		lnode_destroy (list_delete (u->chans, un));
	}
	nlog (LOG_DEBUG3, "del_chan_user: cur users %s %ld (list %d)", c->name, c->users, (int)list_count (c->chanmembers));
	if (c->users <= 0) {
		del_chan (c);
	}
}

/** @brief Process a kick from a channel. 
 *
 * In fact, this does nothing special apart from call part_chan, and processing a channel kick
 * @param kickby, the user nick or servername doing the kick
 * @param chan, channel name user being kicked from
 * @param kicked, the user nick getting kicked
 * @param kickreason the reason the user was kicked
 *
 */

void
kick_chan (const char *kickby, const char *chan, const char *kicked, const char *kickreason)		
{
	Channel *c;
	Chanmem *cm;
	lnode_t *un;
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (kicked);
	if (!u) {
		nlog (LOG_WARNING, "kick_chan: user %s not found %s %s", kicked, chan, kickby);
		if (me.debug_mode) {
			chanalert (ns_botptr->nick, "kick_chan: user %s not found %s %s", kicked, chan, kickby);
			ChanDump (chan);
		}
		return;
	}
	nlog (LOG_DEBUG2, "kick_chan: %s kicking %s from %s for %s", kickby, u->nick, chan, kickreason ? kickreason : "no reason");
	c = findchan (chan);
	if (!c) {
		nlog (LOG_WARNING, "kick_chan: channel %s not found", chan);
		return;
	} else {
		un = list_find (c->chanmembers, u->nick, comparef);
		if (!un) {
			nlog (LOG_WARNING, "kick_chan: %s isn't a member of channel %s", u->nick, chan);
			if (me.debug_mode) {
				chanalert (ns_botptr->nick, "kick_chan: %s isn't a member of channel %s", u->nick, chan);
				ChanDump (c->name);
				UserDump (u->nick);
			}
		} else {
			CmdParams * cmdparams;
			cm = lnode_get (un);
			lnode_destroy (list_delete (c->chanmembers, un));
			free (cm);
			cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
			cmdparams->dest.user = u;
			cmdparams->channel = c;
			//AddStringToList (&av, (char *)kickby, &ac);
			if (kickreason != NULL) {
				cmdparams->param = (char*)kickreason;
			}
			SendAllModuleEvent (EVENT_KICK, cmdparams);
			if ( IsMe (u) ) {
				/* its one of our bots */
				del_chan_bot (u->nick, c->name);
				SendModuleEvent (EVENT_KICKBOT, cmdparams, findbot(u->nick));
			}
			free (cmdparams);
			c->users--;
		}
		del_chan_user(c, u);
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
 * @param reason the reason the user parted, if any
 *
 * @returns Nothing
*/

void
part_chan (User * u, const char *chan, const char *reason)
{
	Channel *c;
	lnode_t *un;
	Chanmem *cm;

	SET_SEGV_LOCATION();
	if (!u) {
		nlog (LOG_WARNING, "part_chan: trying to part NULL user from %s", chan);
		if (me.debug_mode) {
			chanalert (ns_botptr->nick, "part_chan: trying to part NULL user from %s", chan);
			ChanDump (chan);
		}
		return;
	}
	nlog (LOG_DEBUG2, "part_chan: parting %s from %s", u->nick, chan);
	c = findchan (chan);
	if (!c) {
		nlog (LOG_WARNING, "part_chan: channel %s not found", chan);
		return;
	} else {
		un = list_find (c->chanmembers, u->nick, comparef);
		if (!un) {
			nlog (LOG_WARNING, "part_chan: user %s isn't a member of channel %s", u->nick, chan);
			if (me.debug_mode) {
				chanalert (ns_botptr->nick, "part_chan: user %s isn't a member of channel %s", u->nick, chan);
				ChanDump (c->name);
				UserDump (u->nick);
			}
		} else {
			CmdParams * cmdparams;
			cm = lnode_get (un);
			lnode_destroy (list_delete (c->chanmembers, un));
			free (cm);
			cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
			cmdparams->channel = c;
			cmdparams->source.user = u;
			if (reason != NULL) {
				cmdparams->param = (char*)reason;
			}
			SendAllModuleEvent (EVENT_PART, cmdparams);
			if ( IsMe (u) ) {
				/* its one of our bots */
				del_chan_bot (u->nick, c->name);
				SendModuleEvent (EVENT_PARTBOT, cmdparams, findbot(u->nick));
			}
			free (cmdparams);
			c->users--;
		}
		del_chan_user(c, u);
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
ChanNickChange (Channel * c, const char *newnick, const char *oldnick)
{
	lnode_t *cm;
	Chanmem *cml;
	SET_SEGV_LOCATION();
	cm = list_find (c->chanmembers, oldnick, comparef);
	if (!cm) {
		nlog (LOG_WARNING, "ChanNickChange: %s isn't a member of %s", oldnick, c->name);
		if (me.debug_mode) {
			chanalert (ns_botptr->nick, "ChanNickChange: %s isn't a member of %s", oldnick, c->name);
			ChanDump (c->name);
			UserDump (oldnick);
		}
		return;
	} else {
		nlog (LOG_DEBUG3, "ChanNickChange: newnick %s, oldnick %s", newnick, oldnick);
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
join_chan (const char* nick, const char *chan)
{
	CmdParams * cmdparams;
	User* u;
	Channel *c;
	lnode_t *un, *cn;
	Chanmem *cm;
	
	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "join_chan: tried to join unknown user %s to channel %s", nick, chan);
		return;
	}
	if (!ircstrcasecmp ("0", chan)) {
		/* join 0 is actually part all chans */
		nlog (LOG_DEBUG2, "join_chan: parting %s from all channels", u->nick);
		list_process (u->chans, u, UserPart);
		return;
	}
	c = findchan (chan);
	if (!c) {
		/* its a new Channel */
		nlog (LOG_DEBUG2, "join_chan: new channel %s", chan);
		c = new_chan (chan);
	}
	/* add this users details to the channel members hash */
	cm = smalloc (sizeof (Chanmem));
	strlcpy (cm->nick, u->nick, MAXNICK);
	cm->tsjoin = me.now;
	cm->flags = 0;
	cn = lnode_create (cm);
	nlog (LOG_DEBUG2, "join_chan: adding usernode %s to channel %s", u->nick, chan);
	if (list_find (c->chanmembers, u->nick, comparef)) {
		nlog (LOG_WARNING, "join_chan: tried to add %s to channel %s but they are already a member", u->nick, chan);
		if (me.debug_mode) {
			chanalert (ns_botptr->nick, "join_chan: tried to add %s to channel %s but they are already a member", u->nick, chan);
			ChanDump (c->name);
			UserDump (u->nick);
		}
		return;
	}
	if (list_isfull (c->chanmembers)) {
		nlog (LOG_CRITICAL, "join_chan: channel %s member list is full", c->name);
		lnode_destroy (cn);
		free (cm);
		return;
	}
	list_append (c->chanmembers, cn);
	c->users++;
	un = lnode_create (c->name);
	if (list_isfull (u->chans)) {
		nlog (LOG_CRITICAL, "join_chan: user %s member list is full", u->nick);
		lnode_destroy (un);
	} else {
		list_append (u->chans, un);
	}
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;
	cmdparams->channel = c;
	SendAllModuleEvent (EVENT_JOIN, cmdparams);
	free (cmdparams);
	nlog (LOG_DEBUG3, "join_chan: cur users %s %ld (list %d)", c->name, c->users, (int)list_count (c->chanmembers));
	if ( IsMe (u) ) {
		nlog(LOG_DEBUG3, "join_chan: joining bot %s to channel %s", u->nick, c->name);
		add_chan_bot (u->nick, c->name);
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

static void 
dumpchan (Channel* c)
{
	lnode_t *cmn;
 	Chanmem *cm;
	char mode[10];
	int i;
	int j = 0;
	ModesParm *m;

	bzero (mode, 10);
	mode[0] = '+';
	for (i = 0; i < ircd_cmodecount; i++) {
		if (c->modes & chan_modes[i].mode) {
			mode[++j] = chan_modes[i].flag;
		}
	}
	debugtochannel("Channel:    %s", c->name);
	debugtochannel("Mode:       %s creationtime %ld", mode, (long)c->creationtime);
	debugtochannel("TopicOwner: %s TopicTime: %ld Topic: %s", c->topicowner, (long)c->topictime, c->topic);
	debugtochannel("PubChan?:   %d", is_pub_chan (c));
	debugtochannel("Flags:      %lx", c->flags);
	cmn = list_first (c->modeparms);
	while (cmn) {
		m = lnode_get (cmn);
		for (i = 0; i < ircd_cmodecount; i++) {
			if (m->mode & chan_modes[i].mode) {
				debugtochannel("Modes:      %c Parms %s", chan_modes[i].flag, m->param);
			}
		}
		cmn = list_next (c->modeparms, cmn);
	}
	debugtochannel("Members:    %ld (List %d)", c->users, (int)list_count (c->chanmembers));
	cmn = list_first (c->chanmembers);
	while (cmn) {
		cm = lnode_get (cmn);
		bzero (mode, 10);
		j = 0;
		mode[0] = '+';
		for (i = 0; i < ircd_cmodecount; i++) {
			if (cm->flags & chan_modes[i].mode) {
				mode[++j] = chan_modes[i].flag;
			}
		}
		debugtochannel("            %s Modes %s Joined: %ld", cm->nick, mode, (long)cm->tsjoin);
		cmn = list_next (c->chanmembers, cmn);
	}
	debugtochannel("========================================");
}

void ChanDump (const char *chan)
{
	hnode_t *cn;
	hscan_t sc;
	Channel *c;

	SET_SEGV_LOCATION();
	debugtochannel("================CHANDUMP================");
	if (!chan) {
		debugtochannel("Channels %d", (int)hash_count (ch));
		hash_scan_begin (&sc, ch);
		while ((cn = hash_scan_next (&sc)) != NULL) {
			c = hnode_get (cn);
			dumpchan(c);
		}
	} else {
		c = findchan (chan);
		if (c) {
			dumpchan(c);
		} else {
			debugtochannel("ChanDump: can't find channel %s", chan);
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


Channel *
findchan (const char *chan)
{
	Channel *c;
	hnode_t *cn;

	cn = hash_lookup (ch, chan);
	if (cn) {
		c = hnode_get (cn);
		return c;
	}
	nlog (LOG_DEBUG3, "findchan: %s not found", chan);
	return NULL;
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
IsChanMember (Channel *c, User *u) 
{
	if (!u || !c) {
		return 0;
	}
	if (list_find (c->chanmembers, u->nick, comparef)) {
		return 1;
	}
	return 0;
}

/** @brief Returns if the nick has a particular channel status e.g. ChanOp
 *
 * @param chan the channel to check
 * @param nick the nick to check 
 *
 * @returns 1 if they are, 0 if they are not 
*/

int test_chan_user_mode(char* chan, char* nick, int flag)
{
	User* u;
	Channel* c;
	lnode_t *cmn;
 	Chanmem *cm;

	u = finduser(nick);
	c = findchan(chan);
	if (!u || !c) {
		return 0;
	}
	cmn = list_find (c->chanmembers, nick, comparef);
	if (!cmn) {
		return 0;
	}
	cm = lnode_get (cmn);
	if (cm->flags & flag) {
		return 1;
	}	
	return 0;
}

#ifdef SQLSRV

/* display the channel modes */
/* BUFSIZE is probably too small.. oh well */
static char chanmodes[BUFSIZE];

void *display_chanmodes (void *tbl, char *col, char *sql, void *row) 
{
	Channel *c = row;
	lnode_t *cmn;
	char tmp[BUFSIZE];
	int i;
	int j = 1;
	ModesParm *m;
	
	chanmodes[0] = '+';
	for (i = 0; i < ircd_cmodecount; i++) {
		if (c->modes & chan_modes[i].mode) {
			chanmodes[j++] = chan_modes[i].flag;
		}
	}
	chanmodes[j++] = '\0';

	cmn = list_first (c->modeparms);
	while (cmn) {
		m = lnode_get (cmn);
		for (i = 0; i < ircd_cmodecount; i++) {
			if (m->mode & chan_modes[i].mode) {
				ircsnprintf(tmp, BUFSIZE, " +%c %s", chan_modes[i].flag, m->param);
				strlcat(chanmodes, tmp, BUFSIZE);
			}
		}
		cmn = list_next (c->modeparms, cmn);
	}
	return chanmodes;
}
/* its huge, because we can have a *LOT* of users in a channel */

static char chanusers[BUFSIZE*10];
void *display_chanusers (void *tbl, char *col, char *sql, void *row) 
{
	Channel *c = row;
	lnode_t *cmn;
	char sjoin[BUFSIZE];
	char mode[BUFSIZE];
	char final[BUFSIZE*2];
	int i;
	int j = 0;
	int k = 0;
 	Chanmem *cm;
	
	chanusers[0] = '\0';
	cmn = list_first (c->chanmembers);
	while (cmn) {
		cm = lnode_get (cmn);
		j = 0;
		k = 1;
		mode[0] = '+';
		for (i = 0; i < ircd_cmodecount; i++) {
			if (cm->flags & chan_modes[i].mode) {
				if (chan_modes[i].sjoin != 0) 
					sjoin[j++] = chan_modes[i].sjoin;
				else 
					mode[k++] = chan_modes[i].flag;
			}
		}
		mode[k++] = '\0';
		sjoin[j++] = '\0';
		if (k > 2) 
			ircsnprintf(final, BUFSIZE*2, "%s %s%s,", mode, sjoin, cm->nick);
		else 
			ircsnprintf(final, BUFSIZE*2, "%s%s,", sjoin, cm->nick);
		strlcat(chanusers, final, BUFSIZE*10);
		cmn = list_next (c->chanmembers, cmn);
	}
	return chanusers;
}

COLDEF neo_chanscols[] = {
	{
		"chans",
		"name",
		RTA_STR,
		CHANLEN,
		offsetof(struct Channel, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the channel"
	},
	{
		"chans",
		"nomems",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users in the channel"
	},
	{
		"chans",
		"modes",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Channel, modes),
		RTA_READONLY,
		display_chanmodes,
		NULL,
		"The modes of the channel"
	},
	{
		"chans",
		"users",
		RTA_STR,
		BUFSIZE*10,
		offsetof(struct Channel, chanmembers),
		RTA_READONLY,
		display_chanusers,
		NULL,
		"The users of the channel"
	},
	{
		"chans",
		"topic",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Channel, topic),
		RTA_READONLY,
		NULL,
		NULL,
		"The topic of the channel"
	},
	{	
		"chans",
		"topicowner",
		RTA_STR,
		MAXHOST,
		offsetof(struct Channel, topicowner),
		RTA_READONLY,
		NULL,
		NULL,
		"Who set the topic"
	},
	{	
		"chans",
		"topictime",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, topictime),
		RTA_READONLY,
		NULL,
		NULL,
		"When the topic was set"
	},
	{	
		"chans",
		"created",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, creationtime),
		RTA_READONLY,
		NULL,
		NULL,
		"when the channel was created"
	},
	{	
		"chans",
		"flags",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, flags),
		RTA_READONLY,
		NULL,
		NULL,
		"Flags for this channel"
	},

};

TBLDEF neo_chans = {
	"chans",
	NULL, 	/* for now */
	sizeof(struct Channel),
	0,
	TBL_HASH,
	neo_chanscols,
	sizeof(neo_chanscols) / sizeof(COLDEF),
	"",
	"The list of Channels on the IRC network"
};
#endif /* SQLSRV */

/** @brief initialize the channel data
 *
 * initializes the channel data and channel hash ch.
 *
 * @return Nothing
*/

int 
InitChannels ()
{
	ch = hash_create (C_TABLE_SIZE, 0, 0);
	if(!ch)	
		return NS_FAILURE;
#ifdef SQLSRV
	/* add the server hash to the sql library */
	neo_chans.address = ch;
	rta_add_table(&neo_chans);
#endif
	return NS_SUCCESS;
}
