/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "neostats.h"
#include "hostserv.h"

/* hostserv doesn't work on Hybrid, Echo an error and exit the compile */
#ifndef GOTSVSHOST 
#error "Error: This IRCd doesn't support changing a users host. This module will not compile"
#endif

#define MAXPASSWORD	30

#define BANBUFSIZE	4096
static char ban_buf[BANBUFSIZE];

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

static int hs_event_signon(CmdParams* cmdparams);
#ifdef UMODE_REGNICK
static int hs_event_mode(CmdParams* cmdparams);
#endif
static int hs_levels(CmdParams* cmdparams);
static int hs_bans(CmdParams* cmdparams);
static int hs_login(CmdParams* cmdparams);
static int hs_chpass(CmdParams* cmdparams);
static int hs_add(CmdParams* cmdparams);
static int hs_list(CmdParams* cmdparams);
static int hs_view(CmdParams* cmdparams);
static int hs_del(CmdParams* cmdparams);

static void hs_listban(User* u);
static void hs_addban(User* u, char *ban);
static void hs_delban(User* u, char *ban);

static void SaveBans(void);
static void hsdat(char *nick, char *host, char *vhost, char *pass, char *who);
int CleanupHosts(void);
static void LoadHosts(void);
static void LoadConfig(void);

int data_synch;
int load_synch;

char **DelArry;
char **ListArry;
int LoadArryCount = 0;
int DelArryCount = 0;
int ListArryCount = 0;

Bot *hs_bot;
static BotInfo hs_botinfo = 
{
	"HostServ", 
	"HostServ", 
	"HS", 
	"", 
	"Network virtual host service",
};
static Module* hs_module;

ModuleInfo module_info = {
	"HostServ",
	"Network virtual host service",
	ns_copyright,
	hs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

static bot_cmd hs_commands[]=
{
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
	{"NICK",		&hs_botinfo.nick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick, NULL, (void*)"HostServ" },
	{"ALTNICK",		&hs_botinfo.altnick,SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "AltNick",	NULL,	ns_help_set_altnick, NULL, (void*)"HostServ" },
	{"USER",		&hs_botinfo.user,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user, NULL, (void*)"HS" },
	{"HOST",		&hs_botinfo.host,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host, NULL, (void*)"" },
	{"REALNAME",	&hs_botinfo.realname,SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname, NULL, (void*)"Network virtual host service" },
	{"EXPIRE",		&hs_cfg.old,		SET_TYPE_INT,		0, 99, 			NS_ULEVEL_ADMIN, "ExpireDays","days",hs_help_set_expire, NULL, (void*)60 },
	{"HIDDENHOST",	&hs_cfg.regnick,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "UnetVhosts",NULL,	hs_help_set_hiddenhost, NULL, (void*)0 },
	{"HOSTNAME",	&hs_cfg.vhostdom,	SET_TYPE_STRING,	0, MAXHOST, 	NS_ULEVEL_ADMIN, "UnetDomain",NULL,	hs_help_set_hostname, NULL, (void*)"" },
	{NULL,			NULL,				0,					0, 0, 			0,				 NULL,		  NULL,	NULL	},
};

int findnick(const void *key1, const void *key2)
{
	const hs_map *vhost = key1;
	return (ircstrcasecmp(vhost->nnick, (char *)key2));
}

void save_vhost(hs_map *vhost) 
{
	SetData((void *)vhost->host, CFGSTR, "Vhosts", vhost->nnick, "Host");
	SetData((void *)vhost->vhost, CFGSTR, "Vhosts", vhost->nnick , "Vhost");
	SetData((void *)vhost->passwd, CFGSTR, "Vhosts", vhost->nnick , "Passwd");
	SetData((void *)vhost->added, CFGSTR, "Vhosts", vhost->nnick , "Added");
	SetData((void *)vhost->lused, CFGINT, "Vhosts", vhost->nnick , "LastUsed");
	list_sort(vhosts, findnick);
}

void del_vhost(hs_map *vhost) 
{
	DelRow("Vhosts", vhost->nnick);
	sfree(vhost);
	/* no need to list sort here, because its already sorted */
}

void set_moddata(User* u) 
{
	hs_user *hs;

	if (!u->moddata[hs_module->modnum]) {
		hs = smalloc(sizeof(hs_user));
		hs->vhostset = 1;
		u->moddata[hs_module->modnum] = hs;
	}
}

static int hs_event_quit(CmdParams* cmdparams) 
{
	if (cmdparams->source.user->moddata[hs_module->modnum]) {
		dlog(DEBUG2, "hs_event_quit: free module data");
		sfree(cmdparams->source.user->moddata[hs_module->modnum]);
		cmdparams->source.user->moddata[hs_module->modnum] = NULL;
	}
	return 1;
}
	
/* Routine For Setting the Virtual Host */
static int hs_event_signon(CmdParams* cmdparams)
{
	lnode_t *hn;
	hs_map *map;

	SET_SEGV_LOCATION();

	if (!is_synced)
		return 0;

	if (IsMe(cmdparams->source.user)) 
		return 1;

	/* is this user excluded via a global exclusion? */
	if (IsExcluded(cmdparams->source.user)) 
		return 1;

	if (!load_synch)
		return 1;

	/* Check HostName Against Data Contained in vhosts.data */

	hn = list_find(vhosts, cmdparams->source.user->nick, findnick);
	if (hn) {
		map = lnode_get(hn);
		dlog(DEBUG1, "Checking %s against %s", map->host, cmdparams->source.user->hostname);
		if (fnmatch(map->host, cmdparams->source.user->hostname, 0) == 0) {
			ssvshost_cmd(cmdparams->source.user->nick, map->vhost);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick,
				"Automatically setting your Virtual Host to %s",
				map->vhost);
			map->lused = me.now;
			save_vhost(map);
#ifdef UMODE_REGNICK
			set_moddata(cmdparams->source.user);
#endif
			return 1;
		}
	}
	return 1;
}

