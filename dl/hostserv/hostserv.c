/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
** $Id$
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "dotconf.h"
#include "stats.h"
#include "hs_help.c"
#include "log.h"
#include "conf.h"



/* hostserv doesn't work on Hybrid, Echo a error and exit the compile */
#ifndef GOTSVSVHOST
#error "Error: This IRCd doesn't support changing a users host. This module will not compile"
#endif



const char hsversion_date[] = __DATE__;
const char hsversion_time[] = __TIME__;
char s_HostServ[MAXNICK];
typedef struct hs_map_ {
	char nnick[MAXNICK];
	char host[MAXHOST];
	char vhost[MAXHOST];
	char passwd[30];
	char added[MAXNICK];
	time_t lused;
} hs_map;

hash_t *vhosts;
hash_t *bannedvhosts;

struct hs_lvl {
	int view;
	int list;
	int del;
	int add;
	int old;
} hs_lvl;

char nnick[255];

extern const char *hs_help[];
static int hs_sign_on(char **av, int ac);

static void hs_add(User * u, char *cmd, char *m, char *h, char *p);
static void hs_list(User * u);
static void hs_view(User * u, int tmpint);
static void hs_del(User * u, int tmpint);
static void hs_login(User * u, char *login, char *pass);
static void hs_chpass(User * u, char *login, char *pass, char *newpass);
static void hs_listban(User * u);
static void hs_addban(User * u, char *ban);
static void hs_delban(User * u, char *ban);
static void SaveBans();
static int new_m_version(char *origin, char **av, int ac);
void hsdat(char *nick, char *host, char *vhost, char *pass, char *who);
void CleanupHosts();

void Loadhosts();

int data_synch;
int load_synch;

char **DelArry;
char **ListArry;
int LoadArryCount = 0;
int DelArryCount = 0;
int ListArryCount = 0;

Module_Info HostServ_info[] = { {
				 "HostServ",
				 "Network User Virtual Host Service",
				 "2.7.1"}
};

void hs_Config()
{
	char *ban;
	char *host;
	char *host2;
	hnode_t *hn;
	char *ban2;

	strcpy(segv_location, "HostServ-hs_cb_Config");
	GetConf((void *) &hs_lvl.view, CFGINT, "ViewLevel");
	GetConf((void *) &hs_lvl.add, CFGINT, "AddLevel");
	GetConf((void *) &hs_lvl.del, CFGINT, "DelLevel");
	GetConf((void *) &hs_lvl.list, CFGINT, "ListLevel");
	GetConf((void *) &hs_lvl.old, CFGINT, "ExpireDays");
	if ((hs_lvl.list > 200) || (hs_lvl.list <= 0))
		hs_lvl.list = 40;
	if ((hs_lvl.add > 200) || (hs_lvl.view <= 0))
		hs_lvl.add = 40;
	if ((hs_lvl.del > 200) || (hs_lvl.del <= 0))
		hs_lvl.del = 40;
	if ((hs_lvl.view > 200) || (hs_lvl.list <= 0))
		hs_lvl.view = 100;

	/* banned vhosts */
	ban = NULL;
	GetConf((void *) &ban, CFGSTR, "BannedVhosts");
	ban2 = ban;
	if (ban) {
		host = strtok(ban, ";");
		if (host) {
			host2 = malloc(strlen(host));
			strncpy(host2, host, strlen(host));
			host2[strlen(host)] = '\0';
			hn = hnode_create(host2);
			hash_insert(bannedvhosts, hn, host2);
		}
		while ((host = strtok(NULL, ";")) != NULL) {
			host2 = malloc(strlen(host));
			strncpy(host2, host, strlen(host));
			host2[strlen(host)] = '\0';
			hn = hnode_create(host2);
			hash_insert(bannedvhosts, hn, host2);
		}
	}
	free(ban2);
}

int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(351, origin,
		     "Module HostServ Loaded, Version: %s %s %s",
		     HostServ_info[0].module_version, hsversion_date,
		     hsversion_time);
	return 0;
}


