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
#include "modules.h"
#include "ircd.h"
#include "services.h"
#include "commands.h"

/** @brief Channel bot structure
 * 
 */
typedef struct ChanBot {
	/** channel name */
	char chan[CHANLEN];
	/** bot list */
	list_t *bots;
}ChanBot;

#define MAX_CMD_LINE_LENGTH		350

/* @brief Module Bot hash list */
static hash_t *bh;
/* @brief Module Chan Bot hash list */
static hash_t *bch;

int InitBots (void)
{
	bh = hash_create (B_TABLE_SIZE, 0, 0);
	if(!bh) {
		nlog (LOG_CRITICAL, "Unable to create bot hash");
		return NS_FAILURE;
	}
	bch = hash_create (C_TABLE_SIZE, 0, 0);
	if(!bch) {
		nlog (LOG_CRITICAL, "Unable to create bot channel hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

int FiniBots (void)
{
	hash_destroy(bh);
	hash_destroy(bch);
	return NS_SUCCESS;
}

/** @brief Add a bot to a channel
 *
 * @param bot string containing bot name
 * @param chan string containing channel name
 * 
 * @return none
 */
void
add_chan_bot (char *bot, char *chan)
{
	hnode_t *cbn;
	ChanBot *mod_chan_bot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		mod_chan_bot = malloc (sizeof (ChanBot));
		strlcpy (mod_chan_bot->chan, chan, CHANLEN);
		mod_chan_bot->bots = list_create (B_TABLE_SIZE);
		cbn = hnode_create (mod_chan_bot);
		if (hash_isfull (bch)) {
			nlog (LOG_CRITICAL, "add_chan_bot: eek, bot channel hash is full");
			return;
		}
		hash_insert (bch, cbn, mod_chan_bot->chan);
	} else {
		mod_chan_bot = hnode_get (cbn);
	}
	if (list_isfull (mod_chan_bot->bots)) {
		nlog (LOG_CRITICAL, "add_chan_bot: bot channel list is full adding channel %s", chan);
		return;
	}
	botname = sstrdup (bot);
	bmn = lnode_create (botname);
	list_append (mod_chan_bot->bots, bmn);
	return;
}

/** @brief delete a bot from a channel
 *
 * @param bot string containing bot name
 * @param chan string containing channel name
 * 
 * @return none
 */
void
del_chan_bot (char *bot, char *chan)
{
	hnode_t *cbn;
	ChanBot *mod_chan_bot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		nlog (LOG_WARNING, "del_chan_bot: can't find channel %s for botchanhash", chan);
		return;
	}
	mod_chan_bot = hnode_get (cbn);
	bmn = list_find (mod_chan_bot->bots, bot, comparef);
	if (!bmn) {
		nlog (LOG_WARNING, "del_chan_bot: can't find bot %s in %s in botchanhash", bot, chan);
		return;
	}
	list_delete (mod_chan_bot->bots, bmn);
	botname = lnode_get(bmn);
	free (botname);
	lnode_destroy (bmn);
	if (list_isempty (mod_chan_bot->bots)) {
		/* delete the hash and list because its all over */
		hash_delete (bch, cbn);
		list_destroy (mod_chan_bot->bots);
		free (mod_chan_bot->chan);
		hnode_destroy (cbn);
	}
}

/** @brief dump list of module bots and channels
 *
 * @param u
 * 
 * @return none
 */
int
list_bot_chans (CmdParams* cmdparams)
{
	hscan_t hs;
	hnode_t *hn;
	lnode_t *ln;
	ChanBot *mod_chan_bot;

	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "BotChanDump:");
	hash_scan_begin (&hs, bch);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		mod_chan_bot = hnode_get (hn);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s:--------------------------------", mod_chan_bot->chan);
		ln = list_first (mod_chan_bot->bots);
		while (ln) {
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Bot Name: %s", (char *)lnode_get (ln));
			ln = list_next (mod_chan_bot->bots, ln);
		}
	}
	return 0;
}

int process_origin(CmdParams * cmdparams, char* origin)
{
	cmdparams->source.user = finduser (origin);
	if(cmdparams->source.user) {
		if (flood (cmdparams->source.user)) {
			return 0;
		}
		else {
			return 1;
		}
	}
	cmdparams->source.server = findserver (origin);
	if(cmdparams->source.server) {
		return 1;
	}
	return 0;
}

