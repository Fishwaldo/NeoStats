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
#include "neostats.h"
#include "hostserv.h"

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
	int regnick;
	char vhostdom[MAXHOST];
	int modnum;
	int operhosts;
} hs_cfg;

typedef struct hs_user {
	int vhostset;
	hs_map *vhe;
} hs_user;

static int hs_event_signon(CmdParams* cmdparams);
static int hs_event_mode(CmdParams* cmdparams);
static int hs_levels(CmdParams* cmdparams);
static int hs_bans(CmdParams* cmdparams);
static int hs_login(CmdParams* cmdparams);
static int hs_chpass(CmdParams* cmdparams);
static int hs_add(CmdParams* cmdparams);
static int hs_list(CmdParams* cmdparams);
static int hs_view(CmdParams* cmdparams);
static int hs_del(CmdParams* cmdparams);

static void hs_listban(Client * u);
static void hs_addban(Client * u, char *ban);
static void hs_delban(Client * u, char *ban);

static void SaveBans(void);
static void hsdat(char *nick, char *host, char *vhost, char *pass, char *who);
int CleanupHosts(void);
static void LoadHosts(void);
static void LoadConfig(void);

Bot *hs_bot;
static Module* hs_module;

const char *hs_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

ModuleInfo module_info = {
	"HostServ",
	"Network virtual host service",
	hs_copyright,
	hs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	FEATURE_SVSHOST,
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
	{"EXPIRE",		&hs_cfg.old,		SET_TYPE_INT,		0, 99, 			NS_ULEVEL_ADMIN, "ExpireDays","days",hs_help_set_expire, NULL, (void*)60 },
	{"HIDDENHOST",	&hs_cfg.regnick,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "UnetVhosts",NULL,	hs_help_set_hiddenhost, NULL, (void*)0 },
	{"HOSTNAME",	&hs_cfg.vhostdom,	SET_TYPE_STRING,	0, MAXHOST, 	NS_ULEVEL_ADMIN, "UnetDomain",NULL,	hs_help_set_hostname, NULL, (void*)"" },
	{"OPERHOSTS",	&hs_cfg.operhosts,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "operhosts",NULL,	hs_help_set_operhosts, NULL, (void*)0 },
	{NULL,			NULL,				0,					0, 0, 			0,				 NULL,		  NULL,	NULL	},
};

static BotInfo hs_botinfo = 
{
	"HostServ", 
	"HostServ1", 
	"HS", 
	BOT_COMMON_HOST, 
	"Network virtual host service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	hs_commands, 
	hs_settings,
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

void set_moddata(Client * u) 
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
	if(!HaveUmodeRegNick()) 
		return -1;

	if (cmdparams->source->moddata[hs_module->modnum]) {
		dlog(DEBUG2, "hs_event_quit: free module data");
		sfree(cmdparams->source->moddata[hs_module->modnum]);
		cmdparams->source->moddata[hs_module->modnum] = NULL;
	}
	return 1;
}
	
/* Routine For Setting the Virtual Host */
static int hs_event_signon(CmdParams* cmdparams)
{
	lnode_t *hn;
	hs_map *map;

	SET_SEGV_LOCATION();

	if (!is_synched)
		return 0;

	if (IsMe(cmdparams->source)) 
		return 1;

	/* is this user excluded via a global exclusion? */
	if (IsExcluded(cmdparams->source)) 
		return 1;

	/* Check HostName Against Data Contained in vhosts.data */
	hn = list_find(vhosts, cmdparams->source->name, findnick);
	if (hn) {
		map = lnode_get(hn);
		dlog(DEBUG1, "Checking %s against %s", map->host, cmdparams->source->user->hostname);
		if (match(map->host, cmdparams->source->user->hostname)) {
			irc_svshost(cmdparams->source, map->vhost);
			irc_prefmsg(hs_bot, cmdparams->source, 
				"Automatically setting your hidden host to %s", map->vhost);
			map->lused = me.now;
			save_vhost(map);
			if(HaveUmodeRegNick()) {
				set_moddata(cmdparams->source);
			}
		}
	}
	return 1;
}

