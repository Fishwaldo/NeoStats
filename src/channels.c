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

/*  TODO:
 *  - 
 */

#include "neostats.h"
#include "ircd.h"
#include "modes.h"
#include "bots.h"
#include "users.h"
#include "channels.h"
#include "exclude.h"
#include "services.h"
#include "modules.h"

/* @brief hash and list sizes */
#define CHANNEL_TABLE_SIZE	-1
#define CHANNEL_MEM_SIZE	-1
#define CHANNEL_MAXMODES	-1

/* @brief Module channel hash list */
static hash_t *channelhash;
/* @brief quit reason buffer */
static char quitreason[BUFSIZE];

static unsigned int moddatacnt[NUM_MODULES];

/** @brief ChanPartHandler
 *
 *  list handler for channel parts
 *  Channel subsystem use only.
 *
 *  @param list
 *  @param node
 *  @param v
 *
 *  @return none
 */

static void ChanPartHandler (list_t * list, lnode_t * node, void *v)
{
	PartChannel ((Client *)v, lnode_get (node), quitreason[0] != 0 ? quitreason : NULL);
}

/** @brief PartAllChannels
 *
 *  Part client from all channels
 *  NeoStats core use only.
 *
 *  @param u
 *  @param reason
 *
 *  @return none
 */

void PartAllChannels (Client * u, const char* reason)
{
	memset (quitreason, 0, BUFSIZE);
	if(reason) {
		strlcpy(quitreason, reason, BUFSIZE);
		strip_mirc_codes(quitreason);
	}
	list_process (u->user->chans, u, ChanPartHandler);
}

/** @brief ChannelTopic
 *
 *  Process channel topic change and send EVENT_TOPIC to modules
 *  NeoStats core use only.
 *
 *  @param channel name
 *  @param owner who changed the topic
 *  @param time topic was changed
 *  @param topic
 *
 *  @return none
 */