static int hs_event_online(CmdParams* cmdparams)
{
	hs_bot = init_bot(&hs_botinfo, me.servicesumode, BOT_FLAG_DEAF, 
		hs_commands, hs_settings);
	add_timer (CleanupHosts, "CleanupHosts", 7200);
	LoadHosts();
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	hs_event_online},
	{EVENT_SIGNON,	hs_event_signon},
#ifdef UMODE_REGNICK
	{EVENT_UMODE,	hs_event_mode}, 
	{EVENT_QUIT,	hs_event_quit},
	{EVENT_KILL,	hs_event_quit},
#endif
	{EVENT_NULL,	NULL}
};

int ModInit(Module* mod_ptr)
{
	vhosts = list_create(-1);
	bannedvhosts = hash_create(-1, 0, 0);
	if (!vhosts) {
		nlog(LOG_CRITICAL, "Error, can't create vhosts hash");
		return -1;
	}
	ModuleConfig(hs_settings);
	LoadConfig();
	return 1;
}

void ModFini()
{
	lnode_t *hn;

	hn = list_first(vhosts);
	while (hn != NULL) {
		sfree(lnode_get(hn));
		hn = list_next(vhosts, hn);
	}
	list_destroy_nodes(vhosts);
}

#ifdef UMODE_REGNICK
int hs_event_mode(CmdParams* cmdparams) 
{
	int add = 0;
	char *modes;
	char vhost[MAXHOST];

	/* bail out if its not enabled */
	if (hs_cfg.regnick != 1) 
		return -1;

	/* bail if we are not synced */
	if (!is_synced || !load_synch)
		return 0;
		
	if (is_oper(cmdparams->source.user)) 
		/* don't set a opers vhost. Most likely already done */
		return 1;
		
	if (IsExcluded(cmdparams->source.user)) 
		return 1;
		
	/* first, find if its a regnick mode */
	modes = cmdparams->av[1];
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
				if (cmdparams->source.user->moddata[hs_module->modnum] != NULL) {
					dlog(DEBUG2, "not setting hidden host on %s", cmdparams->av[0]);
					return -1;
				}
				dlog(DEBUG2, "Regnick Mode on %s", cmdparams->av[0]);
				ircsnprintf(vhost, MAXHOST, "%s.%s", cmdparams->av[0], hs_cfg.vhostdom);
				ssvshost_cmd(cmdparams->av[0], vhost);
				prefmsg(cmdparams->av[0], hs_bot->nick,
					"Automatically setting your hidden host to %s", vhost);
				set_moddata(cmdparams->source.user);
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

	map = smalloc(sizeof(hs_map));
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

static int hs_levels(CmdParams* cmdparams)
{
	int t;	

	if (cmdparams->ac == 2) {
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"Configured Levels: ADD: %d, DEL: %d, LIST: %d, VIEW: %d",
			hs_cfg.add, hs_cfg.del, hs_cfg.list,hs_cfg.view);
			return NS_SUCCESS;
	} else if (cmdparams->ac == 3) {
		if (UserLevel(cmdparams->source.user) >= NS_ULEVEL_ADMIN) {
			if (!ircstrcasecmp(cmdparams->av[2], "RESET")) {
				hs_cfg.add = 40;
				SetConf((void *) &hs_cfg.add, CFGINT, "AddLevel");
				hs_cfg.del = 40;
				SetConf((void *) &hs_cfg.del, CFGINT, "DelLevel");
				hs_cfg.list = 40;
				SetConf((void *) &hs_cfg.list, CFGINT, "ListLevel");
				hs_cfg.view = 100;
				SetConf((void *) &hs_cfg.view, CFGINT, "ViewLevel");
				return NS_SUCCESS;
			}
		}
		return NS_ERR_NO_PERMISSION;
	} else if (cmdparams->ac == 4) {
		if (UserLevel(cmdparams->source.user) >= NS_ULEVEL_ADMIN) {
			t = atoi(cmdparams->av[3]);
			if ((t <= 0) || (t > NS_ULEVEL_ROOT)) {
				prefmsg(cmdparams->source.user->nick, hs_bot->nick,
					"Invalid Level. Must be between 1 and %d", NS_ULEVEL_ROOT);
				return NS_ERR_SYNTAX_ERROR;
			}
			if (!ircstrcasecmp(cmdparams->av[2], "ADD")) {
				hs_cfg.add = t;
				SetConf((void *) t, CFGINT,
					"AddLevel");
			} else if (!ircstrcasecmp(cmdparams->av[2], "DEL")) {
				hs_cfg.del = t;
				SetConf((void *) t, CFGINT,
					"DelLevel");
			} else if (!ircstrcasecmp(cmdparams->av[2], "LIST")) {
				hs_cfg.list = t;
				SetConf((void *) t, CFGINT,
					"ListLevel");
			} else if (!ircstrcasecmp(cmdparams->av[2], "VIEW")) {
				hs_cfg.view = t;
				SetConf((void *) t, CFGINT,
					"ViewLevel");
			} else {
				prefmsg(cmdparams->source.user->nick, hs_bot->nick,
					"Invalid Level. /msg %s help levels", hs_bot->nick);
				return 1;
			}
			prefmsg(cmdparams->source.user->nick, hs_bot->nick,
				"Level for %s set to %d", cmdparams->av[2], t);
			return NS_SUCCESS;
		}
		return NS_ERR_NO_PERMISSION;
	}
	return NS_ERR_SYNTAX_ERROR;
}

