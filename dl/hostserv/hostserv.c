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

/* hostserv doesn't work on Hybrid, Echo an error and exit the compile */
#ifndef GOTSVSVHOST
#error "Error: This IRCd doesn't support changing a users host. This module will not compile"
#endif

#define MAXPASSWORD	30

#define BANBUFSIZE	4096
static char ban_buf[BANBUFSIZE];


char s_HostServ[MAXNICK];
typedef struct hs_map_ {
	char nnick[MAXNICK];
	char host[MAXHOST];
	char vhost[MAXHOST];
	char passwd[MAXPASSWORD];
	char added[MAXNICK];
	time_t lused;
} hs_map;

list_t *vhosts;
hash_t *bannedvhosts;

struct hs_cfg {
	int view;
	int list;
	int del;
	int add;
	int old;
	int expire;
#ifdef UMODE_REGNICK
	int regnick;
	char vhostdom[MAXHOST];
#endif
	int modnum;
} hs_cfg;

#ifdef UMODE_REGNICK
typedef struct hs_user {
	int vhostset;
	hs_map *vhe;
} hs_user;
#endif

extern const char *hs_help[];
static int hs_sign_on(char **av, int ac);
static int hs_mode(char **av, int ac);

static void hs_add(User * u, char *cmd, char *m, char *h, char *p);
static void hs_list(User * u, int count);
static void hs_view(User * u, int tmpint);
static void hs_del(User * u, int tmpint);
static void hs_login(User * u, char *login, char *pass);
static void hs_chpass(User * u, char *login, char *pass, char *newpass);
static void hs_listban(User * u);
static void hs_addban(User * u, char *ban);
static void hs_delban(User * u, char *ban);
static void SaveBans();
static void hs_set(User * u, char **av, int ac);
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

ModuleInfo __module_info = {
	 "HostServ",
	 "Network User Virtual Host Service",
	 "3.1",
	__DATE__,
	__TIME__
};

int findnick(const void *key1, const void *key2)
{
	const hs_map *vhost = key1;
	return (strcasecmp(vhost->nnick, (char *)key2));
}

void save_vhost(hs_map *vhost) {
	SetData((void *)vhost->host, CFGSTR, "Vhosts", vhost->nnick, "Host");
	SetData((void *)vhost->vhost, CFGSTR, "Vhosts", vhost->nnick , "Vhost");
	SetData((void *)vhost->passwd, CFGSTR, "Vhosts", vhost->nnick , "Passwd");
	SetData((void *)vhost->added, CFGSTR, "Vhosts", vhost->nnick , "Added");
	SetData((void *)vhost->lused, CFGINT, "Vhosts", vhost->nnick , "LastUsed");
	list_sort(vhosts, findnick);
}
void del_vhost(hs_map *vhost) {
	DelRow("Vhosts", vhost->nnick);
	free(vhost);
	/* no need to list sort here, because its already sorted */
}

void set_moddata(User *u) {
	hs_user *hs;
	if (!u->moddata[hs_cfg.modnum]) {
		hs = malloc(sizeof(hs_user));
		hs->vhostset = 1;
		u->moddata[hs_cfg.modnum] = hs;
	}
}

int del_moddata(char **av, int ac) {

	User *u;
	u = finduser(av[0]);
	if (!u) /* User not found */
		return 1;
	if (u->moddata[hs_cfg.modnum]) {
		nlog(LOG_DEBUG2, LOG_MOD, "Freeing Module data");
		free(u->moddata[hs_cfg.modnum]);
		u->moddata[hs_cfg.modnum] = NULL;
	}
	return 1;
}
	

