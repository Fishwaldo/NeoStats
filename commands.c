/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "stats.h"
#include "dl.h"
#include "log.h"

static void bot_cmd_help (ModUser* bot_ptr, User * u, char **av, int ac);

/* hash for services bot command list */
static hash_t *botcmds = NULL;
static ModUser fake_bot;

/* help title strings for different user levels */
char * help_level_title[]=
{
	"Operators",
	"Service Admins",
	"Service Roots",
};

/* Intrinsic commands 
 * (Work in progress, not yet operation)
 * These are all automatically added to a bot for handling 
 * by the core. A module can override these by defining them
 * in it's local bot command array.
 */
/*
bot_cmd intrinsic_commands[]=
{
	{"HELP",		do_help,		0, 	0,			help_on_help, 	1, 	help_help_oneline},
	{NULL,			NULL,			0, 	0,			NULL, 			0,	NULL}
};
*/

/** @brief add_bot_cmd adds a single command to the command hash
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int 
add_bot_cmd(hash_t* cmd_hash, bot_cmd* cmd_ptr) 
{
	hnode_t *cmdnode;
	
	/* Add the command */
	cmdnode = hnode_create(cmd_ptr);
	if (cmdnode) {
		hash_insert(cmd_hash, cmdnode, cmd_ptr->cmd);
		nlog(LOG_DEBUG2, LOG_CORE, "Added a new command %s to Services Bot", cmd_ptr->cmd);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief del_bot_cmd deltes a single command to the command hash
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int 
del_bot_cmd(hash_t* cmd_hash, bot_cmd* cmd_ptr) 
{
	hnode_t *cmdnode;
	
	/* Delete the command */
	cmdnode = hash_lookup(cmd_hash, cmd_ptr->cmd);
	if (cmdnode) {
		hash_delete(cmd_hash, cmdnode);
		hnode_destroy(cmdnode);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief add_bot_cmd_list adds a list of commands to the command hash
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
add_bot_cmd_list(ModUser* bot_ptr, bot_cmd* cmd_list) 
{
	/* If no hash return failure */
	if(bot_ptr->botcmds == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command list and add them */
	while(cmd_list->cmd) {
		add_bot_cmd(bot_ptr->botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief del_bot_cmd_list delete a list of commands to the command hash
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
del_bot_cmd_list(ModUser* bot_ptr, bot_cmd* cmd_list) 
{
	/* If no hash return failure */
	if(bot_ptr->botcmds == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command list and delete them */
	while(cmd_list->cmd) {
		add_bot_cmd(bot_ptr->botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief add_services_cmd_list adds a list of commands to the services bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
add_services_cmd_list(bot_cmd* cmd_list) 
{
	/* init bot hash if not created */
	if(botcmds == NULL) {
		botcmds = hash_create(-1, 0, 0);
	}
	/* If unable to create hash return failure */
	if(botcmds == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command list and add them */
	while(cmd_list->cmd) {
		add_bot_cmd(botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief del_services_cmd_list delete a list of commands from the services bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
del_services_cmd_list(bot_cmd* cmd_list) 
{
	/* If no hash return failure */
	if(botcmds == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command list and delete them */
	while(cmd_list->cmd) {
		del_bot_cmd(botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief servicesbot process services bot command list
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
void
servicesbot (char *nick, char **av, int ac)
{
	User *u;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Unable to finduser %s (%s)", nick, s_Services);
		return;
	}

	me.requests++;

	/* Check user authority to use this command set */
	if (me.onlyopers && (UserLevel (u) < NS_ULEVEL_OPER)) {
		prefmsg (u->nick, s_Services, "This service is only available to IRCops.");
		chanalert (s_Services, "%s Requested %s, but he is Not an Operator!", u->nick, av[1]);
		return;
	}

	/* Use fake bot structure so we can use the main command routine */
	strlcpy(fake_bot.nick, s_Services, MAXNICK);
	fake_bot.botcmds = botcmds;
	run_bot_cmd (&fake_bot, u, av, ac);
}

/** @brief run_bot_cmd process bot command list
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
void
run_bot_cmd (ModUser* bot_ptr, User *u, char **av, int ac)
{
	bot_cmd* cmd_ptr;
	hnode_t *cmdnode;

	SET_SEGV_LOCATION();
	
	/* Check user authority to use this command set */
	if (me.onlyopers && (UserLevel (u) < NS_ULEVEL_OPER)) {
		prefmsg (u->nick, bot_ptr->nick, "This service is only available to IRCops.");
		chanalert (bot_ptr->nick, "%s Requested %s, but he is Not an Operator!", u->nick, av[1]);
		return;
	}

	/* Process command list */
	cmdnode = hash_lookup(bot_ptr->botcmds, av[1]);

	if (cmdnode) {
		cmd_ptr = hnode_get(cmdnode);
	
		/* Is user authorised to issue this command? */
		if (UserLevel (u) < cmd_ptr->ulevel) {
			prefmsg (u->nick, bot_ptr->nick, "Permission Denied");
			chanalert (bot_ptr->nick, "%s tried to use %s, but is not authorised", u->nick, cmd_ptr->cmd);
			return;
		}
		/* First two parameters are bot name and command name so 
		 * subtract 2 to get parameter count */
		if((ac - 2) < cmd_ptr->minparams ) {
			prefmsg (u->nick, bot_ptr->nick, "Syntax error: insufficient parameters");
			prefmsg (u->nick, bot_ptr->nick, "/msg %s HELP %s for more information", bot_ptr->nick, cmd_ptr->cmd);
			return;
		}
		/* Missing handler?! */
		if(!cmd_ptr->handler) {
			prefmsg (u->nick, bot_ptr->nick, "Unable to find handler for %s", cmd_ptr->cmd);
			chanalert (bot_ptr->nick, "Unable to find handler for %s used by %s", cmd_ptr->cmd, u->nick);
			return;
		}
		/* Seems OK so report the command call then call appropriate handler */
		chanalert (bot_ptr->nick, "%s used %s", u->nick, cmd_ptr->cmd);
		cmd_ptr->handler(u, av, ac);
		return;
	}

	/* Handle intrinsic commands */

	/* Help */
	if (!strcasecmp(av[1], "HELP")) {
		bot_cmd_help(bot_ptr, u, av, ac);
		return;
	}

	/* We have run out of commands so report failure */
	prefmsg (u->nick, bot_ptr->nick, "Syntax error: unknown command: \2%s\2", av[1]);
	chanalert (bot_ptr->nick, "%s requested %s, but that is an unknown command", u->nick, av[1]);
}

/** @brief bot_cmd_help process bot help command
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static void 
bot_cmd_help (ModUser* bot_ptr, User * u, char **av, int ac)
{
	char* curlevelmsg=NULL;
	int donemsg=0;
	bot_cmd* cmd_ptr;
	int curlevel, lowlevel;
	hnode_t *cmdnode;
	hscan_t hs;

	if (ac < 3) {
		lowlevel = 0;
		curlevel = NS_ULEVEL_OPER;
		chanalert (bot_ptr->nick, "%s Requested %s Help", u->nick, bot_ptr->nick);
		prefmsg(u->nick, bot_ptr->nick, "The following commands can be used with %s:", bot_ptr->nick);

		restartlevel:
		hash_scan_begin(&hs, bot_ptr->botcmds);
		while ((cmdnode = hash_scan_next(&hs)) != NULL) {
			cmd_ptr = hnode_get(cmdnode);
			if ((cmd_ptr->ulevel < curlevel) && (cmd_ptr->ulevel >= lowlevel)) {
				if(curlevelmsg && !donemsg) {
					prefmsg(u->nick, bot_ptr->nick, "\2Additional commands available to %s:\2", curlevelmsg);
					donemsg = 1;
				}
				prefmsg(u->nick, bot_ptr->nick, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->onelinehelp);
			}
		}
		if (UserLevel(u) >= curlevel) {
			switch (curlevel) {
				case NS_ULEVEL_OPER:
						curlevel = NS_ULEVEL_ADMIN;
						lowlevel = NS_ULEVEL_OPER;
						curlevelmsg=help_level_title[0];
						donemsg=0;
						goto restartlevel;
				case NS_ULEVEL_ADMIN:
						curlevel = NS_ULEVEL_ROOT;
						lowlevel = NS_ULEVEL_ADMIN;
						curlevelmsg=help_level_title[1];
						donemsg=0;
						goto restartlevel;
				case NS_ULEVEL_ROOT:
						curlevel = 201;
						lowlevel = 200;
						curlevelmsg=help_level_title[2];
						donemsg=0;
						goto restartlevel;
				default:	
						break;
			}
		}						
		prefmsg(u->nick, bot_ptr->nick, " ");
		prefmsg(u->nick, bot_ptr->nick, "To use a command, type");
		prefmsg(u->nick, bot_ptr->nick, "    \2/msg %s command\2", bot_ptr->nick);
		prefmsg(u->nick, bot_ptr->nick, "For for more information on a command, type");
		prefmsg(u->nick, bot_ptr->nick, "    \2/msg %s HELP command\2.", bot_ptr->nick);
		return;
	}
	chanalert (bot_ptr->nick, "%s Requested %s Help on %s", u->nick, bot_ptr->nick, av[2]);

	/* Process command list */
	cmdnode = hash_lookup(bot_ptr->botcmds, av[2]);
	if (cmdnode) {
		cmd_ptr = hnode_get(cmdnode);
		if (UserLevel (u) < cmd_ptr->ulevel) {
			prefmsg (u->nick, bot_ptr->nick, "Permission Denied");
			return;
		}		
		if(!cmd_ptr->helptext) {
			/* Missing help text!!! */
			return;
		}
		privmsg_list (u->nick, bot_ptr->nick, cmd_ptr->helptext);
		return;
	}

	/* Handle intrinsic commands */

	/* Help */
	if (!strcasecmp(av[2], "HELP")) {
		prefmsg(u->nick, bot_ptr->nick, "Syntax: \2HELP [command]\2");
		prefmsg(u->nick, bot_ptr->nick, "");
		prefmsg(u->nick, bot_ptr->nick, "Provides help on the bot commands");
		return;
	}

	/* Command not found so report as unknown */
	prefmsg (u->nick, bot_ptr->nick, "Unknown Help Topic: \2%s\2", av[2]);
}

