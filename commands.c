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
#include "conf.h"

static int bot_cmd_help (ModUser* bot_ptr, User * u, char **av, int ac);
static int bot_cmd_set (ModUser* bot_ptr, User * u, char **av, int ac);
#if 0
static int bot_cmd_about (ModUser* bot_ptr, User * u, char **av, int ac);
static int bot_cmd_version (ModUser* bot_ptr, User * u, char **av, int ac);
#endif
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

/*  Intrinsic commands 
 *  (Work in progress, not yet operation)
 *  These are all automatically added to a bot for handling 
 *  by the core. A module can override these by defining them
 *  in it's local bot command array.
 */
static const char cmd_help_oneline[]="Online help";
static const char *cmd_help_help[] = {
	"Syntax: \2HELP [command]\2",
	"",
	"Provides help on the bot commands",
	NULL
};

#if 0
const char cmd_help_about_oneline[] = "About Module";
const char *cmd_help_about[] = {
	"Syntax: \2ABOUT\2",
	"",
	"Provides information about the module",
	NULL
};

const char cmd_help_version_oneline[] = "Display Module version info";
const char *cs_help_version[] = {
	"Syntax: \2VERSION\2",
	"",
	"Show version information",
	NULL
};
#endif


static const char *cmd_help_set[] = {
	"Syntax: \2SET LIST\2",
	"        \2SET <option> [<value>]\2",
	"",
	"LIST    display the current settings",
	"",
	"Available Options are:",
	"",
	NULL
};

/*  Simplified command table for handling intrinsic commands.
 *  We do not require all entries since we only use a few for
 *  intrinsic handling. 
 */
static bot_cmd intrinsic_commands[]=
{
	{"HELP",	NULL,	0, 	0,	cmd_help_help, 		cmd_help_oneline},	
#if 0
	{"VERSION",	NULL,	0, 	0,	cmd_help_version,	cmd_help_version_oneline},
	{"ABOUT",	NULL,	0, 	0,	cmd_help_about, 	cmd_help_about_oneline },
#endif
	{NULL,		NULL,	0, 	0,	NULL, 				NULL}
};


/** @brief calc_cmd_ulevel calculate cmd ulevel
 *  done as a function so we can support potentially complex  
 *  ulevel calculations without impacting other code.
 *
 *  @param pointer to command structure
 *  @return command user level requires
 */
static int calc_cmd_ulevel(bot_cmd* cmd_ptr)
{
	if(cmd_ptr->ulevel > 200) {
		/* int pointer rather than value */
		return(*(int*)cmd_ptr->ulevel);
	}
	/* use cmd entry directly */
	return(cmd_ptr->ulevel);
}

/** @brief add_bot_cmd adds a single command to the command hash
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int 
add_bot_cmd(hash_t* cmd_hash, bot_cmd* cmd_ptr) 
{
	hnode_t *cmdnode;
	
	/* Verify the command is OK before we add it so we do not have to 
	 * check validity during processing. Only check critical elements.
	 * For now we verify help during processing since it is not critical. */
	/* No command, we cannot recover from this */
	if(!cmd_ptr->cmd) {
		nlog (LOG_ERROR, LOG_MOD, "Missing command, command (unknown) not added");
		return NS_FAILURE;
	}
	/* No handler, we cannot recover from this */
	if(!cmd_ptr->handler) {
		nlog (LOG_ERROR, LOG_MOD, "Missing command handler, command %s not added", 
			cmd_ptr->cmd);
		return NS_FAILURE;
	}
	/* Seems OK, add the command */
	cmdnode = hnode_create(cmd_ptr);
	if (cmdnode) {
		hash_insert(cmd_hash, cmdnode, cmd_ptr->cmd);
		nlog(LOG_DEBUG2, LOG_MOD, "Added a new command %s to Services Bot", cmd_ptr->cmd);
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
add_bot_cmd_list(ModUser* bot_ptr, bot_cmd* bot_cmd_list) 
{
	/* If no hash return failure */
	if(bot_ptr->botcmds == NULL) {
		return NS_FAILURE;
	}
	
	/* set the module */
	SET_SEGV_INMODULE(bot_ptr->modname);
	
	/* Cycle through command list and add them */
	while(bot_cmd_list->cmd) {
		add_bot_cmd(bot_ptr->botcmds, bot_cmd_list);
		bot_cmd_list++;
	}
	CLEAR_SEGV_INMODULE();
	return NS_SUCCESS;
}

/** @brief del_bot_cmd_list delete a list of commands to the command hash
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
del_bot_cmd_list(ModUser* bot_ptr, bot_cmd* bot_cmd_list) 
{
	/* If no hash return failure */
	if(bot_ptr->botcmds == NULL) {
		return NS_FAILURE;
	}

	/* set the module */
	SET_SEGV_INMODULE(bot_ptr->modname);

	/* Cycle through command list and delete them */
	while(bot_cmd_list->cmd) {
		del_bot_cmd(bot_ptr->botcmds, bot_cmd_list);
		bot_cmd_list++;
	}
	CLEAR_SEGV_INMODULE();
	return NS_SUCCESS;
}

