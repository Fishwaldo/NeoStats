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
bot_cmd_help (User * u, char **av, int ac)
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