void ChannelTopic (const char* chan, const char *owner, const char* ts, const char *topic)
{
	CmdParams *cmdparams;
	Channel *c;

	c = find_channel (chan);
	if (!c) {
		nlog (LOG_WARNING, "ChannelTopic: can't find channel %s", chan);
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
	SendAllModuleEvent (EVENT_TOPIC, cmdparams);
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
static Channel *new_chan (const char *chan)
{
	CmdParams *cmdparams;
	Channel *c;

	if (hash_isfull (channelhash)) {
		nlog (LOG_CRITICAL, "new_chan: channel hash is full");
		return NULL;
	}
	c = ns_calloc (sizeof (Channel));
	strlcpy (c->name, chan, MAXCHANLEN);
	hnode_create_insert (channelhash, c, c->name);
	c->members = list_create (CHANNEL_MEM_SIZE);
	c->modeparms = list_create (CHANNEL_MAXMODES);
	c->creationtime = me.now;
	/* XXX TODO: Set the channel language */
	c->lang = me.lang;
	/* check exclusions */
	ns_do_exclude_chan(c);
	me.channelcount++;
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
del_chan (Channel *c)
{
	CmdParams *cmdparams;
	hnode_t *cn;

	SET_SEGV_LOCATION();
	dlog (DEBUG2, "del_chan: deleting channel %s", c->name);
	cn = hash_lookup (channelhash, c->name);
	if (!cn) {
		nlog (LOG_WARNING, "del_chan: channel %s not found.", c->name);
		return;
	}
	me.channelcount--;
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->channel = c;
	SendAllModuleEvent (EVENT_DELCHAN, cmdparams);
	ns_free (cmdparams);
	list_destroy_auto (c->modeparms);
	list_destroy (c->members);
	hash_delete (channelhash, cn);
	hnode_destroy (cn);
	ns_free (c);
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

	c->users--;
	un = list_find (u->user->chans, c->name, comparef);
	if (!un) {
		nlog (LOG_WARNING, "del_chan_user: %s not found in channel %s", u->name, c->name);
	} else {
		lnode_destroy (list_delete (u->user->chans, un));
	}
	dlog (DEBUG3, "del_chan_user: cur users %s %d (list %d)", c->name, c->users, (int)list_count (c->members));
	if (c->users <= 0) {
		del_chan (c);
	} else if (c->neousers > 0 && c->neousers == c->users) {
		/* all real users have left the channel */
		handle_dead_channel (c);
	}
}

/** @brief Remove ChannelMemberber
 *
 *
 */
int del_ChannelMemberber (Channel *c, Client *user)
{
	ChannelMember *cm;
	lnode_t *un;
	
	un = list_find (c->members, user->name, comparef);
	if (!un) {
		nlog (LOG_WARNING, "%s isn't a member of channel %s", user->name, c->name);
		return NS_FAILURE;
	}
	cm = lnode_get (un);
	lnode_destroy (list_delete (c->members, un));
	ns_free (cm);
	return NS_SUCCESS;
}

/** @brief Process a kick from a channel. 
 *
 * In fact, this does nothing special apart from call PartChannel, and processing a channel kick
 * @param kickby, the user nick or servername doing the kick
 * @param chan, channel name user being kicked from
 * @param kicked, the user nick getting kicked
 * @param kickreason the reason the user was kicked
 *
 */

void
KickChannel (const char *kickby, const char *chan, const char *kicked, const char *kickreason)		
{
	CmdParams *cmdparams;
	Channel *c;
	Client *u;

	SET_SEGV_LOCATION();
	dlog (DEBUG2, "KickChannel: %s kicked %s from %s for %s", kickby, kicked, chan, kickreason ? kickreason : "no reason");
	u = find_user (kicked);
	if (!u) {
		nlog (LOG_WARNING, "KickChannel: user %s not found", kicked);
		return;
	}
	c = find_channel (chan);
	if (!c) {
		nlog (LOG_WARNING, "KickChannel: channel %s not found", chan);
		return;
	} 
	if (del_ChannelMemberber (c, u) != NS_SUCCESS) {
		return;
	}
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->target = u;
	cmdparams->channel = c;
	cmdparams->source = find_user (kickby);
	if (!cmdparams->source) {
		cmdparams->source = find_server (kickby);
	}
	cmdparams->param = (char*)kickreason;
	SendAllModuleEvent (EVENT_KICK, cmdparams);
	if (IsMe (u)) {
		/* its one of our bots */
		cmdparams->bot = u->user->bot;
		SendModuleEvent (EVENT_KICKBOT, cmdparams, u->user->bot->moduleptr);
	}
	/* If PROTOCOL_KICKPART then we will also get part so DO NOT REMOVE USER */
	if (!(ircd_srv.protocol & PROTOCOL_KICKPART)) {
		del_chan_user (c, u);
	}
	ns_free (cmdparams);
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

void PartChannel (Client * u, const char *chan, const char *reason)
{
	CmdParams *cmdparams;
	Channel *c;

	SET_SEGV_LOCATION();
	dlog (DEBUG2, "PartChannel: parting %s from %s", u->name, chan);
	if (!u) {
		nlog (LOG_WARNING, "PartChannel: trying to part NULL user from %s", chan);
		return;
	}
	c = find_channel (chan);
	if (!c) {
		nlog (LOG_WARNING, "PartChannel: channel %s not found", chan);
		return;
	}
	if (del_ChannelMemberber (c, u) != NS_SUCCESS) {
		return;
	}
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->channel = c;
	cmdparams->source = u;
	cmdparams->param = (char*)reason;
	SendAllModuleEvent (EVENT_PART, cmdparams);
	if (IsMe (u)) {
		/* its one of our bots */
		SendModuleEvent (EVENT_PARTBOT, cmdparams, u->user->bot->moduleptr);
		c->neousers --;
	}
	del_chan_user (c, u);
	ns_free (cmdparams);
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
ChannelNickChange (Channel *c, const char *newnick, const char *oldnick)
{
	ChannelMember *cml;

	SET_SEGV_LOCATION();
	cml = lnode_find (c->members, oldnick, comparef);
	if (!cml) {
		nlog (LOG_WARNING, "ChannelNickChange: %s isn't a member of %s", oldnick, c->name);
		return;
	}
    dlog (DEBUG3, "ChannelNickChange: newnick %s, oldnick %s", newnick, oldnick);
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
JoinChannel (const char* nick, const char *chan)
{
	CmdParams *cmdparams;
	Client * u;
	Channel *c;
	ChannelMember *cm;
	
	SET_SEGV_LOCATION();
	u = find_user (nick);
	if (!u) {
		nlog (LOG_WARNING, "JoinChannel: tried to join unknown user %s to channel %s", nick, chan);
		return;
	}
	if (!ircstrcasecmp ("0", chan)) {
		/* join 0 is actually part all chans */
		dlog (DEBUG2, "JoinChannel: parting %s from all channels", u->name);
		PartAllChannels (u, NULL);
		return;
	}
	c = find_channel (chan);
	if (!c) {
		/* its a new Channel */
		dlog (DEBUG2, "JoinChannel: new channel %s", chan);
		c = new_chan (chan);
	}
	/* add this users details to the channel members hash */
	if (list_find (c->members, u->name, comparef)) {
		nlog (LOG_WARNING, "JoinChannel: tried to add %s to channel %s but they are already a member", u->name, chan);
		return;
	}
	if (list_isfull (c->members)) {
		nlog (LOG_CRITICAL, "JoinChannel: channel %s member list is full", c->name);
		return;
	}
	dlog (DEBUG2, "JoinChannel: adding usernode %s to channel %s", u->name, chan);
	cm = ns_calloc (sizeof (ChannelMember));
	strlcpy (cm->nick, u->name, MAXNICK);
	cm->tsjoin = me.now;
	cm->flags = 0;
	lnode_create_append (c->members, cm);
	c->users++;
	if (list_isfull (u->user->chans)) {
		nlog (LOG_CRITICAL, "JoinChannel: user %s member list is full", u->name);
		return;
	}
	lnode_create_append (u->user->chans, c->name);
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	cmdparams->source = u;
	cmdparams->channel = c;
	if (IsMe (u)) {
		/* its one of our bots */
		c->neousers ++;
	}
	SendAllModuleEvent (EVENT_JOIN, cmdparams);
	ns_free (cmdparams);
	dlog (DEBUG3, "JoinChannel: cur users %s %d (list %d)", c->name, c->users, (int)list_count (c->members));
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

static void ListChannelMembers (CmdParams* cmdparams, Channel *c)
{
 	ChannelMember *cm;
	lnode_t *cmn;

	irc_prefmsg (ns_botptr, cmdparams->source, __("Members:    %d (List %d)", cmdparams->source), c->users, (int)list_count (c->members));
	cmn = list_first (c->members);
	while (cmn) {
		cm = lnode_get (cmn);
		irc_prefmsg (ns_botptr, cmdparams->source, __("            %s Modes %s Joined: %ld", cmdparams->source), cm->nick, CmodeMaskToString (cm->flags), (long)cm->tsjoin);
		cmn = list_next (c->members, cmn);
	}
}

static void ListChannel (CmdParams* cmdparams, Channel *c)
{
	irc_prefmsg (ns_botptr, cmdparams->source, __("Channel:    %s", cmdparams->source), c->name);
	irc_prefmsg (ns_botptr, cmdparams->source, __("Created:    %ld", cmdparams->source), (long)c->creationtime);
	irc_prefmsg (ns_botptr, cmdparams->source, __("TopicOwner: %s TopicTime: %ld Topic: %s", cmdparams->source), c->topicowner, (long)c->topictime, c->topic);
	irc_prefmsg (ns_botptr, cmdparams->source, __("PubChan?:   %d", cmdparams->source), is_pub_chan (c));
	irc_prefmsg (ns_botptr, cmdparams->source, __("Flags:      %x", cmdparams->source), c->flags);
	ListChannelModes (cmdparams, c);
	ListChannelMembers (cmdparams, c);
	irc_prefmsg (ns_botptr, cmdparams->source, "========================================");
}

void ListChannels (CmdParams* cmdparams, const char *chan)
{
	hnode_t *cn;
	hscan_t sc;
	Channel *c;

#ifndef DEBUG
	if (!nsconfig.debug)
		return;
#endif
	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("================CHANLIST================",cmdparams->source));
	if (!chan) {
		irc_prefmsg (ns_botptr, cmdparams->source, __("Channels %d", cmdparams->source), (int)hash_count (channelhash));
		hash_scan_begin (&sc, channelhash);
		while ((cn = hash_scan_next (&sc)) != NULL) {
			c = hnode_get (cn);
			ListChannel (cmdparams, c);
		}
	} else {
		c = find_channel (chan);
		if (c) {
			ListChannel (cmdparams, c);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, __("ListChannels: can't find channel %s", cmdparams->source), chan);
		}
	}
}

/** @brief find_channel
 *
 *  Finds the channel structure for the channel named chan
 *
 *  @param chan the channel name to find
 *
 *  @returns channel structure for chan, or NULL if it can't be found.
*/

Channel *find_channel (const char *chan)
{
	Channel *c;

	c = (Channel *)hnode_find (channelhash, chan);
	if (!c) {
		dlog (DEBUG3, "find_channel: %s not found", chan);
	}
	return c;
}

/** @brief IsChannelMember 
 *
 *  Check whether nick is a member of the channel
 *
 *  @param c the channel to check
 *  @param u the user to check 
 *
 *  @returns NS_TRUE if user is a member of channel, NS_FALSE if not 
*/

int IsChannelMember (Channel *c, Client *u) 
{
	if (!u || !c) {
		return NS_FALSE;
	}
	if (list_find (c->members, u->name, comparef)) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief test_cumode
 *
 *  Whether nick has a particular channel status e.g. ChanOp
 *
 *  @param chan the channel to check
 *  @param nick the nick to check 
 *
 *  @return NS_TRUE if has, else NS_FALSE
 */

int test_cumode (char* chan, char* nick, int flag)
{
	Client * u;
	Channel *c;
 	ChannelMember *cm;

	u = find_user(nick);
	c = find_channel(chan);
	if (!u || !c) {
		return NS_FALSE;
	}
	cm = lnode_find (c->members, nick, comparef);
	if (cm) {
		if (cm->flags & flag) {
			return NS_TRUE;
		}
	}	
	return NS_FALSE;
}

/** @brief InitChannels
 *
 *  initialise channel subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitChannels (void)
{
	channelhash = hash_create (CHANNEL_TABLE_SIZE, 0, 0);
	if(!channelhash)	{
		nlog (LOG_CRITICAL, "Unable to create channel hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniChannels
 *
 *  cleanup channel subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniChannels (void)
{
	hash_destroy(channelhash);
}

void GetChannelList (ChannelListHandler handler, void *v)
{
	hnode_t *node;
	hscan_t scan;
	Channel *c;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, channelhash);
	while ((node = hash_scan_next(&scan)) != NULL) {
		c = hnode_get(node);
		handler (c, v);
	}
}
hash_t *GetChannelHash (void)
{
	return channelhash;
}

void *AllocChannelModPtr (Channel* c, int size)
{
	void *ptr;
	ptr = ns_calloc (size);
	c->modptr[GET_CUR_MODNUM()] = ptr;
	fchannelmoddata |= (1 << GET_CUR_MODNUM());
	moddatacnt[GET_CUR_MODNUM()]++;
	return ptr;
}

void FreeChannelModPtr (Channel *c)
{
	ns_free (c->modptr[GET_CUR_MODNUM()]);
	moddatacnt[GET_CUR_MODNUM()]--;
	if (moddatacnt[GET_CUR_MODNUM()] == 0)
	{
		fchannelmoddata &= ~(1 << GET_CUR_MODNUM());
	}
}

void* GetChannelModPtr (Channel *c)
{
	return c->modptr[GET_CUR_MODNUM()];
}

void ClearChannelModValue (Channel *c)
{
	if (c)
	{
		c->modvalue[GET_CUR_MODNUM()] = NULL;
		moddatacnt[GET_CUR_MODNUM()]--;
	}
	if (moddatacnt[GET_CUR_MODNUM()] == 0)
	{
		fchannelmoddata &= ~(1 << GET_CUR_MODNUM());
	}
}

void SetChannelModValue (Channel *c, void *data)
{
	if (c)
	{
		c->modvalue[GET_CUR_MODNUM()] = data;
		fchannelmoddata |= (1 << GET_CUR_MODNUM());
		moddatacnt[GET_CUR_MODNUM()]++;
	}
}

void *GetChannelModValue (Channel *c)
{
	if (c)
	{
		return c->modvalue[GET_CUR_MODNUM()];
	}
	return NULL;	
}

void CleanupChannelModdata (int index)
{
	hnode_t *node;
	hscan_t scan;
	Channel *c;

	SET_SEGV_LOCATION();
	if (moddatacnt[index] > 0) {
		nlog (LOG_WARNING, "Cleaning up channels after dirty module!");
		hash_scan_begin(&scan, channelhash);
		while ((node = hash_scan_next(&scan)) != NULL) {
			c = hnode_get(node);
			if (c->modptr[index]) {
				ns_free (c->modptr[index]);		
			}
			c->modvalue[index] = NULL;
		}
	}
	fchannelmoddata &= ~(1 << index);
	moddatacnt[index] = 0;
}
