/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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

#include <stdio.h>
#include "neostats.h"
#include "dotconf.h"
#include "services.h"

void sr_cb_config(char *arg, int configtype);
static int ext_auth_list(User *u, char **av, int ac);

const char *sr_help_list[] = {
	"Syntax: \2SRLIST\2",
	"",
	"This command lists the ServiceRoots defined in the configuration file",
	NULL
};

const char sr_help_list_oneline[] = "ServiceRoots List";

bot_cmd extauth_commands[]=
{
	{"SRLIST",	ext_auth_list,	0,	NS_ULEVEL_OPER, sr_help_list,	sr_help_list_oneline},
	{NULL,		NULL,			0, 	0,				NULL, 			NULL}
};

ModuleInfo module_info = {
	"ExtAuth",
	"ServiceRoots Authentication Module",
	NULL,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

static config_option options[] = {
	{"SERVICE_ROOTS", ARG_STR, sr_cb_config, 0}
};


struct srconf {
	list_t *ul;
	int auth;
} srconf;

typedef struct users {
	char nick[MAXNICK];
	char ident[MAXUSER];
	char host[MAXHOST];
	int lvl;
} users;

static int Online(char **av, int ac)
{
	add_services_cmd_list(extauth_commands);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,		Online},
	{NULL, NULL}
};


int ModInit(Module* modptr)
{
	srconf.auth = 0;
	/* only a max of 10 serviceroots */
	srconf.ul = list_create(10);
	if (!config_read(CONFIG_NAME, options) == 0) {
		nlog(LOG_WARNING, "ServiceRoots: config failed");
		/* we can't unload the extauth module so don't return -1 */
	}
	return 1;
}

void ModFini()
{
	lnode_t *un;

	un = list_first(srconf.ul);
	while (un) {
		free(lnode_get(un));
		un = list_next(srconf.ul, un);
	}
	list_destroy_nodes(srconf.ul);
	del_services_cmd_list(extauth_commands);
}

void sr_cb_config(char *arg, int configtype)
{
	lnode_t *un;
	char *nick;
	char *user;
	char *host;
	users *sru;

	SET_SEGV_LOCATION();
	if (configtype == 0) {
		if (list_isfull(srconf.ul)) {
			nlog(LOG_WARNING, "Exceeded maxium ServiceRoots(10)");
			return;
		} else {
			if (strstr(arg, "!")) {
				if (!strstr(arg, "@")) {
					nlog(LOG_WARNING, 
					     "Invalid Entry. Must be of the form nick!ident@host, was %s",
					     arg);
					return;
				}
				nick = strtok(arg, "!");
				user = strtok(NULL, "@");
				host = strtok(NULL, "");
				sru = malloc(sizeof(users));
				strlcpy(sru->nick, nick, MAXNICK);
				strlcpy(sru->ident, user, MAXUSER);
				strlcpy(sru->host, host, MAXHOST);
				sru->lvl = NS_ULEVEL_ROOT;
			} 
			un = lnode_create(sru);
			list_append(srconf.ul, un);
		}
	}
}

int __do_auth(User * u, int curlvl)
{
	lnode_t *un;
	users *sru;

	SET_SEGV_LOCATION();
	un = list_first(srconf.ul);
	while (un) {
		sru = lnode_get(un);
		if ((match(sru->nick, u->nick))
		    && (match(sru->ident, u->username))
		    && (match(sru->host, u->hostname))) {
			return (sru->lvl);
		}
		un = list_next(srconf.ul, un);
	}
	return curlvl;
}

int __list_auth(User * u)
{
	lnode_t *un;
	users *sru;

	SET_SEGV_LOCATION();
	un = list_first(srconf.ul);
	while (un) {
		sru = lnode_get(un);
		numeric(RPL_STATSOLINE, u->nick, "O %s@%s %s %d", sru->ident,
			     sru->host, sru->nick, sru->lvl);
		un = list_next(srconf.ul, un);
	}
	return 1;
}

int ext_auth_list(User *u, char **av, int ac) {
	lnode_t *un;
	users *sru;
	
	SET_SEGV_LOCATION();
	un = list_first(srconf.ul);
	prefmsg(u->nick, ns_botptr->nick, "ServiceRoots:");
	while (un) {
		sru = lnode_get(un);
		prefmsg(u->nick, ns_botptr->nick, "%s!%s@%s %d", sru->nick, sru->ident,
			     sru->host, sru->lvl);
		un = list_next(srconf.ul, un);
	}
	return 1;
}	
