/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: serviceroots.c,v 1.8 2003/01/06 12:07:26 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "dotconf.h"

extern const char version_date[], version_time[];

static int new_m_version(char *origin, char **av, int ac);
void sr_cb_config(char *arg, int configtype);

Module_Info extauth_Info[] = { {
    "extauth",
    "ServiceRoots Authentication Module",
    "1.0"
} };

Functions ServiceRoots_fn_list[] = { 
    { MSG_VERSION,    new_m_version,    1 },
#ifdef HAVE_TOKEN_SUP
    { TOK_VERSION,    new_m_version,    1 },
#endif
    { NULL,        NULL,     0}
};


Module_Info *__module_get_info() {
    return extauth_Info;
};

Functions *__module_get_functions() {
    return ServiceRoots_fn_list;
};

static config_option options[] = {
{ "SERVICE_ROOTS", ARG_STR, sr_cb_config, 0},
{ "SERVICE_ROOTS_AUTH", ARG_STR, sr_cb_config, 1}

};


struct srconf {
	list_t *ul;
	int auth;
} srconf;



int new_m_version(char *origin, char **av, int ac) {
    strcpy(segv_location, "serviceRoots-new_m_version");
    snumeric_cmd(351, origin, "Module ServiceRoots Loaded, Version: %s %s %s",extauth_Info[0].module_version,version_date,version_time);
    return 0;
}

static void _init() {
	srconf.auth=0;
	/* only a max of 10 serviceroots */
	srconf.ul = list_create(10);
	if (!config_read("neostats.cfg", options) ==0 ) {
		log("ehh, config failed");
	}	
}

static void _fini() {
	lnode_t *un;
	un = list_first(srconf.ul);
	while (un) {
		free(lnode_get(un));
		un = list_next(srconf.ul, un);
	}
	list_destroy_nodes(srconf.ul);
}

void sr_cb_config(char *arg, int configtype) {
	lnode_t *un;
	char *nick;
	nick = malloc(strlen(arg)+1);
	bzero(nick, strlen(arg)+1);
	strcpy(segv_location, "StatServ-ss_cb_Config");
	if (configtype == 0) {
		if (list_isfull(srconf.ul)) {
			log("Exceded Maxium Number of ServiceRoots(10)");
			return;
		} else {
			strncpy(nick, arg, strlen(arg));
			if (list_find(srconf.ul, nick, comparef)) {
				return;
			}
			un = lnode_create(nick);
			list_append(srconf.ul, un);
		}	
	} else if (configtype == 1) {
		/* ServiceRootsAuth */
		srconf.auth=1;
	}
}
	
extern int __do_auth(User *u, int curlvl) {
	lnode_t *un;
#ifndef HYBRID7
	if (u->Umode & UMODE_REGNICK) {
#else
	/* this is *baaaaaaaaaaad* */
	if (1) {
#endif
		un = list_first(srconf.ul);
		while (un) {
			if (!strcasecmp(u->nick, lnode_get(un))) {
				if (srconf.auth == 1) {
					return (200);
				} else {
					return (200);
				}
			}
			un = list_next(srconf.ul, un);
		}
	} else {
		if (srconf.auth == 1) {
			curlvl = 0;
		}
	}
	return curlvl;
}

extern int __list_auth(User *u) {

	lnode_t *un;
	un = list_first(srconf.ul);
	while (un) {
		snumeric_cmd(243, u->nick, "O *@* %s S", (char *) lnode_get(un));
		un = list_next(srconf.ul, un);
	}
	return 1;
}