void hs_Config()
{
	char *ban;
	char *host;
	char *host2;
	hnode_t *hn;
	char *ban2;
	int hostlen;

	SET_SEGV_LOCATION();
	hs_cfg.old = 60;
	GetConf((void *) &hs_cfg.view, CFGINT, "ViewLevel");
	if ((hs_cfg.view > NS_ULEVEL_ROOT) || (hs_cfg.list <= 0))
		hs_cfg.view = 100;
	GetConf((void *) &hs_cfg.add, CFGINT, "AddLevel");
	if ((hs_cfg.add > NS_ULEVEL_ROOT) || (hs_cfg.view <= 0))
		hs_cfg.add = 40;
	GetConf((void *) &hs_cfg.del, CFGINT, "DelLevel");
	if ((hs_cfg.del > NS_ULEVEL_ROOT) || (hs_cfg.del <= 0))
		hs_cfg.del = 40;
	GetConf((void *) &hs_cfg.list, CFGINT, "ListLevel");
	if ((hs_cfg.list > NS_ULEVEL_ROOT) || (hs_cfg.list <= 0))
		hs_cfg.list = 40;
	GetConf((void *) &hs_cfg.old, CFGINT, "ExpireDays");
	if (GetConf((void *) &hs_cfg.regnick, CFGINT, "UnetVhosts") > 0) {
		if (GetConf((void *) &ban, CFGSTR, "UnetDomain") > 0) {
			strlcpy(hs_cfg.vhostdom, ban, MAXHOST);
		} else {
			hs_cfg.vhostdom[0] = '\0';
		}
	} else {
		hs_cfg.regnick = 0;
		hs_cfg.vhostdom[0] = '\0';
	}
		
	/* banned vhosts */
	ban = NULL;
	GetConf((void *) &ban, CFGSTR, "BannedVhosts");
	ban2 = ban;
	if (ban) {
		host = strtok(ban, ";");
		while (host != NULL) {
			/* limit host to MAXHOST and avoid unterminated/huge strings */
			hostlen = strnlen(host, MAXHOST);
			/* If less than MAXHOST allocate an extra byte for NULL */
			if(hostlen < MAXHOST)
				hostlen++;
			host2 = malloc(hostlen);
			strlcpy(host2, host, hostlen);
			hn = hnode_create(host2);
			hash_insert(bannedvhosts, hn, host2);
			host = strtok(NULL, ";");
		}
	}
	free(ban2);
}

int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(RPL_VERSION, origin,
		     "Module HostServ Loaded, Version: %s %s %s",
			 __module_info.module_version, __module_info.module_build_date,
			 __module_info.module_build_time);
	return 0;
}


/* Routine For Setting the Virtual Host */
static int hs_sign_on(char **av, int ac)
{
	lnode_t *hn;
	hs_map *map;
	User *u;

	SET_SEGV_LOCATION();

	if (!is_synced)
		return 0;
	u = finduser(av[0]);
	if (!u)
		return 1;

	if (u->server->name == me.name)
		return 1;

	if (findbot(u->nick))
		return 1;
	if (!load_synch)
		return 1;

	/* Check HostName Against Data Contained in vhosts.data */

	hn = list_find(vhosts, u->nick, findnick);
	if (hn) {
		map = lnode_get(hn);
		nlog(LOG_DEBUG1, LOG_MOD, "Checking %s against %s for HostName Match", map->host, u->hostname);
		if (fnmatch(map->host, u->hostname, 0) == 0) {
			ssvshost_cmd(u->nick, map->vhost);
			prefmsg(u->nick, s_HostServ,
				"Automatically setting your Virtual Host to %s",
				map->vhost);
			map->lused = me.now;
			save_vhost(map);
#ifdef UMODE_REGNICK
			set_moddata(u);
#endif
			return 1;
		}
	}
	return 1;
}

Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1}
	,
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1}
	,
#endif
	{NULL, NULL, 0}
};

