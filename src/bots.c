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

/*	TODO:
 *		- find free nick for bots if NICK and ALTNICK are in use
 *		- CTCP handler
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

#define MAX_CMD_LINE_LENGTH		350

/* @brief Module Bot hash list */
static hash_t *bothash;

/** @brief InitBots 
 *
 *  initialise bot systems
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int InitBots (void)
{
	bothash = hash_create (B_TABLE_SIZE, 0, 0);
	if(!bothash) {
		nlog (LOG_CRITICAL, "Unable to create bot hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniBots
 *
 *  cleanup bot systems
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int FiniBots (void)
{
	hash_destroy(bothash);
	return NS_SUCCESS;
}

/** @brief flood_test
 *
 *  Check whether client is flooding NeoStats
 *
 *  @param pointer to client to test
 *
 *  @return NS_TRUE if client flooded, NS_FALSE if not 
 */
int
flood_test (Client * u)
{
	/* sanity test */
	if (!u) {
		nlog (LOG_WARNING, "flood_test: NULL user");
		return NS_FALSE;
	}
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
		irc_svskill (u, "%s!%s (Flooding Services.)", me.name, ns_botptr->name);
		return NS_TRUE;
	}
	u->user->flood++;
	return NS_FALSE;
}

/** @brief process_origin
 *
 *  validate message origin and populate cmdparams structure
 *
 *  @param cmdparam structure to populate 
 *  @param origin nick or server name 
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */
int process_origin(CmdParams * cmdparams, char* origin)
{
	cmdparams->source = find_user (origin);
	if(cmdparams->source) {
		if (flood_test (cmdparams->source)) {
			return NS_FALSE;
		}
		return NS_TRUE;
	}
	cmdparams->source = find_server (origin);
	if(cmdparams->source) {
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief process_target_user
 *
 *  validate message target and populate cmdparams structure
 *
 *  @param cmdparam structure to populate 
 *  @param target nick
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */
int process_target_user(CmdParams * cmdparams, char* target)
{
	cmdparams->target = find_user (target);
	if(cmdparams->target) {
		cmdparams->bot = cmdparams->target->user->bot;
		if(cmdparams->bot) {
			return NS_TRUE;
		}
	}
	dlog(DEBUG1, "process_target_user: user %s not found", target);
	return NS_FALSE;
}

/** @brief process_target_chan
 *
 *  validate message target and populate cmdparams structure
 *
 *  @param cmdparam structure to populate 
 *  @param target channel
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */
int process_target_chan(CmdParams * cmdparams, char* target)
{
	cmdparams->channel = find_chan(target);
	if(cmdparams->channel) {
		return NS_TRUE;
	}
	dlog(DEBUG1, "cmdparams->channel: chan %s not found", target);
	return NS_FALSE;
}

/** @brief list_bots
 *
 *  list all neostats bots
 *
 *  @param cmdparams pointer to command parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
bot_chan_event (Event event, CmdParams* cmdparams)
{
	lnode_t *cm;
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	hash_scan_begin (&bs, bothash);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		botptr = hnode_get (bn);
		cm = list_first (botptr->u->user->chans);
		if (!(botptr->u->user->Umode & UMODE_DEAF)) {
			while (cm) {
				char* chan;

				chan = (char *) lnode_get (cm);
				cmdparams->bot = botptr;
				if (ircstrcasecmp(cmdparams->channel->name, chan) == 0) {
					SendModuleEvent (event, cmdparams, botptr->moduleptr);
				}
				cm = list_next (botptr->u->user->chans, cm);
			}
		}
	}
	return NS_SUCCESS;
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
			if (av[ac - 1][0] == '\1') {
				ctcp_notice (cmdparams);
			} else {
				SendModuleEvent (EVENT_NOTICE, cmdparams, cmdparams->bot->moduleptr);
			}
		}		
	}
	sfree (cmdparams);

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
			if (av[ac - 1][0] == '\1') {
				ctcp_cnotice (cmdparams);
			} else {
				bot_chan_event (EVENT_CNOTICE, cmdparams);
			}
		}
	}
	sfree (cmdparams);
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
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	if(process_origin(cmdparams, origin)) {
		/* Find target bot */
		if (process_target_user(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			/* Check CTCP first to avoid Unknown command messages later */
			if (av[ac - 1][0] == '\1') {
				ctcp_private (cmdparams);
				return;
			} 			
			if ((cmdparams->bot->flags & BOT_FLAG_SERVICEBOT)) {
				if(run_bot_cmd (cmdparams) != NS_FAILURE) {
					sfree (cmdparams);
					return;
				}
			}
			SendModuleEvent (EVENT_PRIVATE, cmdparams, cmdparams->bot->moduleptr);
		}
	}
	sfree (cmdparams);
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
			if (av[ac - 1][0] == '\1') {
				ctcp_cprivate (cmdparams);
			} else {
				bot_chan_event (EVENT_CNOTICE, cmdparams);
			}
		}
	}
	sfree (cmdparams);
}