static int hs_bans(CmdParams* cmdparams)
{
	if (cmdparams->ac == 2) {
		hs_listban(cmdparams->source.user);
		return NS_SUCCESS;
	} else if (cmdparams->ac == 4) {
		if (UserLevel(cmdparams->source.user) >= NS_ULEVEL_ADMIN) {
			if (!ircstrcasecmp(cmdparams->av[2], "ADD")) {
				hs_addban(cmdparams->source.user, cmdparams->av[3]);
				return NS_SUCCESS;
			} else if (!ircstrcasecmp(cmdparams->av[2], "DEL")) {
				hs_delban(cmdparams->source.user, cmdparams->av[3]);
				return NS_SUCCESS;
			}
		} else {
			return NS_ERR_NO_PERMISSION;
		}
	}
	return NS_ERR_SYNTAX_ERROR;
}

static void hs_listban(User* u)
{
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	hash_scan_begin(&hs, bannedvhosts);
	prefmsg(u->nick, hs_bot->nick, "Banned vhosts");
	while ((hn = hash_scan_next(&hs)) != NULL) {
		prefmsg(u->nick, hs_bot->nick, "%d - %s", i, (char *)hnode_get(hn));
		i++;
	}
	prefmsg(u->nick, hs_bot->nick, "End of List.");

}
static void hs_addban(User* u, char *ban)
{
	hnode_t *hn;
	char *host;

	if (hash_lookup(bannedvhosts, ban) != NULL) {
		prefmsg(u->nick, hs_bot->nick,
			"%s already exists in the banned vhost list", ban);
		return;
	}
	host = smalloc(MAXHOST);
	strlcpy(host, ban, MAXHOST);
	hn = hnode_create(host);
	hash_insert(bannedvhosts, hn, host);
	prefmsg(u->nick, hs_bot->nick,
		"%s added to the banned vhosts list", ban);
	chanalert(hs_bot->nick, "%s added %s to the banned vhosts list",
		  u->nick, ban);
	SaveBans();
}