/* Routine For Setting the Virtual Host */
static int hs_sign_on(char **av, int ac)
{
	hnode_t *hn;
	hs_map *map;
	User *u;

	if (!is_synced)
		return 0;

	u = finduser(av[0]);
	if (!u)
		return 1;

	if (u->server->name == me.name)
		return 1;

	strcpy(segv_location, "HostServ-hs_signon");

	if (findbot(u->nick))
		return 1;
	if (!load_synch)
		return 1;

	/* Check HostName Against Data Contained in vhosts.data */

	hn = hash_lookup(vhosts, u->nick);
	if (hn) {
		map = hnode_get(hn);
		nlog(LOG_DEBUG1, LOG_MOD, "Checking %s against %s for HostName Match", map->host, u->hostname);
		if (fnmatch(map->host, u->hostname, 0) == 0) {
			ssvshost_cmd(u->nick, map->vhost);
			prefmsg(u->nick, s_HostServ,
				"Automatically setting your Virtual Host to %s",
				map->vhost);
			map->lused = time(NULL);
			return 1;
		}
	}
	return 1;
}

Functions HostServ_fn_list[] = {
	{MSG_VERSION, new_m_version, 1}
	,
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1}
	,
#endif
	{NULL, NULL, 0}
};

int __Bot_Message(char *origin, char **av, int ac)
{
	int t = 0;
	User *u;
	u = finduser(origin);

	if (!strcasecmp(av[1], "HELP")) {
		if (ac <= 2) {
			privmsg_list(u->nick, s_HostServ, hs_help);
			return 1;
		} else if (!strcasecmp(av[2], "ADD")
			   && (UserLevel(u) >= hs_lvl.add)) {
			privmsg_list(u->nick, s_HostServ, hs_help_add);
			return 1;
		} else if (!strcasecmp(av[2], "DEL")
			   && (UserLevel(u) >= hs_lvl.del)) {
			privmsg_list(u->nick, s_HostServ, hs_help_del);
			return 1;
		} else if (!strcasecmp(av[2], "LIST")
			   && (UserLevel(u) >= hs_lvl.list)) {
			privmsg_list(u->nick, s_HostServ, hs_help_list);
			return 1;
		} else if (!strcasecmp(av[2], "VIEW")
			   && (UserLevel(u) >= hs_lvl.view)) {
			privmsg_list(u->nick, s_HostServ, hs_help_view);
			return 1;
		} else if (!strcasecmp(av[2], "LEVELS")
			   && (UserLevel(u) >= 40)) {
			privmsg_list(u->nick, s_HostServ, hs_help_levels);
			return 1;
		} else if (!strcasecmp(av[2], "LOGIN")) {
			privmsg_list(u->nick, s_HostServ, hs_help_login);
			return 1;
		} else if (!strcasecmp(av[2], "CHPASS")) {
			privmsg_list(u->nick, s_HostServ, hs_help_chpass);
			return 1;
		} else if (!strcasecmp(av[2], "BANS")) {
			privmsg_list(u->nick, s_HostServ, hs_help_listban);
			return 1;
		} else if (!strcasecmp(av[2], "ABOUT")) {
			privmsg_list(u->nick, s_HostServ, hs_help_about);
			return 1;
		} else
			prefmsg(u->nick, s_HostServ,
				"Unknown Help Topic: \2%s\2", av[2]);
	}

	if (!strcasecmp(av[1], "ABOUT")) {
		privmsg_list(u->nick, s_HostServ, hs_help_about);
		prefmsg(u->nick, s_HostServ,
			"Un-used Vhosts expire after %d days", hs_lvl.old);
		return 1;
	} else if (!strcasecmp(av[1], "ADD")
		   && (UserLevel(u) >= hs_lvl.add)) {
		if (ac < 6) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME> <PASSWORD>",
				s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"For addtional help: /msg %s HELP",
				s_HostServ);
			return -1;
		}
		hs_add(u, av[2], av[3], av[4], av[5]);
	} else if (!strcasecmp(av[1], "DEL")
		   && (UserLevel(u) >= hs_lvl.del)) {
		if (!av[2]) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s DEL #", s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"The users # is got from /msg %s LIST",
				s_HostServ);
			return -1;
		}
		t = atoi(av[2]);
		if (!t) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s DEL #", s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"The users # is got from /msg %s LIST",
				s_HostServ);
			return -1;
		}
		hs_del(u, t);
	} else if (!strcasecmp(av[1], "LIST")
		   && (UserLevel(u) >= hs_lvl.list)) {
		hs_list(u);
		return 1;
	} else if (!strcasecmp(av[1], "BANS")) {
		if (ac == 2) {
			hs_listban(u);
			return 1;
		} else if (ac == 4) {
			if (UserLevel(u) >= 185) {
				if (!strcasecmp(av[2], "ADD")) {
					hs_addban(u, av[3]);
					return 1;
				} else if (!strcasecmp(av[2], "DEL")) {
					hs_delban(u, av[3]);
					return 1;
				}
			} else {
				prefmsg(u->nick, s_HostServ,
					"Permission Denied");
				chanalert(s_HostServ,
					  "%s tried to %s bans, but was denied",
					  u->nick, av[2]);
				return 1;
			}
		}
		/* if we get to here, bah, stupid lusers */
		prefmsg(u->nick, s_HostServ,
			"Syntax: /msg %s BANS [DEL/ADD] <options>",
			s_HostServ);
		prefmsg(u->nick, s_HostServ,
			"/msg %s help bans for more info", s_HostServ);
		return -1;
	} else if (!strcasecmp(av[1], "LEVELS") && (UserLevel(u) >= 40)) {
		if (ac == 2) {
			prefmsg(u->nick, s_HostServ,
				"Configured Levels: Add: %d, Del: %d, List: %d, View: %d",
				hs_lvl.add, hs_lvl.del, hs_lvl.list,
				hs_lvl.view);
			return 1;
		} else if (ac == 4) {
			if (UserLevel(u) >= 185) {
				t = atoi(av[3]);
				if ((t <= 0) || (t > 200)) {
					prefmsg(u->nick, s_HostServ,
						"Invalid Level. Must be between 1 and 200");
					return -1;
				}
				if (!strcasecmp(av[2], "ADD")) {
					hs_lvl.add = t;
					SetConf((void *) t, CFGINT,
						"AddLevel");
				} else if (!strcasecmp(av[2], "DEL")) {
					hs_lvl.del = t;
					SetConf((void *) t, CFGINT,
						"DelLevel");
				} else if (!strcasecmp(av[2], "LIST")) {
					hs_lvl.list = t;
					SetConf((void *) t, CFGINT,
						"ListLevel");
				} else if (!strcasecmp(av[2], "VIEW")) {
					hs_lvl.view = t;
					SetConf((void *) t, CFGINT,
						"ViewLevel");
				} else {
					prefmsg(u->nick, s_HostServ,
						"Invalid Level. /msg %s help levels",
						s_HostServ);
					return 1;
				}
				prefmsg(u->nick, s_HostServ,
					"Level for %s set to %d", av[2],
					t);
				return 1;
			}
			prefmsg(u->nick, s_HostServ, "Permission Denied");
		}
		prefmsg(u->nick, s_HostServ,
			"Invalid Syntax. /msg %s help levels", s_HostServ);
		return 1;
	} else if (!strcasecmp(av[1], "VIEW")
		   && (UserLevel(u) >= hs_lvl.view)) {
		if (!av[2]) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s VIEW #", s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"The users # is got from /msg %s LIST",
				s_HostServ);
			return -1;
		}
		t = atoi(av[2]);
		if (!t) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s VIEW #", s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"The users # is got from /msg %s LIST",
				s_HostServ);
			return -1;
		}
		hs_view(u, t);
	} else if (!strcasecmp(av[1], "LOGIN")) {
		if (!av[3]) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s LOGIN <NICK> <PASSWORD>",
				s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"For addtional help: /msg %s HELP LOGIN",
				s_HostServ);
			return -1;
		}
		hs_login(u, av[2], av[3]);
	} else if (!strcasecmp(av[1], "CHPASS")) {
		if (ac < 4) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s CHPASS <nick> <oldpass> <newpass>",
				s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"For additional help: /msg %s HELP CHPASS",
				s_HostServ);
			return -1;
		}
		hs_chpass(u, av[2], av[3], av[4]);
	} else {
		prefmsg(u->nick, s_HostServ,
			"Unknown Command: \2%s\2, perhaps you need some HELP?",
			av[1]);
		chanalert(s_HostServ,
			  "%s tried %s, but they have no access, or command was not found",
			  u->nick, av[1]);
	}
	return 1;

}

