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
 *  - find free nick for bots if NICK and ALTNICK are in use
 */

#include "neostats.h"
#include "modules.h"
#include "ircd.h"
#include "modes.h"
#include "services.h"
#include "commands.h"
#include "bots.h"
#include "users.h"
#include "ctcp.h"
#include "modexclude.h"

#define BOT_TABLE_SIZE		100		/* Max number of bots */

/* @brief Module Bot hash list */
static hash_t *bothash;

/** @brief InitBots 
 *
 *  initialise bot subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitBots (void)
{
	bothash = hash_create (BOT_TABLE_SIZE, 0, 0);
	if (!bothash) {
		nlog (LOG_CRITICAL, "Unable to create bot hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniBots
 *
 *  cleanup bot subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniBots (void)
{
	hash_destroy (bothash);
}

/** @brief flood_test
 *
 *  Check whether client is flooding NeoStats
 *  Bot subsystem use only.
 *
 *  @param pointer to client to test
 *
 *  @return NS_TRUE if client flooded, NS_FALSE if not 
 */

static int flood_test (Client *u)
{
	/* locop or higher are expempt from flood checks */
	if (UserLevel (u) >= NS_ULEVEL_OPER)	
		return NS_FALSE;
	/* calculate and test flood values */
	if ((me.now - u->user->tslastmsg) > 10) {
		u->user->tslastmsg = me.now;
		u->user->flood = 0;
		return NS_FALSE;
	}
	if (u->user->flood >= 5) {
		nlog (LOG_NORMAL, "FLOODING: %s!%s@%s", u->name, u->user->username, u->user->hostname);
		irc_svskill (ns_botptr, u, _("%s!%s (Flooding Services.)"), me.name, ns_botptr->name);
		return NS_TRUE;
	}
	u->user->flood++;
	return NS_FALSE;
}

/** @brief process_origin
 *
 *  validate message origin and populate cmdparams structure
 *  Bot subsystem use only.
 *
 *  @param cmdparam structure to populate 
 *  @param origin nick or server name 
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */

static int process_origin (CmdParams *cmdparams, const char *origin)
{
	cmdparams->source = find_user (origin);
	if (cmdparams->source) {
		if (flood_test (cmdparams->source)) {
			return NS_FALSE;
		}
		return NS_TRUE;
	}
	cmdparams->source = find_server (origin);
	if (cmdparams->source) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief process_target_user
 *
 *  validate message target and populate cmdparams structure
 *  Bot subsystem use only.
 *
 *  @param cmdparam structure to populate 
 *  @param target nick
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */

static int process_target_user (CmdParams *cmdparams, const char *target)
{
	cmdparams->target = find_user (target);
	if (cmdparams->target) {
		cmdparams->bot = cmdparams->target->user->bot;
		if (cmdparams->bot) {
			return NS_TRUE;
		}
	}
	dlog (DEBUG1, "process_target_user: user %s not found", target);
	return NS_FALSE;
}

/** @brief process_target_chan
 *
 *  validate message target and populate cmdparams structure
 *  Bot subsystem use only.
 *
 *  @param cmdparam structure to populate 
 *  @param target channel
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */

static int process_target_chan (CmdParams *cmdparams, const char *target)
{
	cmdparams->channel = find_channel(target);
	if (cmdparams->channel) {
		return NS_TRUE;
	}
	dlog (DEBUG1, "cmdparams->channel: chan %s not found", target);
	return NS_FALSE;
}

/** @brief bot_chan_event
 *
 *  Process event regarding to bot channel
 *  Bot subsystem use only.
 *
 *  @param cmdparams pointer to command parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_chan_event (Event event, CmdParams *cmdparams)
{
	lnode_t *cm;
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;
	int cmdflag = 0;
	char *chan;

	SET_SEGV_LOCATION();
	if (cmdparams->param[0] == nsconfig.cmdchar[0]) {
		/* skip over command char */
		cmdparams->param ++;
		cmdflag = 1;
	}
	hash_scan_begin (&bs, bothash);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		botptr = hnode_get (bn);
		cm = list_first (botptr->u->user->chans);
		if (!(botptr->u->user->Umode & UMODE_DEAF)) {
			while (cm) {	
				chan = (char *) lnode_get (cm);
				cmdparams->bot = botptr;
				if (ircstrcasecmp (cmdparams->channel->name, chan) == 0) {
					if (!cmdflag || run_bot_cmd (cmdparams) != NS_SUCCESS) {
						SendModuleEvent (event, cmdparams, botptr->moduleptr);
					}
				}
				cm = list_next (botptr->u->user->chans, cm);
			}
		}
	}
	return NS_SUCCESS;
}

