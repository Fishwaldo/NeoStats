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
 *  - More error processing
 *  - Configurable user levels for commands
 *  - Make SET options use a hash to speed up lookups?
 */

#include "neostats.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "ns_help.h"
#include "modules.h"
#include "services.h"
#include "ircstring.h"
#include "commands.h"

static int bot_cmd_help (CmdParams *cmdparams);
static int bot_cmd_set (CmdParams *cmdparams);
static int bot_cmd_about (CmdParams *cmdparams);
static int bot_cmd_version (CmdParams *cmdparams);
static int bot_cmd_credits (CmdParams *cmdparams);
static int bot_cmd_levels (CmdParams *cmdparams);

/* help title strings for different user levels */
char *help_level_title[]=
{
	"Operators",
	"Service Admins",
	"Service Roots",
};

/*  Intrinsic commands 
 *  (Work in progress, not yet operation)
 *  These are all automatically added to a bot for handling 
 *  by the core. A module can override these by defining them
 *  in it's local bot command array.
 */

/*  Simplified command table for handling intrinsic commands.
 *  We do not require all entries since we only use a few for
 *  intrinsic handling. 
 */
static bot_cmd intrinsic_commands[]=
{
	{"HELP",	NULL,	0, 	0,	cmd_help_help, 		cmd_help_oneline},	
	{"VERSION",	NULL,	0, 	0,	cmd_help_version,	cmd_help_version_oneline},
	{"ABOUT",	NULL,	0, 	0,	cmd_help_about, 	cmd_help_about_oneline },
	{"CREDITS",	NULL,	0, 	0,	cmd_help_credits, 	cmd_help_credits_oneline },
	{"LEVELS",	NULL,	0, 	0,	cmd_help_levels, 	cmd_help_levels_oneline },
	{NULL,		NULL,	0, 	0,	NULL, 				NULL}
};

/** @brief calc_cmd_ulevel calculate cmd ulevel
 *  done as a function so we can support potentially complex  
 *  ulevel calculations without impacting other code.
 *
 *  @param pointer to command structure
 *  @return command user level requires
 */
static int calc_cmd_ulevel (bot_cmd *cmd_ptr)
{
	if (cmd_ptr->ulevel > NS_ULEVEL_ROOT) {
		/* int pointer rather than value */
		return (*(int*)cmd_ptr->ulevel);
	}
	/* use cmd entry directly */
	return (cmd_ptr->ulevel);
}

/** @brief getuserlevel 
 *
 *	calculate ulevel
 *
 *  @param pointer to command structure
 *  @return command user level requires
 */
static int getuserlevel(CmdParams *cmdparams)
{
	int ulevel = 0;
	int modlevel = 0;

	/* Generally we just want the standard user level */
	ulevel = UserLevel(cmdparams->source);
	/* If less than a locop see if the module can give us a user level */
	if (ulevel < NS_ULEVEL_LOCOPER) {
		if (cmdparams->bot->moduleptr->mod_auth_cb) {
			modlevel = cmdparams->bot->moduleptr->mod_auth_cb (cmdparams->source);
			if (modlevel > ulevel) {
				ulevel = modlevel;
			}
		}
	}
	return ulevel;
}

/** common message handlers */
void command_report (const Bot *botptr, const char *fmt, ...)
{
	static char buf[BUFSIZE];
	va_list ap;

	if (!is_synched || !botptr || !nsconfig.cmdreport)
		return;
	va_start (ap, fmt);
	ircvsnprintf (buf, BUFSIZE, fmt, ap);
	va_end (ap);
	irc_chanalert (botptr, buf);
}

void msg_permission_denied (CmdParams *cmdparams, char *subcommand)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("Permission Denied", cmdparams->source));
	irc_chanalert (cmdparams->bot, _("%s tried to use %s %s, but is not authorised"), 
		cmdparams->source->name, cmdparams->param, subcommand);
	nlog (LOG_NORMAL, "%s tried to use %s %s, but is not authorised", 
		cmdparams->source->name, cmdparams->param, subcommand);
}

void msg_error_need_more_params (CmdParams *cmdparams)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("Syntax error: insufficient parameters", cmdparams->source));
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("/msg %s HELP %s for more information", cmdparams->source), 
		cmdparams->bot->name, cmdparams->cmd);
}

void msg_error_param_out_of_range (CmdParams *cmdparams)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("Parameter out of range.", cmdparams->source));
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("/msg %s HELP %s for more information", cmdparams->source), 
		cmdparams->bot->name, cmdparams->cmd);
}

void msg_syntax_error (CmdParams *cmdparams)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("Syntax error", cmdparams->source));
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("/msg %s HELP %s for more information", cmdparams->source), 
		cmdparams->bot->name, cmdparams->cmd);
}

void msg_unknown_command (CmdParams *cmdparams)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("Syntax error: unknown command: \2%s\2", cmdparams->source), 
		cmdparams->param);
	if (nsconfig.cmdreport) {
		irc_chanalert (cmdparams->bot, _("%s requested %s, but that is an unknown command"),
			cmdparams->source->name, cmdparams->param);
	}
}

void msg_only_opers (CmdParams *cmdparams)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, 
		__("This service is only available to IRC operators.", cmdparams->source));
	irc_chanalert (cmdparams->bot, _("%s requested %s, but is not an operator."), 
		cmdparams->source->name, cmdparams->cmd);
	nlog (LOG_NORMAL, "%s requested %s, but is not an operator.", 
		cmdparams->source->name, cmdparams->cmd);
}