static int hs_event_online(CmdParams* cmdparams)
{
	hs_bot = init_bot(&hs_botinfo);
	add_timer (CleanupHosts, "CleanupHosts", 7200);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	hs_event_online},
	{EVENT_SIGNON,	hs_event_signon},
	{EVENT_UMODE,	hs_event_mode}, 
	{EVENT_QUIT,	hs_event_quit},
	{EVENT_KILL,	hs_event_quit},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* mod_ptr)
{
	hs_module = mod_ptr;
	vhosts = list_create(-1);
	bannedvhosts = hash_create(-1, 0, 0);
	if (!vhosts) {
		nlog(LOG_CRITICAL, "Error, can't create vhosts hash");
		return -1;
	}
	ModuleConfig(hs_settings);
	LoadConfig();
	LoadHosts();
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

int hs_event_mode(CmdParams* cmdparams) 
{
	int add = 0;
	char *modes;
	char vhost[MAXHOST];

	/* bail if we are not synched */
	if (!is_synched)
		return 0;
		
	if(!HaveUmodeRegNick()) 
		return -1;

	/* bail out if its not enabled */
	if (hs_cfg.regnick != 1) 
		return -1;

	if (is_oper(cmdparams->source) && hs_cfg.operhosts == 0) 
		return 1;
		
	if (IsExcluded(cmdparams->source)) 
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
		default:
			if(*modes == UmodeChRegNick) {
				if (add) {
					if (cmdparams->source->moddata[hs_module->modnum] != NULL) {
						dlog(DEBUG2, "not setting hidden host on %s", cmdparams->av[0]);
						return -1;
					}
					dlog(DEBUG2, "Regnick Mode on %s", cmdparams->av[0]);
					ircsnprintf(vhost, MAXHOST, "%s.%s", cmdparams->av[0], hs_cfg.vhostdom);
					irc_svshost(cmdparams->source, vhost);
					irc_prefmsg(hs_bot, cmdparams->source, 
						"Automatically setting your hidden host to %s", vhost);
					set_moddata(cmdparams->source);
				}
			}
			break;
		}
		modes++;
	}
	return 1;
}

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
		irc_prefmsg(hs_bot, cmdparams->source, 
			"Configured Levels: ADD: %d, DEL: %d, LIST: %d, VIEW: %d",
			hs_cfg.add, hs_cfg.del, hs_cfg.list,hs_cfg.view);
			return NS_SUCCESS;
	} else if (cmdparams->ac == 3) {
		if (UserLevel(cmdparams->source) >= NS_ULEVEL_ADMIN) {
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
		if (UserLevel(cmdparams->source) >= NS_ULEVEL_ADMIN) {
			t = atoi(cmdparams->av[3]);
			if ((t <= 0) || (t > NS_ULEVEL_ROOT)) {
				irc_prefmsg(hs_bot, cmdparams->source, 
					"Invalid Level. Must be between 1 and %d", NS_ULEVEL_ROOT);
				return NS_ERR_SYNTAX_ERROR;
			}
			if (!ircstrcasecmp(cmdparams->av[2], "ADD")) {
				hs_cfg.add = t;
				SetConf((void *) t, CFGINT, "AddLevel");
			} else if (!ircstrcasecmp(cmdparams->av[2], "DEL")) {
				hs_cfg.del = t;
				SetConf((void *) t, CFGINT, "DelLevel");
			} else if (!ircstrcasecmp(cmdparams->av[2], "LIST")) {
				hs_cfg.list = t;
				SetConf((void *) t, CFGINT, "ListLevel");
			} else if (!ircstrcasecmp(cmdparams->av[2], "VIEW")) {
				hs_cfg.view = t;
				SetConf((void *) t, CFGINT, "ViewLevel");
			} else {
				irc_prefmsg(hs_bot, cmdparams->source, 
					"Invalid Level. /msg %s help levels", hs_bot->name);
				return 1;
			}
			irc_prefmsg(hs_bot, cmdparams->source, 
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
		hs_listban(cmdparams->source);
		return NS_SUCCESS;
	} else if (cmdparams->ac == 4) {
		if (UserLevel(cmdparams->source) >= NS_ULEVEL_ADMIN) {
			if (!ircstrcasecmp(cmdparams->av[2], "ADD")) {
				hs_addban(cmdparams->source, cmdparams->av[3]);
				return NS_SUCCESS;
			} else if (!ircstrcasecmp(cmdparams->av[2], "DEL")) {
				hs_delban(cmdparams->source, cmdparams->av[3]);
				return NS_SUCCESS;
			}
		} else {
			return NS_ERR_NO_PERMISSION;
		}
	}
	return NS_ERR_SYNTAX_ERROR;
}

static void hs_listban(Client * u)
{
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	hash_scan_begin(&hs, bannedvhosts);
	irc_prefmsg(hs_bot, u,  "Banned vhosts");
	while ((hn = hash_scan_next(&hs)) != NULL) {
		irc_prefmsg(hs_bot, u,  "%d - %s", i, (char *)hnode_get(hn));
		i++;
	}
	irc_prefmsg(hs_bot, u, "End of List.");

}
static void hs_addban(Client * u, char *ban)
{
	hnode_t *hn;
	char *host;

	if (hash_lookup(bannedvhosts, ban) != NULL) {
		irc_prefmsg(hs_bot, u, 
			"%s already exists in the banned vhost list", ban);
		return;
	}
	host = smalloc(MAXHOST);
	strlcpy(host, ban, MAXHOST);
	hn = hnode_create(host);
	hash_insert(bannedvhosts, hn, host);
	irc_prefmsg(hs_bot, u, 
		"%s added to the banned vhosts list", ban);
	irc_chanalert(hs_bot, "%s added %s to the banned vhosts list",
		  u->name, ban);
	SaveBans();
}

static void hs_delban(Client * u, char *ban)
{
	hnode_t *hn;
	hscan_t hs;
	int i = 0;
	int j = 0;

	i = atoi(ban);
	if ((i < 1) || (i > (int)hash_count(bannedvhosts))) {
		irc_prefmsg(hs_bot, u, 
			"Invalid Entry. /msg %s bans for the list",
			hs_bot->name);
		return;
	}

	hash_scan_begin(&hs, bannedvhosts);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		++j;
		if (i == j) {
			hash_scan_delete(bannedvhosts, hn);
			irc_prefmsg(hs_bot, u, 
				"Deleted %s from the banned vhost list",
				(char *) hnode_get(hn));
			irc_chanalert(hs_bot,
				  "%s deleted %s from the banned vhost list",
				  u->name, (char *) hnode_get(hn));
			nlog(LOG_NOTICE,
			     "%s deleted %s from the banned vhost list",
			     u->name, (char *) hnode_get(hn));
			sfree(hnode_get(hn));
			hnode_destroy(hn);
			SaveBans();
			return;
		}
	}
	irc_prefmsg(hs_bot, u,  "Entry %d was not found (Weird?!?)", i);
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
		if ((match(map->host, cmdparams->source->user->hostname))
		    || (UserLevel(cmdparams->source) >= 100)) {
			if (!ircstrcasecmp(map->passwd, oldpass)) {
				strlcpy(map->passwd, newpass, MAXPASSWORD);
				irc_prefmsg(hs_bot, cmdparams->source, 
					"Password Successfully changed");
				irc_chanalert(hs_bot,
					  "%s changed the password for %s",
					  cmdparams->source->name, map->nnick);
				map->lused = me.now;
				save_vhost(map);
				return 1;
			}
		} else {
			irc_prefmsg(hs_bot, cmdparams->source,  
				"Error, Hostname Mis-Match");
			irc_chanalert(hs_bot,
				  "%s tried to change the password for %s, but the hosts do not match (%s->%s)",
				  cmdparams->source->name, map->nnick, cmdparams->source->user->hostname,
				  map->host);
			nlog(LOG_WARNING,
			     "%s tried to change the password for %s but the hosts do not match (%s -> %s)",
			     cmdparams->source->name, map->nnick, cmdparams->source->user->hostname, map->host);
			return 1;
		}
	}

	irc_prefmsg(hs_bot, cmdparams->source, 
		"Invalid Username or Password. Access Denied");
	irc_chanalert(hs_bot,
		  "%s tried to change the password for %s, but got it wrong",
		  cmdparams->source->name, nick);
	return 1;

}

