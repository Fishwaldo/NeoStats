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
#ifndef GOTSVSHOST 
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
	char user[MAXUSER];
	char host[MAXHOST];
	char rname[MAXREALNAME];
} hs_cfg;

#ifdef UMODE_REGNICK
typedef struct hs_user {
	int vhostset;
	hs_map *vhe;
} hs_user;
#endif

static int hs_sign_on(char **av, int ac);
#if UMODE_REGNICK
static int hs_mode(char **av, int ac);
#endif
static int hs_about(User * u, char **av, int ac);
static int hs_levels(User * u, char **av, int ac);
static int hs_bans(User * u, char **av, int ac);
static int hs_login(User * u, char **av, int ac);
static int hs_chpass(User * u, char **av, int ac);
#if 0
static int hs_set(User * u, char **av, int ac);
#endif
static int hs_add(User * u, char **av, int ac);
static int hs_list(User * u, char **av, int ac);
static int hs_view(User * u, char **av, int ac);
static int hs_del(User * u, char **av, int ac);

static void hs_listban(User * u);
static void hs_addban(User * u, char *ban);
static void hs_delban(User * u, char *ban);

static void SaveBans(void);
static void hsdat(char *nick, char *host, char *vhost, char *pass, char *who);
void CleanupHosts(void);
static void LoadHosts(void);
static void LoadConfig(void);

int data_synch;
int load_synch;

char **DelArry;
char **ListArry;
int LoadArryCount = 0;
int DelArryCount = 0;
int ListArryCount = 0;

ModUser *hs_bot;

ModuleInfo __module_info = {
	 "HostServ",
	 "Network User Virtual Host Service",
	 "3.2",
	__DATE__,
	__TIME__
};

static bot_cmd hs_commands[]=
{
	{"ABOUT",	hs_about,	0, 	0,					hs_help_about,	hs_help_about_oneline },
	{"ADD",		hs_add,		4,	(int)&hs_cfg.add,	hs_help_add,	hs_help_add_oneline },
	{"DEL",		hs_del,		1, 	(int)&hs_cfg.del,	hs_help_del,	hs_help_del_oneline },
	{"LIST",	hs_list,	0, 	(int)&hs_cfg.list,	hs_help_list,	hs_help_list_oneline },
	{"BANS",	hs_bans,	0,  NS_ULEVEL_ADMIN,	hs_help_bans,	hs_help_bans_oneline },
	{"LEVELS",	hs_levels,	0, 	NS_ULEVEL_OPER,		hs_help_levels,	hs_help_levels_oneline },
	{"VIEW",	hs_view,	1, 	(int)&hs_cfg.view,	hs_help_view,	hs_help_view_oneline },
	{"LOGIN",	hs_login,	2, 	0,					hs_help_login,	hs_help_login_oneline },
	{"CHPASS",	hs_chpass,	3, 	0,					hs_help_chpass,	hs_help_chpass_oneline },
	{NULL,		NULL,		0, 	0,					NULL, 			NULL}
};

static bot_setting hs_settings[]=
{
	{"EXPIRE",		&hs_cfg.old,		SET_TYPE_INT,		0, 99, 	NS_ULEVEL_ADMIN,	"ExpireDays",	"days",	hs_help_set_expire	},
	{"HIDDENHOST",	&hs_cfg.regnick,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN,	"UnetVhosts",	NULL,	hs_help_set_hiddenhost	},
	{"HOSTNAME",	&hs_cfg.vhostdom,	SET_TYPE_STRING,	0, MAXHOST, 	NS_ULEVEL_ADMIN,	"UnetDomain",	NULL,	hs_help_set_hostname	},
	{NULL,			NULL,				0,					0, 0, 	0,	NULL,			NULL,	NULL	},
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

static int Online(char **av, int ac)
{
	hs_bot = init_mod_bot(s_HostServ, hs_cfg.user, hs_cfg.host, hs_cfg.rname, 
		services_bot_modes, 0, hs_commands, hs_settings, __module_info.module_name);
	add_mod_timer("CleanupHosts", "Cleanup_Old_Vhosts",
		      __module_info.module_name, 7200);
	LoadHosts();
	return 1;
};

EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{EVENT_SIGNON, hs_sign_on},
#ifdef UMODE_REGNICK
	{EVENT_UMODE, hs_mode}, 
	{EVENT_SIGNOFF, del_moddata},
	{EVENT_KILL, del_moddata},
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
	LoadConfig();
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
#if UMODE_REGNICK
int hs_mode(char **av, int ac) {
	int add = 0;
	char *modes;
	User *u;
	char vhost[MAXHOST];
	/* bail out if its not enabled */
	if (hs_cfg.regnick != 1) 
		return -1;

	/* bail if we are not synced */
	if (!is_synced || !load_synch)
		return 0;
		
	u = finduser(av[0]);
	if (!u) /* User not found */
		return 1;
	if (is_oper(u)) 
		/* don't set a opers vhost. Most likely already done */
		return 1;
	/* first, find if its a regnick mode */
	modes = av[1];
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		case 'r':
			if (add) {
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
			break;
		default:
			break;
		}
		modes++;
	}
	return 1;
}
#endif