int process_target_user(CmdParams * cmdparams, char* target)
{
	cmdparams->dest.user = finduser (target);
	if(cmdparams->dest.user) {
		cmdparams->dest.bot = findbot (target);
		if(cmdparams->dest.bot) {
			return 1;
		}
	}
	nlog (LOG_DEBUG1, "process_target_user: user %s not found", target);
	return 0;
}

int process_target_chan(CmdParams * cmdparams, char* target)
{
	cmdparams->channel = findchan(target);
	if(cmdparams->channel) {
		return 1;
	}
	nlog (LOG_DEBUG1, "cmdparams->channel: chan %s not found", target);
	return 0;
}

/** @brief send a message to a bot
 *
 * @param origin 
 * @param av 
 * @param ac
 * 
 * @return none
 */
void bot_notice (char *origin, char **av, int ac)
{
	CmdParams * cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	/* Check origin validity */
	if(process_origin(cmdparams, origin)) {
		/* Find target bot */
		if (process_target_user(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			SendModuleEvent (EVENT_NOTICE, cmdparams, NULL);
		}
	}
	free (cmdparams);
}

/** @brief send a message to a bot
 *
 * @param origin 
 * @param av 
 * @param ac
 * 
 * @return none
 */
void bot_chan_notice (char *origin, char **av, int ac)
{
	CmdParams * cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	if(process_origin(cmdparams, origin)) {
		if(process_target_chan(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			SendModuleEvent (EVENT_CNOTICE, cmdparams, NULL);
			if (av[ac - 1][0] == '\1') {
				/* TODO CTCP handler */
			}
		}
	}
	free (cmdparams);
}

/** @brief send a message to a bot
 *
 * @param origin 
 * @param av 
 * @param ac
 * 
 * @return none
 */
void bot_private (char *origin, char **av, int ac)
{
	CmdParams * cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	if(process_origin(cmdparams, origin)) {
		/* Find target bot */
		if (process_target_user(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			if ((cmdparams->dest.bot->flags & BOT_FLAG_SERVICEBOT)) {
				if(run_bot_cmd (cmdparams) != NS_FAILURE) {
					free (cmdparams);
					return;
				}
			}
			SendModuleEvent (EVENT_PRIVATE, cmdparams, NULL);
			if (av[ac - 1][0] == '\1') {
				/* TODO CTCP handler */
			}
		}
	}
	free (cmdparams);
}

/** @brief send a message to a bot
 *
 * @param origin 
 * @param av 
 * @param ac
 * 
 * @return none
 */
void bot_chan_private (char *origin, char **av, int ac)
{
	CmdParams * cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	if(process_origin(cmdparams, origin)) {
		if(process_target_chan(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			SendModuleEvent (EVENT_CPRIVATE, cmdparams, NULL);
			if (av[ac - 1][0] == '\1') {
				/* TODO CTCP handler */
			}
		}
	}
	free (cmdparams);
}

/** @brief create a new bot
 *
 * @param bot_name string containing bot name
 * 
 * @return none
 */
static Bot *
new_bot (char *bot_name)
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, "new_bot: %s", bot_name);
	botptr = malloc (sizeof (Bot));
	strlcpy (botptr->nick, bot_name, MAXNICK);
	bn = hnode_create (botptr);
	if (hash_isfull (bh)) {
		chanalert (ns_botptr->nick, "Warning bot list is full");
		return NULL;
	}
	hash_insert (bh, bn, botptr->nick);
	return botptr;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
Bot *
add_ns_bot (Module* modptr, char *nick)
{
	Bot *botptr;

	SET_SEGV_LOCATION();
	/* add a brand new user */
	botptr = new_bot (nick);
	if(botptr) {
		botptr->moduleptr = modptr;
		botptr->botcmds = NULL;
		botptr->bot_settings = NULL;	
		botptr->set_ulevel = NS_ULEVEL_ROOT;
		return botptr;
	}
	nlog (LOG_WARNING, "add_ns_bot: Couldn't Add Bot to List");
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
Bot *
findbot (char *bot_name)
{
	hnode_t *bn;

	SET_SEGV_LOCATION(); 
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		return (Bot *) hnode_get (bn);
	}
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
del_ns_bot (char *bot_name)
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		hash_delete (bh, bn);
		botptr = hnode_get (bn);
		del_all_bot_cmds(botptr);
		hnode_destroy (bn);
		free (botptr);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
bot_nick_change (char *oldnick, char *newnick)
{
	User *u;
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	/* First, try to find out if the newnick is unique! */
	u = finduser (oldnick);
	if (!u) {
		nlog (LOG_WARNING, "Unknown bot %s tried to change nick to %s", oldnick, newnick);
		return NS_FAILURE;
	}
	u = finduser (newnick);
	if (u) {
		nlog (LOG_WARNING, "Bot %s tried to change nick to one that already exists %s", oldnick, newnick);
		return NS_FAILURE;
	}
	bn = hash_lookup (bh, oldnick);
	if (!bn) {
		nlog (LOG_NOTICE, "Couldn't find bot %s in bot list", oldnick);
		return NS_FAILURE;
	}
	botptr = hnode_get (bn);
	/* remove old hash entry */
	hash_delete (bh, bn);
	nlog (LOG_DEBUG3, "Bot %s changed nick to %s", oldnick, newnick);
	strlcpy (botptr->nick, newnick, MAXNICK);
	/* insert new hash entry */
	hash_insert (bh, bn, botptr->nick);
	/* send bot nick change */   
	snick_cmd (oldnick, newnick);
	return NS_SUCCESS;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
list_bots (CmdParams* cmdparams)
{
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Module Bot List:");
	hash_scan_begin (&bs, bh);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		botptr = hnode_get (bn);
		if(botptr->moduleptr == 0) {
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "NeoStats");
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Bots: %s", botptr->nick);
		} else {
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Module: %s", botptr->moduleptr->info->name);
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Module Bots: %s", botptr->nick);
		}
	}
	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "End of Module Bot List");
	return 0;
}

/** @brief del_bots
 *
 * 
 *
 * @return none
 */
int	del_bots (Module *mod_ptr)
{
	Bot *botptr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, bh);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		botptr = hnode_get (modnode);
		if (botptr->moduleptr == mod_ptr) {
			nlog (LOG_DEBUG1, "Module %s had bot %s Registered. Deleting..", mod_ptr->info->name, botptr->nick);
			del_bot (botptr, "Module Unloaded");
		}
	}
	return NS_SUCCESS;
}

