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

#include "neostats.h"
#include "hostserv.h"

#define BANBUFSIZE	4096
#define MAXREASON	128
static char ban_buf[BANBUFSIZE];

typedef struct vhostentry {
	char nick[MAXNICK];
	char host[MAXHOST];
	char vhost[MAXHOST];
	char passwd[MAXPASS];
	char added[MAXNICK];
	time_t tslastused;
} vhostentry;

typedef struct banentry {
	char host[MAXHOST];
	char who[MAXNICK];
	char reason[MAXREASON];
} banentry;

struct hs_cfg {
	int expire;
	int regnick;
	char vhostdom[MAXHOST];
	int operhosts;
} hs_cfg;

static int hs_event_signon (CmdParams *cmdparams);
static int hs_event_mode (CmdParams *cmdparams);

static int hs_cmd_bans (CmdParams *cmdparams);
static int hs_cmd_login (CmdParams *cmdparams);
static int hs_cmd_chpass (CmdParams *cmdparams);
static int hs_cmd_add (CmdParams *cmdparams);
static int hs_cmd_list (CmdParams *cmdparams);
static int hs_cmd_view (CmdParams *cmdparams);
static int hs_cmd_del (CmdParams *cmdparams);
static int hs_cmd_bans_list (CmdParams *cmdparams);
static int hs_cmd_bans_add (CmdParams *cmdparams);
static int hs_cmd_bans_del (CmdParams *cmdparams);

static int hs_set_regnick_cb (CmdParams* cmdparams, SET_REASON reason);
static int hs_set_expire_cb (CmdParams* cmdparams, SET_REASON reason);

static void new_vhost (char *nick, char *host, char *vhost, char *pass, char *who);
static void LoadHosts (void);
static void LoadBans (void);
static void SaveBans (void);
int ExpireOldHosts (void);

static list_t *vhosts;
static hash_t *bannedvhosts;

/** Bot pointer */
static Bot *hs_bot;

/** Copyright info */
const char *hs_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
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

/** Bot comand table */
static bot_cmd hs_commands[]=
{
	{"ADD",		hs_cmd_add,		4,	NS_ULEVEL_LOCOPER,	hs_help_add,	hs_help_add_oneline },
	{"DEL",		hs_cmd_del,		1, 	NS_ULEVEL_LOCOPER,	hs_help_del,	hs_help_del_oneline },
	{"LIST",	hs_cmd_list,	0, 	NS_ULEVEL_LOCOPER,	hs_help_list,	hs_help_list_oneline },
	{"BANS",	hs_cmd_bans,	1,  NS_ULEVEL_ADMIN,	hs_help_bans,	hs_help_bans_oneline },
	{"VIEW",	hs_cmd_view,	1, 	NS_ULEVEL_OPER,		hs_help_view,	hs_help_view_oneline },
	{"LOGIN",	hs_cmd_login,	2, 	0,					hs_help_login,	hs_help_login_oneline },
	{"CHPASS",	hs_cmd_chpass,	3, 	0,					hs_help_chpass,	hs_help_chpass_oneline },
	{NULL,		NULL,			0, 	0,					NULL, 			NULL}
};

/** Bot setting table */
static bot_setting hs_settings[]=
{
	{"EXPIRE",		&hs_cfg.expire,		SET_TYPE_INT,		0, 99, 			NS_ULEVEL_ADMIN, "ExpireDays","days",hs_help_set_expire, NULL, (void*)60 },
	{"HIDDENHOST",	&hs_cfg.regnick,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "UnetVhosts",NULL,	hs_help_set_hiddenhost, NULL, (void*)0 },
	{"HOSTNAME",	&hs_cfg.vhostdom,	SET_TYPE_STRING,	0, MAXHOST, 	NS_ULEVEL_ADMIN, "UnetDomain",NULL,	hs_help_set_hostname, NULL, (void*)"" },
	{"OPERHOSTS",	&hs_cfg.operhosts,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "operhosts",NULL,	hs_help_set_operhosts, NULL, (void*)0 },
	{NULL,			NULL,				0,					0, 0, 			0,				 NULL,		  NULL,	NULL	},
};

/** BotInfo */
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

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_SIGNON,	hs_event_signon,	EVENT_FLAG_EXCLUDE_ME | EVENT_FLAG_USE_EXCLUDE},
	{EVENT_UMODE,	hs_event_mode,		EVENT_FLAG_EXCLUDE_ME | EVENT_FLAG_USE_EXCLUDE}, 
	{EVENT_NULL,	NULL}
};

