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
#include "stats.h"
#include "exclude.h"
#include "conf.h"
#include "log.h"
#include "ircstring.h"

list_t *exclude_list;

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
int init_exclude_list() {
	char **row;
	int i;
	excludes *e;
	char *tmp;
	lnode_t *en;
	
	exclude_list = list_create(-1);
	if (GetTableData("Exclusions", &row) > 0) {
		for (i = 0; row[i] != NULL; i++) {
			e = malloc(sizeof(excludes));
			bzero(e, sizeof(excludes));
			if (GetData((void *)&tmp, CFGSTR, "Exclusions", row[i], "Pattern") > 0) {
				strlcpy(e->pattern, tmp, MAXHOST);
			} else {
				nlog(LOG_WARNING, LOG_CORE, "Exclusions: Can't add entry %s, Pattern invalid", row[i]);
				continue;
			}
			if (GetData((void *)&tmp, CFGSTR, "Exclusions", row[i], "AddedBy") > 0) {
				strlcpy(e->addedby, tmp, MAXNICK);
			}
			e->addedon = atoi(row[i]);
			GetData((void *)&e->type, CFGINT, "Exclusions", row[i], "Type");
			nlog(LOG_DEBUG2, LOG_CORE, "Added Exclusion %s (%d) by %s on %d", e->pattern, e->type, e->addedby, (int)e->addedon);
			en = lnode_create(e);
			list_append(exclude_list, en);
		}
	}
	free(row);
	return NS_SUCCESS;
} 



/* @brief add a entry to the exlusion list
 * 
 * @param u the user that sent the request
 * @param type the type of exclusion being added. Chan, Host or Server
 * @param pattern the actual pattern to use 
 * @returns nothing
 */