/** @brief create a new bot
 *
 * @param bot_name string containing bot name
 * 
 * @return none
 */
static Bot *
new_bot (const char *bot_name)
{
	Bot *botptr;

	SET_SEGV_LOCATION();
	if (hash_isfull (bothash)) {
		nlog (LOG_CRITICAL, "new_bot: bot list is full");
		return NULL;
	}
	dlog(DEBUG2, "new_bot: %s", bot_name);
	botptr = scalloc (sizeof (Bot));
	strlcpy (botptr->name, bot_name, MAXNICK);
	hnode_create_insert (bothash, botptr, botptr->name);
	return botptr;
}

/** @brief add_bot
 *
 *  allocate bot
 *
 *  @param modptr pointer to module requesting bot
 *  @param nick name of new bot
 *
 *  @return pointer to bot or NULL if not found
 */
Bot *
add_bot (Module* modptr, const char *nick)
{
	Bot *botptr;

	SET_SEGV_LOCATION();
	/* add a brand new user */
	botptr = new_bot (nick);
	if(botptr) {
		botptr->moduleptr = modptr;
		botptr->set_ulevel = NS_ULEVEL_ROOT;
		return botptr;
	}
	nlog (LOG_WARNING, "add_bot: Couldn't Add Bot to List");
	return NULL;
}

/** @brief find_bot
 *
 *  find bot
 *
 *  @param bot_name name of bot to find
 *
 *  @return pointer to bot or NULL if not found
 */
Bot *
find_bot (const char *bot_name)
{
	Bot* bot;

	SET_SEGV_LOCATION(); 
	bot = (Bot *)hnode_find (bothash, bot_name);
	if (!bot) {
		dlog(DEBUG3, "find_bot: %s not found", bot_name);
	}
	return bot;
}

/** @brief del_bot
 *
 *  delete bot
 *
 *  @param bot_name name of bot to delete
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
del_bot (const char *bot_name)
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bothash, bot_name);
	if (bn) {
		botptr = hnode_get (bn);
		del_all_bot_cmds(botptr);
		del_bot_info_settings(botptr);
		del_all_bot_settings(botptr);
		hash_delete (bothash, bn);
		hnode_destroy (bn);
		sfree (botptr);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief bot_nick_change
 *
 *  change bot nick
 *
 *  @param botptr pointer to bot
 *  @param newnick new nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
bot_nick_change (const Bot *botptr, const char *newnick)
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
	dlog(DEBUG3, "Bot %s changed nick to %s", botptr->name, newnick);
	strlcpy (botptr->name, newnick, MAXNICK);
	/* insert new hash entry */
	hash_insert (bothash, bn, botptr->name);
	return NS_SUCCESS;
}

/** @brief list_bots
 *
 *  list all neostats bots
 *
 *  @param cmdparams pointer to command parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
list_bots (CmdParams* cmdparams)
{
	int i;
	lnode_t *cm;
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, "Module Bot List:");
	hash_scan_begin (&bs, bothash);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		botptr = hnode_get (bn);
		if((botptr->flags & 0x80000000)) {
			irc_prefmsg (ns_botptr, cmdparams->source, "NeoStats");
			irc_prefmsg (ns_botptr, cmdparams->source, "Bot: %s", botptr->name);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, "Module: %s", botptr->moduleptr->info->name);
			irc_prefmsg (ns_botptr, cmdparams->source, "Bot: %s", botptr->name);
		}
		cm = list_first (botptr->u->user->chans);
		i = 0;
		while (cm) {
			if(i==0) {
				irc_chanalert (ns_botptr, "Channels: %s", (char *) lnode_get (cm));
			} else {
				irc_chanalert (ns_botptr, "          %s", (char *) lnode_get (cm));
			}
			cm = list_next (botptr->u->user->chans, cm);
			i++;
		}
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "End of Module Bot List");
	return NS_SUCCESS;
}

/** @brief del_module_bots
 *
 *  delete all bots associated with a given module
 *
 *  @param mod_ptr pointer to module
 *
 *  @return none
 */