/** @brief del_all_bot_cmds delete all commands from the bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
del_all_bot_cmds(ModUser* bot_ptr) 
{
	hnode_t *cmdnode;
	hscan_t hs;

	/* Check we have a command hash */
	if(bot_ptr->botcmds == NULL) {
		return NS_FAILURE;
	}
	
	
	/* set the module */
	SET_SEGV_INMODULE(bot_ptr->modname);

	
	/* Cycle through command hash and delete each command */
	hash_scan_begin(&hs, bot_ptr->botcmds);
	while ((cmdnode = hash_scan_next(&hs)) != NULL) {
		hash_delete(bot_ptr->botcmds, cmdnode);
		hnode_destroy(cmdnode);
	}
	/* Destroy command */
	hash_destroy(bot_ptr->botcmds);
	bot_ptr->botcmds = NULL;
	CLEAR_SEGV_INMODULE();
	return NS_SUCCESS;
}

/** @brief add_services_cmd_list adds a list of commands to the services bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
add_services_cmd_list(bot_cmd* bot_cmd_list) 
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
	while(bot_cmd_list->cmd) {
		add_bot_cmd(botcmds, bot_cmd_list);
		bot_cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief del_services_cmd_list delete a list of commands from the services bot
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int 
del_services_cmd_list(bot_cmd* bot_cmd_list) 
{
	/* If no hash return failure */
	if(botcmds == NULL) {
		return NS_FAILURE;
	}
	/* Cycle through command list and delete them */
	while(bot_cmd_list->cmd) {
		del_bot_cmd(botcmds, bot_cmd_list);
		bot_cmd_list++;
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
	/* Use fake bot structure so we can use the main command routine */
	strlcpy(fake_bot.nick, s_Services, MAXNICK);
	fake_bot.botcmds = botcmds;
	fake_bot.flags = me.onlyopers ? BOT_FLAG_ONLY_OPERS : 0;
	run_bot_cmd (&fake_bot, u, av, ac);
}

