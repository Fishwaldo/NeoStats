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

/* @file global exclusion handling functions
 */

#include "neostats.h"
#include "exclude.h"
#include "services.h"
#include "ircstring.h"

static list_t *exclude_list;

typedef struct Exclude {
	NS_EXCLUDE type;
	char pattern[MAXHOST]; /* because hostname is the biggest this can get */
	char addedby[MAXNICK];
	time_t addedon;
} Exclude;

const char* ExcludeDesc[NS_EXCLUDE_MAX] = {
	"Host",
	"Server",
	"Channel",
};

void new_exclude (void *data)
{
	Exclude *e;

	e = ns_calloc(sizeof(Exclude));
	os_memcpy (e, data, sizeof(Exclude));
	lnode_create_append (exclude_list, e);
	dlog(DEBUG2, "Added Exclusion %s (%d) by %s on %d", e->pattern, e->type, e->addedby, (int)e->addedon);
}

/* @brief initilize the exlusion list
 * 
 * @returns int specifing success/failure
 */
int InitExcludes(void) 
{
	exclude_list = list_create(-1);
	DBAFetchRows ("exclusions", new_exclude);
	return NS_SUCCESS;
} 

void FiniExcludes(void) 
{
	list_destroy_auto (exclude_list);
}

/* @brief add a entry to the exlusion list
 * 
 * @param u the user that sent the request
 * @param type the type of exclusion being added. Chan, Host or Server
 * @param pattern the actual pattern to use 
 * @returns nothing
 */
int ns_cmd_exclude_add(CmdParams* cmdparams) 
{
	Exclude *e;
	
	if (cmdparams->ac < 3) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	/* we dont do any checking to see if a similar entry already exists... oh well, thats upto the user */
	e = ns_calloc (sizeof(Exclude));
	strlcpy(e->addedby, cmdparams->source->name, MAXNICK);
	e->addedon = me.now;
	if (!ircstrcasecmp("HOST", cmdparams->av[1])) {
		if (!index(cmdparams->av[2], '.')) {
			irc_prefmsg(ns_botptr, cmdparams->source, __("Error, Pattern must contain at least one \2.\2",cmdparams->source));
			ns_free(e);
			return NS_SUCCESS;
		}
		e->type = NS_EXCLUDE_HOST;
		strlcpy(e->pattern, collapse(cmdparams->av[2]), MAXHOST);
	} else if (!ircstrcasecmp("CHANNEL", cmdparams->av[1])) {
		if (cmdparams->av[2][0] != '#') {
			irc_prefmsg(ns_botptr, cmdparams->source, __("Error, Pattern must begin with a \2#\2", cmdparams->source));
			ns_free(e);
			return NS_SUCCESS;
		}
		e->type = NS_EXCLUDE_CHANNEL;
		strlcpy(e->pattern, collapse(cmdparams->av[2]), MAXHOST);
	} else if (!ircstrcasecmp("SERVER", cmdparams->av[1])) {
		if (!index(cmdparams->av[2], '.')) {
			irc_prefmsg(ns_botptr, cmdparams->source, __("Error, Pattern must contain at least one \2.\2", cmdparams->source));
			ns_free(e);
			return NS_SUCCESS;
		}
		e->type = NS_EXCLUDE_SERVER;
		strlcpy(e->pattern, collapse(cmdparams->av[2]), MAXHOST);
	} else {
		irc_prefmsg(ns_botptr, cmdparams->source, __("Error, Unknown type %s",cmdparams->source), cmdparams->av[1]);
		ns_free(e);
		return NS_SUCCESS;
	}
	/* if we get here, then e is valid */
	lnode_create_append (exclude_list, e);
	irc_prefmsg(ns_botptr, cmdparams->source, __("Added %s (%s) to exclusion list", cmdparams->source), e->pattern, cmdparams->av[1]);
	irc_chanalert(ns_botptr, _("%s added %s (%s) to the exclusion list"), cmdparams->source->name, e->pattern, cmdparams->av[1]);
	/* now save the exclusion list */
	DBAStore ("exclusions", e->pattern, (void *)e, sizeof (Exclude));
	return NS_SUCCESS;
} 

/* @brief del a entry from the exlusion list
 * 
 * @param u the user that sent the request
 * @param postition the position in the list that we are excluding
 * @returns nothing
 */
int ns_cmd_exclude_del(CmdParams* cmdparams) 
{
	lnode_t *en;
	Exclude *e;
	
	if (cmdparams->ac < 2) {
		return NS_ERR_NEED_MORE_PARAMS;
	}
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (ircstrcasecmp (e->pattern, cmdparams->av[1]) == 0) { 
			DBADelete ("exclusions", e->pattern);
			irc_prefmsg(ns_botptr, cmdparams->source, __("%s delete from exclusion list",cmdparams->source), e->pattern);
			ns_free(e);
			list_delete(exclude_list, en);
			lnode_destroy(en);
			return NS_SUCCESS;
		}
		en = list_next(exclude_list, en);
	}
	/* if we get here, means that we never got a match */
	irc_prefmsg(ns_botptr, cmdparams->source, __("%s not found in the exclusion list",cmdparams->source), cmdparams->av[1]);
	return NS_SUCCESS;
} 

/* @brief list the entries from the exlusion list
 * 
 * @param u the user that sent the request
 * @param from the "user" that this message should come from, so we can call from modules
 * @returns nothing
 */
int ns_cmd_exclude_list(CmdParams* cmdparams) 
{
	lnode_t *en;
	Exclude *e;
	
	irc_prefmsg(ns_botptr, cmdparams->source, __("Global exclusion list:", cmdparams->source));
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);			
		irc_prefmsg(ns_botptr, cmdparams->source, __("%s (%s) Added by %s on %s", cmdparams->source), e->pattern, ExcludeDesc[e->type], e->addedby, sftime(e->addedon));
		en = list_next(exclude_list, en);
	}
	irc_prefmsg(ns_botptr, cmdparams->source, __("End of list.", cmdparams->source));
	return NS_SUCCESS;
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
		if (e->type == NS_EXCLUDE_HOST) {
			if (match(e->pattern, u->user->hostname)) {
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
