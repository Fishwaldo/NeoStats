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

#include <stdio.h>
#include "neostats.h"
#include "exclude.h"
#include "services.h"

static list_t *exclude_list;

typedef struct excludes {
	enum type {
		NS_EXCLUDE_HOST	= 0x1,
		NS_EXCLUDE_SERVER = 0x2,
		NS_EXCLUDE_CHAN = 0x4,
	} type;
	char pattern[MAXHOST]; /* becuase hostname is the biggest this can get */
	char addedby[MAXNICK];
	time_t addedon;
} excludes;


/* @brief initilize the exlusion list
 * 
 * @returns int specifing success/failure
 */
int InitExcludes(void) 
{
	char **row;
	int i;
	excludes *e;
	char *tmp;
	
	exclude_list = list_create(-1);
	if (GetTableData("Exclusions", &row) > 0) {
		for (i = 0; row[i] != NULL; i++) {
			e = ns_calloc(sizeof(excludes));
			if (GetData((void *)&tmp, CFGSTR, "Exclusions", row[i], "Pattern") > 0) {
				strlcpy(e->pattern, tmp, MAXHOST);
			} else {
				nlog(LOG_WARNING, "Exclusions: Can't add entry %s, Pattern invalid", row[i]);
				continue;
			}
			if (GetData((void *)&tmp, CFGSTR, "Exclusions", row[i], "AddedBy") > 0) {
				strlcpy(e->addedby, tmp, MAXNICK);
			}
			e->addedon = atoi(row[i]);
			GetData((void *)&e->type, CFGINT, "Exclusions", row[i], "Type");
			dlog(DEBUG2, "Added Exclusion %s (%d) by %s on %d", e->pattern, e->type, e->addedby, (int)e->addedon);
			lnode_create_append (exclude_list, e);
		}
		free(row);
	}
	return NS_SUCCESS;
} 

/* @brief add a entry to the exlusion list
 * 
 * @param u the user that sent the request
 * @param type the type of exclusion being added. Chan, Host or Server
 * @param pattern the actual pattern to use 
 * @returns nothing
 */
void ns_do_exclude_add(Client *u, char *type, char *pattern) 
{
	excludes *e;
	static char tmp[BUFSIZE];
	
	/* we dont do any checking to see if a similar entry already exists... oh well, thats upto the user */
	e = ns_calloc (sizeof(excludes));
	strlcpy(e->addedby, u->name, MAXNICK);
	e->addedon = me.now;
	if (!ircstrcasecmp("HOST", type)) {
		if (!index(pattern, '.')) {
			irc_prefmsg(ns_botptr, u, __("Error, Pattern must contain at least one \2.\2",u));
			ns_free(e);
			return;
		}
		e->type = NS_EXCLUDE_HOST;
		strlcpy(e->pattern, collapse(pattern), MAXHOST);
	} else if (!ircstrcasecmp("CHAN", type)) {
		if (pattern[0] != '#') {
			irc_prefmsg(ns_botptr, u, __("Error, Pattern must begin with a \2#\2", u));
			ns_free(e);
			return;
		}
		e->type = NS_EXCLUDE_CHAN;
		strlcpy(e->pattern, collapse(pattern), MAXHOST);
	} else if (!ircstrcasecmp("SERVER", type)) {
		if (!index(pattern, '.')) {
			irc_prefmsg(ns_botptr, u, __("Error, Pattern must contain at least one \2.\2", u));
			ns_free(e);
			return;
		}
		e->type = NS_EXCLUDE_SERVER;
		strlcpy(e->pattern, collapse(pattern), MAXHOST);
	} else {
		irc_prefmsg(ns_botptr, u, __("Error, Unknown type %s",u), type);
		ns_free(e);
		return;
	}
	/* if we get here, then e is valid */
	lnode_create_append (exclude_list, e);
	irc_prefmsg(ns_botptr, u, __("Successfully added %s (%s) to Exclusion List", u), e->pattern, type);
	irc_chanalert(ns_botptr, _("%s added %s (%s) to the Exclusion List"), u->name, e->pattern, type);

	/* now save the exclusion list */
	ircsnprintf(tmp, BUFSIZE, "%d", (int)e->addedon);
	SetData((void *)e->type, CFGINT, "Exclusions", tmp, "Type");
	SetData((void *)e->addedby, CFGSTR, "Exclusions", tmp, "AddedBy");
	SetData((void *)e->pattern, CFGSTR, "Exclusions", tmp, "Pattern");
} 

