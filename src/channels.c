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
#include "ircd.h"
#include "modes.h"
#include "bots.h"
#include "hash.h"
#include "users.h"
#include "channels.h"
#include "exclude.h"
#include "services.h"
#include "modules.h"

static hash_t *channelhash;
static char quitreason[BUFSIZE];

static void
ChanPartHandler (list_t * list, lnode_t * node, void *v)
{
	part_chan ((Client *)v, lnode_get (node), quitreason[0] != 0 ? quitreason : NULL);
}

void PartAllChannels (Client * u, const char* reason)
{
	bzero(quitreason, BUFSIZE);
	if(reason) {
		strlcpy(quitreason, reason, BUFSIZE);
		strip_mirc_codes(quitreason);
	}
	list_process (u->user->chans, u, ChanPartHandler);
}

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

	c = find_chan (chan);
	if (!c) {
		nlog (LOG_WARNING, "ChanTopic: can't find channel %s", chan);
		return;
	}
	if(topic) {
		strlcpy (c->topic, topic, BUFSIZE);
	} else {
		c->topic[0] = 0;
	}
	strlcpy (c->topicowner, owner, MAXHOST);
	c->topictime = (ts) ? atoi (ts) : me.now;
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->channel = c;
	AddStringToList (&cmdparams->av, (char*)owner, &cmdparams->ac);
	if(topic) {
		AddStringToList (&cmdparams->av, (char*)topic, &cmdparams->ac);
	} else {
		AddStringToList (&cmdparams->av, "", &cmdparams->ac);
	}
	SendAllModuleEvent (EVENT_TOPIC, cmdparams);
	ns_free (cmdparams->av);
	ns_free (cmdparams);
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

	if (hash_isfull (channelhash)) {
		nlog (LOG_CRITICAL, "new_chan: channel hash is full");
		return NULL;
	}
	c = ns_calloc (sizeof (Channel));
	strlcpy (c->name, chan, MAXCHANLEN);
	hnode_create_insert (channelhash, c, c->name);
	c->chanmembers = list_create (CHAN_MEM_SIZE);
	c->modeparms = list_create (MAXMODES);
	c->creationtime = me.now;
	/* XXX TODO: Set the channel language */
	c->lang = me.lang;
	/* check exclusions */
	ns_do_exclude_chan(c);
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->channel = c;
	SendAllModuleEvent (EVENT_NEWCHAN, cmdparams);
	ns_free (cmdparams);
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

	SET_SEGV_LOCATION();
	cn = hash_lookup (channelhash, c->name);
	if (!cn) {
		nlog (LOG_WARNING, "del_chan: channel %s not found.", c->name);
		return;
	} else {
		dlog(DEBUG2, "del_chan: deleting channel %s", c->name);
		cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
		cmdparams->channel = c;
		SendAllModuleEvent (EVENT_DELCHAN, cmdparams);
		ns_free (cmdparams);

		list_destroy_auto (c->modeparms);
		list_destroy (c->chanmembers);
		hash_delete (channelhash, cn);
		hnode_destroy (cn);
		ns_free (c);
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
void del_chan_user (Channel *c, Client *u)
{
	lnode_t *un;

	un = list_find (u->user->chans, c->name, comparef);
	if (!un) {
		nlog (LOG_WARNING, "del_chan_user: %s not found in channel %s", u->name, c->name);
	} else {
		lnode_destroy (list_delete (u->user->chans, un));
	}
	dlog(DEBUG3, "del_chan_user: cur users %s %ld (list %d)", c->name, c->users, (int)list_count (c->chanmembers));
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
	Client *u;

	SET_SEGV_LOCATION();
	u = find_user (kicked);
	if (!u) {
		nlog (LOG_WARNING, "kick_chan: user %s not found %s %s", kicked, chan, kickby);
		return;
	}
	dlog(DEBUG2, "kick_chan: %s kicking %s from %s for %s", kickby, u->name, chan, kickreason ? kickreason : "no reason");
	c = find_chan (chan);
	if (!c) {
		nlog (LOG_WARNING, "kick_chan: channel %s not found", chan);
		return;
	} else {
		un = list_find (c->chanmembers, u->name, comparef);
		if (!un) {
			nlog (LOG_WARNING, "kick_chan: %s isn't a member of channel %s", u->name, chan);
		} else {
			CmdParams * cmdparams;
			cm = lnode_get (un);
			lnode_destroy (list_delete (c->chanmembers, un));
			ns_free (cm);
			cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
			cmdparams->target = u;
			cmdparams->channel = c;
			//AddStringToList (&av, (char *)kickby, &ac);
			if (kickreason != NULL) {
				cmdparams->param = (char*)kickreason;
			}
			SendAllModuleEvent (EVENT_KICK, cmdparams);
			if ( IsMe (u) ) {
				/* its one of our bots */
				SendModuleEvent (EVENT_KICKBOT, cmdparams, u->user->bot->moduleptr);
			}
			ns_free (cmdparams);
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
part_chan (Client * u, const char *chan, const char *reason)
{
	Channel *c;
	lnode_t *un;
	Chanmem *cm;

	SET_SEGV_LOCATION();
	if (!u) {
		nlog (LOG_WARNING, "part_chan: trying to part NULL user from %s", chan);
		return;
	}
	dlog(DEBUG2, "part_chan: parting %s from %s", u->name, chan);
	c = find_chan (chan);
	if (!c) {
		nlog (LOG_WARNING, "part_chan: channel %s not found", chan);
		return;
	} else {
		un = list_find (c->chanmembers, u->name, comparef);
		if (!un) {
			nlog (LOG_WARNING, "part_chan: user %s isn't a member of channel %s", u->name, chan);
		} else {
			CmdParams * cmdparams;
			cm = lnode_get (un);
			lnode_destroy (list_delete (c->chanmembers, un));
			ns_free (cm);
			cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
			cmdparams->channel = c;
			cmdparams->source = u;
			if (reason != NULL) {
				cmdparams->param = (char*)reason;
			}
			SendAllModuleEvent (EVENT_PART, cmdparams);
			if ( IsMe (u) ) {
				/* its one of our bots */
				SendModuleEvent (EVENT_PARTBOT, cmdparams, u->user->bot->moduleptr);
			}
			ns_free (cmdparams);
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
	Chanmem *cml;

	SET_SEGV_LOCATION();
	cml = lnode_find (c->chanmembers, oldnick, comparef);
	if (!cml) {
		nlog (LOG_WARNING, "ChanNickChange: %s isn't a member of %s", oldnick, c->name);
		return;
	}
    	dlog(DEBUG3, "ChanNickChange: newnick %s, oldnick %s", newnick, oldnick);
	strlcpy (cml->nick, newnick, MAXNICK);
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
	Client * u;
	Channel *c;
	Chanmem *cm;
	
	SET_SEGV_LOCATION();
	u = find_user (nick);
	if (!u) {
		nlog (LOG_WARNING, "join_chan: tried to join unknown user %s to channel %s", nick, chan);
		return;
	}
	if (!ircstrcasecmp ("0", chan)) {
		/* join 0 is actually part all chans */
		dlog(DEBUG2, "join_chan: parting %s from all channels", u->name);
		PartAllChannels (u, NULL);
		return;
	}
	c = find_chan (chan);
	if (!c) {
		/* its a new Channel */
		dlog(DEBUG2, "join_chan: new channel %s", chan);
		c = new_chan (chan);
	}
	/* add this users details to the channel members hash */
	cm = ns_malloc (sizeof (Chanmem));
	strlcpy (cm->nick, u->name, MAXNICK);
	cm->tsjoin = me.now;
	cm->flags = 0;
	dlog(DEBUG2, "join_chan: adding usernode %s to channel %s", u->name, chan);
	if (list_find (c->chanmembers, u->name, comparef)) {
		nlog (LOG_WARNING, "join_chan: tried to add %s to channel %s but they are already a member", u->name, chan);
		return;
	}
	if (list_isfull (c->chanmembers)) {
		nlog (LOG_CRITICAL, "join_chan: channel %s member list is full", c->name);
		ns_free (cm);
		return;
	}
	lnode_create_append (c->chanmembers, cm);
	c->users++;
	if (list_isfull (u->user->chans)) {
		nlog (LOG_CRITICAL, "join_chan: user %s member list is full", u->name);
		return;
	}
	lnode_create_append (u->user->chans, c->name);
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->source = u;
	cmdparams->channel = c;
	SendAllModuleEvent (EVENT_JOIN, cmdparams);
	ns_free (cmdparams);
	dlog(DEBUG3, "join_chan: cur users %s %ld (list %d)", c->name, c->users, (int)list_count (c->chanmembers));
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
dumpchanmembers (CmdParams* cmdparams, Channel* c)
{
 	Chanmem *cm;
	lnode_t *cmn;

	irc_prefmsg (ns_botptr, cmdparams->source, __("Members:    %ld (List %d)", cmdparams->source), c->users, (int)list_count (c->chanmembers));
	cmn = list_first (c->chanmembers);
	while (cmn) {
		cm = lnode_get (cmn);
		irc_prefmsg (ns_botptr, cmdparams->source, __("            %s Modes %s Joined: %ld", cmdparams->source), cm->nick, CmodeMaskToString (cm->flags), (long)cm->tsjoin);
		cmn = list_next (c->chanmembers, cmn);
	}
}

static void 
dumpchan (CmdParams* cmdparams, Channel* c)
{
	irc_prefmsg (ns_botptr, cmdparams->source, __("Channel:    %s", cmdparams->source), c->name);
	irc_prefmsg (ns_botptr, cmdparams->source, __("Created:    %ld", cmdparams->source), (long)c->creationtime);
	irc_prefmsg (ns_botptr, cmdparams->source, __("TopicOwner: %s TopicTime: %ld Topic: %s", cmdparams->source), c->topicowner, (long)c->topictime, c->topic);
	irc_prefmsg (ns_botptr, cmdparams->source, __("PubChan?:   %d", cmdparams->source), is_pub_chan (c));
	irc_prefmsg (ns_botptr, cmdparams->source, __("Flags:      %x", cmdparams->source), c->flags);
	dumpchanmodes (cmdparams, c);
	dumpchanmembers (cmdparams, c);
	irc_prefmsg (ns_botptr, cmdparams->source, "========================================");
}

void ChanDump (CmdParams* cmdparams, const char *chan)
{
	hnode_t *cn;
	hscan_t sc;
	Channel *c;

#ifndef DEBUG
	if (!config.debug)
		return;
#endif
	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("================CHANDUMP================",cmdparams->source));
	if (!chan) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("Channels %d", cmdparams->source), (int)hash_count (channelhash));
		hash_scan_begin (&sc, channelhash);
		while ((cn = hash_scan_next (&sc)) != NULL) {
			c = hnode_get (cn);
			dumpchan(cmdparams, c);
		}
	} else {
		c = find_chan (chan);
		if (c) {
			dumpchan(cmdparams, c);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, __("ChanDump: can't find channel %s", cmdparams->source), chan);
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
find_chan (const char *chan)
{
	Channel* c;

	c = (Channel *)hnode_find (channelhash, chan);
	if (!c) {
		dlog(DEBUG3, "find_chan: %s not found", chan);
	}
	return c;
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
IsChanMember (Channel *c, Client *u) 
{
	if (!u || !c) {
		return 0;
	}
	if (list_find (c->chanmembers, u->name, comparef)) {
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

int test_cumode(char* chan, char* nick, int flag)
{
	Client * u;
	Channel* c;
 	Chanmem *cm;

	u = find_user(nick);
	c = find_chan(chan);
	if (!u || !c) {
		return 0;
	}
	cm = lnode_find (c->chanmembers, nick, comparef);
	if (cm) {
		if (cm->flags & flag) {
			return 1;
		}
	}	
	return 0;
}


/** @brief initialize the channel data
 *
 * initializes the channel data and channel hash channelhash.
 *
 * @return Nothing
*/

int 
InitChannels ()
{
	channelhash = hash_create (C_TABLE_SIZE, 0, 0);
	if(!channelhash)	{
		nlog (LOG_CRITICAL, "Unable to create channel hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

void FiniChannels (void)
{
	hash_destroy(channelhash);
}

void GetChannelList(ChannelListHandler handler)
{
	hnode_t *node;
	hscan_t scan;
	Channel *c;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, channelhash);
	while ((node = hash_scan_next(&scan)) != NULL) {
		c = hnode_get(node);
		handler(c);
	}
}
hash_t *GetChannelHash (void)
{
	return channelhash;
}
