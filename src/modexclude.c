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

/* @file module exclusion handling functions
 */

#include "neostats.h"
#include "services.h"
#include "exclude.h"
#include "modexclude.h"
#include "ns_help.h"

/* this is the size of the exclude list */
#define MAX_EXCLUDES		100

/* this is the list of excludeed hosts/servers */
static list_t *excludelists[NUM_MODULES];

static void new_exclude (void *data)
{
	Exclude *e;

	e = malloc(sizeof(Exclude));
	os_memcpy (e, data, sizeof(Exclude));
	lnode_create_prepend(excludelists[GET_CUR_MODNUM()], e);
	dlog (DEBUG2, "Adding %s (%d) Set by %s for %s to Exclude List", e->pattern, e->type, e->addedby, e->reason);
}

int ModInitExcludes(Module *mod_ptr)
{
	SET_SEGV_LOCATION();
	/* init the exclusions list */
	excludelists[mod_ptr->modnum] = list_create(MAX_EXCLUDES);
	DBAFetchRows ("exclusions", new_exclude);
	return NS_SUCCESS;
}

int ModFiniExcludes(Module *mod_ptr)
{
	list_destroy_auto (excludelists[mod_ptr->modnum]);
	return NS_SUCCESS;
}

int ModIsServerExcluded(Client *s)
{
	lnode_t *node;
	Exclude *e;

	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		e = lnode_get(node);
		if (e->type == NS_EXCLUDE_SERVER) {
			if (match(e->pattern, s->name)) {
				dlog (DEBUG1, "Matched server entry %s in Excludeions", e->pattern);
				return NS_TRUE;
			}
		}
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	return NS_FALSE;
}

