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
#include "services.h"

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
		free(data);
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
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Invalid Syntax. /msg NeoStats help access");
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if (hash_lookup(accesshash, cmdparams->av[0])) {
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Entry for %s already exists", cmdparams->av[0]);
		return NS_SUCCESS;
	}
	if (strstr(cmdparams->av[1], "!")&& !strstr(cmdparams->av[1], "@")) {
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Invalid format for hostmask. Must be of the form nick!user@host.");
		return NS_ERR_SYNTAX_ERROR;
	}
	level = atoi(cmdparams->av[2]);
	if(level < 0 || level > NS_ULEVEL_ROOT) {
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Level our of range. Valid values range from 0 to 200.");
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
	prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Successfully added %s for host %s with level %d to access list", access->nick, access->mask, access->level);
	return NS_SUCCESS;
}

static int AccessDel(CmdParams* cmdparams) 
{
	hnode_t *node;

	SET_SEGV_LOCATION();
	if (cmdparams->ac < 1) {
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Invalid Syntax. /msg %s help access for more info", ns_botptr->nick);
		return NS_ERR_SYNTAX_ERROR;
	}
	node = hash_lookup(accesshash, cmdparams->av[0]);
	if (node) {
		hash_delete(accesshash, node);
		free(hnode_get(node));
		hnode_destroy(node);
		ircsnprintf(confpath, CONFBUFSIZE, "AccessList/%s", cmdparams->av[0]);
		DelConf(confpath);
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Deleted %s from Access List", cmdparams->av[0]);
	} else {
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Error, Could not find %s in access list. /msg %s access list", cmdparams->av[0], ns_botptr->nick);
	}
	return NS_SUCCESS;
}

static int AccessList(CmdParams* cmdparams) 
{
	hscan_t accessscan;
	hnode_t *node;
	NeoAccess *access;

	SET_SEGV_LOCATION();	
	prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Access List (%d):", (int)hash_count(accesshash));
	hash_scan_begin(&accessscan, accesshash);
	while ((node = hash_scan_next(&accessscan)) != NULL) {
		access = hnode_get(node);
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "%s %s (%d)", access->nick, access->mask, access->level);
	}
	prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "End of List.");	
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
	prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "Invalid Syntax. /msg %s help access for more info", ns_botptr->nick);
	return NS_ERR_SYNTAX_ERROR;
}

static int GetAccessLevel(User* u)
{
	hnode_t *node;
	NeoAccess *access;

	dlog(DEBUG2, "GetAccessLevel for %s", u->nick);
	node = hash_lookup(accesshash, u->nick);
	if (node) {
		access = hnode_get(node);
		if ((match(access->mask, u->hostname))) {
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

#ifdef UMODE_REGNICK
static int ea_event_mode(CmdParams* cmdparams) 
{
	int add = 0;
	char *modes;
	char vhost[MAXHOST];

	/* bail if we are not synced */
	if (!is_synced)
		return 0;
		
	/* first, find if its a regnick mode */
	modes = cmdparams->av[1];
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		case 'r':
			if (add) {
				cmdparams->source.user->ulevel = GetAccessLevel(cmdparams->source.user);
				dlog(DEBUG2, "SetAccessLevel for %s to %d", cmdparams->source.user->nick, cmdparams->source.user->ulevel);
				chanalert (ns_botptr->nick, "%s granted access level %d", cmdparams->source.user->nick, cmdparams->source.user->ulevel);
				nlog (LOG_NORMAL, "%s granted access level %d", cmdparams->source.user->nick, cmdparams->source.user->ulevel);
			}
			break;
		default:
			break;
		}
		modes++;
	}
	return 1;
}
#endif

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	ea_event_online},
#ifdef UMODE_REGNICK
	{EVENT_UMODE,	ea_event_mode}, 
#endif
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

int ModAuthUser(User * u, int curlvl)
{
	int templevel = 0;

	SET_SEGV_LOCATION();
	templevel = GetAccessLevel(u);
	if(templevel > curlvl) {
		curlvl = templevel;
	}
	return curlvl;
}