int Online(char **av, int ac)
{
	char *user;
	char *host;
	char *rname;

	if (GetConf((void *) &s_HostServ, CFGSTR, "Nick") < 0) {
/*		s_HostServ = malloc(MAXNICK); */
		snprintf(s_HostServ, MAXNICK, "HostServ");
	}
	if (GetConf((void *) &user, CFGSTR, "User") < 0) {
		user = malloc(MAXUSER);
		snprintf(user, MAXUSER, "HS");
	}
	if (GetConf((void *) &host, CFGSTR, "Host") < 0) {
		host = malloc(MAXHOST);
		snprintf(host, MAXHOST, me.name);
	}
	if (GetConf((void *) &rname, CFGSTR, "RealName") < 0) {
		rname = malloc(MAXHOST);
		snprintf(rname, MAXHOST, "Network Virtual Host Service");
	}


	if (init_bot
	    (s_HostServ, user, host, rname, "+oS",
	     HostServ_info[0].module_name) == -1) {
		/* Nick was in use */
		snprintf(s_HostServ, MAXNICK, "%s_", s_HostServ);
		init_bot(s_HostServ, user, host, rname, "+oS",
			 HostServ_info[0].module_name);
	}
	free(user);
	free(host);
	free(rname);
	chanalert(s_HostServ,
		  "Configured Levels: Add: %d, Del: %d, List: %d, View: %d",
		  hs_lvl.add, hs_lvl.del, hs_lvl.list, hs_lvl.view);

	add_mod_timer("CleanupHosts", "Cleanup_Old_Vhosts",
		      HostServ_info[0].module_name, 7200);
	Loadhosts();

	return 1;
};

