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
#include "services.h"
#include "commands.h"
#include "bots.h"

/** @brief Channel bot structure
 * 
 */
typedef struct ChanBot {
	/** channel name */
	char chan[MAXCHANLEN];
	/** bot list */
	list_t *bots;
}ChanBot;

#define MAX_CMD_LINE_LENGTH		350

/* @brief Module Bot hash list */
static hash_t *bothash;
/* @brief Module Chan Bot hash list */
static hash_t *botchanhash;

int InitBots (void)
{
	bothash = hash_create (B_TABLE_SIZE, 0, 0);
	if(!bothash) {
		nlog (LOG_CRITICAL, "Unable to create bot hash");
		return NS_FAILURE;
	}
	botchanhash = hash_create (C_TABLE_SIZE, 0, 0);
	if(!botchanhash) {
		nlog (LOG_CRITICAL, "Unable to create bot channel hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

int FiniBots (void)
{
	hash_destroy(bothash);
	hash_destroy(botchanhash);
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
add_chan_bot (const char *bot, const char *chan)
{
	hnode_t *cbn;
	ChanBot *chanbot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (botchanhash, chan);
	if (!cbn) {
		if (hash_isfull (botchanhash)) {
			nlog (LOG_CRITICAL, "add_chan_bot: bot channel hash is full");
			return;
		}
		chanbot = smalloc (sizeof (ChanBot));
		strlcpy (chanbot->chan, chan, MAXCHANLEN);
		chanbot->bots = list_create (B_TABLE_SIZE);
		cbn = hnode_create (chanbot);
		hash_insert (botchanhash, cbn, chanbot->chan);
	} else {
		chanbot = hnode_get (cbn);
	}
	if (list_isfull (chanbot->bots)) {
		nlog (LOG_CRITICAL, "add_chan_bot: bot channel list is full adding channel %s", chan);
		return;
	}
	botname = sstrdup (bot);
	bmn = lnode_create (botname);
	list_append (chanbot->bots, bmn);
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
del_chan_bot (const char *bot, const char *chan)
{
	hnode_t *cbn;
	ChanBot *chanbot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (botchanhash, chan);
	if (!cbn) {
		nlog (LOG_WARNING, "del_chan_bot: can't find channel %s for botchanhash", chan);
		return;
	}
	chanbot = hnode_get (cbn);
	bmn = list_find (chanbot->bots, bot, comparef);
	if (!bmn) {
		nlog (LOG_WARNING, "del_chan_bot: can't find bot %s in %s in botchanhash", bot, chan);
		return;
	}
	list_delete (chanbot->bots, bmn);
	botname = lnode_get(bmn);
	free (botname);
	lnode_destroy (bmn);
	if (list_isempty (chanbot->bots)) {
		/* delete the hash and list because its all over */
		hash_delete (botchanhash, cbn);
		list_destroy (chanbot->bots);
		sfree (chanbot->chan);
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
	ChanBot *chanbot;

	prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "BotChanDump:");
	hash_scan_begin (&hs, botchanhash);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		chanbot = hnode_get (hn);
		prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "%s:--------------------------------", chanbot->chan);
		ln = list_first (chanbot->bots);
		while (ln) {
			prefmsg (cmdparams->source.user->nick, ns_botptr->nick, "Bot Name: %s", (char *)lnode_get (ln));
			ln = list_next (chanbot->bots, ln);
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
	dlog(DEBUG1, "process_target_user: user %s not found", target);
	return 0;
}

int process_target_chan(CmdParams * cmdparams, char* target)
{
	cmdparams->channel = findchan(target);
	if(cmdparams->channel) {
		return 1;
	}
	dlog(DEBUG1, "cmdparams->channel: chan %s not found", target);
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
			SendModuleEvent (EVENT_NOTICE, cmdparams, cmdparams->dest.bot->moduleptr);
		}		
		if (!strncasecmp(av[ac - 1], "\1version", 8)) {
			/* skip "\1version " */
			cmdparams->param += 9;
			strlcpy(cmdparams->source.user->version, cmdparams->param, MAXHOST);
 			SendModuleEvent (EVENT_CTCPVERSION, cmdparams, cmdparams->dest.bot->moduleptr);
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
			SendModuleEvent (EVENT_CNOTICE, cmdparams, cmdparams->dest.bot->moduleptr);
			if (av[ac - 1][0] == '\1') {
				/* TODO CTCP handler */
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
	CmdParams * cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	if(process_origin(cmdparams, origin)) {
		/* Find target bot */
		if (process_target_user(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			if ((cmdparams->dest.bot->flags & BOT_FLAG_SERVICEBOT)) {
				if(run_bot_cmd (cmdparams) != NS_FAILURE) {
					sfree (cmdparams);
					return;
				}
			}
			SendModuleEvent (EVENT_PRIVATE, cmdparams, cmdparams->dest.bot->moduleptr);
			if (av[ac - 1][0] == '\1') {
				/* TODO CTCP handler */
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
void bot_chan_private (char *origin, char **av, int ac)
{
	CmdParams * cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	if(process_origin(cmdparams, origin)) {
		if(process_target_chan(cmdparams, av[0])) {
			cmdparams->param = av[ac - 1];
			SendModuleEvent (EVENT_CPRIVATE, cmdparams, cmdparams->dest.bot->moduleptr);
			if (av[ac - 1][0] == '\1') {
				/* TODO CTCP handler */
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
	hnode_t *bn;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "new_bot: %s", bot_name);
	botptr = scalloc (sizeof (Bot));
	strlcpy (botptr->nick, bot_name, MAXNICK);
	bn = hnode_create (botptr);
	if (hash_isfull (bothash)) {
		nlog (LOG_CRITICAL, "new_bot: bot list is full");
		return NULL;
	}
	hash_insert (bothash, bn, botptr->nick);
	return botptr;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
Bot *
add_ns_bot (Module* modptr, const char *nick)
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
findbot (const char *bot_name)
{
	hnode_t *bn;

	SET_SEGV_LOCATION(); 
	bn = hash_lookup (bothash, bot_name);
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
del_ns_bot (const char *bot_name)
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bothash, bot_name);
	if (bn) {
		hash_delete (bothash, bn);
		botptr = hnode_get (bn);
		del_all_bot_cmds(botptr);
		del_all_bot_settings(botptr);
		del_bot_info_settings(botptr);
		hnode_destroy (bn);
		sfree (botptr);
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
bot_nick_change (const char *oldnick, const char *newnick)
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	/* First, try to find out if the newnick is unique! */
	if (!finduser (oldnick)) {
		nlog (LOG_WARNING, "Unknown bot %s tried to change nick to %s", oldnick, newnick);
		return NS_FAILURE;
	}
	if (finduser (newnick)) {
		nlog (LOG_WARNING, "Bot %s tried to change nick to one that already exists %s", oldnick, newnick);
		return NS_FAILURE;
	}
	bn = hash_lookup (bothash, oldnick);
	if (!bn) {
		nlog (LOG_NOTICE, "Couldn't find bot %s in bot list", oldnick);
		return NS_FAILURE;
	}
	botptr = hnode_get (bn);
	/* remove old hash entry */
	hash_delete (bothash, bn);
	dlog(DEBUG3, "Bot %s changed nick to %s", oldnick, newnick);
	strlcpy (botptr->nick, newnick, MAXNICK);
	/* insert new hash entry */
	hash_insert (bothash, bn, botptr->nick);
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
	hash_scan_begin (&bs, bothash);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		botptr = hnode_get (bn);
		if((botptr->flags & 0x80000000)) {
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

	hash_scan_begin (&hscan, bothash);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		botptr = hnode_get (modnode);
		if (botptr->moduleptr == mod_ptr) {
			dlog(DEBUG1, "Module %s had bot %s Registered. Deleting..", mod_ptr->info->name, botptr->nick);
			del_bot (botptr, "Module Unloaded");
		}
	}
	return NS_SUCCESS;
}

/** @brief init_bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
Bot *init_bot (BotInfo* botinfo)
{
	Bot * botptr; 
	User *u;
	long Umode;
	char* nick;
	Module* modptr;
	char* host;

	SET_SEGV_LOCATION();
	/* When we are using single bot mode, for example in client mode where we
	 * can only use one bot, if we have initialised the main bot just add all 
	 * commands and settings to it 
	 */
	if(config.singlebotmode && ns_botptr) {
		add_bot_cmd_list (ns_botptr, botinfo->bot_cmd_list);
		add_bot_setting_list (ns_botptr, botinfo->bot_setting_list);
		return(ns_botptr);
	}
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
	host = ((*botinfo->host)==0?me.name:botinfo->host);
	if(botinfo->flags&BOT_FLAG_SERVICEBOT) {
		Umode = UmodeStringToMask(me.servicesumode, 0);
		signon_newbot (nick, botinfo->user, host, botinfo->realname, me.servicesumode, Umode);
	} else {
		signon_newbot (nick, botinfo->user, host, botinfo->realname, "", 0);
	}
	u = finduser (nick);
	/* set our link back to user struct for bot */
	botptr->u = u;
	if (HaveUmodeDeaf()&&(botinfo->flags&BOT_FLAG_DEAF)) {
		sumode_cmd (nick, nick, UMODE_DEAF);
	}
	botptr->flags = botinfo->flags;
	SET_RUN_LEVEL(modptr);
	if (botinfo->bot_cmd_list) {
		add_bot_cmd_list (botptr, botinfo->bot_cmd_list);
	}
	if (botinfo->bot_setting_list) {
		add_bot_setting_list (botptr, botinfo->bot_setting_list);
	}
	add_bot_info_settings (botptr, botinfo);
	RESET_RUN_LEVEL();
	return botptr;
}

/** @brief del_bot
 *
 * 
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
del_bot (Bot *botptr, const char *reason)
{
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (botptr->nick);
	if (!u) {
		nlog (LOG_WARNING, "Attempting to delete a bot with a nick that does not exist: %s", botptr->nick);
		return NS_FAILURE;
	}
	dlog(DEBUG1, "Deleting bot %s for %s", botptr->nick, reason);
	//XXXX TODO: need to free the channel list hash. We dont according to valgrind
	squit_cmd (botptr->nick, reason);
	del_ns_bot (botptr->nick);
	return NS_SUCCESS;
}