/** @brief run_bot_cmd process bot command list
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
int
run_bot_cmd (ModUser* bot_ptr, User *u, char **av, int ac)
{
	int userlevel;
	bot_cmd* cmd_ptr;
	hnode_t *cmdnode;
	int cmdlevel;
	char* parambuf; 

	SET_SEGV_LOCATION();
	SET_SEGV_INMODULE(bot_ptr->modname);
	userlevel = UserLevel (u);
	/* Check user authority to use this command set */
	if (( (bot_ptr->flags & BOT_FLAG_RESTRICT_OPERS) && (userlevel < NS_ULEVEL_OPER) ) ||
		( (bot_ptr->flags & BOT_FLAG_ONLY_OPERS) && me.onlyopers && (userlevel < NS_ULEVEL_OPER) )){
		prefmsg (u->nick, bot_ptr->nick, "This service is only available to IRC operators.");
		chanalert (bot_ptr->nick, "%s requested %s, but is not an operator.", u->nick, av[1]);
		nlog (LOG_NORMAL, LOG_MOD, "%s requested %s, but is not an operator.", u->nick, av[1]);
		CLEAR_SEGV_INMODULE();
		return NS_SUCCESS;
	}

	/* Process command list */
	cmdnode = hash_lookup(bot_ptr->botcmds, av[1]);
	if (cmdnode) {
		cmd_ptr = hnode_get(cmdnode);	
		cmdlevel = calc_cmd_ulevel(cmd_ptr);
		/* Is user authorised to issue this command? */
		if (userlevel < cmdlevel) {
			prefmsg (u->nick, bot_ptr->nick, "Permission Denied");
			chanalert (bot_ptr->nick, "%s tried to use %s, but is not authorised", u->nick, cmd_ptr->cmd);
			nlog (LOG_NORMAL, LOG_MOD, "%s tried to use %s, but is not authorised", u->nick, cmd_ptr->cmd);
			CLEAR_SEGV_INMODULE();
			return NS_SUCCESS;
		}
		/* First two parameters are bot name and command name so 
		 * subtract 2 to get parameter count */
		if((ac - 2) < cmd_ptr->minparams ) {
			prefmsg (u->nick, bot_ptr->nick, "Syntax error: insufficient parameters");
			prefmsg (u->nick, bot_ptr->nick, "/msg %s HELP %s for more information", bot_ptr->nick, cmd_ptr->cmd);
			CLEAR_SEGV_INMODULE();
			return NS_SUCCESS;
		}
		/* Seems OK so report the command call so modules do not have to */
		chanalert (bot_ptr->nick, "%s used %s", u->nick, cmd_ptr->cmd);
		/* Grab the parameters for the log so modules do not have to log */
		if(ac > 2) {
			parambuf = joinbuf(av, ac, 2);
			nlog (LOG_NORMAL, LOG_MOD, "%s used %s %s", u->nick, cmd_ptr->cmd, parambuf);
			free(parambuf);
		} else {
			nlog (LOG_NORMAL, LOG_MOD, "%s used %s", u->nick, cmd_ptr->cmd);
		}
		/* call handler */
		cmd_ptr->handler(u, av, ac);
		CLEAR_SEGV_INMODULE();
		return NS_SUCCESS;
	}

	/* Handle intrinsic commands */
	/* Help */
	if (!strcasecmp(av[1], "HELP")) {
		bot_cmd_help(bot_ptr, u, av, ac);
		CLEAR_SEGV_INMODULE();
		return NS_SUCCESS;
	}
	/* Handle SET if we have it */
	if (bot_ptr->bot_settings && !strcasecmp(av[1], "SET") ) {
		bot_cmd_set(bot_ptr, u, av, ac);
		CLEAR_SEGV_INMODULE();
		return NS_SUCCESS;
	}

#if 0
	/* About */
	if (!strcasecmp(av[1], "ABOUT")) {
		bot_cmd_about(bot_ptr, u, av, ac);
		CLEAR_SEGV_INMODULE();
		return NS_SUCCESS;
	}

	/* Version */
	if (!strcasecmp(av[1], "VERSION")) {
		bot_cmd_version(bot_ptr, u, av, ac);
		CLEAR_SEGV_INMODULE();
		return NS_SUCCESS;
	}
#endif

	/* We have run out of commands so report failure */
	prefmsg (u->nick, bot_ptr->nick, "Syntax error: unknown command: \2%s\2", av[1]);
	chanalert (bot_ptr->nick, "%s requested %s, but that is an unknown command", u->nick, av[1]);
	CLEAR_SEGV_INMODULE();
	return NS_SUCCESS;
}

