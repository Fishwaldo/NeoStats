/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
#include "sock.h"
#include "ns_help.h"
#include "users.h"
#include "server.h"
#include "chans.h"
#include "hash.h"

/* hash for command list */
hash_t *botcmds;

static char quitmsg[BUFSIZE];
static char no_reason[]="no reason given";

static void ns_set_debug (User * u, char **av, int ac);
static void ns_shutdown (User * u, char **av, int ac);
static void ns_reload (User * u, char **av, int ac);
static void ns_logs (User * u, char **av, int ac);
static void ns_jupe (User * u, char **av, int ac);
#ifdef USE_RAW
static void ns_raw (User * u, char **av, int ac);
#endif
static void ns_user_dump (User * u, char **av, int ac);
static void ns_server_dump (User * u, char **av, int ac);
static void ns_chan_dump (User * u, char **av, int ac);
static void ns_info (User * u, char **av, int ac);
static void ns_version (User * u, char **av, int ac);
static void ns_show_level (User * u, char **av, int ac);
static void ns_do_help (User * u, char **av, int ac);
static void ns_load_module (User * u, char **av, int ac);
static void ns_unload_module (User * u, char **av, int ac);

bot_cmd ns_commands[]=
{
	{"HELP",		ns_do_help,		0, 	0,					ns_help_on_help, 	1, 	ns_help_help_oneline},
	{"LEVEL",		ns_show_level,	0, 	0,					ns_level_help, 		1, 	ns_level_help_oneline},
	{"INFO",		ns_info,		0, 	0,					ns_info_help, 		1, 	ns_info_help_oneline},
	{"VERSION",		ns_version,		0, 	0,					ns_version_help, 	1, 	ns_version_help_oneline},
	{"SHUTDOWN",	ns_shutdown,	0, 	NS_ULEVEL_ADMIN, 	ns_shutdown_help, 	1, 	ns_shutdown_help_oneline},
	{"RELOAD",		ns_reload,		0, 	NS_ULEVEL_ADMIN, 	ns_reload_help,		1, 	ns_reload_help_oneline},
	{"LOGS",		ns_logs,		0, 	NS_ULEVEL_OPER, 	ns_logs_help,		1, 	ns_logs_help_oneline},
	{"MODLIST",		list_modules,	0, 	NS_ULEVEL_ADMIN,  	ns_modlist_help, 	1,	ns_modlist_help_oneline},
	{"LOAD",		ns_load_module,	1, 	NS_ULEVEL_ADMIN, 	ns_load_help, 		1, 	ns_load_help_oneline},
	{"UNLOAD",		ns_unload_module,1, NS_ULEVEL_ADMIN, 	ns_unload_help, 	1, 	ns_unload_help_oneline},
	{"JUPE",		ns_jupe,		1, 	NS_ULEVEL_ADMIN, 	ns_jupe_help,		1, 	ns_jupe_help_oneline},
#ifdef USE_RAW																		
	{"RAW",			ns_raw,			0, 	NS_ULEVEL_ADMIN, 	ns_raw_help, 		1, 	ns_raw_help_oneline},
#endif																				
	{"DEBUG",		ns_set_debug,	1, 	NS_ULEVEL_ROOT,  	ns_debug_help,		1,	ns_debug_help_oneline},
	{"BOTLIST",		list_bots,		0, 	NS_ULEVEL_ROOT,  	ns_botlist_help,	1,	ns_botlist_help_oneline},
	{"SOCKLIST",	list_sockets,	0, 	NS_ULEVEL_ROOT,  	ns_socklist_help, 	1,	ns_socklist_help_oneline},
	{"TIMERLIST",	list_timers,	0, 	NS_ULEVEL_ROOT,  	ns_timerlist_help, 	1,	ns_timerlist_help_oneline},
	{"BOTCHANLIST",	list_bot_chans,	0, 	NS_ULEVEL_ROOT,  	ns_botchanlist_help,1,	ns_botchanlist_help_oneline},
	{"USERDUMP",	ns_user_dump,	0, 	NS_ULEVEL_ROOT,  	ns_userdump_help, 	1,	ns_userdump_help_oneline},
	{"CHANDUMP",	ns_chan_dump,	0, 	NS_ULEVEL_ROOT,  	ns_chandump_help, 	1,	ns_chandump_help_oneline},
	{"SERVERDUMP",	ns_server_dump,	0, 	NS_ULEVEL_ROOT,  	ns_serverdump_help, 1,	ns_serverdump_help_oneline},
	{NULL,			NULL,			0, 	0,			NULL, 			0,	NULL}
};

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
add_services_cmd(const char *cmd, bot_cmd_handler handler, int minparams, int ulevel, const char** helptext, const char* onelinehelp) 
{
	bot_cmd *cmd_ptr;
	hnode_t *cmdnode;
	
	cmd_ptr = malloc(sizeof(bot_cmd));
	cmd_ptr->cmd = cmd;
	cmd_ptr->handler = handler;
	cmd_ptr->minparams = minparams;
	cmd_ptr->ulevel = ulevel;
	cmd_ptr->helptext = helptext;
	cmd_ptr->internal = 0;
	cmd_ptr->onelinehelp = onelinehelp;
	
	cmdnode = hnode_create(cmd_ptr);
	hash_insert(botcmds, cmdnode, cmd_ptr->cmd);
	nlog(LOG_DEBUG2, LOG_CORE, "Added a new command %s to Services Bot", cmd);
	return 1;
}

