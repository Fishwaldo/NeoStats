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

#include <stdio.h>
#include "neostats.h"

static const char *ns_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

const char *ea_help_access[] = {
	"Syntax: \2ACCESS ADD <nick> <mask> <level>\2",
	"        \2ACCESS DEL <nick>\2",
	"        \2ACCESS LIST\2",
	"",
	"Manage the list of users having access to NeoStats",
	"<mask> must be of the form nick!user@host",
	"<level> must be between 0 and 200",
	NULL
};

const char ea_help_access_oneline[] = "Manage NeoStats user access list";

static int AccessAdd(CmdParams* cmdparams);
static int AccessDel(CmdParams* cmdparams);
static int AccessList(CmdParams* cmdparams);

typedef struct NeoAccess{
	char nick[MAXNICK];
	char mask[MAXHOST];
	int level;
}NeoAccess;

static hash_t *accesshash;
static char confpath[CONFBUFSIZE];

static int LoadAccessList(void) 
{
	char **data, *tmp;
	NeoAccess *access;
	int i;
	
	SET_SEGV_LOCATION();
	accesshash = hash_create(-1, 0, 0);
	if (GetDir("AccessList", &data) > 0) {
		for (i = 0; data[i] != NULL; i++) {	
			access = malloc(sizeof(NeoAccess));
			strlcpy(access->nick, data[i], MAXNICK);
			ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s/mask", access->nick);
			if (GetConf((void *)&tmp, CFGSTR, confpath) <= 0) {
				free(access);
			} else {
				strlcpy(access->mask, tmp, MAXHOST);
				free(tmp);
				ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s/level", access->nick);
				if (GetConf((void *)&access->level, CFGINT, confpath) <= 0) {
					free(access);
				} else {
					hnode_create_insert (accesshash, access, access->nick);
				}
			}
		}
		if(*data) {
			free(data);
		}
	}	
	return 1;
}

static int AccessAdd(CmdParams* cmdparams) 
{
	int level = 0;
	NeoAccess *access;
	
	SET_SEGV_LOCATION();
	if (cmdparams->ac < 3) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if (hash_lookup(accesshash, cmdparams->av[1])) {
		irc_prefmsg(NULL, cmdparams->source, "Entry for %s already exists", cmdparams->av[1]);
		return NS_SUCCESS;
	}
	if (strstr(cmdparams->av[2], "!")&& !strstr(cmdparams->av[2], "@")) {
		irc_prefmsg(NULL, cmdparams->source, "Invalid format for hostmask. Must be of the form nick!user@host.");
		return NS_ERR_SYNTAX_ERROR;
	}
	level = atoi(cmdparams->av[3]);
	if(level < 0 || level > NS_ULEVEL_ROOT) {
		irc_prefmsg(NULL, cmdparams->source, "Level out of range. Valid values range from 0 to 200.");
		return NS_ERR_PARAM_OUT_OF_RANGE;
	}
	access = malloc(sizeof(NeoAccess));
	strlcpy(access->nick, cmdparams->av[1], MAXNICK);
	strlcpy(access->mask, cmdparams->av[2], MAXHOST);
	access->level = level;
 	hnode_create_insert (accesshash, access, access->nick);
	/* save the entry */
	ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s/mask", access->nick);
	SetConf((void *)access->mask, CFGSTR, confpath);
	ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s/level", access->nick);
	SetConf((void *)access->level, CFGINT, confpath);
	irc_prefmsg(NULL, cmdparams->source, "Successfully added %s for host %s with level %d to access list", access->nick, access->mask, access->level);
	return NS_SUCCESS;
}

static int AccessDel(CmdParams* cmdparams) 
{
	hnode_t *node;

	SET_SEGV_LOCATION();
	if (cmdparams->ac < 1) {
		return NS_ERR_SYNTAX_ERROR;
	}
	node = hash_lookup(accesshash, cmdparams->av[1]);
	if (node) {
		sfree(hnode_get(node));
		hash_delete(accesshash, node);
		hnode_destroy(node);
		ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s", cmdparams->av[1]);
		DelConf(confpath);
		irc_prefmsg(NULL, cmdparams->source, "Deleted %s from Access List", cmdparams->av[1]);
	} else {
		irc_prefmsg(NULL, cmdparams->source, "Error, Could not find %s in access list.", cmdparams->av[1]);
	}
	return NS_SUCCESS;
}

static int AccessList(CmdParams* cmdparams) 
{
	hscan_t accessscan;
	hnode_t *node;
	NeoAccess *access;

	SET_SEGV_LOCATION();	
	irc_prefmsg(NULL, cmdparams->source, "Access List (%d):", (int)hash_count(accesshash));
	hash_scan_begin(&accessscan, accesshash);
	while ((node = hash_scan_next(&accessscan)) != NULL) {
		access = hnode_get(node);
		irc_prefmsg(NULL, cmdparams->source, "%s %s (%d)", access->nick, access->mask, access->level);
	}
	irc_prefmsg(NULL, cmdparams->source, "End of List.");	
	return NS_SUCCESS;
}

static int ea_cmd_access(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!strcasecmp(cmdparams->av[0], "add")) {
		return AccessAdd(cmdparams);
	} else if (!strcasecmp(cmdparams->av[0], "del")) {
		return AccessDel(cmdparams);
	} else if (!strcasecmp(cmdparams->av[0], "list")) {
		return AccessList(cmdparams);
	}
	irc_prefmsg(NULL, cmdparams->source, "Invalid Syntax.");
	return NS_ERR_SYNTAX_ERROR;
}

static int GetAccessLevel(Client * u)
{
	static char hostmask[MAXHOST];
	NeoAccess *access;

	dlog (DEBUG2, "GetAccessLevel for %s", u->name);
	access = (NeoAccess *)hnode_find (accesshash, u->name);
	if (access) {
		ircsnprintf (hostmask, MAXHOST, "%s@%s", u->user->username, u->user->hostname);
		if ((match (access->mask, hostmask))) {
			return(access->level);		
		}
	}		
	return 0;
}

bot_cmd extauth_commands[]=
{
	{"ACCESS",	ea_cmd_access,	0,	NS_ULEVEL_ROOT, ea_help_access,	ea_help_access_oneline},
	{NULL,		NULL,			0, 	0,				NULL, 			NULL}
};

ModuleInfo module_info = {
	"ExtAuth",
	"Access List Authentication Module",
	ns_copyright,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
};

static int ea_event_online(CmdParams* cmdparams)
{
	add_services_cmd_list(extauth_commands);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	ea_event_online},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* modptr)
{
	LoadAccessList();
	return 1;
}

void ModFini()
{
	del_services_cmd_list(extauth_commands);
}

int ModAuthUser(Client * u)
{
	return GetAccessLevel(u);
}