/** @brief bot_notice
 *
 *  send a notice to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_notice (char *origin, char **av, int ac)
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	/* Check origin validity */
	if (process_origin (cmdparams, origin)) {
		/* Find target bot */
		if (process_target_user (cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			if (av[ac - 1][0] == '\1') {
				ctcp_notice (cmdparams);
			} else {
				SendModuleEvent (EVENT_NOTICE, cmdparams, cmdparams->bot->moduleptr);
			}
		}		
	}
	ns_free (cmdparams);
}

/** @brief bot_chan_notice
 *
 *  send a channel notice to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_chan_notice (char *origin, char **av, int ac)
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	if (process_origin (cmdparams, origin)) {
		if (process_target_chan (cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			if (av[ac - 1][0] == '\1') {
				ctcp_cnotice (cmdparams);
			} else {
				bot_chan_event (EVENT_CNOTICE, cmdparams);
			}
		}
	}
	ns_free (cmdparams);
}

/** @brief bot_private
 *
 *  send a private message to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_private (char *origin, char **av, int ac)
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	if (process_origin (cmdparams, origin)) {
		/* Find target bot */
		if (process_target_user (cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			/* Check CTCP first to avoid Unknown command messages later */
			if (av[ac - 1][0] == '\1') {
				ctcp_private (cmdparams);
			} else {
				if (!(cmdparams->bot->flags & BOT_FLAG_SERVICEBOT) ||
					run_bot_cmd (cmdparams) == NS_FAILURE) {
					SendModuleEvent (EVENT_PRIVATE, cmdparams, cmdparams->bot->moduleptr);
				}
			}
		}
	}
	ns_free (cmdparams);
}

/** @brief bot_chan_private
 *
 *  send a channel private message to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_chan_private (char *origin, char **av, int ac)
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	if (process_origin (cmdparams, origin)) {
		if (process_target_chan (cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			if (av[ac - 1][0] == '\1') {
				ctcp_cprivate (cmdparams);
			} else {
				bot_chan_event (EVENT_CPRIVATE, cmdparams);
			}
		}
	}
	ns_free (cmdparams);
}

/** @brief find_bot
 *
 *  find bot
 *
 *  @param bot_name name of bot to find
 *
 *  @return pointer to bot or NULL if not found
 */

Bot *find_bot (const char *bot_name)
{
	Bot* bot;

	SET_SEGV_LOCATION(); 
	bot = (Bot *)hnode_find (bothash, bot_name);
	if (!bot) {
		dlog (DEBUG3, "find_bot: %s not found", bot_name);
	}
	return bot;
}

/** @brief DelBot
 *
 *  delete bot
 *  NeoStats core use only.
 *
 *  @param bot_name name of bot to delete
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DelBot (const char *bot_name)
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bothash, bot_name);
	if (!bn) {
		return NS_FAILURE;
	}
	botptr = hnode_get (bn);
	del_all_bot_cmds (botptr);
	del_bot_info_settings (botptr);
	del_all_bot_settings (botptr);
	hash_delete (bothash, bn);
	hnode_destroy (bn);
	ns_free (botptr);
	return NS_SUCCESS;
}

/** @brief BotNickChange
 *
 *  change bot nick
 *  NeoStats core use only.
 *
 *  @param botptr pointer to bot
 *  @param newnick new nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int BotNickChange (const Bot *botptr, const char *newnick)
{
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bothash, botptr->name);
	if (!bn) {
		nlog (LOG_NOTICE, "Couldn't find bot %s in bot list", botptr->name);
		return NS_FAILURE;
	}
	/* remove old hash entry */
	hash_delete (bothash, bn);
	dlog (DEBUG3, "Bot %s changed nick to %s", botptr->name, newnick);
	strlcpy ((char*)botptr->name, newnick, MAXNICK);
	/* insert new hash entry */
	hash_insert (bothash, bn, botptr->name);
	return NS_SUCCESS;
}