void ns_do_exclude_add(User *u, char *type, char *pattern) {
	lnode_t *en;
	excludes *e;
	char tmp[BUFSIZE];
	
	/* we dont do any checking to see if a similar entry already exists... oh well, thats upto the user */
	e = malloc(sizeof(excludes));
	strlcpy(e->addedby, u->nick, MAXNICK);
	e->addedon = me.now;
	if (!ircstrcasecmp("HOST", type)) {
		if (!index(pattern, '.')) {
			prefmsg(u->nick, s_Services, "Error, Pattern must contain at least one \2.\2");
			free(e);
			return;
		}
		e->type = NS_EXCLUDE_HOST;
		strlcpy(e->pattern, collapse(pattern), MAXHOST);
	} else if (!ircstrcasecmp("CHAN", type)) {
		if (pattern[0] != '#') {
			prefmsg(u->nick, s_Services, "Error, Pattern must begin with a \2#\2");
			free(e);
			return;
		}
		e->type = NS_EXCLUDE_CHAN;
		strlcpy(e->pattern, collapse(pattern), MAXHOST);
	} else if (!ircstrcasecmp("SERVER", type)) {
		if (!index(pattern, '.')) {
			prefmsg(u->nick, s_Services, "Error, Pattern must contain at least one \2.\2");
			free(e);
			return;
		}
		e->type = NS_EXCLUDE_SERVER;
		strlcpy(e->pattern, collapse(pattern), MAXHOST);
	} else {
		prefmsg(u->nick, s_Services, "Error, Unknown type %s", type);
		free(e);
		return;
	}
	/* if we get here, then e is valid */
	en = lnode_create(e);
	list_append(exclude_list, en);
	prefmsg(u->nick, s_Services, "Successfully added %s (%s) to Exclusion List", e->pattern, type);
	chanalert(u->nick, s_Services, "%s added %s (%s) to the Exclusion List", u->nick, e->pattern, type);

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
void ns_do_exclude_del(User *u, char *position) {
	lnode_t *en;
	excludes *e;
	int i, pos;
	char tmp[BUFSIZE];
	
	pos = atoi(position);
	if (pos < 1) {
		prefmsg(u->nick, s_Services, "Error, Position %d is out of range", pos);
		return;
	}
	en = list_first(exclude_list);
	i = 1;
	while (en != NULL) {
		if (i == pos) {
			e = lnode_get(en);
			ircsnprintf(tmp, BUFSIZE, "%d", (int)e->addedon);
			DelRow("Exclusions", tmp);
			prefmsg(u->nick, s_Services, "Deleted %s out of Exclusion List", e->pattern);
			free(e);
			list_delete(exclude_list, en);
			lnode_destroy(en);
			return;
		}
		en = list_next(exclude_list, en);
		i++;
	}
	/* if we get here, means that we never got a match */
	prefmsg(u->nick, s_Services, "Entry %d was not found in the exclusion list", pos);
	
} 

/* @brief list the entries from the exlusion list
 * 
 * @param u the user that sent the request
 * @param from the "user" that this message should come from, so we can call from modules
 * @returns nothing
 */
void ns_do_exclude_list(User *u, char *from) {
	lnode_t *en;
	excludes *e;
	int i = 0;
	char tmp[BUFSIZE];
	
	prefmsg(u->nick, from, "Global Exclusion List:");
	en = list_first(exclude_list);
	i = 1;
	while (en != NULL) {
		e = lnode_get(en);
		switch (e->type) {
			case NS_EXCLUDE_HOST:
				ircsnprintf(tmp, BUFSIZE, "Host");
				break;
			case NS_EXCLUDE_CHAN:
				ircsnprintf(tmp, BUFSIZE, "Chan");
				break;
			case NS_EXCLUDE_SERVER:
				ircsnprintf(tmp, BUFSIZE, "Server");
				break;
			default:
				ircsnprintf(tmp, BUFSIZE, "Unknown");
				break;
		}
				
		prefmsg(u->nick, from, "%d) %s (%s) Added by %s on %s", i, e->pattern, tmp, e->addedby, sftime(e->addedon));
		i++;
		en = list_next(exclude_list, en);
	}
	prefmsg(u->nick, from, "End of Global Exclude List.");
} 

/* @brief check if a user is matched against a exclusion
 * 
 * Modifies the flags field of the User struct to indicate if the user is matched
 * against a exclusion. This function is called when the user signs on
 *
 * @param u the user to check
 * @returns nothing
 */

void ns_do_exclude_user(User *u) {
	lnode_t *en;
	excludes *e;
	
	/* first thing we check is the server flag. if the server
	 * is excluded, then the user is excluded as well
	 */
	 if (u->server->flags && NS_FLAGS_EXCLUDED) {
	 	u->flags |= NS_FLAGS_EXCLUDED;
	 	return;
	 }
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_HOST) {
			if (match(e->pattern, u->hostname)) {
				u->flags &= ~NS_FLAGS_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	u->flags |= NS_FLAGS_EXCLUDED;
}

/* @brief check if a server is matched against a exclusion
 * 
 * Modifies the flags field of the server struct to indicate if the server is matched
 * against a exclusion. This function is called when the server connects
 *
 * @param s the Server to check
 * @returns nothing
 */

void ns_do_exclude_server(Server *s) {
	lnode_t *en;
	excludes *e;
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_SERVER) {
			if (match(e->pattern, s->name)) {
				s->flags &= ~NS_FLAGS_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	s->flags |= NS_FLAGS_EXCLUDED;
}


/* @brief check if a channel is matched against a exclusion
 * 
 * Modifies the flags field of the channel struct to indicate if the channel is matched
 * against a exclusion. This function is called when the channel is created
 *
 * @param c the channel to check
 * @returns nothing
 */

void ns_do_exclude_chan(Chans *c) {
	lnode_t *en;
	excludes *e;
	
	en = list_first(exclude_list);
	while (en != NULL) {
		e = lnode_get(en);
		if (e->type == NS_EXCLUDE_CHAN) {
			if (match(e->pattern, c->name)) {
				c->flags &= ~NS_FLAGS_EXCLUDED;
				return;
			}
		}
		en = list_next(exclude_list, en);
	}
	/* if we are here, there is no match */
	c->flags |= NS_FLAGS_EXCLUDED;
}