static void hs_delban(User* u, char *ban)
{
	hnode_t *hn;
	hscan_t hs;
	int i = 0;
	int j = 0;

	i = atoi(ban);
	if ((i < 1) || (i > hash_count(bannedvhosts))) {
		prefmsg(u->nick, hs_bot->nick,
			"Invalid Entry. /msg %s bans for the list",
			hs_bot->nick);
		return;
	}

	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		++j;
		if (i == j) {
			hash_scan_delete(bannedvhosts, hn);
			prefmsg(u->nick, hs_bot->nick,
				"Deleted %s from the banned vhost list",
				(char *) hnode_get(hn));
			chanalert(hs_bot->nick,
				  "%s deleted %s from the banned vhost list",
				  u->nick, (char *) hnode_get(hn));
			nlog(LOG_NOTICE,
			     "%s deleted %s from the banned vhost list",
			     u->nick, (char *) hnode_get(hn));
			sfree(hnode_get(hn));
			hnode_destroy(hn);
			SaveBans();
			return;
		}
	}
	prefmsg(u->nick, hs_bot->nick, "Entry %d was not found (Weird?!?)", i);
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

static int hs_chpass(CmdParams* cmdparams)
{
	lnode_t *hn;
	hs_map *map;
	char *nick;
	char *oldpass;
	char *newpass;

	nick = cmdparams->av[2];
	oldpass = cmdparams->av[3];
	newpass = cmdparams->av[4];

	hn = list_find(vhosts, nick, findnick);
	if (hn) {
		map = lnode_get(hn);
		if ((fnmatch(map->host, cmdparams->source.user->hostname, 0) == 0)
		    || (UserLevel(cmdparams->source.user) >= 100)) {
			if (!ircstrcasecmp(map->passwd, oldpass)) {
				strlcpy(map->passwd, newpass, MAXPASSWORD);
				prefmsg(cmdparams->source.user->nick, hs_bot->nick,
					"Password Successfully changed");
				chanalert(hs_bot->nick,
					  "%s changed the password for %s",
					  cmdparams->source.user->nick, map->nnick);
				map->lused = me.now;
				save_vhost(map);
				return 1;
			}
		} else {
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, 
				"Error, Hostname Mis-Match");
			chanalert(hs_bot->nick,
				  "%s tried to change the password for %s, but the hosts do not match (%s->%s)",
				  cmdparams->source.user->nick, map->nnick, cmdparams->source.user->hostname,
				  map->host);
			nlog(LOG_WARNING,
			     "%s tried to change the password for %s but the hosts do not match (%s -> %s)",
			     cmdparams->source.user->nick, map->nnick, cmdparams->source.user->hostname, map->host);
			return 1;
		}
	}

	prefmsg(cmdparams->source.user->nick, hs_bot->nick,
		"Invalid Username or Password. Access Denied");
	chanalert(hs_bot->nick,
		  "%s tried to change the password for %s, but got it wrong",
		  cmdparams->source.user->nick, nick);
	return 1;

}

