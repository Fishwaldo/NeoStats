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

const char *ns_copyright[] = {
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
	hnode_t *node;
	
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
					node = hnode_create(access);
					hash_insert(accesshash, node, access->nick);
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
	hnode_t *node;
	
	SET_SEGV_LOCATION();
	if (cmdparams->ac < 3) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if (hash_lookup(accesshash, cmdparams->av[0])) {
		irc_prefmsg(NULL, cmdparams->source, "Entry for %s already exists", cmdparams->av[0]);
		return NS_SUCCESS;
	}
	if (strstr(cmdparams->av[1], "!")&& !strstr(cmdparams->av[1], "@")) {
		irc_prefmsg(NULL, cmdparams->source, "Invalid format for hostmask. Must be of the form nick!user@host.");
		return NS_ERR_SYNTAX_ERROR;
	}
	level = atoi(cmdparams->av[2]);
	if(level < 0 || level > NS_ULEVEL_ROOT) {
		irc_prefmsg(NULL, cmdparams->source, "Level out of range. Valid values range from 0 to 200.");
		return NS_ERR_PARAM_OUT_OF_RANGE;
	}
	access = malloc(sizeof(NeoAccess));
	strlcpy(access->nick, cmdparams->av[0], MAXNICK);
	strlcpy(access->mask, cmdparams->av[1], MAXHOST);
	access->level = level;
	node = hnode_create(access);
 	hash_insert(accesshash, node, access->nick);
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
	node = hash_lookup(accesshash, cmdparams->av[0]);
	if (node) {
		hash_delete(accesshash, node);
		free(hnode_get(node));
		hnode_destroy(node);
		ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s", cmdparams->av[0]);
		DelConf(confpath);
		irc_prefmsg(NULL, cmdparams->source, "Deleted %s from Access List", cmdparams->av[0]);
	} else {
		irc_prefmsg(NULL, cmdparams->source, "Error, Could not find %s in access list.", cmdparams->av[0]);
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
		irc_prefmsg(NULL, cmdparams->source, NULL, "%s %s (%d)", access->nick, access->mask, access->level);
	}
	irc_prefmsg(NULL, cmdparams->source, "End of List.");	
	return NS_SUCCESS;
}

static int ea_cmd_access(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!strcasecmp(cmdparams->av[2], "add")) {
		return AccessAdd(cmdparams);
	} else if (!strcasecmp(cmdparams->av[2], "del")) {
		return AccessDel(cmdparams);
	} else if (!strcasecmp(cmdparams->av[2], "list")) {
		return AccessList(cmdparams);
	}
	irc_prefmsg(NULL, cmdparams->source, "Invalid Syntax.");
	return NS_ERR_SYNTAX_ERROR;
}

static int GetAccessLevel(Client * u)
{
	hnode_t *node;
	NeoAccess *access;

	dlog(DEBUG2, "GetAccessLevel for %s", u->name);
	node = hash_lookup(accesshash, u->name);
	if (node) {
		access = hnode_get(node);
		if ((match(access->mask, u->user->hostname))) {
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
	0,
	0,
};

static int ea_event_online(CmdParams* cmdparams)
{
	add_services_cmd_list(extauth_commands);
	return 1;
};

static int ea_event_mode(CmdParams* cmdparams) 
{
	int add = 0;
	char *modes;

	/* bail if we are not synched */
	if (!is_synched)
		return 0;
		
	if(!HaveUmodeRegNick()) 
		return -1;

	/* first, find if its a regnick mode */
	modes = cmdparams->param;
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if(*modes == UmodeChRegNick) {
				if (add) {
					cmdparams->source->user->ulevel = GetAccessLevel(cmdparams->source);
					dlog(DEBUG2, "SetAccessLevel for %s to %d", cmdparams->source->name, cmdparams->source->user->ulevel);
					irc_chanalert (NULL, "%s granted access level %d", cmdparams->source->name, cmdparams->source->user->ulevel);
					nlog (LOG_NORMAL, "%s granted access level %d", cmdparams->source->name, cmdparams->source->user->ulevel);
				}
			}
			break;
		}
		modes++;
	}
	return 1;
}

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	ea_event_online},
	{EVENT_UMODE,	ea_event_mode}, 
	{EVENT_NULL,	NULL}
};

int ModInit(Module* modptr)
{
	LoadAccessList();
	return 1;
}

void ModFini()
{
}

int ModAuthUser(Client * u)
{
	return GetAccessLevel(u);
}