static int hs_set_regnick_cb (CmdParams* cmdparams, SET_REASON reason)
{
	if (hs_cfg.regnick) {
		EnableEvent (EVENT_UMODE);
	} else {
		DisableEvent (EVENT_UMODE);
	}
	return NS_SUCCESS;
}

static int hs_set_expire_cb (CmdParams* cmdparams, SET_REASON reason)
{
	/* Ignore bootup callback */
	if (reason == SET_LOAD) {
		return NS_SUCCESS;
	}
	if (hs_cfg.expire) {
		add_timer (TIMER_TYPE_INTERVAL, ExpireOldHosts, "ExpireOldHosts", 7200);
	} else {
		del_timer ("ExpireOldHosts");
	}
	return NS_SUCCESS;
}

int findnick (const void *key1, const void *key2)
{
	const vhostentry *vhost = key1;
	return (ircstrcasecmp (vhost->nick, (char *)key2));
}

void save_vhost (vhostentry *vhe) 
{
	vhe->tslastused = me.now;
	SetData ((void *)vhe->host, CFGSTR, "Vhosts", vhe->nick, "Host");
	SetData ((void *)vhe->vhost, CFGSTR, "Vhosts", vhe->nick , "Vhost");
	SetData ((void *)vhe->passwd, CFGSTR, "Vhosts", vhe->nick , "Passwd");
	SetData ((void *)vhe->added, CFGSTR, "Vhosts", vhe->nick , "Added");
	SetData ((void *)vhe->tslastused, CFGINT, "Vhosts", vhe->nick , "LastUsed");
	list_sort (vhosts, findnick);
}

void del_vhost (vhostentry *vhost) 
{
	DelRow ("Vhosts", vhost->nick);
	ns_free (vhost);
	/* no need to list sort here, because its already sorted */
}

/* Routine For Setting the vhost */
static int hs_event_signon (CmdParams *cmdparams)
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	/* Check HostName Against Data Contained in vhosts.data */
	vhe = lnode_find (vhosts, cmdparams->source->name, findnick);
	if (vhe) {
		dlog (DEBUG1, "Checking %s against %s", vhe->host, cmdparams->source->user->hostname);
		if (match (vhe->host, cmdparams->source->user->hostname)) {
			irc_svshost (hs_bot, cmdparams->source, vhe->vhost);
			irc_prefmsg (hs_bot, cmdparams->source, 
				"Automatically setting your hidden host to %s", vhe->vhost);
			save_vhost (vhe);
		}
	}
	return NS_SUCCESS;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if succeeds else NS_FAILURE
 */