/* Routine for ADD */
static int hs_add(CmdParams* cmdparams)
{

	hnode_t *hn;
	hscan_t hs;
	User *nu;
	char *cmd;
	char *m;
	char *h; 
	char *p;

	cmd = cmdparams->av[2];
	m = cmdparams->av[3];
	h = cmdparams->av[4]; 
	p = cmdparams->av[5];

	SET_SEGV_LOCATION();
	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		if (fnmatch(hnode_get(hn), h, 0) == 0) {
			prefmsg(cmdparams->source.user->nick, hs_bot->nick,
				"%s has been matched against the vhost ban %s",
				h, (char *) hnode_get(hn));
			chanalert(cmdparams->source.user->nick,
				  "%s tried to add a banned vhost %s",
				  cmdparams->source.user->nick, h);
			return 1;
		}
	}

	if(validate_host (h) == NS_FAILURE) {
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"%s is an invalid host", h);
		return 1;
	}

	if (!list_find(vhosts, cmd, findnick)) {
		hsdat(cmd, m, h, p, cmdparams->source.user->nick);
		nlog(LOG_NOTICE,
		     "%s added a vhost for %s with realhost %s vhost %s and password %s",
		     cmdparams->source.user->nick, cmd, m, h, p);
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"%s has sucessfully been registered under realhost: %s vhost: %s and password: %s",
			cmd, m, h, p);
		chanalert(hs_bot->nick,
			  "%s added a vhost %s for %s with realhost %s",
			  cmdparams->source.user->nick, h, cmd, m);
		/* Apply The New Hostname If The User Is Online */
		if ((nu = finduser(cmd)) != NULL) {
			if (IsMe(nu)) 
				return 1;
			if (fnmatch(m, nu->hostname, 0) == 0) {
				ssvshost_cmd(nu->nick, h);
				prefmsg(cmdparams->source.user->nick, hs_bot->nick,
					"%s is online now, setting vhost to %s",
					cmd, h);
				prefmsg(cmd, hs_bot->nick,
					"Your vhost has been created with hostmask of %s and username %s with password %s",
					m, cmd, p);
				prefmsg(cmd, hs_bot->nick,
					"For security, you should change your vhost password. See /msg %s help chpass",
					hs_bot->nick);
#ifdef UMODE_REGNICK
				set_moddata(nu);
#endif
				return 1;
			}
		}
	} else {
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"%s already has a vhost entry", cmd);
	}
	return 1;
}


/* Routine for 'HostServ' to print out its data */
static int hs_list(CmdParams* cmdparams)
{
	int i;
	lnode_t *hn;
	hs_map *map;
	int start = 0;

	SET_SEGV_LOCATION();
	if (cmdparams->ac == 2) {
		start = 0;
	} else if (cmdparams->ac == 3) {
		start = atoi(cmdparams->av[2]);
	}

	if (start >= list_count(vhosts)) {
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,  "Value out of range. There are only %d entries", (int)list_count(vhosts));
		return 1;
	}
		
	i = 1;
	prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Current vhost list: ");
	prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Showing %d to %d entries of %d vhosts", start+1, start+20, (int)list_count(vhosts));
	prefmsg(cmdparams->source.user->nick, hs_bot->nick, "%-5s %-12s %-30s", "Num", "Nick", "Vhost");
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i <= start) {
			i++;
			hn = list_next(vhosts, hn);
			continue;
		}
		map = lnode_get(hn);
		prefmsg(cmdparams->source.user->nick, hs_bot->nick, "%-5d %-12s %-30s", i,
			map->nnick, map->vhost);
		i++;
		/* limit to x entries per screen */
		if (i > start + 20) 
			break;
		hn = list_next(vhosts, hn);
	}
	prefmsg(cmdparams->source.user->nick, hs_bot->nick,
		"For more information on someone use /msg %s VIEW #",
		hs_bot->nick);
	prefmsg(cmdparams->source.user->nick, hs_bot->nick, "--- End of List ---");
	if (list_count(vhosts) >= i) 
	prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Type \2/msg %s list %d\2 to see next page", hs_bot->nick, i-1);
	return 1;
}

/* Routine for VIEW */
static int hs_view(CmdParams* cmdparams)
{
	int i;
	lnode_t *hn;
	hs_map *map;
	char ltime[80];
	int tmpint;

	SET_SEGV_LOCATION();
	tmpint = atoi(cmdparams->av[2]);
	if (!tmpint) {
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"Syntax: /msg %s VIEW #", hs_bot->nick);
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"The users # is got from /msg %s LIST",
			hs_bot->nick);
		return 1;
	}
	i = 1;
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (tmpint == i) {
			map = lnode_get(hn);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Virtual Host information:");
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Nick:      %s", map->nnick);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Real host: %s", map->host);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Vhost:     %s", map->vhost);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Password:  %s", map->passwd);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Added by:  %s",
				map->added ? map->added : "<unknown>");
			strftime(ltime, 80, "%d/%m/%Y[%H:%M]", localtime(&map->lused));
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "Last used: %s", ltime);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "--- End of information ---");
		}
		hn = list_next(vhosts, hn);
		i++;
	}
	if (tmpint > i)
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"ERROR: There is no vhost on list number \2%d\2",
			tmpint);
	return 1;
}




