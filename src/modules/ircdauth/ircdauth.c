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
#include "services.h"

static int auth_cmd_authmodelist(CmdParams* cmdparams);

typedef struct UserAuthModes{
	char* modename;
	unsigned long umode;
	int level;
} UserAuthModes;

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
	0,
	0,
};

UserAuthModes user_auth_modes[] = {
	{"Services",		UMODE_SERVICES,	NS_ULEVEL_ROOT},
	{"Network admin",	UMODE_NETADMIN,	NS_ULEVEL_ADMIN},
	{"Services admin",	UMODE_SADMIN,	NS_ULEVEL_ADMIN},
	{"Server admin",	UMODE_ADMIN,	NS_ULEVEL_OPER},
	{"Co-admin",		UMODE_COADMIN,	NS_ULEVEL_OPER},
	{"IRC operator",	UMODE_OPER,		NS_ULEVEL_OPER},
	{"Local operator",	UMODE_LOCOP,	NS_ULEVEL_LOCOPER},
	{"Registered nick",	UMODE_REGNICK,	NS_ULEVEL_REG},
};

const int user_auth_mode_count = ((sizeof (user_auth_modes) / sizeof (user_auth_modes[0])));

#ifdef GOTUSERSMODES
UserAuthModes user_auth_smodes[] = {
	{"Network admin",	SMODE_NETADMIN, 190},
/*	{SMODE_CONET, 175},
	{SMODE_TECHADMIN, 150},
	{SMODE_COTECH, 125},
	{SMODE_SERVADMIN, 100},
	{SMODE_GUEST, 100},
	{SMODE_COADMIN, 75},*/
};

const int user_auth_smode_count = ((sizeof (user_auth_smodes) / sizeof (user_auth_smodes[0])));
#endif

static int auth_cmd_authmodelist(CmdParams* cmdparams)
{
	int i;

	prefmsg(cmdparams->source.user->nick, ns_botptr->nick,
		"User mode auth levels:");
	for (i = 0; i < user_auth_mode_count; i++) {
		prefmsg(cmdparams->source.user->nick, ns_botptr->nick, "%s: %d", 
			user_auth_modes[i].modename, user_auth_modes[i].level);
	}
	return 1;
}

static int auth_event_online(CmdParams* cmdparams)
{
	add_services_cmd_list(auth_commands);
	return 1;
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE,	auth_event_online},
	{EVENT_NULL,	NULL}
};

int ModInit(Module* modptr)
{
	return 1;
}

void ModFini()
{
	del_services_cmd_list(auth_commands);
}

int ModAuthUser(User * u, int curlvl)
{
	int i, tmplvl;

	/* Check umodes */
	tmplvl = 0;
	for (i = 0; i < user_auth_mode_count; i++) {
		if(user_auth_modes[i].level == 0)
			break;
		if (u->Umode & user_auth_modes[i].umode) {
			tmplvl = user_auth_modes[i].level;
			break;
		}
	}
	nlog (LOG_DEBUG1, "UmodeAuth: umode level for %s is %d", u->nick, tmplvl);
	if(tmplvl > curlvl)
		curlvl = tmplvl;
#ifdef GOTUSERSMODES
	/* Check smodes */
	tmplvl = 0;
	for (i = 0; i < user_auth_mode_count; i++) {
		if(user_auth_modes[i].level == 0)
			break;
		if (u->Smode & user_auth_modes[i].umode) {
			tmplvl = user_auth_modes[i].level;
			break;
		}
	}
	nlog (LOG_DEBUG1, "UmodeAuth: smode level for %s is %d", u->nick, tmplvl);
	if(tmplvl > curlvl)
		curlvl = tmplvl;
#endif
	/* Return new level */
	return curlvl;
}

int ModAuthList(User * u)
{
	return 1;
}