int 
add_services_cmd_list(bot_cmd* cmd_list) 
{
	bot_cmd *cmd_ptr;
	hnode_t *cmdnode;
	
	while(cmd_list->cmd) {
		cmd_ptr = malloc(sizeof(bot_cmd));
		cmd_ptr->cmd = cmd_list->cmd;
		cmd_ptr->handler = cmd_list->handler;
		cmd_ptr->minparams = cmd_list->minparams;
		cmd_ptr->ulevel = cmd_list->ulevel;
		cmd_ptr->helptext = cmd_list->helptext;
		cmd_ptr->internal = 0;
		cmd_ptr->onelinehelp = cmd_list->onelinehelp;
		
		cmdnode = hnode_create(cmd_ptr);
		hash_insert(botcmds, cmdnode, cmd_ptr->cmd);
		nlog(LOG_DEBUG2, LOG_CORE, "Added a new command %s to Services Bot", cmd_list->cmd);
		cmd_list++;
	}
	return 1;
}

int 
del_services_cmd(const char *cmd) 
{
	bot_cmd *cmd_ptr;
	hnode_t *cmdnode;
	
	cmdnode = hash_lookup(botcmds, cmd);
	if (cmdnode) {
		hash_delete(botcmds, cmdnode);
		cmd_ptr = hnode_get(cmdnode);
		hnode_destroy(cmdnode);
		/* free if its a external (malloc'd) command */
		if (cmd_ptr->internal == 0) {
			free(cmd_ptr);
		}
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

int 
del_services_cmd_list(bot_cmd* cmd_list) 
{
	bot_cmd *cmd_ptr;
	hnode_t *cmdnode;
	
	while(cmd_list->cmd) {
		cmdnode = hash_lookup(botcmds, cmd_list->cmd);
		if (cmdnode) {
			hash_delete(botcmds, cmdnode);
			cmd_ptr = hnode_get(cmdnode);
			hnode_destroy(cmdnode);
			/* free if its a external (malloc'd) command */
			if (cmd_ptr->internal == 0) {
				free(cmd_ptr);
			}
		}
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

static void
ns_shutdown (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();

	char *tmp;
	if (ac <= 2) {
		ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested SHUTDOWN for %s.", 
			u->nick, u->username, u->hostname, no_reason);
	} else {
		tmp = joinbuf (av, ac, 2);
		chanalert (s_Services, "%s Wants me to SHUTDOWN for %s", u->nick, tmp);
		ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested SHUTDOWN for %s.", 
			u->nick, u->username, u->hostname, tmp);
		free (tmp);
	}
	globops (s_Services, quitmsg);
	nlog (LOG_NOTICE, LOG_CORE, quitmsg);
	do_exit (NS_EXIT_NORMAL, quitmsg);
}

static void
ns_reload (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();

	char *tmp;
	if (ac <= 2) {
		prefmsg (u->nick, s_Services, "You must supply a Reason to Reload");
		return;
	}
	tmp = joinbuf (av, ac, 2);
	chanalert (s_Services, "%s Wants me to RELOAD! for %s", u->nick, tmp);
	ircsnprintf (quitmsg, BUFSIZE, "%s [%s](%s) requested RELOAD for %s.", 
		u->nick, u->username, u->hostname, tmp);
	free (tmp);

	globops (s_Services, quitmsg);
	nlog (LOG_NOTICE, LOG_CORE, quitmsg);

	do_exit (NS_EXIT_RELOAD, quitmsg);
}

static void
ns_logs (User * u, char **av, int ac)
{
#ifdef DEBUG
	prefmsg (u->nick, s_Services, "This command is disabled while in DEBUG.");
#else
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	fp = fopen ("logs/NeoStats.log", "r");
	if (!fp) {
		prefmsg (u->nick, s_Services, "Unable to open neostats.log");
		return;
	}
	while (fgets (buf, BUFSIZE, fp)) {
		buf[strlen (buf)] = '\0';
		prefmsg (u->nick, s_Services, buf);
	}
	fclose (fp);
#endif
}

static void
ns_jupe (User * u, char **av, int ac)
{
	char infoline[255];

	SET_SEGV_LOCATION();
	ircsnprintf (infoline, 255, "[Jupitered by %s]", u->nick);
	sserver_cmd (av[2], 1, infoline);
	nlog (LOG_NOTICE, LOG_CORE, "%s!%s@%s jupitered %s", u->nick, u->username, u->hostname, av[2]);
	chanalert (s_Services, "%s Wants to JUPE this Server %s", u->nick, av[2]);
	prefmsg(u->nick, s_Services, "%s has been Jupitered", av[2]);
}

static void
ns_set_debug (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if ((!strcasecmp(av[2], "YES")) || (!strcasecmp(av[2], "ON"))) {
		me.debug_mode = 1;
		globops (me.name, "\2DEBUG MODE\2 Activated by %s", u->nick);
		prefmsg (u->nick, s_Services, "Debuging Mode Enabled!");
	} else if ((!strcasecmp(av[2], "NO")) || (!strcasecmp(av[2], "OFF"))) {
		me.debug_mode = 0;
		globops (me.name, "\2DEBUG MODE\2 Deactivated by %s", u->nick);
		prefmsg (u->nick, s_Services, "Debuging Mode Disabled");
	} else {
		prefmsg(u->nick, s_Services,
			"Syntax Error: /msg %s HELP DEBUG for more info",
			s_Services);
		return;
	}
}

static void
ns_user_dump (User * u, char **av, int ac)
{
	if (!me.debug_mode) {
		prefmsg (u->nick, s_Services, "\2Error:\2 Debug Mode Disabled");
		return;
	}
	chanalert (s_Services, "\2DEBUG\2 \2%s\2 Requested a UserDump!", u->nick);
	UserDump (((ac < 3) ? NULL : av[2]));
}
static void
ns_server_dump (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (u->nick, s_Services, "\2Error:\2 Debug Mode Disabled");
		return;
	}
	chanalert (s_Services, "\2DEBUG\2 \2%s\2 Requested a ServerDump!", u->nick);
	ServerDump ();
}
static void
ns_chan_dump (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	if (!me.debug_mode) {
		prefmsg (u->nick, s_Services, "\2Error:\2 Debug Mode Disabled");
		return;
	}
	chanalert (s_Services, "\2DEBUG\2 \2%s\2 Requested a ChannelDump!", u->nick);
	ChanDump (((ac < 3) ? NULL : av[2]));
}

static void 
ns_info (User * u, char **av, int ac)
{
	int uptime = me.now - me.t_start;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "%s Information:", s_Services);
	if (uptime > 86400) {
		prefmsg (u->nick, s_Services, "%s up \2%ld\2 day%s, \2%02ld:%02ld\2", s_Services, uptime / 86400, (uptime / 86400 == 1) ? "" : "s", (uptime / 3600) % 24, (uptime / 60) % 60);
	} else if (uptime > 3600) {
		prefmsg (u->nick, s_Services, "%s up \2%ld hour%s, %ld minute%s\2", s_Services, uptime / 3600, uptime / 3600 == 1 ? "" : "s", (uptime / 60) % 60, (uptime / 60) % 60 == 1 ? "" : "s");
	} else if (uptime > 60) {
		prefmsg (u->nick, s_Services, "%s up \2%ld minute%s, %ld second%s\2", s_Services, uptime / 60, uptime / 60 == 1 ? "" : "s", uptime % 60, uptime % 60 == 1 ? "" : "s");
	} else {
		prefmsg (u->nick, s_Services, "%s up \2%ld second%s\2", s_Services, uptime, uptime == 1 ? "" : "s");
	}
	prefmsg (u->nick, s_Services, "Sent %ld Messages Totaling %ld Bytes", me.SendM, me.SendBytes);
	prefmsg (u->nick, s_Services, "Received %ld Messages, Totaling %ld Bytes", me.RcveM, me.RcveBytes);
	prefmsg (u->nick, s_Services, "Reconnect Time: %d", me.r_time);
	prefmsg (u->nick, s_Services, "Statistic Requests: %d", me.requests);
	prefmsg (u->nick, s_Services, "Max Sockets: %d (in use: %d)", me.maxsocks, me.cursocks);
	if (me.debug_mode)
		prefmsg (u->nick, s_Services, "Debugging Mode is \2ON!\2");
	else
		prefmsg (u->nick, s_Services, "Debugging Mode is Disabled!");
	prefmsg (u->nick, s_Services, "End of Information.");
}
static void
ns_version (User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "\2NeoStats Version Information\2");
	prefmsg (u->nick, s_Services, "NeoStats Version: %d.%d.%d%s", MAJOR, MINOR, REV, ircd_version);
	prefmsg (u->nick, s_Services, "http://www.neostats.net");
}