void check_cmd_result (CmdParams *cmdparams, int cmdret, char *extra)
{
	switch(cmdret) {
		case NS_ERR_SYNTAX_ERROR:
			msg_syntax_error (cmdparams);
			break;
		case NS_ERR_NEED_MORE_PARAMS:
			msg_error_need_more_params (cmdparams);
			break;				
		case NS_ERR_PARAM_OUT_OF_RANGE:
			msg_error_param_out_of_range (cmdparams);
			break;				
		case NS_ERR_UNKNOWN_COMMAND:
		case NS_ERR_NO_PERMISSION:
			break;				
		case NS_FAILURE:
		case NS_SUCCESS:
		default:
			break;
	}
}

/** @brief add_bot_cmd adds a single command to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int add_bot_cmd (hash_t *cmd_hash, bot_cmd *cmd_ptr) 
{
	bot_cmd *hashentry;
	char confcmd[32];
	char *temp = NULL;
	int ulevel = 0;

	/* Verify the command is OK before we add it so we do not have to 
	 * check validity during processing. Only check critical elements.
	 * For now we verify help during processing since it is not critical. */
	/* No command, we cannot recover from this */
	if (!cmd_ptr->cmd) {
		nlog (LOG_ERROR, "add_bot_cmd: missing command");
		return NS_FAILURE;
	}
	if ( hash_lookup (cmd_hash, cmd_ptr->cmd) ) {
		nlog (LOG_ERROR, "add_bot_cmd: attempt to add duplicate command %s", cmd_ptr->cmd);
		return NS_FAILURE;
	}
	/* No handler, we cannot recover from this */
	if (!cmd_ptr->handler) {
		nlog (LOG_ERROR, "add_bot_cmd: missing command handler, command %s not added", 
			cmd_ptr->cmd);
		return NS_FAILURE;
	}
	/* Seems OK, add the command */
	hnode_create_insert (cmd_hash, cmd_ptr, cmd_ptr->cmd);

	snprintf (confcmd, 32, "command%s", cmd_ptr->cmd);
	if (DBAFetchConfigInt (confcmd, &ulevel) == NS_SUCCESS) {
		hashentry = (bot_cmd *) hnode_find (cmd_hash, cmd_ptr->cmd);
		hashentry->ulevel = ulevel;
	}

	dlog (DEBUG3, "add_bot_cmd: added a new command %s to services bot", cmd_ptr->cmd);
	return NS_SUCCESS;
}