void del_module_bots (Module *mod_ptr)
{
	Bot *botptr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, bothash);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		botptr = hnode_get (modnode);
		if (botptr->moduleptr == mod_ptr) {
			dlog(DEBUG1, "Deleting module %s bot %s", mod_ptr->info->name, botptr->name);
			irc_quit (botptr, "Module Unloaded");
		}
	}
	return;
}

/** @brief init_bot
 *
 *  initialise a new bot
 *
 *  @param botinfo pointer to bot description
 *
 *  @return pointer to bot or NULL if failed
 */
Bot *init_bot (BotInfo* botinfo)
{
	Bot * botptr; 
	char* nick;
	Module* modptr;
	char* host;

	SET_SEGV_LOCATION();
	modptr = GET_CUR_MODULE();
	/* When we are using single bot mode, for example in client mode where we
	 * can only use one bot, if we have initialised the main bot just add all 
	 * commands and settings to it 
	 */
	if(config.singlebotmode && ns_botptr) {
		if (botinfo->bot_cmd_list) {
			SET_RUN_LEVEL(modptr);
			add_bot_cmd_list (ns_botptr, botinfo->bot_cmd_list);
			RESET_RUN_LEVEL();
		}
		if (botinfo->bot_setting_list) {
			SET_RUN_LEVEL(modptr);
			add_bot_setting_list (ns_botptr, botinfo->bot_setting_list);
			RESET_RUN_LEVEL();
		}
		return(ns_botptr);
	}
	nick = botinfo->nick;
	if (find_user (nick)) {
		nlog (LOG_WARNING, "Bot nick %s already in use", botinfo->nick);
		if(botinfo->altnick) {
			nick = botinfo->altnick;
			if(find_user (nick)) {
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
	botptr = add_bot (modptr, nick);
	if (!botptr) {
		nlog (LOG_WARNING, "add_bot failed for module %s bot %s", modptr->info->name, nick);
		return NULL;
	}
	host = ((*botinfo->host)==0?me.name:botinfo->host);
	/* For more efficient transversal of bot/user lists, link 
	 * associated user struct to bot and link bot into user struct */
	botptr->u = AddUser (nick, botinfo->user, host, botinfo->realname, me.name, NULL, NULL, NULL);
	botptr->u->user->bot = botptr;
	if(botinfo->flags&BOT_FLAG_SERVICEBOT) {
		irc_nick (nick, botinfo->user, host, botinfo->realname, me.servicesumode);
		UserMode (nick, me.servicesumode);
		irc_join(botptr, me.serviceschan, me.servicescmode);
	} else {
		irc_nick (nick, botinfo->user, host, botinfo->realname, "");
		if ((config.allbots > 0)) {
			irc_join(botptr, me.serviceschan, me.servicescmode);
		}
	}	
	if (botinfo->flags & BOT_FLAG_DEAF) {
		if (HaveUmodeDeaf()) {
			/* Set deaf mode at IRCd level */
			irc_usermode (botptr, nick, UMODE_DEAF);
		} else {
			/* No ircd support, so fake it internally */
			botptr->u->user->Umode |= UMODE_DEAF;
		}
	}
	botptr->flags = botinfo->flags;
	if (botinfo->bot_cmd_list) {
		SET_RUN_LEVEL(modptr);
		add_bot_cmd_list (botptr, botinfo->bot_cmd_list);
		RESET_RUN_LEVEL();
	}
	if (botinfo->bot_setting_list) {
		SET_RUN_LEVEL(modptr);
		add_bot_setting_list (botptr, botinfo->bot_setting_list);
		RESET_RUN_LEVEL();
	}
	SET_RUN_LEVEL(modptr);
	add_bot_info_settings (botptr, botinfo);
	RESET_RUN_LEVEL();
	return botptr;
}