/* Routine for registrations with the 'vhosts.db' file */
static void hsdat(char *nick, char *host, char *vhost, char *pass, char *who)
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

static int hs_about(User * u, char **av, int ac)
{
	privmsg_list(u->nick, s_HostServ, hs_help_about);
	return 1;
}

static int hs_levels(User * u, char **av, int ac)
{
	int t;	
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
				SetConf((void *) &hs_cfg.add, CFGINT, "AddLevel");
				hs_cfg.del = 40;
				SetConf((void *) &hs_cfg.del, CFGINT, "DelLevel");
				hs_cfg.list = 40;
				SetConf((void *) &hs_cfg.list, CFGINT, "ListLevel");
				hs_cfg.view = 100;
				SetConf((void *) &hs_cfg.view, CFGINT, "ViewLevel");
			}
		}
		prefmsg(u->nick, s_HostServ, "Permission Denied");
	} else if (ac == 4) {
		if (UserLevel(u) >= NS_ULEVEL_ADMIN) {
			t = atoi(av[3]);
			if ((t <= 0) || (t > NS_ULEVEL_ROOT)) {
				prefmsg(u->nick, s_HostServ,
					"Invalid Level. Must be between 1 and %d", NS_ULEVEL_ROOT);
				return 1;
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
}

static int hs_bans(User * u, char **av, int ac)
{
	if (ac == 2) {
		hs_listban(u);
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
	} else {
		/* if we get to here, bah, stupid lusers */
		prefmsg(u->nick, s_HostServ,
			"Syntax: /msg %s BANS [DEL/ADD] <options>",
			s_HostServ);
		prefmsg(u->nick, s_HostServ, "/msg %s help bans for more info", s_HostServ);
	}
	return 1;
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

	if (hash_lookup(bannedvhosts, ban) != NULL) {
		prefmsg(u->nick, s_HostServ,
			"%s already exists in the banned vhost list", ban);
		return;
	}
	host = malloc(MAXHOST);
	strlcpy(host, ban, MAXHOST);
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
				"Deleted %s from the banned vhost list",
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
	prefmsg(u->nick, s_HostServ, "Entry %d was not found (Weird?!?)", i);
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

static int hs_chpass(User * u, char **av, int ac)
{
	lnode_t *hn;
	hs_map *map;
	char *nick;
	char *oldpass;
	char *newpass;

	nick = av[2];
	oldpass = av[3];
	newpass = av[4];

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
				return 1;
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
			return 1;
		}
	}

	prefmsg(u->nick, s_HostServ,
		"Invalid Username or Password. Access Denied");
	chanalert(s_HostServ,
		  "%s tried to change the password for %s, but got it wrong",
		  u->nick, nick);
	return 1;

}

/* Routine for ADD */
static int hs_add(User * u, char **av, int ac)
{

	hnode_t *hn;
	hscan_t hs;
	User *nu;
	char *cmd;
	char *m;
	char *h; 
	char *p;

	cmd = av[2];
	m = av[3];
	h = av[4]; 
	p = av[5];

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
			return 1;
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
				return 1;
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
				return 1;
			}
		}
	} else {
		prefmsg(u->nick, s_HostServ,
			"%s already has a HostServ Entry", cmd);
	}
	return 1;
}


/* Routine for 'HostServ' to print out its data */
static int hs_list(User * u, char **av, int ac)
{
	int i;
	lnode_t *hn;
	hs_map *map;
	int start = 0;

	SET_SEGV_LOCATION();
	if (ac == 2) {
		start = 0;
	} else if (ac == 3) {
		start = atoi(av[2]);
	}

	if (start >= list_count(vhosts)) {
		prefmsg(u->nick, s_HostServ,  "Value out of Range. There are only %d entries", (int)list_count(vhosts));
		return 1;
	}
		
	i = 1;
	prefmsg(u->nick, s_HostServ, "Current HostServ VHOST list: ");
	prefmsg(u->nick, s_HostServ, "Showing %d to %d entries of %d total Vhosts", start+1, start+20, (int)list_count(vhosts));
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
	return 1;
}

