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

/* hash for command list */
hash_t *botcmds;

extern const char *ns_help_on_help[];
extern bot_cmd ns_commands[];

void bot_cmd_help (ModUser* bot_ptr, User * u, char **av, int ac);

int 
init_services() 
{
	bot_cmd *cmd_ptr;
	hnode_t *cmdnode;
	
	/* init bot hash first */
	
	botcmds = hash_create(-1, 0, 0);

	/* Process command list */
	cmd_ptr = ns_commands;
	while(cmd_ptr->handler) {
		cmdnode = hnode_create(cmd_ptr);
		hash_insert(botcmds, cmdnode, cmd_ptr->cmd);
		cmd_ptr++;
	}
	return NS_SUCCESS;
}
	
int 
add_bot_cmd(hash_t* cmd_hash, bot_cmd* cmd_ptr) 
{
	hnode_t *cmdnode;
	
	cmdnode = hnode_create(cmd_ptr);
	hash_insert(cmd_hash, cmdnode, cmd_ptr->cmd);
	nlog(LOG_DEBUG2, LOG_CORE, "Added a new command %s to Services Bot", cmd_ptr->cmd);
	return NS_SUCCESS;
}

int 
del_bot_cmd(hash_t* cmd_hash, bot_cmd* cmd_ptr) 
{
	hnode_t *cmdnode;
	
	cmdnode = hash_lookup(cmd_hash, cmd_ptr->cmd);
	if (cmdnode) {
		hash_delete(cmd_hash, cmdnode);
		hnode_destroy(cmdnode);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

int 
add_bot_cmd_list(ModUser* bot_ptr, bot_cmd* cmd_list) 
{
	while(cmd_list->cmd) {
		add_bot_cmd(bot_ptr->botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

int 
del_bot_cmd_list(ModUser* bot_ptr, bot_cmd* cmd_list) 
{
	while(cmd_list->cmd) {
		add_bot_cmd(bot_ptr->botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

int 
add_services_cmd_list(bot_cmd* cmd_list) 
{
	while(cmd_list->cmd) {
		add_bot_cmd(botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

int 
del_services_cmd_list(bot_cmd* cmd_list) 
{
	while(cmd_list->cmd) {
		del_bot_cmd(botcmds, cmd_list);
		cmd_list++;
	}
	return NS_SUCCESS;
}

void
servicesbot (char *nick, char **av, int ac)
{
	User *u;
	bot_cmd* cmd_ptr;
	hnode_t *cmdnode;

	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Unable to finduser %s (%s)", nick, s_Services);
		return;
	}
	SET_SEGV_LOCATION();
	
	me.requests++;

	/* Check user authority to use this command set */
	if (me.onlyopers && (UserLevel (u) < NS_ULEVEL_OPER)) {
		prefmsg (u->nick, s_Services, "This service is only available to IRCops.");
		chanalert (s_Services, "%s Requested %s, but he is Not an Operator!", u->nick, av[1]);
		return;
	}
	/* Process command list */
	cmdnode = hash_lookup(botcmds, av[1]);

	if (cmdnode) {
		cmd_ptr = hnode_get(cmdnode);
	
		/* Is user authorised to issue this command? */
		if (UserLevel (u) < cmd_ptr->ulevel) {
			prefmsg (u->nick, s_Services, "Permission Denied");
			chanalert (s_Services, "%s tried to use %s, but is not authorised", u->nick, cmd_ptr->cmd);
			return;
		}
		/* First two parameters are bot name and command name so 
		 * subtract 2 to get parameter count */
		if((ac - 2) < cmd_ptr->minparams ) {
			prefmsg (u->nick, s_Services, "Syntax error: insufficient parameters");
			prefmsg (u->nick, s_Services, "/msg %s HELP %s for more information", s_Services, cmd_ptr->cmd);
			return;
		}
		/* Missing handler?! */
		if(!cmd_ptr->handler) {
			return;
		}
		/* Seems OK so report the command call then call appropriate handler */
		chanalert (s_Services, "%s used %s", u->nick, cmd_ptr->cmd);
		cmd_ptr->handler(u, av, ac);
		return;
	}

	/* We have run out of commands so report failure */
	prefmsg (u->nick, s_Services, "Syntax error: unknown command: \2%s\2", av[1]);
	chanalert (s_Services, "%s requested %s, but that is an unknown command", u->nick, av[1]);
}

void
run_bot_cmd (ModUser* bot_ptr, char *nick, char **av, int ac)
{
	User *u;
	bot_cmd* cmd_ptr;
	hnode_t *cmdnode;

	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Unable to finduser %s (%s)", nick, bot_ptr->nick);
		return;
	}
	SET_SEGV_LOCATION();
	
	me.requests++;

	/* Check user authority to use this command set */
	if (me.onlyopers && (UserLevel (u) < NS_ULEVEL_OPER)) {
		prefmsg (u->nick, bot_ptr->nick, "This service is only available to IRCops.");
		chanalert (bot_ptr->nick, "%s Requested %s, but he is Not an Operator!", u->nick, av[1]);
		return;
	}

	/* force support for help command */
	if (!strcasecmp(av[1], "HELP")) {
		bot_cmd_help(bot_ptr, u, av, ac);
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
			return;
		}
		/* Seems OK so report the command call then call appropriate handler */
		chanalert (bot_ptr->nick, "%s used %s", u->nick, cmd_ptr->cmd);
		cmd_ptr->handler(u, av, ac);
		return;
	}

	/* We have run out of commands so report failure */
	prefmsg (u->nick, bot_ptr->nick, "Syntax error: unknown command: \2%s\2", av[1]);
	chanalert (bot_ptr->nick, "%s requested %s, but that is an unknown command", u->nick, av[1]);
}


void 
services_cmd_help (User * u, char **av, int ac)
{
	bot_cmd* cmd_ptr;
	int curlevel, lowlevel;
	hnode_t *cmdnode;
	hscan_t hs;

	if (ac < 3) {
		lowlevel = 0;
		curlevel = NS_ULEVEL_OPER;
		chanalert (s_Services, "%s Requested %s Help", u->nick, s_Services);
		prefmsg(u->nick, s_Services, "The following commands can be used with NeoStats:");

		restartlevel:
		hash_scan_begin(&hs, botcmds);
		while ((cmdnode = hash_scan_next(&hs)) != NULL) {
			cmd_ptr = hnode_get(cmdnode);
			if ((cmd_ptr->ulevel < curlevel) && (cmd_ptr->ulevel >= lowlevel)) {
				prefmsg(u->nick, s_Services, "%-20s %s", cmd_ptr->cmd, cmd_ptr->onelinehelp);
			}
		}
		if (UserLevel(u) >= curlevel) {
			switch (curlevel) {
				case NS_ULEVEL_OPER:
						curlevel = NS_ULEVEL_ADMIN;
						lowlevel = NS_ULEVEL_OPER;
						prefmsg(u->nick, s_Services, "\2Commands Available to Opers and Above:\2");
						goto restartlevel;
				case NS_ULEVEL_ADMIN:
						curlevel = NS_ULEVEL_ROOT;
						lowlevel = NS_ULEVEL_ADMIN;
						prefmsg(u->nick, s_Services, "\2Commands Available to Service Admins and Above:\2");
						goto restartlevel;
				case NS_ULEVEL_ROOT:
						curlevel = 201;
						lowlevel = 200;
						prefmsg(u->nick, s_Services, "\2Commands Available to Service Roots:\2");
						goto restartlevel;
				default:	
						break;
			}
		}						
		privmsg_list (u->nick, s_Services, ns_help_on_help);
		return;
	}
	chanalert (s_Services, "%s Requested %s Help on %s", u->nick, s_Services, av[2]);

	cmdnode = hash_lookup(botcmds, av[2]);
	if (cmdnode) {
		cmd_ptr = hnode_get(cmdnode);
		if (UserLevel (u) < cmd_ptr->ulevel) {
			prefmsg (u->nick, s_Services, "Permission Denied");
			return;
		}		
		if(!cmd_ptr->helptext) {
			/* Missing help text!!! */
			return;
		}
		privmsg_list (u->nick, s_Services, cmd_ptr->helptext);
		return;
	}
	prefmsg (u->nick, s_Services, "Unknown Help Topic: \2%s\2", av[2]);
}

const char * help_level_title[]=
{
	"\2Commands Available to Opers and Above:\2",
	"\2Commands Available to Service Admins and Above:\2",
	"\2Commands Available to Service Roots:\2",
};

void 
bot_cmd_help (ModUser* bot_ptr, User * u, char **av, int ac)
{
	char* curmsg=NULL;
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
				if(curmsg && !donemsg) {
					prefmsg(u->nick, bot_ptr->nick, curmsg);
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
						curmsg=help_level_title[0];
						donemsg=0;
						goto restartlevel;
				case NS_ULEVEL_ADMIN:
						curlevel = NS_ULEVEL_ROOT;
						lowlevel = NS_ULEVEL_ADMIN;
						curmsg=help_level_title[1];
						donemsg=0;
						goto restartlevel;
				case NS_ULEVEL_ROOT:
						curlevel = 201;
						lowlevel = 200;
						curmsg=help_level_title[2];
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
	prefmsg (u->nick, bot_ptr->nick, "Unknown Help Topic: \2%s\2", av[2]);
}

