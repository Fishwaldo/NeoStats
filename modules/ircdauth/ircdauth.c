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
** $Id: serviceroots.c 1721 2004-04-09 22:17:19Z Mark $
*/

#include <stdio.h>
#include "neostats.h"

static int auth_cmd_authmodelist(CmdParams* cmdparams);

typedef struct UserAuthModes{
	unsigned long umode;
	int level;
} UserAuthModes;

static const char *ns_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

const char *auth_help_authmodelist[] = {
	"Syntax: \2AUTHMODELIST\2",
	"",
	"Lists the user modes and their level",
	NULL
};

const char auth_help_authmodelist_oneline[] = "User mode auth list";

bot_cmd auth_commands[]=
{
	{"AUTHMODELIST",	auth_cmd_authmodelist,	0,	NS_ULEVEL_OPER, auth_help_authmodelist,	auth_help_authmodelist_oneline},
	{NULL,		NULL,			0, 	0,				NULL, 			NULL}
};

ModuleInfo module_info = {
	"IRCDAuth",
	"IRCD User Mode Authentication Module",
	ns_copyright,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
};

UserAuthModes user_auth_modes[] = {
#ifdef UMODE_DEBUG
	{UMODE_DEBUG,		NS_ULEVEL_ROOT},
#endif
	{UMODE_TECHADMIN,	NS_ULEVEL_ADMIN},
#ifdef UMODE_SERVICESOPER
	{UMODE_SERVICESOPER,NS_ULEVEL_OPER},
#endif
#ifdef UMODE_IRCADMIN
	{UMODE_IRCADMIN,	NS_ULEVEL_OPER},
#endif
#ifdef UMODE_SUPER
	{UMODE_SUPER,		NS_ULEVEL_OPER},
#endif
#ifdef UMODE_SRA
	{UMODE_SRA,			NS_ULEVEL_ROOT},
#endif
	{UMODE_SERVICES,	NS_ULEVEL_ROOT},
	{UMODE_NETADMIN,	NS_ULEVEL_ADMIN},
	{UMODE_SADMIN,		NS_ULEVEL_ADMIN},
	{UMODE_ADMIN,		NS_ULEVEL_OPER},
	{UMODE_COADMIN,		NS_ULEVEL_OPER},
	{UMODE_OPER,		NS_ULEVEL_OPER},
	{UMODE_LOCOP,		NS_ULEVEL_LOCOPER},
	{UMODE_REGNICK,		NS_ULEVEL_REG},
};

const int user_auth_mode_count = ((sizeof (user_auth_modes) / sizeof (user_auth_modes[0])));

UserAuthModes user_auth_smodes[] = {
	{SMODE_NETADMIN,	NS_ULEVEL_ADMIN},
	{SMODE_CONETADMIN,	175},
	{SMODE_TECHADMIN,	150},
	{SMODE_COTECHADMIN,	125},
	{SMODE_ADMIN,		100},
	{SMODE_GUESTADMIN,	100},
	{SMODE_COADMIN,		NS_ULEVEL_OPER},
};

const int user_auth_smode_count = ((sizeof (user_auth_smodes) / sizeof (user_auth_smodes[0])));

static int auth_cmd_authmodelist(CmdParams* cmdparams)
{
	int i;

	irc_prefmsg (NULL, cmdparams->source, 
		"User mode auth levels:");
	for (i = 0; i < user_auth_mode_count; i++) {
		irc_prefmsg (NULL, cmdparams->source, "%s: %d", 
			GetUmodeDesc (user_auth_modes[i].umode), user_auth_modes[i].level);
	}
	if (HaveFeature (FEATURE_USERSMODES)) {
		for (i = 0; i < user_auth_smode_count; i++) {
			irc_prefmsg (NULL, cmdparams->source, "%s: %d", 
				GetSmodeDesc (user_auth_smodes[i].umode), user_auth_smodes[i].level);
		}
	}
	return NS_SUCCESS;
}

static int auth_event_online(CmdParams* cmdparams)
{
	add_services_cmd_list(auth_commands);
	return NS_SUCCESS;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	auth_event_online},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* modptr)
{
	return NS_SUCCESS;
}

void ModFini()
{
	del_services_cmd_list(auth_commands);
}

int ModAuthUser(Client * u)
{
	int i, authlevel;

	/* Check umodes */
	authlevel = 0;
	for (i = 0; i < user_auth_mode_count; i++) {
		if (u->user->Umode & user_auth_modes[i].umode) {
			if(user_auth_modes[i].level > authlevel) {
				authlevel = user_auth_modes[i].level;
			}
		}
	}
	dlog(DEBUG1, "UmodeAuth: umode level for %s is %d", u->name, authlevel);
	if (HaveFeature (FEATURE_USERSMODES)) {
		/* Check smodes */
		for (i = 0; i < user_auth_smode_count; i++) {
			if (u->user->Smode & user_auth_smodes[i].umode) {
				if(user_auth_smodes[i].level > authlevel) {
					authlevel = user_auth_smodes[i].level;
				}
			}
		}
		dlog(DEBUG1, "UmodeAuth: smode level for %s is %d", u->name, authlevel);
	}
	/* Return new level */
	return authlevel;
}
