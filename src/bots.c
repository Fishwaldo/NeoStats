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

#define MAX_CMD_LINE_LENGTH		350

/* @brief Module Bot hash list */
static hash_t *bh;
/* @brief Module Chan Bot hash list */
static hash_t *bch;

int InitBots (void)
{
	bh = hash_create (B_TABLE_SIZE, 0, 0);
	if(!bh)
		return NS_FAILURE;
	bch = hash_create (C_TABLE_SIZE, 0, 0);
	if(!bch)
		return NS_FAILURE;
	return NS_SUCCESS;
}

int FiniBots (void)
{
	hash_destroy(bh);
	hash_destroy(bch);
}

/** @brief Add a bot to a channel
 *
 * @param bot string containing bot name
 * @param chan string containing channel name
 * 
 * @return none
 */
void
add_bot_to_chan (char *bot, char *chan)
{
	hnode_t *cbn;
	ModChanBot *mod_chan_bot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		mod_chan_bot = malloc (sizeof (ModChanBot));
		strlcpy (mod_chan_bot->chan, chan, CHANLEN);
		mod_chan_bot->bots = list_create (B_TABLE_SIZE);
		cbn = hnode_create (mod_chan_bot);
		if (hash_isfull (bch)) {
			nlog (LOG_CRITICAL, LOG_CORE, "add_bot_to_chan: eek, bot channel hash is full");
			return;
		}
		hash_insert (bch, cbn, mod_chan_bot->chan);
	} else {
		mod_chan_bot = hnode_get (cbn);
	}
	if (list_isfull (mod_chan_bot->bots)) {
		nlog (LOG_CRITICAL, LOG_CORE, "add_bot_to_chan: bot channel list is full adding channel %s", chan);
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
del_bot_from_chan (char *bot, char *chan)
{
	hnode_t *cbn;
	ModChanBot *mod_chan_bot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		nlog (LOG_WARNING, LOG_CORE, "del_bot_from_chan: can't find channel %s for botchanhash", chan);
		return;
	}
	mod_chan_bot = hnode_get (cbn);
	bmn = list_find (mod_chan_bot->bots, bot, comparef);
	if (!bmn) {
		nlog (LOG_WARNING, LOG_CORE, "del_bot_from_chan: can't find bot %s in %s in botchanhash", bot, chan);
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

void handle_ctcp_private (ModUser *mod_usr, User* u, char* msg)
{

}

void handle_ctcp_notice (ModUser *mod_usr, User* u, char* msg)
{

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
	int argc;
	char **argv;
	User *u;
	User *bot_user;
	ModUser *mod_usr;

	SET_SEGV_LOCATION();
	u = finduser (origin);
	if(!u) {
		return;
	}
	if (flood (u)) {
		return;
	}
	bot_user = finduser(av[0]);
	if (!bot_user) {
		nlog (LOG_DEBUG1, LOG_CORE, "bot_message: bot %s not found", av[0]);
		return;
	}
	mod_usr = findbot (bot_user->nick);
	/* Check to see if any of the Modules have this nick Registered */
	if (!mod_usr) {
		nlog (LOG_DEBUG1, LOG_CORE, "bot_message: mod_usr %s not found", bot_user->nick);
		return;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "bot_message: bot %s", mod_usr->nick);

	if (av[ac - 1][0] == '\1') {
		handle_ctcp_notice (mod_usr, u, av[ac - 1]);
		nlog (LOG_NORMAL, LOG_MOD, "%s requested %s", u->nick, av[1]);
		return;
	}

	argc = 0;
	AddStringToList (&argv, origin, &argc);
	AddStringToList (&argv, av[0], &argc);
	AddStringToList (&argv, av[ac-1], &argc);
	if(av[0][0] == '#') {
		SendModuleEvent (EVENT_CNOTICE, argv, argc);
	} else {
		SendModuleEvent (EVENT_NOTICE, argv, argc);
	}
	free (argv);
}

/** @brief send a message to a bot
 *
 * @param origin 
 * @param av 
 * @param ac
 * 
 * @return none
 */
void bot_message (char *origin, char **av, int ac)
{
	int argc;
	char **argv;
	User *u;
	User *bot_user;
	ModUser *mod_usr;

	SET_SEGV_LOCATION();
	u = finduser (origin);
	if(!u) {
		return;
	}
	if (flood (u)) {
		return;
	}
	bot_user = finduser(av[0]);
	if (!bot_user) {
		nlog (LOG_DEBUG1, LOG_CORE, "bot_message: bot %s not found", av[0]);
		return;
	}
	mod_usr = findbot (bot_user->nick);
	/* Check to see if any of the Modules have this nick Registered */
	if (!mod_usr) {
		nlog (LOG_DEBUG1, LOG_CORE, "bot_message: mod_usr %s not found", bot_user->nick);
		return;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "bot_message: bot %s", mod_usr->nick);

	if (av[ac - 1][0] == '\1') {
		handle_ctcp_private (mod_usr, u, av[ac - 1]);
		nlog (LOG_NORMAL, LOG_MOD, "%s requested %s", u->nick, av[1]);
		return;
	}

	if (mod_usr->botcmds) {
		if (setjmp (sigvbuf) == 0) {
			int ret;

			SET_SEGV_INMODULE(mod_usr->modname);
			ret = run_bot_cmd(mod_usr, u, av[ac - 1]);
			CLEAR_SEGV_INMODULE();
			if(ret == NS_SUCCESS)
				return;
		}
	}
	argc = 0;
	AddStringToList (&argv, origin, &argc);
	AddStringToList (&argv, av[0], &argc);
	AddStringToList (&argv, av[ac-1], &argc);
	if(av[0][0] == '#') {
		SendModuleEvent (EVENT_CNOTICE, argv, argc);
	} else {
		SendModuleEvent (EVENT_NOTICE, argv, argc);
	}
	free (argv);
}

/** @brief dump list of module bots and channels
 *
 * @param u
 * 
 * @return none
 */
int
list_bot_chans (User * u, char **av, int ac)
{
	hscan_t hs;
	hnode_t *hn;
	lnode_t *ln;
	ModChanBot *mod_chan_bot;

	prefmsg (u->nick, s_Services, "BotChanDump:");
	hash_scan_begin (&hs, bch);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		mod_chan_bot = hnode_get (hn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_chan_bot->chan);
		ln = list_first (mod_chan_bot->bots);
		while (ln) {
			prefmsg (u->nick, s_Services, "Bot Name: %s", (char *)lnode_get (ln));
			ln = list_next (mod_chan_bot->bots, ln);
		}
	}
	return 0;
}

/** @brief create a new bot
 *
 * @param bot_name string containing bot name
 * 
 * @return none
 */
static ModUser *
new_bot (char *bot_name)
{
	ModUser *mod_usr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "new_bot: %s", bot_name);
	mod_usr = malloc (sizeof (ModUser));
	strlcpy (mod_usr->nick, bot_name, MAXNICK);
	bn = hnode_create (mod_usr);
	if (hash_isfull (bh)) {
		chanalert (s_Services, "Warning ModuleBotlist is full");
		return NULL;
	}
	hash_insert (bh, bn, mod_usr->nick);
	return mod_usr;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
ModUser *
add_mod_user (char *nick, char *mod_name)
{
	ModUser *mod_usr;
	Module *mod_ptr;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	mod_ptr = get_mod_ptr(mod_name);
	if(mod_ptr) {
		/* add a brand new user */
		mod_usr = new_bot (nick);
		if(mod_usr) {
			strlcpy (mod_usr->modname, mod_name, MAX_MOD_NAME);
			mod_usr->botcmds = NULL;
			return mod_usr;
		}
	}
	nlog (LOG_WARNING, LOG_CORE, "add_mod_user: Couldn't Add ModuleBot to List");
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
ModUser *
add_neostats_mod_user (char *nick)
{
	ModUser *mod_usr;

	SET_SEGV_LOCATION();
	/* add a brand new user */
	mod_usr = new_bot (nick);
	if(mod_usr) {
#if 0
		strlcpy (mod_usr->modname, "NeoStats", MAX_MOD_NAME);
#else
		mod_usr->modname[0] = 0;
#endif
		mod_usr->botcmds = hash_create(-1, 0, 0);
		return mod_usr;
	}
	nlog (LOG_WARNING, LOG_CORE, "add_neostats_mod_user: Couldn't Add ModuleBot to List");
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
ModUser *
findbot (char *bot_name)
{
	hnode_t *bn;

	SET_SEGV_LOCATION();

	bn = hash_lookup (bh, bot_name);
	if (bn) {
		return (ModUser *) hnode_get (bn);
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
del_mod_user (char *bot_name)
{
	ModUser *mod_usr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		hash_delete (bh, bn);
		mod_usr = hnode_get (bn);
		del_all_bot_cmds(mod_usr);
		hnode_destroy (bn);
		free (mod_usr);
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
	ModUser *mod_usr_new, *mod_usr;

	SET_SEGV_LOCATION();
	/* First, try to find out if the newnick is unique! */
	u = finduser (oldnick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "A non-registered bot(%s) attempted to change its nick to %s", oldnick, newnick);
		return NS_FAILURE;
	}
	u = finduser (newnick);
	if (!u) {
		if ((mod_usr = findbot (oldnick)) != NULL) {
			nlog (LOG_DEBUG3, LOG_CORE, "Bot %s Changed its nick to %s", oldnick, newnick);
			mod_usr_new = new_bot (newnick);

			/* add a brand new user */ 
			strlcpy (mod_usr_new->nick, newnick, MAXNICK);
			strlcpy (mod_usr_new->modname, mod_usr->modname, MAX_MOD_NAME);

			/* Now Delete the Old bot nick */   
			del_mod_user (oldnick);
			snick_cmd (oldnick, newnick);
			return NS_SUCCESS;
		}
	}
	nlog (LOG_NOTICE, LOG_CORE, "Couldn't find Bot Nick %s in Bot list", oldnick);
	return NS_FAILURE;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
list_bots (User * u, char **av, int ac)
{
	ModUser *mod_usr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Module Bot List:");
	hash_scan_begin (&bs, bh);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		mod_usr = hnode_get (bn);
		if(mod_usr->modname[0] == 0) {
			prefmsg (u->nick, s_Services, "NeoStats");
			prefmsg (u->nick, s_Services, "Bots: %s", mod_usr->nick);
		} else {
			prefmsg (u->nick, s_Services, "Module: %s", mod_usr->modname);
			prefmsg (u->nick, s_Services, "Module Bots: %s", mod_usr->nick);
		}
	}
	prefmsg (u->nick, s_Services, "End of Module Bot List");
	return 0;
}

/** @brief del_bots
 *
 * 
 *
 * @return none
 */
int	del_bots (char* module_name)
{
	ModUser *mod_usr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, bh);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_usr = hnode_get (modnode);
		if (!ircstrcasecmp (mod_usr->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "Module %s had bot %s Registered. Deleting..", module_name, mod_usr->nick);
			del_bot (mod_usr->nick, "Module Unloaded");
		}
	}
	return NS_SUCCESS;
}