EventFnList HostServ_Event_list[] = {
	{"ONLINE", Online}
	,
	{"SIGNON", hs_sign_on}
	,
	{NULL, NULL}
};

Module_Info *__module_get_info()
{
	return HostServ_info;
};

Functions *__module_get_functions()
{
	return HostServ_fn_list;
};

EventFnList *__module_get_events()
{
	return HostServ_Event_list;
};


void _init()
{
	strcpy(segvinmodule, HostServ_info[0].module_name);

	vhosts = hash_create(-1, 0, 0);
	bannedvhosts = hash_create(-1, 0, 0);
	if (!vhosts) {
		nlog(LOG_CRITICAL, LOG_CORE,
		     "Error, Can't create vhosts hash");
		chanalert(s_Services, "Error, Can't create Vhosts Hash");
	}
	hs_lvl.add = 40;
	hs_lvl.del = 40;
	hs_lvl.list = 40;
	hs_lvl.view = 100;
	hs_lvl.old = 60;
	hs_Config();

}

void _fini()
{
	hnode_t *hn;
	hscan_t hs;
	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		hash_scan_delete(vhosts, hn);
		free(hnode_get(hn));
	}
	hash_destroy(vhosts);

};


void write_database()
{
	FILE *hsfile = fopen("data/vhosts.db", "w");
	hnode_t *hn;
	hscan_t hs;
	hs_map *map;

	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		map = hnode_get(hn);
		fprintf(hsfile, ":%s %s %s %s %s %ld\n", map->nnick,
			map->host, map->vhost, map->passwd, map->added,
			map->lused);
	}
	fclose(hsfile);
}

/* Routine for registrations with the 'vhosts.db' file */
void hsdat(char *nick, char *host, char *vhost, char *pass, char *who)
{
	hnode_t *hn;
	hs_map *map;

	map = malloc(sizeof(hs_map));
	strncpy(map->nnick, nick, MAXNICK);
	strncpy(map->host, host, MAXHOST);
	strncpy(map->vhost, vhost, MAXHOST);
	strncpy(map->passwd, pass, 30);
	strncpy(map->added, who, MAXNICK);
	map->lused = time(NULL);
	hn = hnode_create(map);
	hash_insert(vhosts, hn, map->nnick);

	write_database();

}
static void hs_listban(User * u)
{
	hnode_t *hn;
	hscan_t hs;
	int i = 0;
	hash_scan_begin(&hs, bannedvhosts);
	prefmsg(u->nick, s_HostServ, "Banned Hostnames for Vhosts");
	while ((hn = hash_scan_next(&hs)) != NULL) {
		prefmsg(u->nick, s_HostServ, "%d - %s", ++i,
			(char *) hnode_get(hn));
	}
	prefmsg(u->nick, s_HostServ, "End of List.");

}
static void hs_addban(User * u, char *ban)
{
	hnode_t *hn;
	char *host;


	host = malloc(strlen(ban));
	strncpy(host, ban, strlen(ban) + 1);
	if (hash_lookup(bannedvhosts, host) != NULL) {
		prefmsg(u->nick, s_HostServ,
			"%s already exists in the banned vhost list", ban);
		free(host);
		return;
	}
	hn = hnode_create(host);
	hash_insert(bannedvhosts, hn, host);
	prefmsg(u->nick, s_HostServ,
		"%s has been added to the Banned Vhosts list", ban);
	chanalert(s_HostServ, "%s added %s to the banned vhosts list",
		  u->nick, ban);
	SaveBans();
}