/** @brief ns_cmd_botlist
 *
 *  list all neostats bots
 *  NeoStats core use only.
 *
 *  @param cmdparams pointer to command parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ns_cmd_botlist (CmdParams *cmdparams)
{
	int i;
	lnode_t *cm;
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("Module Bot List:", cmdparams->source));
	hash_scan_begin (&bs, bothash);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		botptr = hnode_get (bn);
		if ((botptr->flags & 0x80000000)) {
			irc_prefmsg (ns_botptr, cmdparams->source, __("NeoStats", cmdparams->source));
			irc_prefmsg (ns_botptr, cmdparams->source, __("Bot: %s", cmdparams->source), botptr->name);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, __("Module: %s", cmdparams->source), botptr->moduleptr->info->name);
			irc_prefmsg (ns_botptr, cmdparams->source, __("Bot: %s", cmdparams->source), botptr->name);
		}
		cm = list_first (botptr->u->user->chans);
		i = 0;
		while (cm) {
			if (i==0) {
				irc_chanalert (ns_botptr, _("Channels: %s"), (char *) lnode_get (cm));
			} else {
				irc_chanalert (ns_botptr, "          %s", (char *) lnode_get (cm));
			}
			cm = list_next (botptr->u->user->chans, cm);
			i++;
		}
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("End of Module Bot List", cmdparams->source));
	return NS_SUCCESS;
}

/** @brief DelModuleBots
 *
 *  delete all bots associated with a given module
 *  NeoStats core use only.
 *
 *  @param mod_ptr pointer to module
 *
 *  @return none
 */

void DelModuleBots (Module *mod_ptr)
{
	Bot *botptr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, bothash);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		botptr = hnode_get (modnode);
		if (botptr->moduleptr == mod_ptr) {
			dlog (DEBUG1, "Deleting module %s bot %s", mod_ptr->info->name, botptr->name);
			irc_quit (botptr, _("Module Unloaded"));
		}
	}
	return;
}

/** @brief new_bot
 *
 *  allocate a new bot
 *  Bot subsystem use only.
 *
 *  @param bot_name string containing bot name
 * 
 *  @return none
 */

static Bot *new_bot (const char *bot_name)
{
	Bot *botptr;

	SET_SEGV_LOCATION();
	if (hash_isfull (bothash)) {
		nlog (LOG_CRITICAL, "new_bot: bot list is full");
		return NULL;
	}
	dlog (DEBUG2, "new_bot: %s", bot_name);
	botptr = ns_calloc (sizeof (Bot));
	strlcpy (botptr->name, bot_name, MAXNICK);
	hnode_create_insert (bothash, botptr, botptr->name);
	return botptr;
}

/** @brief GetBotNick
 *
 *  check the requested nick
 *  Bot subsystem use only.
 *
 *  @param botinfo pointer to bot description
 *  @param pointer to nick buffer
 *
 *  @return nick or NULL if failed
 */

static char *GetBotNick (BotInfo *botinfo, char *nickbuf)
{
	char* nick;

	/* Check primary nick */
	nick = botinfo->nick;
	if (find_user (nick)) {
		nlog (LOG_WARNING, "Bot nick %s already in use", nick);
		/* Check alternate nick */
		if (botinfo->altnick) {
			nick = botinfo->altnick;
			if (find_user (nick)) {
				nlog (LOG_WARNING, "Bot alt nick %s already in use", nick);
				/* TODO: try and find a free nick */
				return NULL;
			}
		} else {
			/* TODO: try and find a free nick */
			return NULL;
		}
	} 
	strlcpy (nickbuf, nick, MAXNICK);
	return nickbuf;
}

/** @brief ConnectBot
 *
 *  Connect bot to IRC
 *  Bot subsystem use only.
 *
 *  @param botptr pointer to bot 
 *
 *  @return none
 */

static void ConnectBot (Bot *botptr)
{
	if (botptr->flags&BOT_FLAG_SERVICEBOT) {
		irc_nick (botptr->name, botptr->u->user->username, botptr->u->user->hostname, botptr->u->info, me.servicesumode);
		UserMode (botptr->name, me.servicesumode);
		if (nsconfig.joinserviceschan) {
			irc_join (botptr, me.serviceschan, me.servicescmode);
		}
	} else {
		irc_nick (botptr->name, botptr->u->user->username, botptr->u->user->hostname, botptr->u->info, "");
		if ((nsconfig.allbots > 0)) {
			irc_join (botptr, me.serviceschan, me.servicescmode);
		}
	}	
	if (botptr->flags & BOT_FLAG_DEAF) {
		if (HaveUmodeDeaf()) {
			/* Set deaf mode at IRCd level */
			irc_umode (botptr, botptr->name, UMODE_DEAF);
		} else {
			/* No ircd support, so fake it internally */
			botptr->u->user->Umode |= UMODE_DEAF;
		}
	}
}

