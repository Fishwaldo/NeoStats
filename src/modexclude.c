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
#define MAX_EXEMPTS		100

/* this is the list of excludeed hosts/servers */
static list_t *excludelists[NUM_MODULES];

static void new_exclude (void *data)
{
	Exclude *excludes;

	excludes = malloc(sizeof(Exclude));
	os_memcpy (excludes, data, sizeof(Exclude));
	lnode_create_prepend(excludelists[GET_CUR_MODNUM()], excludes);
	dlog (DEBUG2, "Adding %s (%d) Set by %s for %s to Exempt List", excludes->pattern, excludes->type, excludes->addedby, excludes->reason);
}

int ModInitExempts(Module *mod_ptr)
{
	SET_SEGV_LOCATION();
	/* init the exclusions list */
	excludelists[mod_ptr->modnum] = list_create(MAX_EXEMPTS);
	DBAFetchRows ("exclusions", new_exclude);
	return NS_SUCCESS;
}

int ModFiniExempts(Module *mod_ptr)
{
	list_destroy_auto (excludelists[mod_ptr->modnum]);
	return NS_SUCCESS;
}

int ModIsServerExempt(Client *s)
{
	lnode_t *node;
	Exclude *exempts;

	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		exempts = lnode_get(node);
		if (exempts->type == NS_EXCLUDE_SERVER) {
			if (match(exempts->pattern, s->name)) {
				dlog (DEBUG1, "Matched server entry %s in Exemptions", exempts->pattern);
				return NS_TRUE;
			}
		}
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	return NS_FALSE;
}

int ModIsUserExempt(Client *u) 
{
	lnode_t *node;
	Exclude *excludes;

	SET_SEGV_LOCATION();
	if (!strcasecmp(u->uplink->name, me.name)) {
		dlog (DEBUG1, "User %s Exempt. its Me!", u->name);
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		excludes = lnode_get(node);
		if (excludes->type == NS_EXCLUDE_SERVER) {
			/* match a server */
			if (match(excludes->pattern, u->uplink->name)) {
				dlog (DEBUG1, "User %s exclude. Matched server entry %s in Exemptions", u->name, excludes->pattern);
				return NS_TRUE;
			}
		} else if (excludes->type == NS_EXCLUDE_HOST) {
			/* match a hostname */
			if (match(excludes->pattern, u->user->hostname)) {
				dlog (DEBUG1, "User %s is exclude. Matched Host Entry %s in Exceptions", u->name, excludes->pattern);
				return NS_TRUE;
			}
		}				
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	return NS_FALSE;
}

int ModIsChanExempt(Channel *c) 
{
	lnode_t *node;
	Exclude *excludes;

	SET_SEGV_LOCATION();
	if (IsServicesChannel( c )) {
		dlog (DEBUG1, "Services channel %s is exclude.", c->name);
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		excludes = lnode_get(node);
		if (excludes->type == NS_EXCLUDE_CHANNEL) {
			/* match a channel */
			if (match(excludes->pattern, c->name)) {
				dlog (DEBUG1, "Channel %s exclude. Matched Channel entry %s in Exemptions", c->name, excludes->pattern);
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
	Exclude *excludes;

	SET_SEGV_LOCATION();
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	irc_prefmsg (cmdparams->bot, cmdparams->source, "Exception List:");
	while (node) {
		excludes = lnode_get(node);
		irc_prefmsg (cmdparams->bot, cmdparams->source, "%s (%s) Added by %s for %s", excludes->pattern, ExcludeDesc[excludes->type], excludes->addedby, excludes->reason);
		node = list_next(excludelists[GET_CUR_MODNUM()], node);
	}
	irc_prefmsg (cmdparams->bot, cmdparams->source, "End of List.");
	return NS_SUCCESS;
}

static int mod_cmd_exclude_add(CmdParams *cmdparams)
{
	NS_EXCLUDE type;
	char *buf;
	Exclude *excludes;

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
	excludes = ns_calloc (sizeof(Exclude));
	excludes->type = type;
	excludes->addedon = me.now;
	strlcpy(excludes->pattern, cmdparams->av[2], MAXHOST);
	strlcpy(excludes->addedby, cmdparams->source->name, MAXNICK);
	buf = joinbuf(cmdparams->av, cmdparams->ac, 3);
	strlcpy(excludes->reason, buf, MAXREASON);
	ns_free (buf);
	lnode_create_append (excludelists[GET_CUR_MODNUM()], excludes);
	irc_prefmsg (cmdparams->bot, cmdparams->source, "Added %s (%s) exception to list", excludes->pattern, ExcludeDesc[excludes->type]);
	if (nsconfig.cmdreport) {
		irc_chanalert (cmdparams->bot, "%s added %s (%s) exception to list", cmdparams->source->name, excludes->pattern, ExcludeDesc[excludes->type]);
	}
	DBAStore ("exclusions", excludes->pattern, excludes, sizeof(Exclude));
	return NS_SUCCESS;
}

static int mod_cmd_exclude_del(CmdParams *cmdparams)
{
	lnode_t *node;
	Exclude *excludes = NULL;

	SET_SEGV_LOCATION();
	if (cmdparams->ac < 2) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		excludes = lnode_get(node);
		if (ircstrcasecmp (cmdparams->av[1], excludes->pattern) == 0) {
			list_delete(excludelists[GET_CUR_MODNUM()], node);
			lnode_destroy(node);
			ns_free (excludes);
			DBADelete ("exclusions", excludes->pattern);
			irc_prefmsg (cmdparams->bot, cmdparams->source, "Deleted %s %s out of exception list", excludes->pattern, ExcludeDesc[excludes->type]);
			if (nsconfig.cmdreport) {
				irc_chanalert (cmdparams->bot, "%s deleted %s %s out of exception list", cmdparams->source->name, excludes->pattern, ExcludeDesc[excludes->type]);
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