int ModInit (Module *mod_ptr)
{
	vhosts = list_create(-1);
	if (!vhosts) {
		nlog (LOG_CRITICAL, "Unable to create vhosts list");
		return -1;
	}
	bannedvhosts = hash_create(-1, 0, 0);
	if (!bannedvhosts) {
		nlog (LOG_CRITICAL, "Unable to create bannedvhosts hash");
		return -1;
	}
	ModuleConfig(hs_settings);
	LoadBans();
	LoadHosts();
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds else NS_FAILURE
 */

int ModSynch (void)
{
	hs_bot = AddBot (&hs_botinfo);
	if (!hs_bot) {
		return NS_FAILURE;
	}
	if (hs_cfg.expire) {
		add_timer (TIMER_TYPE_INTERVAL, ExpireOldHosts, "ExpireOldHosts", 7200);
	}
	if (!HaveUmodeRegNick()) 
	{
		DisableEvent (EVENT_UMODE);
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return none
 */

void ModFini (void)
{
	list_destroy_auto (vhosts);
}

int hs_event_mode (CmdParams *cmdparams) 
{
	int add = 0;
	char *modes;
	char vhost[MAXHOST];

	if (is_oper(cmdparams->source) && hs_cfg.operhosts == 0) 
		return NS_SUCCESS;
		
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
			if (*modes == UmodeChRegNick) {
				if (add) {
					if (IsUserSetHosted (cmdparams->source)) {
						dlog (DEBUG2, "not setting hidden host on %s", cmdparams->av[0]);
						return -1;
					}
					dlog (DEBUG2, "Regnick Mode on %s", cmdparams->av[0]);
					ircsnprintf(vhost, MAXHOST, "%s.%s", cmdparams->av[0], hs_cfg.vhostdom);
					irc_svshost (hs_bot, cmdparams->source, vhost);
					irc_prefmsg (hs_bot, cmdparams->source, 
						"Setting your host to %s", vhost);
				}
			}
			break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/* Routine for registrations with the 'vhosts.db' file */
static void new_vhost (char *nick, char *host, char *vhost, char *pass, char *who)
{
	vhostentry *vhe;

	vhe = ns_malloc (sizeof(vhostentry));
	strlcpy (vhe->nick, nick, MAXNICK);
	strlcpy (vhe->host, host, MAXHOST);
	strlcpy (vhe->vhost, vhost, MAXHOST);
	strlcpy (vhe->passwd, pass, MAXPASS);
	strlcpy (vhe->added, who, MAXNICK);
	lnode_create_append (vhosts, vhe);
	save_vhost (vhe);
}

static int hs_cmd_bans (CmdParams *cmdparams)
{
	if (!ircstrcasecmp (cmdparams->av[0], "LIST")) {
		return hs_cmd_bans_list (cmdparams);
	} else if (!ircstrcasecmp (cmdparams->av[0], "ADD")) {
		if (cmdparams->ac < 3) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		return hs_cmd_bans_add (cmdparams);
	} else if (!ircstrcasecmp (cmdparams->av[0], "DEL")) {
		if (cmdparams->ac < 2) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		return hs_cmd_bans_del (cmdparams);
	}
	return NS_ERR_SYNTAX_ERROR;
}

static int hs_cmd_bans_list (CmdParams *cmdparams)
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	if (hash_count (bannedvhosts) == 0) {
		irc_prefmsg (hs_bot, cmdparams->source, "No bans are defined.");
		return NS_SUCCESS;
	}
	hash_scan_begin (&hs, bannedvhosts);
	irc_prefmsg (hs_bot, cmdparams->source, "Banned vhosts");
	while ((hn = hash_scan_next (&hs)) != NULL) {
		ban = ((banentry *)hnode_get (hn));
		irc_prefmsg (hs_bot, cmdparams->source, "%d - %s added by %s for %s", i, ban->host, ban->who, ban->reason);
		i++;
	}
	irc_prefmsg (hs_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static int hs_cmd_bans_add (CmdParams *cmdparams)
{
	banentry *ban;
	char *buf;

	if (hash_lookup(bannedvhosts, cmdparams->av[1]) != NULL) {
		irc_prefmsg (hs_bot, cmdparams->source, 
			"%s already exists in the banned vhost list", cmdparams->av[1]);
		return NS_SUCCESS;
	}
	ban = malloc(sizeof(banentry));
	strlcpy(ban->host, cmdparams->av[1], MAXHOST);
	strlcpy(ban->who, cmdparams->source->name, MAXNICK);
	buf = joinbuf(cmdparams->av, cmdparams->ac, 2);
	strlcpy(ban->reason, buf, MAXREASON);
	free(buf);

	hnode_create_insert (bannedvhosts, ban, ban->host);
	irc_prefmsg (hs_bot, cmdparams->source, 
		"%s added to the banned vhosts list", cmdparams->av[1]);
	irc_chanalert (hs_bot, "%s added %s to the banned vhosts list",
		  cmdparams->source->name, cmdparams->av[1]);
	SaveBans ();
	return NS_SUCCESS;
}

static int hs_cmd_bans_del (CmdParams *cmdparams)
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;

	hash_scan_begin (&hs, bannedvhosts);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		ban = (banentry *)hnode_get (hn);
		if (ircstrcasecmp (ban->host, cmdparams->av[1]) == 0) {
			hash_scan_delete (bannedvhosts, hn);
			irc_prefmsg (hs_bot, cmdparams->source, 
				"Deleted %s from the banned vhost list", cmdparams->av[1]);
			irc_chanalert (hs_bot, "%s deleted %s from the banned vhost list",
				cmdparams->source->name, cmdparams->av[1]);
			nlog (LOG_NOTICE, "%s deleted %s from the banned vhost list",
				cmdparams->source->name, cmdparams->av[1]);
			hnode_destroy (hn);
			DelRow ("Ban", ban->host);
			ns_free (ban);
			SaveBans ();
			return NS_SUCCESS;
		}
	}
	irc_prefmsg (hs_bot, cmdparams->source, "No entry for %s", cmdparams->av[1]);
	return NS_SUCCESS;
}

static int hs_cmd_chpass (CmdParams *cmdparams)
{
	vhostentry *vhe;

	vhe = lnode_find (vhosts, cmdparams->av[0], findnick);
	if (!vhe) {
		irc_prefmsg (hs_bot, cmdparams->source, "No vhost for that user.");
		irc_chanalert (hs_bot, "%s tried to change the password for %s, but there is no user with that name",
			cmdparams->source->name, cmdparams->av[0]);
		return NS_SUCCESS;
	}
	if ((match (vhe->host, cmdparams->source->user->hostname))
		|| (UserLevel (cmdparams->source) >= 100)) {
		if (!ircstrcasecmp (vhe->passwd, cmdparams->av[1])) {
			strlcpy (vhe->passwd, cmdparams->av[2], MAXPASS);
			irc_prefmsg (hs_bot, cmdparams->source, "Password changed");
			irc_chanalert (hs_bot, "%s changed the password for %s",
					cmdparams->source->name, vhe->nick);
			save_vhost (vhe);
			return NS_SUCCESS;
		}
		return NS_SUCCESS;
	}
	irc_prefmsg (hs_bot, cmdparams->source, "Error, hostname mismatch");
	irc_chanalert (hs_bot, "%s tried to change the password for %s, but the hosts do not match (%s -> %s)",
			cmdparams->source->name, vhe->nick, cmdparams->source->user->hostname, vhe->host);
	nlog (LOG_WARNING, "%s tried to change the password for %s but the hosts do not match (%s -> %s)",
			cmdparams->source->name, vhe->nick, cmdparams->source->user->hostname, vhe->host);
	return NS_SUCCESS;
}

/* Routine for ADD */
static int hs_cmd_add (CmdParams *cmdparams)
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;
	Client *u;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, bannedvhosts);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		ban = (banentry *) hnode_get (hn);
		if (match (ban->host, cmdparams->av[2])) {
			irc_prefmsg (hs_bot, cmdparams->source, 
				"%s has been matched against the vhost ban %s",
				cmdparams->av[2], ban->host);
			irc_chanalert (hs_bot, "%s tried to add a banned vhost %s",
				  cmdparams->source->name, cmdparams->av[2]);
			return NS_SUCCESS;
		}
	}
	if (validate_host (cmdparams->av[2]) == NS_FAILURE) {
		irc_prefmsg (hs_bot, cmdparams->source, 
			"%s is an invalid host", cmdparams->av[2]);
		return NS_SUCCESS;
	}
	if (list_find (vhosts, cmdparams->av[0], findnick)) {
		irc_prefmsg (hs_bot, cmdparams->source, 
			"%s already has a vhost entry", cmdparams->av[0]);
		return NS_SUCCESS;
	}
	new_vhost (cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3], cmdparams->source->name);
	nlog (LOG_NOTICE, "%s added a vhost for %s with realhost %s vhost %s and password %s",
	    cmdparams->source->name, cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3]);
	irc_prefmsg (hs_bot, cmdparams->source, 
		"%s has successfully been registered under realhost: %s vhost: %s and password: %s",
		cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3]);
	irc_chanalert (hs_bot, "%s added a vhost %s for %s with realhost %s",
		cmdparams->source->name, cmdparams->av[2], cmdparams->av[0], cmdparams->av[1]);
	/* Apply The New Hostname If The User Is Online */
	u = find_user(cmdparams->av[0]);
	if (u && !IsMe(u)) {
		if (match (cmdparams->av[1], u->user->hostname)) {
			irc_svshost (hs_bot, u, cmdparams->av[2]);
			irc_prefmsg (hs_bot, cmdparams->source, 
				"%s is online now, setting vhost to %s",
				cmdparams->av[0], cmdparams->av[2]);
			irc_prefmsg (hs_bot, u, 
				"Your vhost has been created with hostmask of %s and username %s with password %s",
				cmdparams->av[1], cmdparams->av[0], cmdparams->av[3]);
			irc_prefmsg (hs_bot, u, 
				"For security, you should change your vhost password. See /msg %s help chpass",
				hs_bot->name);
		}
	}
	return NS_SUCCESS;
}