static void hs_delban(User * u, char *ban)
{
	hnode_t *hn;
	hscan_t hs;
	int i = 0;
	int j = 0;

	i = atoi(ban);
	if ((i < 1) || (i > hash_count(bannedvhosts))) {
		prefmsg(u->nick, s_HostServ,
			"Invalid Entry. /msg %s bans for the list",
			s_HostServ);
		return;
	}

	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		++j;
		if (i == j) {
			hash_scan_delete(bannedvhosts, hn);
			prefmsg(u->nick, s_HostServ,
				"Deleted the Folling Banned Vhosts: %s",
				(char *) hnode_get(hn));
			chanalert(s_HostServ,
				  "%s deleted %s from the banned vhost list",
				  u->nick, (char *) hnode_get(hn));
			nlog(LOG_NOTICE, LOG_MOD,
			     "%s deleted %s from the banned vhost list",
			     u->nick, (char *) hnode_get(hn));
			free(hnode_get(hn));
			hnode_destroy(hn);
			SaveBans();
			return;
		}
	}
	prefmsg(u->nick, s_HostServ, "Entry %d was not found (Weird?!?)",
		i);
}

static void SaveBans()
{
	hnode_t *hn;
	hscan_t hs;
	char bans[4096];
	bzero(bans, 4096);
	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		strncat(bans, (char *) hnode_get(hn), 4096 - strlen(bans));
		strncat(bans, ";", 4096 - strlen(bans));
	}
	SetConf((void *) bans, CFGSTR, "BannedVhosts");
}


static void hs_chpass(User * u, char *nick, char *oldpass, char *newpass)
{
	hnode_t *hn;
	hs_map *map;

	hn = hash_lookup(vhosts, nick);
	if (hn) {
		map = hnode_get(hn);
		if ((fnmatch(map->host, u->hostname, 0) == 0)
		    || (UserLevel(u) >= 100)) {
			if (!strcasecmp(map->passwd, oldpass)) {
				strncpy(map->passwd, newpass, 30);
				prefmsg(u->nick, s_HostServ,
					"Password Successfully changed");
				chanalert(s_HostServ,
					  "%s changed the password for %s",
					  u->nick, map->nnick);
				map->lused = time(NULL);
				write_database();
				return;
			}
		} else {
			prefmsg(u->nick, s_HostServ,
				"Error, Hostname Mis-Match");
			chanalert(s_HostServ,
				  "%s tried to change the password for %s, but the hosts do not match (%s->%s)",
				  u->nick, map->nnick, u->hostname,
				  map->host);
			nlog(LOG_WARNING, LOG_MOD,
			     "%s tried to change the password for %s but the hosts do not match (%s -> %s)",
			     u->nick, map->nnick, u->hostname, map->host);
			return;
		}
	}

	prefmsg(u->nick, s_HostServ,
		"Invalid Username or Password. Access Denied");
	chanalert(s_HostServ,
		  "%s tried to change the password for %s, but got it wrong",
		  u->nick, nick);

}