/* Routine for VIEW */
static int hs_view(User * u, char **av, int ac)
{
	int i;
	lnode_t *hn;
	hs_map *map;
	char ltime[80];
	int tmpint;

	SET_SEGV_LOCATION();
	tmpint = atoi(av[2]);
	if (!tmpint) {
		prefmsg(u->nick, s_HostServ,
			"Syntax: /msg %s VIEW #", s_HostServ);
		prefmsg(u->nick, s_HostServ,
			"The users # is got from /msg %s LIST",
			s_HostServ);
		return 1;
	}
	i = 1;
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (tmpint == i) {
			map = lnode_get(hn);
			prefmsg(u->nick, s_HostServ, "Virtual Host information:");
			prefmsg(u->nick, s_HostServ, "Nick:     %s", map->nnick);
			prefmsg(u->nick, s_HostServ, "RealHost: %s", map->host);
			prefmsg(u->nick, s_HostServ, "V-host:   %s", map->vhost);
			prefmsg(u->nick, s_HostServ, "Password: %s", map->passwd);
			prefmsg(u->nick, s_HostServ, "Added by: %s",
				map->added ? map->added : "<unknown>");
			strftime(ltime, 80, "%d/%m/%Y[%H:%M]", localtime(&map->lused));
			prefmsg(u->nick, s_HostServ, "Last Used on %s", ltime);
			prefmsg(u->nick, s_HostServ, "--- End of information for %s ---",
				map->nnick);
		}
		hn = list_next(vhosts, hn);
		i++;
	}
	if (tmpint > i)
		prefmsg(u->nick, s_HostServ,
			"ERROR: There is no vhost on list number \2%d\2",
			tmpint);
	return 1;
}




/* Routine For Loading The Hosts into Array */
static void LoadHosts()
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
	free(LoadArry);
	list_sort(vhosts, findnick);
}


/* Routine for HostServ to delete the given information */
static int hs_del(User * u, char **av, int ac)
{
	int i = 1;
	lnode_t *hn;
	hs_map *map;
	int tmpint;

	SET_SEGV_LOCATION();
	tmpint = atoi(av[2]);
	if (!tmpint) {
		prefmsg(u->nick, s_HostServ,
			"Syntax: /msg %s DEL #", s_HostServ);
		prefmsg(u->nick, s_HostServ,
			"The users # is got from /msg %s LIST",
			s_HostServ);
		return 1;
	}

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
			return 1;
		}
		hn = list_next(vhosts, hn);
		i++;
	}

	if (tmpint > i)
		prefmsg(u->nick, s_HostServ,
			"ERROR: There is no vhost on list number \2%d\2",
			tmpint);
	return 1;
}


/* Routine to allow users to login and get their vhosts */
static int hs_login(User * u, char **av, int ac)
{
	hs_map *map;
	lnode_t *hn;
	char *login;
	char *pass;

	SET_SEGV_LOCATION();
	login = av[2];
	pass = av[3];
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
			return 1;
		}
	}
	prefmsg(u->nick, s_HostServ,
		"Incorrect Login or Password.  Do you have a vhost added?");
	return 1;
}

void CleanupHosts()
{
	lnode_t *hn, *hn2;
	hs_map *map;

	SET_SEGV_LOCATION();
	/* Use zero value to disable this feature */
	if(hs_cfg.old == 0)
		return;
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

/* 
 * Load HostServ Configuration file and set defaults if does not exist
 */
static void LoadConfig(void)
{
	char *temp = NULL;
	char *host;
	char *host2;
	hnode_t *hn;

	SET_SEGV_LOCATION();

	if (GetConf((void *) &temp, CFGSTR, "Nick") < 0) {
		strlcpy(s_HostServ, "HostServ", MAXNICK);
	}
	else {
		strlcpy(s_HostServ, temp, MAXNICK);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "User") < 0) {
		strlcpy(hs_cfg.user, "HS", MAXUSER);
	}
	else {
		strlcpy(hs_cfg.user, temp, MAXUSER);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "Host") < 0) {
		strlcpy(hs_cfg.host, me.name, MAXHOST);
	}
	else {
		strlcpy(hs_cfg.host, temp, MAXHOST);
		free(temp);
	}
	if(GetConf((void *) &temp, CFGSTR, "RealName") < 0) {
		strlcpy(hs_cfg.rname, "Network Virtual Host Service", MAXREALNAME);
	}
	else {
		strlcpy(hs_cfg.rname, temp, MAXREALNAME);
		free(temp);
	}

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
	if(GetConf((void *) &hs_cfg.old, CFGINT, "ExpireDays") < 0) {
		hs_cfg.old = 60;
	}
	GetConf((void *) &hs_cfg.regnick, CFGINT, "UnetVhosts");
	if (hs_cfg.regnick < 0) {
		 hs_cfg.regnick = 0;
	}
	if (GetConf((void *) &temp, CFGSTR, "UnetDomain") > 0) {
		strlcpy(hs_cfg.vhostdom, temp, MAXHOST);
		free(temp);
	} else {
		hs_cfg.vhostdom[0] = '\0';
	}
		
	/* banned vhosts */
	if (GetConf((void *) &temp, CFGSTR, "BannedVhosts") >= 0) {
		host = strtok(temp, ";");
		while (host != NULL) {
			/* limit host to MAXHOST and avoid unterminated/huge strings */
			host2 = malloc(MAXHOST);
			strlcpy(host2, host, MAXHOST);
			hn = hnode_create(host2);
			hash_insert(bannedvhosts, hn, host2);
			host = strtok(NULL, ";");
		}
		free(temp);
	}
}