/* Routine for 'HostServ' to print out its data */
static int hs_cmd_list (CmdParams *cmdparams)
{
	int i;
	lnode_t *hn;
	vhostentry *vhe;
	int start = 0;
	int vhostcount;

	SET_SEGV_LOCATION();
	if (cmdparams->ac == 1) {
		start = atoi(cmdparams->av[0]);
	}
	vhostcount = list_count(vhosts);
	if (vhostcount == 0) {
		irc_prefmsg (hs_bot, cmdparams->source, "No vhosts are defined.");
		return NS_SUCCESS;
	}
	if (start >= vhostcount) {
		irc_prefmsg (hs_bot, cmdparams->source, "Value out of range. There are only %d entries", (int)vhostcount);
		return NS_SUCCESS;
	}		
	i = 1;
	irc_prefmsg (hs_bot, cmdparams->source, "Current vhost list: ");
	irc_prefmsg (hs_bot, cmdparams->source, "Showing %d to %d entries of %d vhosts", start+1, start+20, (int)vhostcount);
	irc_prefmsg (hs_bot, cmdparams->source, "%-5s %-12s %-30s", "Num", "Nick", "Vhost");
	hn = list_first(vhosts);
	while (hn != NULL) {
		if (i <= start) {
			i++;
			hn = list_next(vhosts, hn);
			continue;
		}
		vhe = lnode_get(hn);
		irc_prefmsg (hs_bot, cmdparams->source, "%-5d %-12s %-30s", i,
			vhe->nick, vhe->vhost);
		i++;
		/* limit to x entries per screen */
		if (i > start + 20) 
			break;
		hn = list_next(vhosts, hn);
	}
	irc_prefmsg (hs_bot, cmdparams->source, 
		"For more information on someone use /msg %s VIEW <nick>",
		hs_bot->name);
	irc_prefmsg (hs_bot, cmdparams->source, "End of list.");
	if (vhostcount >= i) 
		irc_prefmsg (hs_bot, cmdparams->source, "Type \2/msg %s list %d\2 to see next page", hs_bot->name, i-1);
	return NS_SUCCESS;
}