/* Routine for ADD */
static void hs_add(User * u, char *cmd, char *m, char *h, char *p)
{

	hnode_t *hn;
	hscan_t hs;
	User *nu;

	strcpy(segv_location, "hs_add");
	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		if (fnmatch(hnode_get(hn), h, 0) == 0) {
			prefmsg(u->nick, s_HostServ,
				"The Hostname %s has been matched against the banned hostname %s",
				h, (char *) hnode_get(hn));
			chanalert(u->nick,
				  "%s tried to add a banned vhosts %s",
				  u->nick, h);
			return;
		}
	}


	if (!hash_lookup(vhosts, cmd)) {
		hsdat(cmd, m, h, p, u->nick);
		nlog(LOG_NOTICE, LOG_MOD,
		     "%s added a vhost for %s with realhost %s vhost %s and password %s",
		     u->nick, cmd, m, h, p);
		prefmsg(u->nick, s_HostServ,
			"%s has sucessfuly been registered under realhost: %s vhost: %s and password: %s",
			cmd, m, h, p);
		chanalert(s_HostServ,
			  "%s added a vhost %s for %s with realhost %s",
			  u->nick, h, cmd, m);
		/* Apply The New Hostname If The User Is Online */
		if ((nu = finduser(cmd)) != NULL) {
			if (findbot(cmd))
				return;
			if (fnmatch(m, nu->hostname, 0) == 0) {
				ssvshost_cmd(nu->nick, h);
				prefmsg(u->nick, s_HostServ,
					"%s is online now, setting vhost to %s",
					cmd, h);
				prefmsg(cmd, s_HostServ,
					"You Vhost has been created with Real HostMask of %s and username %s with password %s",
					m, cmd, p);
				prefmsg(cmd, s_HostServ,
					"For security, you should change your vhost password. See /msg %s help chpass",
					s_HostServ);
				return;
			}
		}
	} else {
		prefmsg(u->nick, s_HostServ,
			"%s already has a HostServ Entry", cmd);
		return;
	}
}


/* Routine for 'HostServ' to print out its data */
static void hs_list(User * u)
{
	int i;
	hnode_t *hn;
	hscan_t hs;
	hs_map *map;

	strcpy(segv_location, "hs_list");

	i = 1;
	prefmsg(u->nick, s_HostServ, "Current HostServ VHOST list:");
	prefmsg(u->nick, s_HostServ, "%-5s %-12s %-30s", "Num", "Nick",
		"Vhost");
	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		map = hnode_get(hn);
		prefmsg(u->nick, s_HostServ, "%-5d %-12s %-30s", i,
			map->nnick, map->vhost);
		i++;
	}
	prefmsg(u->nick, s_HostServ,
		"For more information on someone use /msg %s VIEW #",
		s_HostServ);
	prefmsg(u->nick, s_HostServ, "--- End of List ---");
}

/* Routine for VIEW */
static void hs_view(User * u, int tmpint)
{
	int i;
	hnode_t *hn;
	hscan_t hs;
	hs_map *map;
	char ltime[80];
	strcpy(segv_location, "hs_view");

	i = 1;
	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		if (tmpint == i) {
			map = hnode_get(hn);
			prefmsg(u->nick, s_HostServ,
				"Virtual Host information:");
			prefmsg(u->nick, s_HostServ, "Nick:     %s",
				map->nnick);
			prefmsg(u->nick, s_HostServ, "RealHost: %s",
				map->host);
			prefmsg(u->nick, s_HostServ, "V-host:   %s",
				map->vhost);
			prefmsg(u->nick, s_HostServ, "Password: %s",
				map->passwd);
			prefmsg(u->nick, s_HostServ, "Added by: %s",
				map->added ? map->added : "<unknown>");
			strftime(ltime, 80, "%d/%m/%Y[%H:%M]",
				 localtime(&map->lused));
			prefmsg(u->nick, s_HostServ, "Last Used on %s",
				ltime);
			prefmsg(u->nick, s_HostServ,
				"--- End of information for %s ---",
				map->nnick);
		}
		i++;
	}
	if (tmpint > i)
		prefmsg(u->nick, s_HostServ,
			"ERROR: There is no vhost on list number \2%d\2",
			tmpint);
}