/** @brief AddBot
 *
 *  Add a new bot
 *  NeoStats core use and Module API call.
 *
 *  @param botinfo pointer to bot description
 *
 *  @return pointer to bot or NULL if failed
 */

Bot *AddBot (BotInfo *botinfo)
{
	static char nick[MAXNICK];
	Bot *botptr; 
	Module *modptr;

	SET_SEGV_LOCATION();
	modptr = GET_CUR_MODULE();
	if (!modptr->insynch) {
		nlog (LOG_WARNING, "Module %s attempted to init a bot %s but is not yet synched", modptr->info->name, botinfo->nick);
		modptr->error = 1;
		return NULL;
	}
	/* In single bot mode, just add all commands and settings to main bot */
	if (nsconfig.singlebotmode && ns_botptr) {
		add_bot_cmd_list (ns_botptr, botinfo->bot_cmd_list);
		add_bot_setting_list (ns_botptr, botinfo->bot_setting_list);
		return(ns_botptr);
	}
	if (GetBotNick (botinfo, nick) == NULL) {
		nlog (LOG_WARNING, "Unable to init bot");
		return NULL;
	}
	botptr = new_bot (nick);
	if (!botptr) {
		nlog (LOG_WARNING, "new_bot failed for module %s bot %s", modptr->info->name, nick);
		return NULL;
	}
	botptr->moduleptr = modptr;
	botptr->set_ulevel = NS_ULEVEL_ROOT;
	/* For more efficient transversal of bot/user lists, link 
	 * associated user struct to bot and link bot into user struct */
	botptr->u = AddUser (botptr->name, botinfo->user, ((*botinfo->host) == 0 ? me.name : botinfo->host), botinfo->realname, me.name, NULL, NULL, NULL);
	botptr->u->user->bot = botptr;
	botptr->flags = botinfo->flags;
	ConnectBot (botptr);
	/* Only add commands and settings for service bots */
	if (botptr->flags & BOT_FLAG_SERVICEBOT) {
		add_bot_cmd_list (botptr, botinfo->bot_cmd_list);
		add_bot_setting_list (botptr, botinfo->bot_setting_list);
		add_bot_info_settings (botptr, botinfo);
		if (botptr->moduleptr->info->flags & MODULE_FLAG_LOCAL_EXCLUDES) {
			add_bot_cmd_list (botptr, mod_exclude_commands);
		}
	}
	return botptr;
}

/** @brief handle_dead_channel
 *
 *  remove bots from dead channel
 *  NeoStats core use only.
 *
 *  @param channel to process
 *
 *  @return none
 */

void handle_dead_channel (Channel *c)
{
	CmdParams *cmdparams;
	hnode_t *bn;
	hscan_t bs;
	lnode_t *cm;
	char *chan;

	SET_SEGV_LOCATION();
	/* If services channel ignore it */
	if (IsServicesChannel( c )) {
		return;
	}
	/* If channel has persistent bot(s) ignore it */
	if (c->persistentusers) {
		return;
	}
	hash_scan_begin (&bs, bothash);
	cmdparams = ns_calloc (sizeof (CmdParams));
	cmdparams->channel = c;
	while ((bn = hash_scan_next (&bs)) != NULL) {
		cmdparams->bot = hnode_get (bn);
		cm = list_first (cmdparams->bot->u->user->chans);
		while (cm) {
			chan = (char *) lnode_get (cm);
			if (ircstrcasecmp (cmdparams->channel->name, chan) == 0) {
				/* Force the bot to leave the channel */
				irc_part (cmdparams->bot, cmdparams->channel->name);
				/* Tell the module we kicked them out */
				SendModuleEvent (EVENT_EMPTYCHAN, cmdparams, cmdparams->bot->moduleptr);
				break;
			}
			cm = list_next (cmdparams->bot->u->user->chans, cm);
		}
	}
	free (cmdparams);
}