/* Routine for VIEW */
static int hs_cmd_view (CmdParams *cmdparams)
{
	vhostentry *vhe;
	char ltime[80];

	SET_SEGV_LOCATION();
	vhe = lnode_find (vhosts, cmdparams->av[0], findnick);
	if (!vhe) {
		irc_prefmsg (hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0]);
		return NS_SUCCESS;
	}
	irc_prefmsg (hs_bot, cmdparams->source, "Vhost information:");
	irc_prefmsg (hs_bot, cmdparams->source, "Nick:      %s", vhe->nick);
	irc_prefmsg (hs_bot, cmdparams->source, "Real host: %s", vhe->host);
	irc_prefmsg (hs_bot, cmdparams->source, "Vhost:     %s", vhe->vhost);
	irc_prefmsg (hs_bot, cmdparams->source, "Password:  %s", vhe->passwd);
	irc_prefmsg (hs_bot, cmdparams->source, "Added by:  %s",
		vhe->added ? vhe->added : "<unknown>");
	strftime(ltime, 80, "%d/%m/%Y[%H:%M]", localtime(&vhe->tslastused));
	irc_prefmsg (hs_bot, cmdparams->source, "Last used: %s", ltime);
	irc_prefmsg (hs_bot, cmdparams->source, "--- End of information ---");
	return NS_SUCCESS;
}

/* Routine For Loading The Hosts into Array */
static void LoadHosts()
{
	vhostentry *vhe;
	char *tmp;
	int count;
	char **LoadArray;

	if (GetTableData("Vhosts", &LoadArray) > 0) {
		for (count = 0; LoadArray[count] != NULL; count++) {
			vhe = ns_malloc (sizeof(vhostentry));
			strlcpy (vhe->nick, LoadArray[count], MAXNICK);
			if (GetData ((void *)&tmp, CFGSTR, "Vhosts", vhe->nick, "Host") > 0) 
				strlcpy (vhe->host, tmp, MAXHOST);
			if (GetData ((void *)&tmp, CFGSTR, "Vhosts", vhe->nick, "Vhost") > 0) 
				strlcpy (vhe->vhost, tmp, MAXHOST);
			if (GetData ((void *)&tmp, CFGSTR, "Vhosts", vhe->nick, "Passwd") > 0) 
				strlcpy (vhe->passwd, tmp, MAXPASS);
			if (GetData ((void *)&tmp, CFGSTR, "Vhosts", vhe->nick, "Added") > 0)
				strlcpy (vhe->added, tmp, MAXNICK);
			GetData ((void *)&vhe->tslastused, CFGINT, "Vhosts", vhe->nick, "LastUsed");
			lnode_create_append (vhosts, vhe);
			dlog (DEBUG1, "Loaded %s (%s) into Vhosts", vhe->nick, vhe->vhost);
		}
	}			
	ns_free (LoadArray);
	list_sort (vhosts, findnick);
}