/* @brief del a entry from the exlusion list
 * 
 * @param u the user that sent the request
 * @param postition the position in the list that we are excluding
 * @returns nothing
 */
void ns_do_exclude_del(Client *u, char *position) 
{
	lnode_t *en;
	excludes *e;
	int i, pos;
	static char tmp[BUFSIZE];
	
	pos = atoi(position);
	if (pos < 1) {
		irc_prefmsg(ns_botptr, u, __("Error, Position %d is out of range",u), pos);
		return;
	}
	en = list_first(exclude_list);
	i = 1;
	while (en != NULL) {
		if (i == pos) {
			e = lnode_get(en);
			ircsnprintf(tmp, BUFSIZE, "%d", (int)e->addedon);
			DelRow("Exclusions", tmp);
			irc_prefmsg(ns_botptr, u, __("Deleted %s out of Exclusion List",u), e->pattern);
			ns_free(e);
			list_delete(exclude_list, en);
			lnode_destroy(en);
			return;
		}
		en = list_next(exclude_list, en);
		i++;
	}
	/* if we get here, means that we never got a match */
	irc_prefmsg(ns_botptr, u, __("Entry %d was not found in the exclusion list",u), pos);
	
} 

/* @brief list the entries from the exlusion list
 * 
 * @param u the user that sent the request
 * @param from the "user" that this message should come from, so we can call from modules
 * @returns nothing
 */
void ns_do_exclude_list(Client *source, Bot* botptr) 
{
	lnode_t *en;
	excludes *e;
	int i = 0;
	static char tmp[BUFSIZE];
	
	irc_prefmsg(botptr, source, __("Global Exclusion List:", source));
	en = list_first(exclude_list);
	i = 1;
	while (en != NULL) {
		e = lnode_get(en);
		switch (e->type) {
			case NS_EXCLUDE_HOST:
				ircsnprintf(tmp, BUFSIZE, __("Host", source));
				break;
			case NS_EXCLUDE_CHAN:
				ircsnprintf(tmp, BUFSIZE, __("Chan", source));
				break;
			case NS_EXCLUDE_SERVER:
				ircsnprintf(tmp, BUFSIZE, __("Server", source));
				break;
			default:
				ircsnprintf(tmp, BUFSIZE, __("Unknown", source));
				break;
		}
				
		irc_prefmsg(botptr, source, __("%d) %s (%s) Added by %s on %s", source), i, e->pattern, tmp, e->addedby, sftime(e->addedon));
		i++;
		en = list_next(exclude_list, en);
	}
	irc_prefmsg(botptr, source, __("End of Global Exclude List.", source));
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
	excludes *e;
	
	/* first thing we check is the server flag. if the server
	 * is excluded, then the user is excluded as well
	 */
	if (u->uplink->flags & NS_FLAGS_EXCLUDED) {
	 	u->flags |= NS_FLAGS_EXCLUDED;
		return;
	}
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_HOST) {
			if (match(e->pattern, u->user->hostname)) {
				u->flags |= NS_FLAGS_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	u->flags &= ~NS_FLAGS_EXCLUDED;
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
	excludes *e;
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_SERVER) {
			if (match(e->pattern, s->name)) {
				s->flags |= NS_FLAGS_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	s->flags &= ~NS_FLAGS_EXCLUDED;
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
	excludes *e;
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_CHAN) {
			if (match(e->pattern, c->name)) {
				c->flags |= NS_FLAGS_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	c->flags &= ~NS_FLAGS_EXCLUDED;
}
