/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      serviceroots.c, 
** Version: 3.1
** Date:    08/03/2002
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
    { TOK_VERSION,    new_m_version,    1 },
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

void _init() {
log("haha");
	srconf.auth=0;
	/* only a max of 10 serviceroots */
	srconf.ul = list_create(10);
	if (!config_read("stats.cfg", options) ==0 ) {
		log("ehh, config failed");
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
	nick = malloc(strlen(arg));
	strcpy(segv_location, "StatServ-ss_cb_Config");

	if (configtype == 0) {
		if (list_isfull(srconf.ul)) {
			log("Exceded Maxium Number of ServiceRoots(10)");
			return;
		} else {
			strcpy(nick, arg);
			un = lnode_create(nick);
			printf("nick %s\n", nick);
			list_append(srconf.ul, un);
		}	
	} else if (configtype == 1) {
		/* ServiceRootsAuth */
		srconf.auth=1;
	}
}
	
extern int __do_auth(User *u, int curlvl) {
	lnode_t *un;
	char *nick;
	if (u->Umode & UMODE_REGNICK) {
		un = list_first(srconf.ul);
		while (un) {
			if (!strcasecmp(u->nick, lnode_get(un))) {
				if (srconf.auth == 1) {
					return (1200);
				} else {
					return (200);
				}
			}
			un = list_next(srconf.ul, un);
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
