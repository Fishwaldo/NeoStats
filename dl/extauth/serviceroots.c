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
** $Id: serviceroots.c,v 1.10 2003/04/17 13:48:14 fishwaldo Exp $
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"
#include "dotconf.h"
#include "log.h"
#include "conf.h"

extern const char version_date[], version_time[];

static int new_m_version(char *origin, char **av, int ac);
void sr_cb_config(char *arg, int configtype);

Module_Info extauth_Info[] = { {
    "extauth",
    "ServiceRoots Authentication Module",
    "1.1"
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
{ "SERVICE_ROOTS", ARG_STR, sr_cb_config, 0}
};


struct srconf {
	list_t *ul;
	int auth;
} srconf;

struct users {
	char nick[MAXNICK];
	char ident[MAXUSER];
	char host[MAXHOST];
	int lvl;
} users;



int new_m_version(char *origin, char **av, int ac) {
    strcpy(segv_location, "serviceRoots-new_m_version");
    snumeric_cmd(351, origin, "Module ServiceRoots Loaded, Version: %s %s %s",extauth_Info[0].module_version,version_date,version_time);
    return 0;
}

void _init() {
	srconf.auth=0;
	/* only a max of 10 serviceroots */
	srconf.ul = list_create(10);
	if (!config_read("neostats.cfg", options) ==0 ) {
		nlog(LOG_WARNING, LOG_CORE, "ServiceRoots: ehh, config failed");
	}	
}

void _fini() {
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
	char *user;
	char *host;
	struct users *sru;
	
	
	strcpy(segv_location, "StatServ-ss_cb_Config");
	if (configtype == 0) {
		if (list_isfull(srconf.ul)) {
			nlog(LOG_WARNING, LOG_CORE, "Exceded Maxium Number of ServiceRoots(10)");
			return;
		} else {
			/* new code to do hostname. ident lookups */
			if (strstr(arg, "!")) {
				if (!strstr(arg, "@")) {
					nlog(LOG_WARNING, LOG_CORE, "Invalid ServiceRoots Entry. Must be of the form nick!ident@host, was %s", arg);
					return;
				}
				nick = strtok(arg, "!");
				user = strtok(NULL, "@");
				host = strtok(NULL, "");
				sru = malloc(sizeof(users));
				strncpy(sru->nick, nick, MAXNICK);
				strncpy(sru->ident, user, MAXUSER);
				strncpy(sru->host, host, MAXHOST);
				sru->lvl = 200;				
			} else {
				/* old format... Warn, but keep going */
				sru = malloc(sizeof(users));
				strncpy(sru->nick, arg, MAXNICK);
				strncpy(sru->ident, "*", MAXUSER);
				strncpy(sru->host, "*", MAXHOST);
				sru->lvl = 200;
				nlog(LOG_WARNING, LOG_CORE, "Old ServiceRoots Entry Detected. Suggest you upgrade ASAP to <nick>!<ident>@<host> (WildCards are allowed)");
			}
			un = lnode_create(sru);
			list_append(srconf.ul, un);
		}	
	}
}
	
extern int __do_auth(User *u, int curlvl) {
	lnode_t *un;
	struct users *sru;
	un = list_first(srconf.ul);
	while (un) {
		sru = lnode_get(un);
		if ((!fnmatch(sru->nick, u->nick, 0)) && (!fnmatch(sru->ident, u->username, 0)) && (!fnmatch(sru->host, u->hostname, 0))) {
			return (sru->lvl);
		}
		un = list_next(srconf.ul, un);
	}
	return curlvl;
}

extern int __list_auth(User *u) {

	lnode_t *un;
	struct users *sru;
	un = list_first(srconf.ul);
	while (un) {
		sru = lnode_get(un);
		snumeric_cmd(243, u->nick, "O %s@%s %s %d", sru->ident, sru->host, sru->nick, sru->lvl);
		un = list_next(srconf.ul, un);
	}
	return 1;
}