static void 
ns_show_level (User * u, char **av, int ac)
{
	if(ac > 2) {
		User * otheruser;
		otheruser = finduser(av[2]);
		if(otheruser) {
			prefmsg (u->nick, s_Services, "User Level for %s is %d", otheruser->nick, UserLevel (otheruser));
		}
	} else {
		prefmsg (u->nick, s_Services, "Your Level is %d", UserLevel (u));
	}
}

static void 
ns_do_help (User * u, char **av, int ac)
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

static void 
ns_load_module (User * u, char **av, int ac)
{
	if (load_module (av[2], u) == NS_SUCCESS) {
		chanalert (s_Services, "%s Loaded Module %s", u->nick, av[2]);
	} else {
		chanalert (s_Services, "%s Tried to Load Module %s, but Failed", u->nick, av[2]);
	}
}

static void 
ns_unload_module (User * u, char **av, int ac)
{
	if (unload_module (av[2], u) > 0) {
		chanalert (s_Services, "%s Unloaded Module %s", u->nick, av[2]);
	}
}

#ifdef USE_RAW
static void
ns_raw (User * u, char **av, int ac)
{
	char *message;

	message = joinbuf (av, ac, 2);
	SET_SEGV_LOCATION();
	chanalert (s_Services, "\2RAW COMMAND\2 \2%s\2 Issued a Raw Command!(%s)", u->nick, message);
	nlog (LOG_INFO, LOG_CORE, "RAW COMMAND %sIssued a Raw Command!(%s)", u->nick, message);
	sts(message);
	free (message);
}
#endif