/** @brief bot_cmd_help process bot help command
 *
 * @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int 
bot_cmd_help (ModUser* bot_ptr, User * u, char **av, int ac)
{
	char* curlevelmsg=NULL;
	int donemsg=0;
	bot_cmd* cmd_ptr;
	int curlevel, lowlevel;
	hnode_t *cmdnode;
	hscan_t hs;
	int userlevel;
	int cmdlevel;

	userlevel = UserLevel(u);

	/* If no parameter to help, generate main help text */
	if (ac < 3) {
		lowlevel = 0;
		curlevel = NS_ULEVEL_OPER;
		chanalert (bot_ptr->nick, "%s requested %s help", u->nick, bot_ptr->nick);
		nlog (LOG_NORMAL, LOG_MOD, "%s requested %s help", u->nick, bot_ptr->nick);
		prefmsg(u->nick, bot_ptr->nick, "The following commands can be used with %s:", bot_ptr->nick);

		/* Handle intrinsic commands */
		cmd_ptr = intrinsic_commands;
		while(cmd_ptr->cmd) {
			/* Check for module override */	
			if(!hash_lookup(bot_ptr->botcmds, cmd_ptr->cmd))
				prefmsg(u->nick, bot_ptr->nick, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->onelinehelp);
			cmd_ptr++;
		}
		/* Do we have a set command? */
		if(bot_ptr->bot_settings && userlevel >= bot_ptr->set_ulevel) {
			prefmsg(u->nick, bot_ptr->nick, "SET                 Configure %s", bot_ptr->nick);
		}
		restartlevel:
		hash_scan_begin(&hs, bot_ptr->botcmds);
		while ((cmdnode = hash_scan_next(&hs)) != NULL) {
			cmd_ptr = hnode_get(cmdnode);
			cmdlevel = calc_cmd_ulevel(cmd_ptr);
			if ((cmdlevel < curlevel) && (cmdlevel >= lowlevel)) {
				if(curlevelmsg && !donemsg) {
					prefmsg(u->nick, bot_ptr->nick, "\2Additional commands available to %s:\2", curlevelmsg);
					donemsg = 1;
				}
				/* Warn about missing help text then skip */ 
				if(!cmd_ptr->onelinehelp) {
					/* Missing help text!!! */
					nlog (LOG_WARNING, LOG_MOD, "Missing one line help text for command %s", cmd_ptr->cmd);
				} else {					
					prefmsg(u->nick, bot_ptr->nick, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->onelinehelp);
				}
			}
		}
		if (userlevel >= curlevel) {
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
						curlevel = (NS_ULEVEL_ROOT + 1);
						lowlevel = NS_ULEVEL_ROOT;
						curlevelmsg=help_level_title[2];
						donemsg=0;
						goto restartlevel;
				default:	
						break;
			}
		}						
		/* Generate help on help footer text */
		prefmsg(u->nick, bot_ptr->nick, " ");
		prefmsg(u->nick, bot_ptr->nick, "To use a command, type");
		prefmsg(u->nick, bot_ptr->nick, "    \2/msg %s command\2", bot_ptr->nick);
		prefmsg(u->nick, bot_ptr->nick, "For for more information on a command, type");
		prefmsg(u->nick, bot_ptr->nick, "    \2/msg %s HELP command\2.", bot_ptr->nick);
		return 1;
	}
	chanalert (bot_ptr->nick, "%s requested %s help on %s", u->nick, bot_ptr->nick, av[2]);
	nlog (LOG_NORMAL, LOG_MOD, "%s requested %s help on %s", u->nick, bot_ptr->nick, av[2]);

	/* Process command list */
	cmdnode = hash_lookup(bot_ptr->botcmds, av[2]);
	if (cmdnode) {
		cmd_ptr = hnode_get(cmdnode);
		cmdlevel = calc_cmd_ulevel(cmd_ptr);
		if (UserLevel (u) < cmdlevel) {
			prefmsg (u->nick, bot_ptr->nick, "Permission Denied");
			return 1;
		}		
		if(!cmd_ptr->helptext) {
			/* Warn about missing help text then skip */ 
			nlog (LOG_WARNING, LOG_MOD, "Missing help text for command %s", cmd_ptr->cmd);
			return 1;
		}
		privmsg_list (u->nick, bot_ptr->nick, cmd_ptr->helptext);
		return 1;
	}

	/* Handle intrinsic commands */
	cmd_ptr = intrinsic_commands;
	while(cmd_ptr->cmd) {
		if (!strcasecmp(av[2], cmd_ptr->cmd)) {
			privmsg_list (u->nick, bot_ptr->nick, cmd_ptr->helptext);
			return 1;
		}
		cmd_ptr++;
	}
	/* Handle SET if we have it */
	if (bot_ptr->bot_settings && userlevel >= bot_ptr->set_ulevel && !strcasecmp(av[2], "SET") ) {
		bot_setting* set_ptr;
		set_ptr = bot_ptr->bot_settings;
		/* Display HELP SET intro text and LIST command */
		privmsg_list (u->nick, bot_ptr->nick, cmd_help_set);
		/* Display option specific text for current user level */
		while(set_ptr->option)
		{
			if(set_ptr->helptext && userlevel >= set_ptr->ulevel)
			{
				privmsg_list (u->nick, bot_ptr->nick, set_ptr->helptext);
				prefmsg(u->nick, bot_ptr->nick, " ");
			}
			set_ptr++;
		}
		return 1;
	}

	/* Command not found so report as unknown */
	prefmsg (u->nick, bot_ptr->nick, "No help available or unknown help topic: \2%s\2", av[2]);
	return 1;
}