/** @brief del_bot_cmd deltes a single command to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int del_bot_cmd (hash_t *cmd_hash, bot_cmd *cmd_ptr) 
{
	hnode_t *cmdnode;
	
	/* Delete the command */
	cmdnode = hash_lookup (cmd_hash, cmd_ptr->cmd);
	if (cmdnode) {
		dlog (DEBUG3, "deleting command %s from services bot", ((bot_cmd*)hnode_get(cmdnode))->cmd);
		hash_delete (cmd_hash, cmdnode);
		hnode_destroy (cmdnode);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief add_bot_cmd_list adds a list of commands to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int add_bot_cmd_list (Bot* bot_ptr, bot_cmd *bot_cmd_list) 
{
	if (!bot_cmd_list) {
		return NS_FAILURE;
	}
	/* If no hash create */
	if (bot_ptr->botcmds == NULL) {
		bot_ptr->botcmds = hash_create(-1, 0, 0);
	}
	/* Cycle through command list and add them */
	while (bot_cmd_list->cmd) {
		add_bot_cmd(bot_ptr->botcmds, bot_cmd_list);
		bot_cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief del_bot_cmd_list delete a list of commands to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int del_bot_cmd_list (Bot* bot_ptr, bot_cmd *bot_cmd_list) 
{
	/* If no bot pointer return failure */
	if (!bot_ptr) {
		return NS_FAILURE;
	}
	/* If no hash return failure */
	if (!bot_ptr->botcmds) {
		return NS_FAILURE;
	}
	/* Cycle through command list and delete them */
	while (bot_cmd_list->cmd) {
		del_bot_cmd(bot_ptr->botcmds, bot_cmd_list);
		bot_cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief del_all_bot_cmds delete all commands from the bot
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int del_all_bot_cmds (Bot* bot_ptr) 
{
	hnode_t *cmdnode;
	hscan_t hs;

	/* Check we have a command hash */
	if (bot_ptr->botcmds == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command hash and delete each command */
	hash_scan_begin (&hs, bot_ptr->botcmds);
	while ((cmdnode = hash_scan_next(&hs)) != NULL) {
		dlog (DEBUG3, "deleting command %s from services bot", ((bot_cmd*)hnode_get(cmdnode))->cmd);
		hash_delete (bot_ptr->botcmds, cmdnode);
		hnode_destroy (cmdnode);
	}
	/* Destroy command */
	hash_destroy(bot_ptr->botcmds);
	bot_ptr->botcmds = NULL;
	return NS_SUCCESS;
}

/** @brief add_services_cmd_list adds a list of commands to the services bot
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int add_services_cmd_list (bot_cmd *bot_cmd_list) 
{
	if (!GET_CUR_MODULE()->insynch)
	{
		GET_CUR_MODULE()->error = 1;
		return NS_FAILURE;
	}	
	return (add_bot_cmd_list(ns_botptr, bot_cmd_list));
}

/** @brief del_services_cmd_list delete a list of commands from the services bot
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int del_services_cmd_list (bot_cmd *bot_cmd_list) 
{
	return (del_bot_cmd_list (ns_botptr, bot_cmd_list));
}

/** @brief intrinsic_handler
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
intrinsic_handler (CmdParams *cmdparams, bot_cmd_handler handler)
{
	int cmdret;

	SET_RUN_LEVEL(cmdparams->bot->moduleptr);
	cmdret = handler (cmdparams);
	RESET_RUN_LEVEL();
	check_cmd_result (cmdparams, cmdret, NULL);
	return NS_SUCCESS;
}

/** @brief run_intrinsic_cmds process bot intrinsic command list
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
run_intrinsic_cmds (const char *cmd, CmdParams *cmdparams)
{
	/* Handle intrinsic commands */
	/* Help */
	if (!ircstrcasecmp (cmd, "HELP")) {
		intrinsic_handler (cmdparams, bot_cmd_help);
		return NS_SUCCESS;
	}
	/* Handle SET if we have it */
	if (cmdparams->bot->botsettings && !ircstrcasecmp (cmd, "SET") ) {
		intrinsic_handler (cmdparams, bot_cmd_set);
		return NS_SUCCESS;
	}
	/* About */
	if (!ircstrcasecmp (cmd, "ABOUT") && cmdparams->bot->moduleptr && cmdparams->bot->moduleptr->info->about_text ) {
		intrinsic_handler (cmdparams, bot_cmd_about);
		return NS_SUCCESS;
	}
	/* Version */
	if (!ircstrcasecmp (cmd, "VERSION")) {
		intrinsic_handler (cmdparams, bot_cmd_version);
		return NS_SUCCESS;
	}
	/* Credits */
	if (!ircstrcasecmp (cmd, "CREDITS") && cmdparams->bot->moduleptr && cmdparams->bot->moduleptr->info->copyright ) {
		intrinsic_handler (cmdparams, bot_cmd_credits);
		return NS_SUCCESS;
	}
	/* Level */
	if (!ircstrcasecmp (cmd, "LEVELS")) {
		intrinsic_handler (cmdparams, bot_cmd_levels);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief run_bot_cmd process bot command list
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int
run_bot_cmd (CmdParams *cmdparams)
{
	static char privmsgbuffer[BUFSIZE];
	int userlevel;
	bot_cmd *cmd_ptr;
	int cmdret = 0;
	int cmdlevel;
	char **av;
	int ac = 0;
	int i;

	SET_SEGV_LOCATION();
	strlcpy (privmsgbuffer, cmdparams->param, BUFSIZE);
	ac = split_buf (privmsgbuffer, &av, 0);
	cmdparams->cmd = av[0];
	for (i = 1; i < ac; i++) {
		AddStringToList (&cmdparams->av, av[i], &cmdparams->ac);
	}
	userlevel = getuserlevel (cmdparams); 
	/* Check user authority to use this command set */
	if (( (cmdparams->bot->flags & BOT_FLAG_RESTRICT_OPERS) && (userlevel < NS_ULEVEL_OPER) ) ||
		( (cmdparams->bot->flags & BOT_FLAG_ONLY_OPERS) && nsconfig.onlyopers && (userlevel < NS_ULEVEL_OPER) )){
		msg_only_opers (cmdparams);
		ns_free (av);
		ns_free (cmdparams->av);
		return NS_SUCCESS;
	}	
	if (cmdparams->bot->botcmds) {
		/* Process command list */
		cmd_ptr = (bot_cmd *)hnode_find (cmdparams->bot->botcmds, av[0]);
		if (cmd_ptr) {
			cmdlevel = calc_cmd_ulevel(cmd_ptr);
			/* Is user authorised to issue this command? */
			if (userlevel < cmdlevel) {
				msg_permission_denied(cmdparams, NULL);
			/* Check parameter count */
			} else if (cmdparams->ac < cmd_ptr->minparams ) {		
				msg_error_need_more_params(cmdparams);
			} else {
				/* Seems OK so report the command call so modules do not have to */
				SET_RUN_LEVEL(cmdparams->bot->moduleptr);
				if (nsconfig.cmdreport) {
					irc_chanalert (cmdparams->bot, _("%s used %s"), cmdparams->source->name, cmd_ptr->cmd);
				}
				/* Log command message */
				nlog (LOG_NORMAL, "%s used %s", cmdparams->source->name, cmdparams->param);
				/* call handler */
				if (setjmp (sigvbuf) == 0) {
					cmdret = cmd_ptr->handler(cmdparams);
				}
				check_cmd_result (cmdparams, cmdret, NULL);
				RESET_RUN_LEVEL();
			}
			ns_free (av);
			ns_free (cmdparams->av);
			return NS_SUCCESS;
		}
	}
	cmdret = run_intrinsic_cmds (av[0], cmdparams);
	if (cmdret == NS_SUCCESS) {
		ns_free (av);
		ns_free (cmdparams->av);
		return NS_SUCCESS;
	}
	/* We have run out of commands so report failure */
	msg_unknown_command (cmdparams);
	ns_free (av);
	ns_free (cmdparams->av);
	return NS_FAILURE;
}

/** @brief bot_cmd_help_set process bot help command
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
bot_cmd_help_set (CmdParams *cmdparams, int userlevel)
{
	hnode_t *setnode;
	hscan_t hs;
	bot_setting* set_ptr;

	/* Display HELP SET intro text and LIST command */
	irc_prefmsg_list (cmdparams->bot, cmdparams->source, cmd_help_set);
	/* Display option specific text for current user level */
	hash_scan_begin (&hs, cmdparams->bot->botsettings);
	while ((setnode = hash_scan_next(&hs)) != NULL) {
		set_ptr = hnode_get(setnode);
		if (set_ptr->helptext && userlevel >= set_ptr->ulevel)
		{
			irc_prefmsg_list (cmdparams->bot, cmdparams->source, set_ptr->helptext);
			irc_prefmsg (cmdparams->bot, cmdparams->source, " ");
		}
	}
	return NS_SUCCESS;
}
	
/** @brief bot_cmd_help process bot help command
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
bot_cmd_help (CmdParams *cmdparams)
{
	char *curlevelmsg=NULL;
	int donemsg=0;
	bot_cmd *cmd_ptr;
	int curlevel, lowlevel;
	hscan_t hs;
	int userlevel;
	int cmdlevel;

	userlevel = getuserlevel (cmdparams);

	/* If no parameter to help, generate main help text */
	if (cmdparams->ac < 1) {
		lowlevel = 0;
		curlevel = 30;
		if (nsconfig.cmdreport) {
			irc_chanalert (cmdparams->bot, _("%s requested %s help"), cmdparams->source->name, cmdparams->bot->name);
		}
		nlog (LOG_NORMAL, "%s requested %s help", cmdparams->source->name, cmdparams->bot->name);
		irc_prefmsg (cmdparams->bot, cmdparams->source, __("\2The following commands can be used with %s:\2",cmdparams->source), cmdparams->bot->name);

		/* Handle intrinsic commands */
		cmd_ptr = intrinsic_commands;
		while (cmd_ptr->cmd) {
			/* Check for module override */	
			if (!cmdparams->bot->botcmds || !hash_lookup (cmdparams->bot->botcmds, cmd_ptr->cmd)) {
				irc_prefmsg (cmdparams->bot, cmdparams->source, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->onelinehelp);
			}
			cmd_ptr++;
		}
		/* Do we have a set command? */
		if (cmdparams->bot->botsettings && userlevel >= cmdparams->bot->set_ulevel) {
			irc_prefmsg (cmdparams->bot, cmdparams->source, "    %-20s Configure %s", "SET", cmdparams->bot->name);
		}
		if (cmdparams->bot->botcmds) {
			while (1) {
				hnode_t* cmdnode;

				hash_scan_begin (&hs, cmdparams->bot->botcmds);
				while ((cmdnode = hash_scan_next(&hs)) != NULL) {
					cmd_ptr = hnode_get(cmdnode);
					cmdlevel = calc_cmd_ulevel(cmd_ptr);
					if ((cmdlevel < curlevel) && (cmdlevel >= lowlevel)) {
						if (curlevelmsg && !donemsg) {
							irc_prefmsg (cmdparams->bot, cmdparams->source, __("\2Additional commands available to %s:\2", cmdparams->source), curlevelmsg);
							donemsg = 1;
						}
						if (!cmd_ptr->onelinehelp) {
							irc_prefmsg (cmdparams->bot, cmdparams->source, __("    %-20s *** Missing help text ***",cmdparams->source), cmd_ptr->cmd);
						} else {					
							irc_prefmsg (cmdparams->bot, cmdparams->source, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->onelinehelp);
						}
					}
				}
				if (lowlevel >= userlevel) {
					break;
				}
				if (userlevel >= curlevel) {
					switch (curlevel) {
						case 30:
							curlevel = NS_ULEVEL_OPER;
							lowlevel = 30;
							curlevelmsg=NULL;
							donemsg=0;
							break;
						case NS_ULEVEL_OPER:
							curlevel = NS_ULEVEL_ADMIN;
							lowlevel = NS_ULEVEL_OPER;
							curlevelmsg=help_level_title[0];
							donemsg=0;
							break;
						case NS_ULEVEL_ADMIN:
							curlevel = NS_ULEVEL_ROOT;
							lowlevel = NS_ULEVEL_ADMIN;
							curlevelmsg=help_level_title[1];
							donemsg=0;
							break;
						case NS_ULEVEL_ROOT:
							curlevel = (NS_ULEVEL_ROOT + 1);
							lowlevel = NS_ULEVEL_ROOT;
							curlevelmsg=help_level_title[2];
							donemsg=0;
							break;
						default:	
							break;
					}
				}						
			}
		}
		/* Generate help on help footer text */
		irc_prefmsg (cmdparams->bot, cmdparams->source, " ");
		irc_prefmsg (cmdparams->bot, cmdparams->source, __("To execute a command:", cmdparams->source));
		irc_prefmsg (cmdparams->bot, cmdparams->source, "    \2/msg %s command\2", cmdparams->bot->name);
		irc_prefmsg (cmdparams->bot, cmdparams->source, __("For help on a command:", cmdparams->source));
		irc_prefmsg (cmdparams->bot, cmdparams->source, "    \2/msg %s HELP command\2", cmdparams->bot->name);
		return NS_SUCCESS;
	}
	if (nsconfig.cmdreport) {
		irc_chanalert (cmdparams->bot, _("%s requested %s help on %s"), cmdparams->source->name, cmdparams->bot->name, cmdparams->av[0]);
	}
	nlog (LOG_NORMAL, "%s requested %s help on %s", cmdparams->source->name, cmdparams->bot->name, cmdparams->av[0]);

	/* Process command list */
	if (cmdparams->bot->botcmds) {
		cmd_ptr = (bot_cmd*)hnode_find (cmdparams->bot->botcmds, cmdparams->av[0]);
		if (cmd_ptr) {
			cmdlevel = calc_cmd_ulevel(cmd_ptr);
			if (userlevel < cmdlevel) {
				msg_permission_denied(cmdparams, NULL);
				return NS_ERR_NO_PERMISSION;
			}		
			if (!cmd_ptr->helptext) {
				irc_prefmsg (cmdparams->bot, cmdparams->source, __("Missing help text for command", cmdparams->source));
			} else {
				irc_prefmsg_list (cmdparams->bot, cmdparams->source, cmd_ptr->helptext);
			}
			return NS_SUCCESS;
		}
	}

	/* Handle intrinsic commands */
	cmd_ptr = intrinsic_commands;
	while (cmd_ptr->cmd) {
		if (!ircstrcasecmp (cmdparams->av[0], cmd_ptr->cmd)) {
			irc_prefmsg_list (cmdparams->bot, cmdparams->source, cmd_ptr->helptext);
			return NS_SUCCESS;
		}
		cmd_ptr++;
	}
	/* Handle SET if we have it */	
	if (cmdparams->bot->botsettings && userlevel >= cmdparams->bot->set_ulevel && !ircstrcasecmp (cmdparams->av[0], "SET") ) {
		bot_cmd_help_set (cmdparams, userlevel);		
		return NS_SUCCESS;
	}

	/* Command not found so report as unknown */
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("No help available or unknown help topic: \2%s\2", cmdparams->source), cmdparams->av[2]);
	return NS_ERR_UNKNOWN_COMMAND;
}

/**	Support function for command handlers to call to check that target nick 
 *	is not the bot and is on IRC. Done in core to avoid module code bloat.
 */ 
Client * find_valid_user(Bot* botptr, Client * sourceuser, const char *target_nick)
{
	Client * target;
	
	/* Check target user is on IRC */
	target = find_user(target_nick);
	if (!target) {
		irc_prefmsg (botptr, sourceuser, 
			__("%s cannot be found on IRC, message not sent.", sourceuser), target_nick);
		return NULL;
	}
	/* Check for message to self */
	if (IsMe(target)) {
		irc_prefmsg (botptr, sourceuser, __("Cannot send message to a service bot.", sourceuser));
		return NULL;
	}
	/* User OK */
	return target;
}

/** @brief bot_cmd_set_list process bot set list command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
bot_cmd_set_list (CmdParams *cmdparams)
{
	hnode_t *setnode;
	hscan_t hs;
	bot_setting* set_ptr;
	int userlevel;

	irc_prefmsg (cmdparams->bot, cmdparams->source, __("Current %s settings:", cmdparams->source), cmdparams->bot->name);
	userlevel = getuserlevel (cmdparams);
	hash_scan_begin (&hs, cmdparams->bot->botsettings);
	while ((setnode = hash_scan_next(&hs)) != NULL) {
		set_ptr = hnode_get(setnode);
		/* Only list authorised SETTINGS */
		if ( userlevel >= set_ptr->ulevel) {
			switch(set_ptr->type) {
				case SET_TYPE_BOOLEAN:
					irc_prefmsg (cmdparams->bot, cmdparams->source, "%s: %s",
						set_ptr->option, *(int*)set_ptr->varptr ? __("Enabled", cmdparams->source) : __("Disabled", cmdparams->source));
					break;
				case SET_TYPE_INT:
					if (set_ptr->desc) {
						irc_prefmsg (cmdparams->bot, cmdparams->source, "%s: %d %s",
							set_ptr->option, *(int*)set_ptr->varptr, set_ptr->desc);
					} else {
						irc_prefmsg (cmdparams->bot, cmdparams->source, "%s: %d",
							set_ptr->option, *(int*)set_ptr->varptr);
					}
					break;				
				case SET_TYPE_MSG:
				case SET_TYPE_STRING:
				case SET_TYPE_NICK:
				case SET_TYPE_USER:
				case SET_TYPE_HOST:
				case SET_TYPE_REALNAME:
				case SET_TYPE_IPV4:	
				case SET_TYPE_CHANNEL:							
					irc_prefmsg (cmdparams->bot, cmdparams->source, "%s: %s",
						set_ptr->option, (char*)set_ptr->varptr);
					break;
				case SET_TYPE_CUSTOM:
					if (set_ptr->handler) {
						set_ptr->handler(cmdparams, SET_LIST);
					}
					break;
				default:
					irc_prefmsg (cmdparams->bot, cmdparams->source, __("%s: uses an unsupported type", cmdparams->source),
						set_ptr->option);
					break;
			}
		}
		set_ptr++;
	}
	return NS_SUCCESS;
}

static int 
bot_cmd_set_report (CmdParams *cmdparams, bot_setting* set_ptr, char *new_setting)
{
	if (nsconfig.cmdreport) {
		irc_chanalert(cmdparams->bot, _("%s set to %s by \2%s\2"), 
			set_ptr->option, new_setting, cmdparams->source->name);
	}
	nlog(LOG_NORMAL, "%s!%s@%s set %s to %s", 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, set_ptr->option, new_setting);
	irc_prefmsg (cmdparams->bot, cmdparams->source, 
		__("%s set to %s", cmdparams->source), set_ptr->option, new_setting);
	return NS_SUCCESS;
} 

/** @brief bot_cmd_set helper functions
 *  validate the pamater based on type and perform appropriate action
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
bot_cmd_set_boolean (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (!ircstrcasecmp (cmdparams->av[1], "ON")) {
		*(int*)set_ptr->varptr = 1;
		if (set_ptr->confitem) {
			DBAStoreConfigBool (set_ptr->confitem, set_ptr->varptr);
		}
		bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
		return NS_SUCCESS;
	} else if (!ircstrcasecmp (cmdparams->av[1], "OFF")) {
		*(int*)set_ptr->varptr = 0;
		if (set_ptr->confitem) {
			DBAStoreConfigBool (set_ptr->confitem, set_ptr->varptr);
		}
		bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
		return NS_SUCCESS;
	}
	msg_syntax_error (cmdparams);
	return NS_ERR_SYNTAX_ERROR;
}

static int 
bot_cmd_set_int (CmdParams *cmdparams, bot_setting* set_ptr)
{
	int intval;

	intval = atoi(cmdparams->av[1]);	
	/* atoi will return 0 for a string instead of a digit so check it! */
	if (intval == 0 && (strcmp(cmdparams->av[1],"0")!=0)) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%s invalid setting for %s", cmdparams->source), cmdparams->av[1], set_ptr->option);
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("Valid values are %d to %d", cmdparams->source), set_ptr->min, set_ptr->max);
		return NS_ERR_SYNTAX_ERROR;
	}
	/* Check limits */
	if ((set_ptr->min != -1 && intval < set_ptr->min) || (set_ptr->max != -1 && intval > set_ptr->max)) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%d out of range for %s", cmdparams->source), intval, set_ptr->option);
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("Valid values are %d to %d", cmdparams->source), set_ptr->min, set_ptr->max);
		return NS_ERR_SYNTAX_ERROR;
	}
	/* Set the new value */
	*(int*)set_ptr->varptr = intval;
	if (set_ptr->confitem) {
		DBAStoreConfigInt (set_ptr->confitem, set_ptr->varptr);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_string (CmdParams *cmdparams, bot_setting* set_ptr)
{
	strlcpy((char*)set_ptr->varptr, cmdparams->av[1], set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, cmdparams->av[1], set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_channel (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (validate_channel (cmdparams->av[1]) == NS_FAILURE) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%s contains invalid characters", cmdparams->source), cmdparams->av[1]);
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy((char*)set_ptr->varptr, cmdparams->av[1], set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, cmdparams->av[1], set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_msg (CmdParams *cmdparams, bot_setting* set_ptr)
{
	char *buf;

	buf = joinbuf(cmdparams->av, cmdparams->ac, 1);
	strlcpy((char*)set_ptr->varptr, buf, set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, buf, set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, buf);
	ns_free (buf);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_nick (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (validate_nick (cmdparams->av[1]) == NS_FAILURE) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%s contains invalid characters", cmdparams->source), cmdparams->av[1]);
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy((char*)set_ptr->varptr, cmdparams->av[1], set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, cmdparams->av[1], set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_user (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (validate_user (cmdparams->av[1]) == NS_FAILURE) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%s contains invalid characters", cmdparams->source), cmdparams->av[1]);
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy((char*)set_ptr->varptr, cmdparams->av[1], set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, cmdparams->av[1], set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_host (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (!index(cmdparams->av[1], '.')) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%s is an invalid hostname", cmdparams->source), cmdparams->av[1]);
		return NS_ERR_SYNTAX_ERROR;
	}
	if (validate_host (cmdparams->av[1]) == NS_FAILURE) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("%s contains invalid characters", cmdparams->source), cmdparams->av[1]);
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy((char*)set_ptr->varptr, cmdparams->av[1], set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, cmdparams->av[1], set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_realname (CmdParams *cmdparams, bot_setting* set_ptr)
{
	char *buf;

	buf = joinbuf(cmdparams->av, cmdparams->ac, 1);
	strlcpy((char*)set_ptr->varptr, buf, set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, buf, set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, buf);
	ns_free (buf);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_ipv4 (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (!inet_addr(cmdparams->av[1])) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, 
			__("Invalid IPV4 format. Should be dotted quad, e.g. 1.2.3.4", cmdparams->source));
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy((char*)set_ptr->varptr, cmdparams->av[1], set_ptr->max);
	if (set_ptr->confitem) {
		DBAStoreConfigStr (set_ptr->confitem, cmdparams->av[1], set_ptr->max);
	}
	bot_cmd_set_report (cmdparams, set_ptr, cmdparams->av[1]);
	return NS_SUCCESS;
}

static int 
bot_cmd_set_custom (CmdParams *cmdparams, bot_setting* set_ptr)
{
	if (set_ptr->handler) {
		set_ptr->handler(cmdparams, SET_CHANGE);
	}
	return NS_SUCCESS;
}

typedef int (*bot_cmd_set_handler) (CmdParams *cmdparams, bot_setting* set_ptr);

static bot_cmd_set_handler bot_cmd_set_handlers[] = 
{
	bot_cmd_set_boolean,
	bot_cmd_set_int,
	bot_cmd_set_string,
	bot_cmd_set_channel,
	bot_cmd_set_msg,
	bot_cmd_set_nick,
	bot_cmd_set_user,
	bot_cmd_set_host,
	bot_cmd_set_realname,
	bot_cmd_set_ipv4,
	bot_cmd_set_custom,
};

/** @brief bot_cmd_set process bot set command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
bot_cmd_set (CmdParams *cmdparams)
{
	bot_cmd_set_handler set_handler;
	bot_setting* set_ptr;
	int userlevel;

	if (cmdparams->ac < 1) {
		return NS_ERR_SYNTAX_ERROR;
	} 
	userlevel = getuserlevel (cmdparams);
	if ( userlevel < cmdparams->bot->set_ulevel) {
		msg_permission_denied(cmdparams, NULL);
		return NS_ERR_NO_PERMISSION;
	}
	if (!ircstrcasecmp (cmdparams->av[0], "LIST"))
	{
		bot_cmd_set_list (cmdparams);
		return NS_SUCCESS;
	}
	if (cmdparams->ac < 2) {
		return NS_ERR_SYNTAX_ERROR;
	} 
	set_ptr= (bot_setting*)hnode_find (cmdparams->bot->botsettings, cmdparams->av[0]);
	if (set_ptr) {
		if ( userlevel < set_ptr->ulevel) {
			msg_permission_denied (cmdparams, cmdparams->av[0]);
			return NS_ERR_NO_PERMISSION;
		}
		set_handler = bot_cmd_set_handlers[set_ptr->type];
		if (set_handler (cmdparams, set_ptr)!=NS_SUCCESS) {
			return NS_FAILURE;
		}
		/* Call back after SET so that a module can "react" to a change in a setting */
		if (set_ptr->type != SET_TYPE_CUSTOM) {
			if (set_ptr->handler) {
				set_ptr->handler (cmdparams, SET_CHANGE);
			}
		}
		return NS_SUCCESS;
	}
	irc_prefmsg (cmdparams->bot, cmdparams->source, 
		__("Unknown set option. /msg %s HELP SET for more info", cmdparams->source),
		cmdparams->bot->name);
	return NS_ERR_UNKNOWN_OPTION;
}

/** @brief bot_cmd_about process bot about command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_about (CmdParams *cmdparams)
{
	irc_prefmsg_list (cmdparams->bot, cmdparams->source, cmdparams->bot->moduleptr->info->about_text);
	return NS_SUCCESS;
}

/** @brief bot_cmd_version process bot version command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_version (CmdParams *cmdparams)
{
	irc_prefmsg (cmdparams->bot, cmdparams->source, __("\2%s version\2", cmdparams->source), 
		cmdparams->bot->moduleptr->info->name);
	irc_prefmsg (cmdparams->bot, cmdparams->source, "%s %s %s", 
		cmdparams->bot->moduleptr->info->version, 
		cmdparams->bot->moduleptr->info->build_date, 
		cmdparams->bot->moduleptr->info->build_time);
	return NS_SUCCESS;
}

/** @brief bot_cmd_credits process bot credits command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_credits (CmdParams *cmdparams)
{
	irc_prefmsg_list (cmdparams->bot, cmdparams->source, 
		cmdparams->bot->moduleptr->info->copyright);
	return NS_SUCCESS;
}

/** @brief bot_cmd_levels process bot level command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_levels (CmdParams *cmdparams)
{
	char confcmd[32];
	bot_cmd *cmd_ptr;
	int userlevel;

	if (cmdparams->ac < 1) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		hnode_t *cmdnode;
		hscan_t hs;

		/* Cycle through command hash and delete each command */
		hash_scan_begin (&hs, cmdparams->bot->botcmds);
		while ((cmdnode = hash_scan_next(&hs)) != NULL) {
			cmd_ptr = ((bot_cmd*)hnode_get(cmdnode));
			irc_prefmsg (cmdparams->bot, cmdparams->source, "%s %d", cmd_ptr->cmd, cmd_ptr->ulevel);
		}
		return NS_SUCCESS;
	}
	if (cmdparams->ac < 2) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	userlevel = getuserlevel (cmdparams);
	if (userlevel < NS_ULEVEL_ROOT) {
		msg_permission_denied (cmdparams, cmdparams->cmd);
		return NS_ERR_NO_PERMISSION;
	}
	cmd_ptr = (bot_cmd *)hnode_find (cmdparams->bot->botcmds, cmdparams->av[0]);
	if (cmd_ptr) {
		int newlevel = 0;

		newlevel = atoi (cmdparams->av[1]);
		if (newlevel >= 0 && newlevel <= NS_ULEVEL_ROOT) {
			cmd_ptr->ulevel = newlevel;
			snprintf (confcmd, 32, "command%s", cmd_ptr->cmd);
			DBAStoreConfigInt (confcmd, &newlevel);
			return NS_SUCCESS;
		}
	}
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief add_bot_setting adds a single set option 
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
add_bot_setting (hash_t *set_hash, bot_setting* set_ptr) 
{
	hnode_create_insert (set_hash, set_ptr, set_ptr->option);
	dlog (DEBUG3, "add_bot_setting: added a new set option %s", set_ptr->option);
	return NS_SUCCESS;
}

/** @brief del_bot_setting delete a single set option 
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int 
del_bot_setting (hash_t *set_hash, bot_setting* set_ptr) 
{
	hnode_t *setnode;
	
	setnode = hash_lookup (set_hash, set_ptr->option);
	if (setnode) {
		hash_delete (set_hash, setnode);
		hnode_destroy (setnode);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief add_bot_setting_list adds a list of set options
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
add_bot_setting_list (Bot* bot_ptr, bot_setting* set_ptr) 
{
	if (!set_ptr) {
		return NS_FAILURE;
	}
	/* If no hash create */
	if (bot_ptr->botsettings == NULL) {
		bot_ptr->botsettings = hash_create(-1, 0, 0);
	}
	/* Default SET to ROOT only */
	bot_ptr->set_ulevel = NS_ULEVEL_ROOT;
	/* Now calculate minimum defined user level */
	while (set_ptr->option != NULL) {
		if (set_ptr->ulevel < bot_ptr->set_ulevel) {
			bot_ptr->set_ulevel = set_ptr->ulevel;
		}
		add_bot_setting (bot_ptr->botsettings, set_ptr);
		set_ptr++;
	}
	return NS_SUCCESS;
}

/** @brief del_bot_setting_list delete a list of set options
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int 
del_bot_setting_list (Bot* bot_ptr, bot_setting* set_ptr) 
{
	/* If no bot pointer return failure */
	if (!bot_ptr) {
		return NS_FAILURE;
	}
	/* If no hash return failure */
	if (!bot_ptr->botsettings) {
		return NS_FAILURE;
	}
	/* Cycle through command list and delete them */
	while (set_ptr->option) {
		del_bot_setting (bot_ptr->botsettings, set_ptr);
		set_ptr++;
	}
	return NS_SUCCESS;
}

int del_all_bot_settings (Bot *bot_ptr)
{
	hnode_t *setnode;
	hscan_t hs;

	/* Check we have a command hash */
	if (bot_ptr->botsettings == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command hash and delete each command */
	hash_scan_begin (&hs, bot_ptr->botsettings);
	while ((setnode = hash_scan_next(&hs)) != NULL) {
		hash_delete (bot_ptr->botsettings, setnode);
		hnode_destroy (setnode);
	}
	/* Destroy command */
	hash_destroy (bot_ptr->botsettings);
	bot_ptr->botsettings = NULL;
	return NS_SUCCESS;
}

int add_services_set_list (bot_setting *bot_setting_list)
{
	add_bot_setting_list (ns_botptr, bot_setting_list);
	return NS_SUCCESS;
}

int del_services_set_list (bot_setting *bot_setting_list)
{
	del_bot_setting_list (ns_botptr, bot_setting_list);
	return NS_SUCCESS;
}

int bot_set_nick_cb(CmdParams* cmdparams, SET_REASON reason)
{
	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	irc_nickchange (cmdparams->bot, cmdparams->av[1]);
	return NS_SUCCESS;
}

int bot_set_altnick_cb(CmdParams* cmdparams, SET_REASON reason)
{
	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	return NS_SUCCESS;
}

int bot_set_user_cb(CmdParams* cmdparams, SET_REASON reason)
{
	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	irc_setident (cmdparams->bot, cmdparams->av[1]);
	return NS_SUCCESS;
}

int bot_set_host_cb(CmdParams* cmdparams, SET_REASON reason)
{
	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	irc_sethost (cmdparams->bot, cmdparams->av[1]);
	return NS_SUCCESS;
}

int bot_set_realname_cb(CmdParams* cmdparams, SET_REASON reason)
{
	char *buf;

	/* Ignore bootup and list callback */
	if (reason != SET_CHANGE) {
		return NS_SUCCESS;
	}
	buf = joinbuf (cmdparams->av, cmdparams->ac, 1);
	irc_setname (cmdparams->bot, buf);
	ns_free (buf);
	return NS_SUCCESS;
}

static bot_setting bot_info_settings[]=
{
	{"NICK",	NULL,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick, bot_set_nick_cb, NULL },
	{"ALTNICK",	NULL,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "AltNick",	NULL,	ns_help_set_altnick, bot_set_altnick_cb, NULL },
	{"USER",	NULL,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user, bot_set_user_cb, NULL },
	{"HOST",	NULL,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host, bot_set_host_cb, NULL },
	{"REALNAME",NULL,	SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname, bot_set_realname_cb, NULL },
	{NULL,		NULL,	0,					0, 0, 			0,				 NULL,		NULL,	NULL,				NULL,	NULL },
};

int add_bot_info_settings (Bot *bot_ptr, BotInfo* botinfo)
{
	bot_ptr->bot_info_settings = ns_calloc (sizeof(bot_info_settings));
	if (bot_ptr->bot_info_settings) {
		memcpy(bot_ptr->bot_info_settings, bot_info_settings, sizeof(bot_info_settings));
		bot_ptr->bot_info_settings[0].varptr = &botinfo->nick;
		bot_ptr->bot_info_settings[1].varptr = &botinfo->altnick;
		bot_ptr->bot_info_settings[2].varptr = &botinfo->user;
		bot_ptr->bot_info_settings[3].varptr = &botinfo->host;
		bot_ptr->bot_info_settings[4].varptr = &botinfo->realname;
		ModuleConfig (bot_ptr->bot_info_settings);
		add_bot_setting_list (bot_ptr, bot_ptr->bot_info_settings);
	}
	return NS_SUCCESS;
}

int del_bot_info_settings (Bot *bot_ptr)
{
	if (bot_ptr->bot_info_settings) {
		del_bot_setting_list (bot_ptr, bot_ptr->bot_info_settings);
		ns_free (bot_ptr->bot_info_settings);
	}
	return NS_SUCCESS;
}
