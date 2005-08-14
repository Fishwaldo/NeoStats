/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* @file global exclusion handling functions
 */

/*  TODO:
 *  - Real time exclusions??? possibly optional.
 */

#include "neostats.h"
#include "exclude.h"
#include "services.h"
#include "ircstring.h"
#include "helpstrings.h"

int mod_cmd_exclude(CmdParams *cmdparams);

/* this is the size of the mod exclude list */
#define MAX_MOD_EXCLUDES		100

/* this is the list of excluded hosts/servers */
static list_t *exclude_list;
static list_t *excludelists[NUM_MODULES];
static bot_cmd *bot_cmd_lists[NUM_MODULES];

const char* ExcludeDesc[NS_EXCLUDE_MAX] = {
	"Host",
	"Server",
	"Channel",
	"Userhost"
};

bot_cmd mod_exclude_commands[]=
{
	{"EXCLUDE",	mod_cmd_exclude,1,	NS_ULEVEL_ADMIN,ns_help_exclude},
	{NULL,		NULL,			0, 	0,				NULL}
};

static void new_exclude(list_t *elist, void *data)
{
	Exclude *e;

	e = ns_calloc(sizeof(Exclude));
	os_memcpy (e, data, sizeof(Exclude));
	lnode_create_append (elist, e);
	dlog(DEBUG2, "Added exclusion %s (%d) by %s on %d", e->pattern, e->type, e->addedby, (int)e->addedon);
}

static int new_global_exclude( void *data, int size )
{
	new_exclude(exclude_list, data);
	return NS_FALSE;
}

static int new_mod_exclude( void *data, int size)
{
	new_exclude(excludelists[GET_CUR_MODNUM()], data);
	return NS_FALSE;
}

/* @brief initilize the exlusion list
 * 
 * @returns int specifing success/failure
 */
int InitExcludes(void) 
{
	exclude_list = list_create(-1);
	DBAFetchRows ("exclusions", new_global_exclude);
	return NS_SUCCESS;
} 

int InitModExcludes(Module *mod_ptr)
{
	SET_SEGV_LOCATION();
	/* init the exclusions list */
	SET_RUN_LEVEL(mod_ptr);
	excludelists[mod_ptr->modnum] = list_create(MAX_MOD_EXCLUDES);
	bot_cmd_lists[mod_ptr->modnum] = ns_malloc( sizeof( mod_exclude_commands ) );
	os_memcpy( bot_cmd_lists[mod_ptr->modnum], mod_exclude_commands, sizeof( mod_exclude_commands ) );
	DBAFetchRows ("exclusions", new_mod_exclude);
	RESET_RUN_LEVEL();
	return NS_SUCCESS;
}

void FiniExcludes(void) 
{
	DBACloseTable("exclusions");
	list_destroy_auto (exclude_list);
}

void FiniModExcludes(Module *mod_ptr)
{
	ns_free( bot_cmd_lists[mod_ptr->modnum] );
	list_destroy_auto (excludelists[mod_ptr->modnum]);
}

/* @brief add a entry to the exlusion list
 * 
 * @param u the user that sent the request
 * @param type the type of exclusion being added. Channel, Host or Server
 * @param pattern the actual pattern to use 
 * @returns nothing
 */