int ModIsUserExcluded(Client *u) 
{
	lnode_t *node;
	Exclude *e;

	SET_SEGV_LOCATION();
	if (!strcasecmp(u->uplink->name, me.name)) {
		dlog (DEBUG1, "User %s Exclude. its Me!", u->name);
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		e = lnode_get(node);
		if (e->type == NS_EXCLUDE_SERVER) {
			/* match a server */
			if (match(e->pattern, u->uplink->name)) {
				dlog (DEBUG1, "User %s exclude. Matched server entry %s in Excludeions", u->name, e->pattern);
				return NS_TRUE;
			}
		} else if (e->type == NS_EXCLUDE_HOST) {
			/* match a hostname */
			if (match(e->pattern, u->user->hostname)) {
				dlog (DEBUG1, "User %s is exclude. Matched Host Entry %s in Exceptions", u->name, e->pattern);
				return NS_TRUE;
			}
		}				
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	return NS_FALSE;
}

int ModIsChannelExcluded(Channel *c) 
{
	lnode_t *node;
	Exclude *e;

	SET_SEGV_LOCATION();
	if (IsServicesChannel( c )) {
		dlog (DEBUG1, "Services channel %s is exclude.", c->name);
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		e = lnode_get(node);
		if (e->type == NS_EXCLUDE_CHANNEL) {
			/* match a channel */
			if (match(e->pattern, c->name)) {
				dlog (DEBUG1, "Channel %s exclude. Matched Channel entry %s in Excludeions", c->name, e->pattern);
				return NS_TRUE;
			}
		}				
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	return NS_FALSE;
}

static int mod_cmd_exclude_list(CmdParams *cmdparams)
{
	lnode_t *node;
	Exclude *e;

	SET_SEGV_LOCATION();
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	irc_prefmsg (cmdparams->bot, cmdparams->source, "Exception List:");
	while (node) {
		e = lnode_get(node);
		irc_prefmsg (cmdparams->bot, cmdparams->source, "%s (%s) Added by %s for %s", e->pattern, ExcludeDesc[e->type], e->addedby, e->reason);
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	irc_prefmsg (cmdparams->bot, cmdparams->source, "End of List.");
	return NS_SUCCESS;
}

static int mod_cmd_exclude_add(CmdParams *cmdparams)
{
	NS_EXCLUDE type;
	char *buf;
	Exclude *e;

	SET_SEGV_LOCATION();
	if (cmdparams->ac < 4) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if (list_isfull(excludelists[GET_CUR_MODNUM()])) {
		irc_prefmsg (cmdparams->bot, cmdparams->source, "Error, Exception list is full");
		return NS_SUCCESS;
	}
	if (!ircstrcasecmp("HOST", cmdparams->av[1])) {
		if (!index(cmdparams->av[2], '.')) {
			irc_prefmsg (cmdparams->bot, cmdparams->source, "Invalid host name");
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_HOST;
	} else if (!ircstrcasecmp("CHANNEL", cmdparams->av[1])) {
		if (cmdparams->av[2][0] != '#') {
			irc_prefmsg (cmdparams->bot, cmdparams->source, "Invalid channel name");
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_CHANNEL;
	} else if (!ircstrcasecmp("SERVER", cmdparams->av[1])) {
		if (!index(cmdparams->av[2], '.')) {
			irc_prefmsg (cmdparams->bot, cmdparams->source, "Invalid host name");
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_SERVER;
	} else {
		irc_prefmsg (cmdparams->bot, cmdparams->source, "Invalid exclude type");
		return NS_SUCCESS;
	}
	e = ns_calloc (sizeof(Exclude));
	e->type = type;
	e->addedon = me.now;
	strlcpy(e->pattern, cmdparams->av[2], MAXHOST);
	strlcpy(e->addedby, cmdparams->source->name, MAXNICK);
	buf = joinbuf(cmdparams->av, cmdparams->ac, 3);
	strlcpy(e->reason, buf, MAXREASON);
	ns_free (buf);
	lnode_create_append (excludelists[GET_CUR_MODNUM()], e);
	irc_prefmsg (cmdparams->bot, cmdparams->source, "Added %s (%s) exception to list", e->pattern, ExcludeDesc[e->type]);
	if (nsconfig.cmdreport) {
		irc_chanalert (cmdparams->bot, "%s added %s (%s) exception to list", cmdparams->source->name, e->pattern, ExcludeDesc[e->type]);
	}
	DBAStore ("exclusions", e->pattern, e, sizeof(Exclude));
	return NS_SUCCESS;
}

static int mod_cmd_exclude_del(CmdParams *cmdparams)
{
	lnode_t *node;
	Exclude *e = NULL;

	SET_SEGV_LOCATION();
	if (cmdparams->ac < 2) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		e = lnode_get(node);
		if (ircstrcasecmp (cmdparams->av[1], e->pattern) == 0) {
			list_delete(excludelists[GET_CUR_MODNUM()], node);
			lnode_destroy(node);
			ns_free (e);
			DBADelete ("exclusions", e->pattern);
			irc_prefmsg (cmdparams->bot, cmdparams->source, "Deleted %s %s out of exception list", e->pattern, ExcludeDesc[e->type]);
			if (nsconfig.cmdreport) {
				irc_chanalert (cmdparams->bot, "%s deleted %s %s out of exception list", cmdparams->source->name, e->pattern, ExcludeDesc[e->type]);
			}
			return NS_SUCCESS;
		}
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}		
	irc_prefmsg (cmdparams->bot, cmdparams->source, "Error, Can't find entry %s.", cmdparams->av[1]);
	return NS_SUCCESS;
}

int mod_cmd_exclude(CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	if (!strcasecmp(cmdparams->av[0], "LIST")) {
		return mod_cmd_exclude_list(cmdparams);
	} else if (!strcasecmp(cmdparams->av[0], "ADD")) {
		return mod_cmd_exclude_add(cmdparams);
	} else if (!strcasecmp(cmdparams->av[0], "DEL")) {
		return mod_cmd_exclude_del(cmdparams);
	}
	return NS_ERR_SYNTAX_ERROR;
}

bot_cmd mod_exclude_commands[]=
{
	{"EXCLUDE",	mod_cmd_exclude,1,	NS_ULEVEL_ADMIN,ns_help_exclude,ns_help_exclude_oneline},
	{NULL,		NULL,			0, 	0,				NULL, 			NULL}
};