/**	Support function for command handlers to call to check that target nick 
 *	is not the bot and is on IRC. Done in core to avoid module code bloat.
 */ 
int is_target_valid(char* bot_name, User* u, char* target_nick)
{
	/* Check for message to self */
	if (!strcasecmp(target_nick, bot_name)) {
		prefmsg(u->nick, bot_name,
			"Surely we have better things to do with our time than make a service message itself!");
		return 0;
	}
	/* Check target user is on IRC */
	if (!finduser(target_nick)) {
		prefmsg(u->nick, bot_name,
			"%s cannot be found on IRC, your message was not sent. Please check the spelling and try again.", target_nick);
		return 0;
	}
	/* User OK */
	return 1;
}

/** @brief bot_cmd_set process bot set command
 *	work in progress
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int 
bot_cmd_set (ModUser* bot_ptr, User * u, char **av, int ac)
{
	int intval;
	bot_setting* set_ptr;
	int userlevel;

	if (ac < 3) {
		prefmsg(u->nick, bot_ptr->nick,
			"Invalid Syntax. /msg %s HELP SET for more info", 
			bot_ptr->nick);
		return 1;
	} 

	userlevel = UserLevel(u);
	if( userlevel < bot_ptr->set_ulevel) {
		prefmsg (u->nick, bot_ptr->nick, "Permission Denied");
		chanalert (bot_ptr->nick, "%s tried to use SET, but is not authorised", u->nick);
		nlog (LOG_NORMAL, LOG_MOD, "%s tried to use SET, but is not authorised", u->nick);
		return 1;
	}

	if(!strcasecmp(av[2], "LIST"))
	{
		prefmsg(u->nick, bot_ptr->nick, "Current %s settings:", bot_ptr->nick);
		set_ptr = bot_ptr->bot_settings;
		while(set_ptr->option)
		{
			/* Only list authorised SETTINGS */
			if( userlevel >= set_ptr->ulevel) {
				switch(set_ptr->type) {
					case SET_TYPE_BOOLEAN:
						prefmsg(u->nick, bot_ptr->nick, "%s: %s",
							set_ptr->option, *(int*)set_ptr->varptr ? "Enabled" : "Disabled");
						break;
					case SET_TYPE_INT:
						if(set_ptr->desc) {
								prefmsg(u->nick, bot_ptr->nick, "%s: %d %s",
									set_ptr->option, *(int*)set_ptr->varptr, set_ptr->desc);
							} else {
								prefmsg(u->nick, bot_ptr->nick, "%s: %d",
									set_ptr->option, *(int*)set_ptr->varptr);
							}
						break;
					case SET_TYPE_STRING:
						prefmsg(u->nick, bot_ptr->nick, "%s: %s",
							set_ptr->option, (char*)set_ptr->varptr);
						break;
					default:
						prefmsg(u->nick, bot_ptr->nick, "%s: uses an unsupported type",
							set_ptr->option);
						break;
				}
			}
			set_ptr++;
		}
		return 1;
	}

	if (ac < 4) {
		prefmsg(u->nick, bot_ptr->nick,
			"Invalid Syntax. /msg %s HELP SET for more info", 
			bot_ptr->nick);
		return 1;
	} 

	set_ptr = bot_ptr->bot_settings;
	while(set_ptr->option)
	{
		if(!strcasecmp(av[2], set_ptr->option))
			break;
		set_ptr++;
	}
	if(!set_ptr->option) {
		prefmsg(u->nick, bot_ptr->nick,
			"Unknown set option. /msg %s HELP SET for more info",
			bot_ptr->nick);
		return 1;
	}
	if( userlevel < set_ptr->ulevel) {
		prefmsg (u->nick, bot_ptr->nick, "Permission Denied");
		chanalert (bot_ptr->nick, "%s tried to use SET %s, but is not authorised", u->nick, av[2]);
		nlog (LOG_NORMAL, LOG_MOD, "%s tried to use SET %s, but is not authorised", u->nick, av[2]);
		return 1;
	}
	switch(set_ptr->type) {
		case SET_TYPE_BOOLEAN:
			if (!strcasecmp(av[3], "ON")) {
				*(int*)set_ptr->varptr = 1;
				SetConf((void *) 1, CFGBOOL, set_ptr->confitem);
				chanalert(bot_ptr->nick, "%s enabled by \2%s\2", 
					set_ptr->option, u->nick);
				nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s enabled %s",
					u->nick, u->username, u->hostname, set_ptr->option);
				prefmsg(u->nick, bot_ptr->nick,
					"\2%s\2 enabled", set_ptr->option);
			} else if (!strcasecmp(av[3], "OFF")) {
				*(int*)set_ptr->varptr = 0;
				SetConf(0, CFGBOOL, set_ptr->confitem);
				chanalert(bot_ptr->nick, "%s disabled by \2%s\2", 
					set_ptr->option, u->nick);
				nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s disabled %s ", 
					u->nick, u->username, u->hostname, set_ptr->option);
				prefmsg(u->nick, bot_ptr->nick,
					"\2%s\2 disabled", set_ptr->option);
			} else {
				prefmsg(u->nick, bot_ptr->nick,
					"Invalid Syntax. /msg %s HELP SET for more info",
					bot_ptr->nick);
				return 1;
			}
			break;
		case SET_TYPE_INT:
			intval = atoi(av[3]);	
			/* atoi will return 0 for a string instead of a digit so check it! */
			if(intval == 0) {
				if(strcmp(av[3],"0")!=0) {
					prefmsg(u->nick, bot_ptr->nick,
						"%s invalid setting for %s", av[3], set_ptr->option);
					prefmsg(u->nick, bot_ptr->nick,
						"Valid values are %d to %d", set_ptr->min, set_ptr->max);
					return 1;
				}
			}
			if(set_ptr->min != -1 && intval < set_ptr->min) {
				prefmsg(u->nick, bot_ptr->nick,
					"%d out of range for %s", intval, set_ptr->option);
				prefmsg(u->nick, bot_ptr->nick,
					"Valid values are %d to %d", set_ptr->min, set_ptr->max);
				return 1;
			}
			if(set_ptr->max != -1 && intval > set_ptr->max) {
				prefmsg(u->nick, bot_ptr->nick,
					"%d out of range for %s", intval, set_ptr->option);
				prefmsg(u->nick, bot_ptr->nick,
					"Valid values are %d to %d", set_ptr->min, set_ptr->max);
				return 1;
			}
			*(int*)set_ptr->varptr = intval;
			SetConf((void *)intval, CFGINT, set_ptr->confitem);
			chanalert(bot_ptr->nick, "%s set to %d by \2%s\2", 
				set_ptr->option, intval, u->nick);
			nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s set %s to %d", 
				u->nick, u->username, u->hostname, set_ptr->option, intval);
			prefmsg(u->nick, bot_ptr->nick,
				"%s set to %d", set_ptr->option, intval);
			break;
		case SET_TYPE_STRING:
			strlcpy((char*)set_ptr->varptr, av[3], set_ptr->max);
			SetConf((void *)av[3], CFGSTR, set_ptr->confitem);
			chanalert(bot_ptr->nick, "%s set to %s by \2%s\2", 
				set_ptr->option, av[3], u->nick);
			nlog(LOG_NORMAL, LOG_MOD, "%s!%s@%s set %s to %s", 
				u->nick, u->username, u->hostname, set_ptr->option, av[3]);
			prefmsg(u->nick, bot_ptr->nick,
				"%s set to %s", set_ptr->option, av[3]);
			break;
		default:
			chanalert(bot_ptr->nick, "Unsupported SET type %d requested by %s for %s %s", 
				set_ptr->type, u->nick, set_ptr->option, av[3]);
			nlog(LOG_NORMAL, LOG_MOD, "Unsupported SET type %d requested by %s for %s %s", 
				set_ptr->type, u->nick, set_ptr->option, av[3]);
			prefmsg(u->nick, bot_ptr->nick,"Unsupported SET type %d for %s %s", 
				set_ptr->type, set_ptr->option, av[3]);
			break;
	}
	return 1;
}

#if 0
/** @brief bot_cmd_about process bot about command
 *	work in progress
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int bot_cmd_about (ModUser* bot_ptr, User * u, char **av, int ac)
{
	return 1;
}
/** @brief bot_cmd_version process bot version command
 *	work in progress
 *  @return NS_SUCCESS if suceeds, NS_FAILURE if not 
 */
static int bot_cmd_version (ModUser* bot_ptr, User * u, char **av, int ac)
{
	return 1;
}
#endif