static int do_exclude_add(list_t *elist, CmdParams* cmdparams) 
{
	NS_EXCLUDE type;
	char *buf;
	Exclude *e, *etst;
	lnode_t *ln;
	
	if (cmdparams->ac < 4) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if (list_isfull(elist)) {
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
	} else if (!ircstrcasecmp("USERHOST", cmdparams->av[1])) {
		if (!index(cmdparams->av[2], '!') || !index(cmdparams->av[2], '@')) {
			irc_prefmsg (cmdparams->bot, cmdparams->source, "Invalid userhost mask");
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_USERHOST;
	} else {
		irc_prefmsg (cmdparams->bot, cmdparams->source, "Invalid exclude type");
		return NS_SUCCESS;
	}
	ln = list_first(elist);
	while( ln != NULL  ) 
	{
		etst = lnode_get(ln);
		if( etst->type == type ) {
			if( match( etst->pattern, cmdparams->av[2] ) ) {
				irc_prefmsg (cmdparams->bot, cmdparams->source, "Mask already matched by %s", etst->pattern);
				return NS_SUCCESS;
			}
		}
		ln = list_next(elist, ln);
	}
	e = ns_calloc (sizeof(Exclude));
	e->type = type;
	e->addedon = me.now;
	strlcpy(e->pattern, collapse(cmdparams->av[2]), MAXHOST);
	strlcpy(e->addedby, cmdparams->source->name, MAXNICK);
	buf = joinbuf(cmdparams->av, cmdparams->ac, 3);
	strlcpy(e->reason, buf, MAXREASON);
	ns_free (buf);
	/* if we get here, then e is valid */
	lnode_create_append (elist, e);
	irc_prefmsg(cmdparams->bot, cmdparams->source, __("Added %s (%s) to exclusion list", cmdparams->source), e->pattern, cmdparams->av[1]);
	if (nsconfig.cmdreport) {
		irc_chanalert(cmdparams->bot, _("%s added %s (%s) to the exclusion list"), cmdparams->source->name, e->pattern, cmdparams->av[1]);
	}
	/* now save the exclusion list */
	DBAStore ("exclusions", e->pattern, (void *)e, sizeof (Exclude));
	return NS_SUCCESS;
} 

static int ns_cmd_exclude_add(CmdParams* cmdparams) 
{
	return do_exclude_add(exclude_list, cmdparams);
} 

static int mod_cmd_exclude_add(CmdParams *cmdparams)
{
	return do_exclude_add(excludelists[cmdparams->bot->moduleptr->modnum], cmdparams);
}

/* @brief del a entry from the exlusion list
 * 
 * @param u the user that sent the request
 * @param postition the position in the list that we are excluding
 * @returns nothing
 */
static int do_exclude_del(list_t *elist, CmdParams* cmdparams) 
{
	lnode_t *en;
	Exclude *e;
	
	if (cmdparams->ac < 2) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	en = list_first(elist);
	while (en != NULL) {
		e = lnode_get(en);
		if (ircstrcasecmp (e->pattern, cmdparams->av[1]) == 0) { 
			list_delete(elist, en);
			lnode_destroy(en);
			DBADelete ("exclusions", e->pattern);
			irc_prefmsg(cmdparams->bot, cmdparams->source, __("%s delete from exclusion list",cmdparams->source), e->pattern);
			ns_free(e);
			return NS_SUCCESS;
		}
		en = list_next(elist, en);
	}
	/* if we get here, means that we never got a match */
	irc_prefmsg(cmdparams->bot, cmdparams->source, __("%s not found in the exclusion list",cmdparams->source), cmdparams->av[1]);
	return NS_SUCCESS;
} 

static int ns_cmd_exclude_del(CmdParams* cmdparams) 
{
	return do_exclude_del(exclude_list, cmdparams);
} 

static int mod_cmd_exclude_del(CmdParams *cmdparams)
{
	return do_exclude_del(excludelists[cmdparams->bot->moduleptr->modnum], cmdparams);
}

/* @brief list the entries from the exlusion list
 * 
 * @param u the user that sent the request
 * @param from the "user" that this message should come from, so we can call from modules
 * @returns nothing
 */
static int do_exclude_list(list_t *elist, CmdParams* cmdparams) 
{
	lnode_t *en;
	Exclude *e;
	
	irc_prefmsg(cmdparams->bot, cmdparams->source, __("Exclusion list:", cmdparams->source));
	en = list_first(elist);
	while (en != NULL) {
		e = lnode_get(en);			
		irc_prefmsg(cmdparams->bot, cmdparams->source, __("%s (%s) Added by %s on %s for %s", cmdparams->source), e->pattern, ExcludeDesc[e->type], e->addedby, sftime(e->addedon), e->reason);
		en = list_next(elist, en);
	}
	irc_prefmsg(cmdparams->bot, cmdparams->source, __("End of list.", cmdparams->source));
	return NS_SUCCESS;
} 


static int ns_cmd_exclude_list(CmdParams* cmdparams) 
{
	return do_exclude_list(exclude_list, cmdparams);
} 

static int mod_cmd_exclude_list(CmdParams *cmdparams)
{
	return do_exclude_list(excludelists[cmdparams->bot->moduleptr->modnum], cmdparams);
}

/** @brief EXCLUDE command handler
 *
 *  maintain global exclusion list, which modules can take advantage off
 *   
 *  @param cmdparams structure with command information
 *  @returns none
 */

int ns_cmd_exclude (CmdParams* cmdparams) 
{
	if (!ircstrcasecmp(cmdparams->av[0], "ADD")) {
		return ns_cmd_exclude_add(cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "DEL")) {
		return ns_cmd_exclude_del(cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		return ns_cmd_exclude_list(cmdparams);
	}
	return NS_ERR_SYNTAX_ERROR;
}

int mod_cmd_exclude(CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	if (!ircstrcasecmp(cmdparams->av[0], "ADD")) {
		return mod_cmd_exclude_add(cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "DEL")) {
		return mod_cmd_exclude_del(cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		return mod_cmd_exclude_list(cmdparams);
	}
	return NS_ERR_SYNTAX_ERROR;
}

/* @brief check if a user is matched against a exclusion
 * 
 * Modifies the flags field of the User struct to indicate if the user is matched
 * against a exclusion. This function is called when the user signs on
 *
 * @param u the user to check
 * @returns nothing
 */

void ns_do_exclude_user(Client *u) 
{
	lnode_t *en;
	Exclude *e;
	
	/* if the server is excluded, user is excluded as well */
	if (u->uplink->flags & NS_FLAG_EXCLUDED) {
	 	u->flags |= NS_FLAG_EXCLUDED;
		return;
	}	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_HOST) 
		{
			if (match(e->pattern, u->user->hostname)) {
				u->flags |= NS_FLAG_EXCLUDED;
				return;
			}
		}
		else if (e->type == NS_EXCLUDE_USERHOST) 
		{
			if (match(e->pattern, u->user->userhostmask)) {
				u->flags |= NS_FLAG_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	u->flags &= ~NS_FLAG_EXCLUDED;
}

/* @brief check if a server is matched against a exclusion
 * 
 * Modifies the flags field of the server struct to indicate if the server is matched
 * against a exclusion. This function is called when the server connects
 *
 * @param s the Server to check
 * @returns nothing
 */

void ns_do_exclude_server(Client *s) 
{
	lnode_t *en;
	Exclude *e;
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_SERVER) {
			if (match(e->pattern, s->name)) {
				s->flags |= NS_FLAG_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	s->flags &= ~NS_FLAG_EXCLUDED;
}

/* @brief check if a channel is matched against a exclusion
 * 
 * Modifies the flags field of the channel struct to indicate if the channel is matched
 * against a exclusion. This function is called when the channel is created
 *
 * @param c the channel to check
 * @returns nothing
 */

void ns_do_exclude_chan(Channel *c) 
{
	lnode_t *en;
	Exclude *e;
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_CHANNEL) {
			if (match(e->pattern, c->name)) {
				c->flags |= NS_FLAG_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	c->flags &= ~NS_FLAG_EXCLUDED;
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
				dlog (DEBUG1, "Matched server entry %s in exclusions", e->pattern);
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
	if (!ircstrcasecmp(u->uplink->name, me.name)) {
		dlog (DEBUG1, "User %s Exclude. its Me!", u->name);
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first(excludelists[GET_CUR_MODNUM()]);
	while (node) {
		e = lnode_get(node);
		if (e->type == NS_EXCLUDE_SERVER) {
			dlog (DEBUG4, "Testing %s against server %s", u->uplink->name, e->pattern);
			/* match a server */
			if (match(e->pattern, u->uplink->name)) {
				dlog (DEBUG1, "User %s excluded. Matched server entry %s in exclusions", u->name, e->pattern);
				return NS_TRUE;
			}
		} else if (e->type == NS_EXCLUDE_HOST) {
			dlog (DEBUG4, "Testing %s against host %s", u->user->hostname, e->pattern);
			/* match a hostname */
			if (match(e->pattern, u->user->hostname)) {
				dlog (DEBUG1, "User %s is excluded. Matched host entry %s in exclusions", u->name, e->pattern);
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
	if (IsServicesChannel ( c ) ) {
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

void AddBotExcludeCommands( Bot *botptr )
{
	add_bot_cmd_list( botptr, bot_cmd_lists[botptr->moduleptr->modnum] );
}