/* Routine for ADD */
static int hs_add(CmdParams* cmdparams)
{

	hnode_t *hn;
	hscan_t hs;
	Client *nu;
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
		if (match(hnode_get(hn), h)) {
			irc_prefmsg(hs_bot, cmdparams->source, 
				"%s has been matched against the vhost ban %s",
				h, (char *) hnode_get(hn));
			irc_chanalert(hs_bot,
				  "%s tried to add a banned vhost %s",
				  cmdparams->source->name, h);
			return 1;
		}
	}

	if(validate_host (h) == NS_FAILURE) {
		irc_prefmsg(hs_bot, cmdparams->source, 
			"%s is an invalid host", h);
		return 1;
	}

	if (list_find(vhosts, cmd, findnick)) {
		irc_prefmsg(hs_bot, cmdparams->source, 
			"%s already has a vhost entry", cmd);
		return 1;
	}
	hsdat(cmd, m, h, p, cmdparams->source->name);
	nlog(LOG_NOTICE,
		    "%s added a vhost for %s with realhost %s vhost %s and password %s",
		    cmdparams->source->name, cmd, m, h, p);
	irc_prefmsg(hs_bot, cmdparams->source, 
		"%s has sucessfully been registered under realhost: %s vhost: %s and password: %s",
		cmd, m, h, p);
	irc_chanalert(hs_bot,
			"%s added a vhost %s for %s with realhost %s",
			cmdparams->source->name, h, cmd, m);
	/* Apply The New Hostname If The User Is Online */
	if ((nu = find_user(cmd)) != NULL) {
		if (!IsMe(nu)) {
			if (match(m, nu->user->hostname)) {
				irc_svshost(nu, h);
				irc_prefmsg(hs_bot, cmdparams->source, 
					"%s is online now, setting vhost to %s",
					cmd, h);
				irc_prefmsg(hs_bot, nu, 
					"Your vhost has been created with hostmask of %s and username %s with password %s",
					m, cmd, p);
				irc_prefmsg(hs_bot, nu, 
					"For security, you should change your vhost password. See /msg %s help chpass",
					hs_bot->name);
				if(HaveUmodeRegNick()) 
					set_moddata(nu);
			}
		}
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
	int vhostcount;

	SET_SEGV_LOCATION();
	if (cmdparams->ac == 2) {
		start = 0;
	} else if (cmdparams->ac == 3) {
		start = atoi(cmdparams->av[2]);
	}

	vhostcount = list_count(vhosts);

	if (start >= vhostcount) {
		irc_prefmsg(hs_bot, cmdparams->source,   "Value out of range. There are only %d entries", (int)vhostcount);
		return 1;
	}
		
	i = 1;
	irc_prefmsg(hs_bot, cmdparams->source,  "Current vhost list: ");
	irc_prefmsg(hs_bot, cmdparams->source,  "Showing %d to %d entries of %d vhosts", start+1, start+20, (int)vhostcount);
	irc_prefmsg(hs_bot, cmdparams->source,  "%-5s %-12s %-30s", "Num", "Nick", "Vhost");
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i <= start) {
			i++;
			hn = list_next(vhosts, hn);
			continue;
		}
		map = lnode_get(hn);
		irc_prefmsg(hs_bot, cmdparams->source,  "%-5d %-12s %-30s", i,
			map->nnick, map->vhost);
		i++;
		/* limit to x entries per screen */
		if (i > start + 20) 
			break;
		hn = list_next(vhosts, hn);
	}
	irc_prefmsg(hs_bot, cmdparams->source, 
		"For more information on someone use /msg %s VIEW #",
		hs_bot->name);
	irc_prefmsg(hs_bot, cmdparams->source,  "--- End of List ---");
	if (vhostcount >= i) 
		irc_prefmsg(hs_bot, cmdparams->source,  "Type \2/msg %s list %d\2 to see next page", hs_bot->name, i-1);
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
		irc_prefmsg(hs_bot, cmdparams->source, 
			"Syntax: /msg %s VIEW #", hs_bot->name);
		irc_prefmsg(hs_bot, cmdparams->source, 
			"The users # is got from /msg %s LIST",
			hs_bot->name);
		return 1;
	}
	i = 1;
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (tmpint == i) {
			map = lnode_get(hn);
			irc_prefmsg(hs_bot, cmdparams->source,  "Virtual Host information:");
			irc_prefmsg(hs_bot, cmdparams->source,  "Nick:      %s", map->nnick);
			irc_prefmsg(hs_bot, cmdparams->source,  "Real host: %s", map->host);
			irc_prefmsg(hs_bot, cmdparams->source,  "Vhost:     %s", map->vhost);
			irc_prefmsg(hs_bot, cmdparams->source,  "Password:  %s", map->passwd);
			irc_prefmsg(hs_bot, cmdparams->source,  "Added by:  %s",
				map->added ? map->added : "<unknown>");
			strftime(ltime, 80, "%d/%m/%Y[%H:%M]", localtime(&map->lused));
			irc_prefmsg(hs_bot, cmdparams->source,  "Last used: %s", ltime);
			irc_prefmsg(hs_bot, cmdparams->source,  "--- End of information ---");
		}
		hn = list_next(vhosts, hn);
		i++;
	}
	if (tmpint > i)
		irc_prefmsg(hs_bot, cmdparams->source, 
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
	char **LoadArray;

	if (GetTableData("Vhosts", &LoadArray) > 0) {
		for (count = 0; LoadArray[count] != NULL; count++) {
			map = smalloc(sizeof(hs_map));
			strlcpy(map->nnick, LoadArray[count], MAXNICK);
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
	sfree(LoadArray);
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
		irc_prefmsg(hs_bot, cmdparams->source, 
			"Syntax: /msg %s DEL #", hs_bot->name);
		irc_prefmsg(hs_bot, cmdparams->source, 
			"The users # is got from /msg %s LIST",
			hs_bot->name);
		return 1;
	}

	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i == tmpint) {
			map = lnode_get(hn);
			irc_prefmsg(hs_bot, cmdparams->source,  "removed vhost %s for %s",
				map->nnick, map->vhost);
			nlog(LOG_NOTICE, "%s removed the vhost %s for %s", 
				 cmdparams->source->name, map->vhost, map->nnick);
			irc_chanalert(hs_bot, "%s removed vhost %s for %s",
				  cmdparams->source->name, map->vhost, map->nnick);
			del_vhost(map);
			list_delete(vhosts, hn);
			lnode_destroy(hn);
			return 1;
		}
		hn = list_next(vhosts, hn);
		i++;
	}

	if (tmpint > i)
		irc_prefmsg(hs_bot, cmdparams->source,  "ERROR: no vhost number \2%d\2",
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
			irc_svshost(cmdparams->source, map->vhost);
			map->lused = me.now;
			irc_prefmsg(hs_bot, cmdparams->source, 
				"Your vhost %s has been set.", map->vhost);
			nlog(LOG_NORMAL, "%s used LOGIN to obtain vhost of %s",
			    cmdparams->source->name, map->vhost);
			irc_chanalert(hs_bot, "%s used login to get vhost %s", 
				cmdparams->source->name, map->vhost);
			if(HaveUmodeRegNick()) 
				set_moddata(cmdparams->source);
			save_vhost(map);
			return 1;
		}
	}
	irc_prefmsg(hs_bot, cmdparams->source, 
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