/* Routine For Loading The Hosts into Array */
static void LoadHosts()
{
	hs_map *map;
	lnode_t *hn;
	char *tmp;
	int count;
	char **LoadArry;

	if (GetTableData("Vhosts", &LoadArry) > 0) {
		load_synch = 1;
		for (count = 0; LoadArry[count] != NULL; count++) {
			map = smalloc(sizeof(hs_map));
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
			dlog(DEBUG1, "Loaded %s (%s) into Vhosts",
			     map->nnick, map->vhost);
		}
	}			
	sfree(LoadArry);
	list_sort(vhosts, findnick);
}


/* Routine for HostServ to delete the given information */
static int hs_del(CmdParams* cmdparams)
{
	int i = 1;
	lnode_t *hn;
	hs_map *map;
	int tmpint;

	SET_SEGV_LOCATION();
	tmpint = atoi(cmdparams->av[2]);
	if (!tmpint) {
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"Syntax: /msg %s DEL #", hs_bot->nick);
		prefmsg(cmdparams->source.user->nick, hs_bot->nick,
			"The users # is got from /msg %s LIST",
			hs_bot->nick);
		return 1;
	}

	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i == tmpint) {
			map = lnode_get(hn);
			prefmsg(cmdparams->source.user->nick, hs_bot->nick, "removed vhost %s for %s",
				map->nnick, map->vhost);
			nlog(LOG_NOTICE, "%s removed the vhost %s for %s", 
				 cmdparams->source.user->nick, map->vhost, map->nnick);
			chanalert(hs_bot->nick, "%s removed vhost %s for %s",
				  cmdparams->source.user->nick, map->vhost, map->nnick);
			del_vhost(map);
			list_delete(vhosts, hn);
			lnode_destroy(hn);
			return 1;
		}
		hn = list_next(vhosts, hn);
		i++;
	}

	if (tmpint > i)
		prefmsg(cmdparams->source.user->nick, hs_bot->nick, "ERROR: no vhost number \2%d\2",
			tmpint);
	return 1;
}


/* Routine to allow users to login and get their vhosts */
static int hs_login(CmdParams* cmdparams)
{
	hs_map *map;
	lnode_t *hn;
	char *login;
	char *pass;

	SET_SEGV_LOCATION();
	login = cmdparams->av[2];
	pass = cmdparams->av[3];
	/* Check HostName Against Data Contained in vhosts.data */
	hn = list_find(vhosts, login, findnick);
	if (hn) {
		map = lnode_get(hn);
		if (!ircstrcasecmp(map->passwd, pass)) {
			ssvshost_cmd(cmdparams->source.user->nick, map->vhost);
			map->lused = me.now;
			prefmsg(cmdparams->source.user->nick, hs_bot->nick,
				"Your vhost %s has been set.", map->vhost);
			nlog(LOG_NORMAL, "%s used LOGIN to obtain vhost of %s",
			    cmdparams->source.user->nick, map->vhost);
			chanalert(hs_bot->nick, "%s used login to get vhost %s", 
				cmdparams->source.user->nick, map->vhost);
#ifdef UMODE_REGNICK
			set_moddata(cmdparams->source.user);
#endif
			save_vhost(map);
			return 1;
		}
	}
	prefmsg(cmdparams->source.user->nick, hs_bot->nick,
		"Incorrect login or password. Do you have a vhost added?");
	return 1;
}

int CleanupHosts()
{
	lnode_t *hn, *hn2;
	hs_map *map;

	SET_SEGV_LOCATION();
	/* Use zero value to disable this feature */
	if(hs_cfg.old == 0)
		return 1;
	hn = list_first(vhosts);
	while (hn != NULL) {
		map = lnode_get(hn);
		if (map->lused < (me.now - (hs_cfg.old * 86400))) {
			nlog(LOG_NOTICE, "Expiring old vhost: %s for %s",
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
		return 1;
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
	/* banned vhosts */
	if (GetConf((void *) &temp, CFGSTR, "BannedVhosts") >= 0) {
		host = strtok(temp, ";");
		while (host != NULL) {
			/* limit host to MAXHOST and avoid unterminated/huge strings */
			host2 = smalloc(MAXHOST);
			strlcpy(host2, host, MAXHOST);
			hn = hnode_create(host2);
			hash_insert(bannedvhosts, hn, host2);
			host = strtok(NULL, ";");
		}
		sfree(temp);
	}
}