int __BotMessage(char *origin, char **av, int ac)
{
	int t = 0;
	User *u;
	u = finduser(origin);
	if (!u) /* User not found */
		return 1;

	if (!strcasecmp(av[1], "HELP")) {
		if (ac <= 2) {
			privmsg_list(u->nick, s_HostServ, hs_help);
			if (UserLevel(u) >= NS_ULEVEL_OPER)
				privmsg_list(u->nick, s_HostServ, hs_user_help);
			privmsg_list(u->nick, s_HostServ, hs_help_on_help);
			return 1;
		} else if (!strcasecmp(av[2], "ADD")
			   && (UserLevel(u) >= hs_cfg.add)) {
			privmsg_list(u->nick, s_HostServ, hs_help_add);
			return 1;
		} else if (!strcasecmp(av[2], "DEL")
			   && (UserLevel(u) >= hs_cfg.del)) {
			privmsg_list(u->nick, s_HostServ, hs_help_del);
			return 1;
		} else if (!strcasecmp(av[2], "LIST")
			   && (UserLevel(u) >= hs_cfg.list)) {
			privmsg_list(u->nick, s_HostServ, hs_help_list);
			return 1;
		} else if (!strcasecmp(av[2], "VIEW")
			   && (UserLevel(u) >= hs_cfg.view)) {
			privmsg_list(u->nick, s_HostServ, hs_help_view);
			return 1;
		} else if (!strcasecmp(av[2], "LEVELS")
			   && (UserLevel(u) >= NS_ULEVEL_OPER)) {
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
		} else if (!strcasecmp(av[2], "SET") && (UserLevel(u) >= NS_ULEVEL_ADMIN)) {
			privmsg_list(u->nick, s_HostServ, hs_help_set);
			return 1;
		} else
			prefmsg(u->nick, s_HostServ,
				"Unknown Help Topic: \2%s\2", av[2]);
				return 1;
	}

	if (!strcasecmp(av[1], "ABOUT")) {
		privmsg_list(u->nick, s_HostServ, hs_help_about);
		return 1;
	} else if (!strcasecmp(av[1], "ADD")
		   && (UserLevel(u) >= hs_cfg.add)) {
		if (ac < 6) {
			prefmsg(u->nick, s_HostServ,
				"Syntax: /msg %s ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME> <PASSWORD>",
				s_HostServ);
			prefmsg(u->nick, s_HostServ,
				"For additional help: /msg %s HELP",
				s_HostServ);
			return -1;
		}
		hs_add(u, av[2], av[3], av[4], av[5]);
	} else if (!strcasecmp(av[1], "DEL")
		   && (UserLevel(u) >= hs_cfg.del)) {
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
		   && (UserLevel(u) >= hs_cfg.list)) {
		if (ac == 2) {
			hs_list(u, 0);
		} else if (ac == 3) {
			hs_list(u, atoi(av[2]));
		}
		return 1;
	} else if (!strcasecmp(av[1], "BANS")) {
		if (ac == 2) {
			hs_listban(u);
			return 1;
		} else if (ac == 4) {
			if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
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
					  "%s tried to %s bans, but was not authorized",
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
				hs_cfg.add, hs_cfg.del, hs_cfg.list,
				hs_cfg.view);
			return 1;
		} else if (ac == 3) {
			if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
				if (!strcasecmp(av[2], "RESET")) {
					hs_cfg.add = 40;
					SetConf((void *) t, CFGINT, "AddLevel");
					hs_cfg.del = 40;
					SetConf((void *) t, CFGINT, "DelLevel");
					hs_cfg.list = 40;
					SetConf((void *) t, CFGINT, "ListLevel");
					hs_cfg.view = 100;
					SetConf((void *) t, CFGINT, "ViewLevel");
				}
			}
			prefmsg(u->nick, s_HostServ, "Permission Denied");
		} else if (ac == 4) {
			if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
				t = atoi(av[3]);
				if ((t <= 0) || (t > NS_ULEVEL_ROOT)) {
					prefmsg(u->nick, s_HostServ,
						"Invalid Level. Must be between 1 and %d", NS_ULEVEL_ROOT);
					return -1;
				}
				if (!strcasecmp(av[2], "ADD")) {
					hs_cfg.add = t;
					SetConf((void *) t, CFGINT,
						"AddLevel");
				} else if (!strcasecmp(av[2], "DEL")) {
					hs_cfg.del = t;
					SetConf((void *) t, CFGINT,
						"DelLevel");
				} else if (!strcasecmp(av[2], "LIST")) {
					hs_cfg.list = t;
					SetConf((void *) t, CFGINT,
						"ListLevel");
				} else if (!strcasecmp(av[2], "VIEW")) {
					hs_cfg.view = t;
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
		   && (UserLevel(u) >= hs_cfg.view)) {
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
				"For additional help: /msg %s HELP LOGIN",
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
	} else if (!strcasecmp(av[1], "SET")) {
		if (UserLevel(u) < NS_ULEVEL_ADMIN) {
			prefmsg(u->nick, s_HostServ, "Permission Denied");
			chanalert(s_HostServ, "%s tried set, but permission was denied", u->nick);
			return -1;
		}
		hs_set(u, av, ac);
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
static void hs_set(User * u, char **av, int ac)
{
	int i;
	if (ac <= 2) {
		prefmsg(u->nick, s_HostServ, "Current Settings:");
		prefmsg(u->nick, s_HostServ, "Expire Time: %d (Days)", hs_cfg.old);
		prefmsg(u->nick, s_HostServ, "Undernet Style Hidden Hosts: %s", hs_cfg.regnick ? hs_cfg.vhostdom : "Disabled");
		prefmsg(u->nick, s_HostServ, "End of List.");
		return;
	}
	if (ac < 4) {
		prefmsg(u->nick, s_HostServ,
			"Syntax: /msg %s SET <option> <value>",
			s_HostServ);
		prefmsg(u->nick, s_HostServ,
			"For additional help: /msg %s HELP SET",
			s_HostServ);
		return;
	}
	if (!strcasecmp(av[2], "EXPIRE")) {
		i = atoi(av[3]);	
		if ((i <= 0) || (i > 100)) {
			prefmsg(u->nick, s_HostServ, "Value out of Range.");
			return;
		}
		/* if we get here, all is ok */
		hs_cfg.old = i;
		prefmsg(u->nick, s_HostServ, "Expire Time is is set to %d (days)", i);
		chanalert(s_HostServ, "%s Set Expire Time to %d", u->nick, i);
		SetConf((void *)i, CFGINT, "ExpireDays");
		return;
	} else if (!strcasecmp(av[2], "HIDDENHOST")) {
		if (!strcasecmp(av[3], "off")) {
			hs_cfg.regnick = 0;
			SetConf((void *)0, CFGINT, "UnetVhosts");
			prefmsg(u->nick, s_HostServ, "Undernet Style Hidden Hosts disabled for Registered Users");
			chanalert(s_HostServ, "%s disabled Undernet Style Hidden Hosts");
			return;
		} else {
			hs_cfg.regnick = 1;
			strlcpy(hs_cfg.vhostdom, av[3], MAXHOST);
			SetConf((void *)1, CFGINT, "UnetVhosts");
			SetConf((void *)av[3], CFGSTR, "UnetDomain");
			prefmsg(u->nick, s_HostServ, "Undernet Style Hidden Hosts enabled for Registered Users");
			chanalert(s_HostServ, "%s enabled Undernet Style Hidden Hosts with domain %s", u->nick, av[3]);
			return;
		}
	} else {
		prefmsg(u->nick, s_HostServ, "Unknown Option %s. Prehaps you need help?", av[2]);
		return;
	}			
}
int Online(char **av, int ac)
{
	char *user = NULL;
	char *host = NULL;
	char *rname = NULL;

	if (GetConf((void *) &s_HostServ, CFGSTR, "Nick") < 0) {
/*		s_HostServ = malloc(MAXNICK); */
		strlcpy(s_HostServ, "HostServ", MAXNICK);
	}
	if (GetConf((void *) &user, CFGSTR, "User") < 0) {
		user = malloc(MAXUSER);
		strlcpy(user, "HS", MAXUSER);
	}
	if (GetConf((void *) &host, CFGSTR, "Host") < 0) {
		host = malloc(MAXHOST);
		strlcpy(host, me.name, MAXHOST);
	}
	if (GetConf((void *) &rname, CFGSTR, "RealName") < 0) {
		rname = malloc(MAXREALNAME);
		strlcpy(rname, "Network Virtual Host Service", MAXREALNAME);
	}


	if (init_bot
	    (s_HostServ, user, host, rname, services_bot_modes,
	     __module_info.module_name) == -1) {
		/* Nick was in use */
		strlcat(s_HostServ, "_", MAXNICK);
		init_bot(s_HostServ, user, host, rname, services_bot_modes,
			 __module_info.module_name);
	}
	if(user)
		 free(user);
	if(host)
		free(host);
	if(rname)
		free(rname);
	chanalert(s_HostServ,
		  "Configured Levels: Add: %d, Del: %d, List: %d, View: %d",
		  hs_cfg.add, hs_cfg.del, hs_cfg.list, hs_cfg.view);

	add_mod_timer("CleanupHosts", "Cleanup_Old_Vhosts",
		      __module_info.module_name, 7200);
	Loadhosts();

	return 1;
};

EventFnList __module_events[] = {
	{EVENT_ONLINE, Online}
	,
	{EVENT_SIGNON, hs_sign_on}
	,
#ifdef UMODE_REGNICK
	{EVENT_UMODE, hs_mode}
	, 
	{EVENT_SIGNOFF, del_moddata}
	,
	{EVENT_KILL, del_moddata}
	,
#endif
	{NULL, NULL}
};

int __ModInit(int modnum, int apiver)
{
	vhosts = list_create(-1);
	bannedvhosts = hash_create(-1, 0, 0);
	if (!vhosts) {
		nlog(LOG_CRITICAL, LOG_CORE,
		     "Error, Can't create vhosts hash");
		chanalert(s_Services, "Error, Can't create Vhosts Hash");
		return -1;
	}
	hs_cfg.modnum = modnum;
	hs_Config();
	return 1;
}

void __ModFini()
{
	lnode_t *hn;
	hn = list_first(vhosts);
	while (hn != NULL) {
		free(lnode_get(hn));
		hn = list_next(vhosts, hn);
	}
	list_destroy_nodes(vhosts);

};
int hs_mode(char **av, int ac) {
#if UMODE_REGNICK
	User *u;
	char vhost[MAXHOST];
	/* bail out if its not enabled */
	if (hs_cfg.regnick != 1) 
		return -1;

	/* bail if we are not synced */
	if (!is_synced || !load_synch)
		return 0;
		
	/* first, find if its a regnick mode */
	if (index(av[1], 'r')) {
		u = finduser(av[0]);
		if (!u) /* User not found */
			return 1;
		if (is_oper(u)) 
			/* don't set a opers vhost. Most likely already done */
			return 1;
			
		if (u->moddata[hs_cfg.modnum] != NULL) {
			nlog(LOG_DEBUG2, LOG_MOD, "not setting hidden host on %s", av[0]);
			return -1;
		}
		nlog(LOG_DEBUG2, LOG_MOD, "Regnick Mode on %s", av[0]);
		ircsnprintf(vhost, MAXHOST, "%s.%s", av[0], hs_cfg.vhostdom);
		ssvshost_cmd(av[0], vhost);
		prefmsg(av[0], s_HostServ,
			"Automatically setting your Hidden Host to %s", vhost);
		set_moddata(u);
	}
#endif
	return 1;
}

/* Routine for registrations with the 'vhosts.db' file */
void hsdat(char *nick, char *host, char *vhost, char *pass, char *who)
{
	lnode_t *hn;
	hs_map *map;

	map = malloc(sizeof(hs_map));
	strlcpy(map->nnick, nick, MAXNICK);
	strlcpy(map->host, host, MAXHOST);
	strlcpy(map->vhost, vhost, MAXHOST);
	strlcpy(map->passwd, pass, MAXPASSWORD);
	strlcpy(map->added, who, MAXNICK);
	map->lused = me.now;
	hn = lnode_create(map);
	list_append(vhosts, hn);
	save_vhost(map);
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
	int hostlen;

	/* limit host to MAXHOST and avoid unterminated/huge strings */
	hostlen = strnlen(ban, MAXHOST);
	/* If less than MAXHOST allocate an extra byte for NULL */
	if(hostlen < MAXHOST)
		hostlen++;

	host = malloc(hostlen);
	strlcpy(host, ban, hostlen);
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

	bzero(ban_buf, BANBUFSIZE);
	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		strlcat(ban_buf, (char *) hnode_get(hn), BANBUFSIZE);
		strlcat(ban_buf, ";", BANBUFSIZE);
	}
	SetConf((void *) ban_buf, CFGSTR, "BannedVhosts");
}


static void hs_chpass(User * u, char *nick, char *oldpass, char *newpass)
{
	lnode_t *hn;
	hs_map *map;

	hn = list_find(vhosts, nick, findnick);
	if (hn) {
		map = lnode_get(hn);
		if ((fnmatch(map->host, u->hostname, 0) == 0)
		    || (UserLevel(u) >= 100)) {
			if (!strcasecmp(map->passwd, oldpass)) {
				strlcpy(map->passwd, newpass, MAXPASSWORD);
				prefmsg(u->nick, s_HostServ,
					"Password Successfully changed");
				chanalert(s_HostServ,
					  "%s changed the password for %s",
					  u->nick, map->nnick);
				map->lused = me.now;
				save_vhost(map);
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

	SET_SEGV_LOCATION();
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


	if (!list_find(vhosts, cmd, findnick)) {
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
					"Your Vhost has been created with Real HostMask of %s and username %s with password %s",
					m, cmd, p);
				prefmsg(cmd, s_HostServ,
					"For security, you should change your vhost password. See /msg %s help chpass",
					s_HostServ);
#ifdef UMODE_REGNICK
				set_moddata(nu);
#endif
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
static void hs_list(User * u, int start)
{
	int i;
	lnode_t *hn;
	hs_map *map;

	SET_SEGV_LOCATION();

	if (start >= list_count(vhosts)) {
		prefmsg(u->nick, s_HostServ,  "Value out of Range. There are only %d entries", list_count(vhosts));
		return;
	}
		
	i = 1;
	prefmsg(u->nick, s_HostServ, "Current HostServ VHOST list: ");
	prefmsg(u->nick, s_HostServ, "Showing %d to %d entries of %d total Vhosts", start+1, start+20, list_count(vhosts));
	prefmsg(u->nick, s_HostServ, "%-5s %-12s %-30s", "Num", "Nick",
		"Vhost");
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i <= start) {
			i++;
			hn = list_next(vhosts, hn);
			continue;
		}
		map = lnode_get(hn);
		prefmsg(u->nick, s_HostServ, "%-5d %-12s %-30s", i,
			map->nnick, map->vhost);
		i++;
		/* limit to x entries per screen */
		if (i > start + 20) 
			break;
		hn = list_next(vhosts, hn);
	}
	prefmsg(u->nick, s_HostServ,
		"For more information on someone use /msg %s VIEW #",
		s_HostServ);
	prefmsg(u->nick, s_HostServ, "--- End of List ---");
	if (list_count(vhosts) >= i) 
	prefmsg(u->nick, s_HostServ, "Type \2/msg %s list %d\2 to see next page", s_HostServ, i-1);
}

/* Routine for VIEW */
static void hs_view(User * u, int tmpint)
{
	int i;
	lnode_t *hn;
	hs_map *map;
	char ltime[80];
	SET_SEGV_LOCATION();

	i = 1;
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (tmpint == i) {
			map = lnode_get(hn);
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
		hn = list_next(vhosts, hn);
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
	hs_map *map;
	lnode_t *hn;
	char buf[BUFSIZE];
	char *tmp;
	int count;
	char **LoadArry;
	FILE *fp;
	fp = fopen("data/vhosts.db", "r");

	/* if fp is valid, means we need to upgrade the database */
	if (fp) {
		load_synch = 1;
		while (fgets(buf, BUFSIZE, fp)) {
			strip(buf);
			LoadArryCount = split_buf(buf, &LoadArry, 0);
			if (!list_find(vhosts, LoadArry[0], findnick)) {
				map = malloc(sizeof(hs_map));
				strlcpy(map->nnick, LoadArry[0], MAXNICK);
				strlcpy(map->host, LoadArry[1], MAXHOST);
				strlcpy(map->vhost, LoadArry[2], MAXHOST);
				if (LoadArryCount > 3) {	/* Check for upgrades from earlier versions */
					strlcpy(map->passwd, LoadArry[3],
						MAXPASSWORD);
				} else	/* Upgrading from earlier version, no passwds exist */
					strlcpy(map->passwd, NULL, MAXPASSWORD);
				if (LoadArryCount > 4) {	/* Does who set it exist? Yes? go ahead */
					strlcpy(map->added, LoadArry[4],
						MAXNICK);
				} else	/* We have no information on who set it so its null */
					map->added[0] = 0;
				if (LoadArryCount > 5) {
					map->lused = atoi(LoadArry[5]);
				} else
					map->lused = me.now;
				/* add it to the hash */
				hn = lnode_create(map);
				list_append(vhosts, hn);
				save_vhost(map);
				nlog(LOG_DEBUG1, LOG_CORE,
				     "Upgraded Database Entry %s (%s) into Vhosts",
				     map->nnick, map->vhost);
			} else {
				nlog(LOG_NOTICE, LOG_CORE,
				     "HostServ: db entry for %s already exists",
				     LoadArry[0]);
			}
			free(LoadArry);
		}
		fclose(fp);
		unlink("data/vhosts.db");
	} else if (GetTableData("Vhosts", &LoadArry) > 0) {
		load_synch = 1;
		for (count = 0; LoadArry[count] != NULL; count++) {
			map = malloc(sizeof(hs_map));
			strlcpy(map->nnick, LoadArry[count], MAXNICK);
			if (GetData((void *)&tmp, CFGSTR, "Vhosts", map->nnick, "Host") > 0) 
				strlcpy(map->host, tmp, MAXHOST);
			if (GetData((void *)&tmp, CFGSTR, "Vhosts", map->nnick, "Vhost") > 0) 
				strlcpy(map->vhost, tmp, MAXHOST);
			if (GetData((void *)&tmp, CFGSTR, "Vhosts", map->nnick, "Passwd") > 0) 
				strlcpy(map->passwd, tmp, MAXPASSWORD);
			if (GetData((void *)&tmp, CFGSTR, "Vhosts", map->nnick, "Added") > 0)
				strlcpy(map->added, tmp, MAXNICK);
			GetData((void *)&map->lused, CFGINT, "Vhosts", map->nnick, "LastUsed");
			hn = lnode_create(map);
			list_append(vhosts, hn);
			nlog(LOG_DEBUG1, LOG_CORE,
			     "Loaded %s (%s) into Vhosts",
			     map->nnick, map->vhost);
		}
	}			
	list_sort(vhosts, findnick);
}


/* Routine for HostServ to delete the given information */
static void hs_del(User * u, int tmpint)
{

	int i = 1;
	lnode_t *hn;
	hs_map *map;

	SET_SEGV_LOCATION();

	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i == tmpint) {
			map = lnode_get(hn);
			prefmsg(u->nick, s_HostServ,
				"The following vhost was removed from the Vhosts Database");
			prefmsg(u->nick, s_HostServ, "\2%s - %s\2",
				map->nnick, map->vhost);
			nlog(LOG_NOTICE, LOG_MOD,
			     "%s removed the VHOST: %s for %s", u->nick,
			     map->vhost, map->nnick);
			chanalert(s_HostServ, "%s removed vhost %s for %s",
				  u->nick, map->vhost, map->nnick);
			del_vhost(map);
			list_delete(vhosts, hn);
			lnode_destroy(hn);
			return;
		}
		hn = list_next(vhosts, hn);
		i++;
	}

	if (tmpint > i)
		prefmsg(u->nick, s_HostServ,
			"ERROR: There is no vhost on list number \2%d\2",
			tmpint);
	return;
}


/* Routine to allow users to login and get their vhosts */
static void hs_login(User * u, char *login, char *pass)
{
	hs_map *map;
	lnode_t *hn;

	SET_SEGV_LOCATION();

	/* Check HostName Against Data Contained in vhosts.data */
	hn = list_find(vhosts, login, findnick);
	if (hn) {
		map = lnode_get(hn);
		if (!strcasecmp(map->passwd, pass)) {
			ssvshost_cmd(u->nick, map->vhost);
			map->lused = me.now;
			prefmsg(u->nick, s_HostServ,
				"Your VHOST %s has been set.", map->vhost);
			nlog(LOG_NORMAL, LOG_MOD,
			     "%s used LOGIN to obtain userhost of %s",
			     u->nick, map->vhost);
			chanalert(s_HostServ,
				  "%s used login to get Vhost %s", u->nick,
				  map->vhost);
#ifdef UMODE_REGNICK
			set_moddata(u);
#endif
			save_vhost(map);
			return;
		}
	}
	prefmsg(u->nick, s_HostServ,
		"Incorrect Login or Password.  Do you have a vhost added?");
	return;
}

void CleanupHosts()
{
	lnode_t *hn, *hn2;
	hs_map *map;

	SET_SEGV_LOCATION();

	hn = list_first(vhosts);
	while (hn != NULL) {
		map = lnode_get(hn);
		if (map->lused < (me.now - (hs_cfg.old * 86400))) {
			nlog(LOG_NOTICE, LOG_MOD,
			     "Old Vhost Automatically removed: %s for %s",
			     map->vhost, map->nnick);
			del_vhost(map);
			hn2 = list_next(vhosts, hn);
			list_delete(vhosts, hn);
			lnode_destroy(hn);
			hn = hn2;
		} else {
			hn = list_next(vhosts, hn);
		}
	}
}