/** @brief init_bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
Bot * init_bot (BotInfo* botinfo, const char* modes, unsigned int flags, bot_cmd *bot_cmd_list, bot_setting *bot_setting_list)
{
	Bot * botptr; 
	User *u;
	long Umode;
	char* nick;
	Module* modptr;

	SET_SEGV_LOCATION();
	modptr = GET_CUR_MODULE();
	nick = botinfo->nick;
	u = finduser (nick);
	if (u) {
		nlog (LOG_WARNING, "Bot nick %s already in use", botinfo->nick);
		if(botinfo->altnick) {
			nick = botinfo->altnick;
			u = finduser (nick);
			if(u) {
				nlog (LOG_WARNING, "Bot alt nick %s already in use", botinfo->altnick);
				/* TODO: try and find a free nick */
				nlog (LOG_WARNING, "Unable to init bot");
				return NULL;
			}
		} else {
			/* TODO: try and find a free nick */
			nlog (LOG_WARNING, "Unable to init bot");
			return NULL;
		}
	} 
	botptr = add_ns_bot (modptr, nick);
	if (!botptr) {
		nlog (LOG_WARNING, "add_ns_bot failed for module %s bot %s", modptr->info->name, nick);
		return NULL;
	}
	Umode = UmodeStringToMask(modes, 0);
	signon_newbot (nick, botinfo->user, botinfo->host, botinfo->realname, Umode);
	/* Mark bot as services bot if needed */
	if ((Umode & services_bot_umode)) {
		flags |= BOT_FLAG_SERVICEBOT;
	}
#ifdef UMODE_DEAF
	if (flags&BOT_FLAG_DEAF) {
		sumode_cmd (nick, nick, UMODE_DEAF);
	}
#endif
	SET_RUN_LEVEL(modptr);
	botptr->flags = flags;
	if (bot_cmd_list) {
		add_bot_cmd_list (botptr, bot_cmd_list);
	}
	if (bot_setting_list) {
		add_bot_settings (botptr, bot_setting_list);
	}
	return botptr;
}

/** @brief del_bot
 *
 * 
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
del_bot (Bot *botptr, char *reason)
{
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (botptr->nick);
	if (!u) {
		nlog (LOG_WARNING, "Attempting to delete a bot with a nick that does not exist: %s", botptr->nick);
		return NS_FAILURE;
	}
	nlog (LOG_DEBUG1, "Deleting bot %s for %s", botptr->nick, reason);
	//XXXX TODO: need to free the channel list hash. We dont according to valgrind
	squit_cmd (botptr->nick, reason);
	del_ns_bot (botptr->nick);
	return NS_SUCCESS;
}