/* Routine for HostServ to delete the given information */
static int hs_cmd_del (CmdParams *cmdparams)
{
	lnode_t *hn;
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	hn = list_find (vhosts, cmdparams->av[0], findnick);
	if (!hn) {
		irc_prefmsg (hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0]);
		return NS_SUCCESS;
	}
	vhe = lnode_get (hn);
	if (!vhe) {
		irc_prefmsg (hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0]);
		return NS_SUCCESS;
	}
	irc_prefmsg (hs_bot, cmdparams->source, "removed vhost %s for %s",
		vhe->nick, vhe->vhost);
	nlog (LOG_NOTICE, "%s removed the vhost %s for %s", 
			cmdparams->source->name, vhe->vhost, vhe->nick);
	irc_chanalert (hs_bot, "%s removed vhost %s for %s",
			cmdparams->source->name, vhe->vhost, vhe->nick);
	del_vhost (vhe);
	list_delete (vhosts, hn);
	lnode_destroy (hn);
	return NS_SUCCESS;
}

/* Routine to allow users to login and get their vhosts */
static int hs_cmd_login (CmdParams *cmdparams)
{
	vhostentry *vhe;
	char *login;
	char *pass;

	SET_SEGV_LOCATION();
	login = cmdparams->av[0];
	pass = cmdparams->av[1];
	/* Check HostName Against Data Contained in vhosts.data */
	vhe = lnode_find (vhosts, login, findnick);
	if (vhe) {
		if (!ircstrcasecmp (vhe->passwd, pass)) {
			irc_svshost (hs_bot, cmdparams->source, vhe->vhost);
			irc_prefmsg (hs_bot, cmdparams->source, 
				"Your vhost %s has been set.", vhe->vhost);
			nlog (LOG_NORMAL, "%s used LOGIN to obtain vhost of %s",
			    cmdparams->source->name, vhe->vhost);
			irc_chanalert (hs_bot, "%s used login to get vhost %s", 
				cmdparams->source->name, vhe->vhost);
			save_vhost (vhe);
			return NS_SUCCESS;
		}
	}
	irc_prefmsg (hs_bot, cmdparams->source, 
		"Incorrect login or password. Do you have a vhost added?");
	return NS_SUCCESS;
}

int ExpireOldHosts (void)
{
	lnode_t *hn, *hn2;
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	hn = list_first(vhosts);
	while (hn != NULL) {
		vhe = lnode_get(hn);
		if (vhe->tslastused < (me.now - (hs_cfg.expire * 86400))) {
			nlog (LOG_NOTICE, "Expiring old vhost: %s for %s",
			     vhe->vhost, vhe->nick);
			del_vhost (vhe);
			hn2 = list_next (vhosts, hn);
			list_delete (vhosts, hn);
			lnode_destroy (hn);
			hn = hn2;
		} else {
			hn = list_next (vhosts, hn);
		}
	}
	return NS_SUCCESS;
}

/* 
 * Save HostServ bans
 */
static void SaveBans (void)
{
	static char ban_buf[CONFBUFSIZE];
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;

	hash_scan_begin (&hs, bannedvhosts);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		ban = (banentry *) hnode_get (hn);
		SetData ((void *)ban->who, CFGSTR, "Ban", ban->host, "Who");
		SetData ((void *)ban->reason, CFGSTR, "Ban", ban->host, "Reason");
	}
}

/* 
 * Load HostServ bans
 */
static void LoadBans (void)
{
	char *tmp;
	static char ban_buf[CONFBUFSIZE];
	banentry *ban;
	int i;
	char **data;

	if (GetTableData ("Ban", &data) > 0) {
		for (i = 0; data[i] != NULL; i++) {
			ban = ns_malloc (sizeof (banentry));
			strlcpy (ban->host, data[i], MAXHOST);

			if (GetData ((void *)&tmp, CFGSTR, "Ban", ban->host, "Who") <= 0) {
				ns_free (ban);
				continue;
			} else {
				strlcpy (ban->who, tmp, MAXNICK);
				ns_free (tmp);
			}
			if (GetData ((void *)&tmp, CFGSTR, "Ban", ban->host, "Reason") <= 0) {
				ns_free (ban);
				continue;
			} else {
				strlcpy (ban->reason, tmp, MAXREASON);
				ns_free (tmp);
			}
			hnode_create_insert (bannedvhosts, ban, ban->host);
		}
		free(data);
	}
}