/* Routine For Loading The Hosts into Array */
void Loadhosts()
{
	FILE *fp;
	hs_map *map;
	hnode_t *hn;
	char buf[512];
	char **LoadArry;
	fp = fopen("data/vhosts.db", "r");
	if (fp) {
		load_synch = 1;
		while (fgets(buf, 512, fp)) {
			strip(buf);
			LoadArryCount = split_buf(buf, &LoadArry, 0);
			if (!hash_lookup(vhosts, LoadArry[0])) {
				map = malloc(sizeof(hs_map));
				strncpy(map->nnick, LoadArry[0], MAXNICK);
				strncpy(map->host, LoadArry[1], MAXHOST);
				strncpy(map->vhost, LoadArry[2], MAXHOST);
				if (LoadArryCount > 3) {	/* Check for upgrades from earlier versions */
					strncpy(map->passwd, LoadArry[3],
						50);
				} else	/* Upgrading from earlier version, no passwds exist */
					strncpy(map->passwd, NULL, 50);
				if (LoadArryCount > 4) {	/* Does who set it exist? Yes? go ahead */
					strncpy(map->added, LoadArry[4],
						MAXNICK);
				} else	/* We have no information on who set it so its null */
					strcpy(map->added, "\0");
				if (LoadArryCount > 5) {
					map->lused = atoi(LoadArry[5]);
				} else
					map->lused = time(NULL);
				map->nnick[strlen(map->nnick)] = '\0';
				/* add it to the hash */
				hn = hnode_create(map);
				hash_insert(vhosts, hn, map->nnick);
				nlog(LOG_DEBUG1, LOG_CORE,
				     "Loaded %s (%s) into Vhosts",
				     map->nnick, map->vhost);
			} else {
				nlog(LOG_NOTICE, LOG_CORE,
				     "HostServ: db entry for %s already exists",
				     LoadArry[0]);
			}
			free(LoadArry);
		}
		fclose(fp);
	}
}


/* Routine for HostServ to delete the given information */
static void hs_del(User * u, int tmpint)
{

	int i = 1;
	hnode_t *hn;
	hscan_t hs;
	hs_map *map;

	strcpy(segv_location, "hs_del");

	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		if (i == tmpint) {
			map = hnode_get(hn);
			prefmsg(u->nick, s_HostServ,
				"The following vhost was removed from the Vhosts Database");
			prefmsg(u->nick, s_HostServ, "\2%s - %s\2",
				map->nnick, map->vhost);
			nlog(LOG_NOTICE, LOG_MOD,
			     "%s removed the VHOST: %s for %s", u->nick,
			     map->vhost, map->nnick);
			chanalert(s_HostServ, "%s removed vhost %s for %s",
				  u->nick, map->vhost, map->nnick);
			hash_scan_delete(vhosts, hn);
			free(map);
			hnode_destroy(hn);
		}
		i++;
	}

	if (tmpint > i)
		prefmsg(u->nick, s_HostServ,
			"ERROR: There is no vhost on list number \2%d\2",
			tmpint);
	else
		write_database();
	return;
}


/* Routine to allow users to login and get their vhosts */
static void hs_login(User * u, char *login, char *pass)
{
	hs_map *map;
	hnode_t *hn;

	strcpy(segv_location, "HostServ-hs_login");

	/* Check HostName Against Data Contained in vhosts.data */
	hn = hash_lookup(vhosts, login);
	if (hn) {
		map = hnode_get(hn);
		if (!strcasecmp(map->passwd, pass)) {
			ssvshost_cmd(u->nick, map->vhost);
			map->lused = time(NULL);
			prefmsg(u->nick, s_HostServ,
				"Your VHOST %s has been set.", map->vhost);
			nlog(LOG_NORMAL, LOG_MOD,
			     "%s used LOGIN to obtain userhost of %s",
			     u->nick, map->vhost);
			chanalert(s_HostServ,
				  "%s used login to get Vhost %s", u->nick,
				  map->vhost);
			return;
		}
	}
	prefmsg(u->nick, s_HostServ,
		"Incorrect Login or Password.  Do you have a vhost added?");
	return;
}

void CleanupHosts()
{
	hnode_t *hn;
	hscan_t hs;
	hs_map *map;

	strcpy(segv_location, "hs_del");

	hash_scan_begin(&hs, vhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		map = hnode_get(hn);
		if (map->lused < (time(NULL) - (hs_lvl.old * 86400))) {
			nlog(LOG_NOTICE, LOG_MOD,
			     "Old Vhost Automatically removed: %s for %s",
			     map->vhost, map->nnick);
			hash_scan_delete(vhosts, hn);
			free(map);
			hnode_destroy(hn);
		}
	}
}
