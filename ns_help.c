/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
** $Id: ns_help.c,v 1.8 2003/06/13 13:11:49 fishwaldo Exp $
*/

#include "stats.h"

void init_ns_help()
{
	if (usr_mds);
}


const char *ns_help[] = {
	"*** NeoStats Help ***",
	"",
	"Commands:",
	"\2INFO\2       Stats Info on NeoStats.",
	"\2VERSION\2    Shows you the current StatServ Version.",
	"\2LEVEL\2      Show you your Permission Level for NeoStats.",
	"",
	"End of Help.",
	NULL
};

const char *ns_myuser_help[] = {
	"",
	"*** Additional Commands For Service Roots!***",
	"",
	"\2SHUTDOWN\2   Shutdown NeoStats.",
	"\2RELOAD\2     Force NeoStats to Reload Itself.",
	"\2LOAD\2       Load a Module.",
	"\2UNLOAD\2     Unload a Module.",
	"\2MODLIST\2    List Loaded Modules.",
	"\2LOGS\2       View logfiles.",
	"\2RAW\2        Send a Raw Command from this Server!",
	"\2JUPE\2       Jupiter a Server",
	"End of Help",
	NULL
};


const char *ns_level_help[] = {
	"*** NeoStats: \2LEVEL\2 Help ***",
	"",
	"Usage: \2LEVEL\2",
	"",
	"Allows you to see your Permissions level with regards",
	"NeoStats. 0 is lowest, 200 is highest, you could be anywhere in between!",
	"",
	"End of Help.",
	NULL
};

const char *ns_jupe_help[] = {
	"*** NeoStats: \2JUPE\2 Help ***",
	"",
	"Usage: \2JUPE \37Server\37\2",
	"",
	"Allows you to Jupe a server on the network",
	"",
	"End of Help.",
	NULL
};

const char *ns_raw_help[] = {
	"*** NeoStats: \2RAW\2 Help ***",
	"",
	"Usage: \2RAW \37Command\37\2",
	"",
	"Allow you to send Raw IRC commands from this Server",
	"Nothing is Returned to the User after a Raw Command",
	"",
	"End of Help.",
	NULL
};

const char *ns_load_help[] = {
	"*** NeoStats: \2LOAD\2 Help ***",
	"",
	"Usage: \2LOAD \37module file name\37\2",
	"",
	"Allows you to Load a Module while NeoStats is running",
	"Some Modules can Not be loaded at Runtime, and will return a error",
	"",
	"End of Help.",
	NULL
};


const char *ns_unload_help[] = {
	"*** NeoStats: \2UNLOAD\2 Help ***",
	"",
	"Usage: \2UNLOAD \37module name\37\2",
	"",
	"Allows you to UnLoad a Module while NeoStats is running",
	"Some Modules can Not be Unloaded at Runtime, and will return a error",
	"",
	"End of Help.",
	NULL
};

const char *ns_modlist_help[] = {
	"*** NeoStats: \2MODLIST\2 Help ***",
	"",
	"Usage: \2MODLIST\2",
	"",
	"Will Display Module Names and Descriptions of Loaded Modules",
	"",
	"End of Help.",
	NULL
};


const char *ns_debug_help[] = {
	"*** NeoStats: \2DEBUG\2 Help ***",
	"",
	"Usage: \2DEBUG\2",
	"",
	"This Toggles the Debug Command, Introducing a New User on the Server called",
	"stats_debug. Any User that has their Coder Flag set will Recieve Debuging Information",
	"",
	"On a Large Network, this Command should be used considered Dangerous as a Large amount of Information may be sent to you.",
	"End of Help.",
	NULL
};

const char *ns_version_help[] = {
	"*** NeoStats: \2VERSION\2 Help ***",
	"Usage: \2VERSION\2",
	"",
	"Shows the current StatServ Version.",
	"",
	"End of Help.",
	NULL
};

const char *ns_shutdown_help[] = {
	"*** NeoStats: \2SHUTDOWN\2 Help ***",
	"Usage: \2SHUTDOWN\2 <REASON>",
	"",
	"Force NeoStats to exit immediately.",
	"This command should be used wisely.",
	"<REASON> is optional.",
	"",
	"End of Help.",
	NULL
};

const char *ns_reload_help[] = {
	"*** NeoStats: \2RELOAD\2 Help ***",
	"Usage: \2RELOAD\2 <REASON>",
	"",
	"Force NeoStats to Reload Itself.",
	"This command will cause StatServ to split from",
	"the network and reload datafiles and connect.",
	"This command SHOULD be used wisely.",
	"<REASON> is optional.",
	"",
	"End of Help.",
	NULL
};


const char *ns_userdump_help[] = {
	"*** NeoStats: \2UserDump\2 Help ***",
	"Usage: \2USERDUMP\2",
	"",
	"When in Debug Mode, Neostats will send Coders its Entire User table",
	"Only really usefull when you are debuging Neostats",
	"",
	"End of Help.",
	NULL
};


const char *ns_serverdump_help[] = {
	"*** NeoStats: \2ServerDump\2 Help ***",
	"Usage: \2SERVERDUMP\2",
	"",
	"When in Debug Mode, Neostats will send Coders its Entire SERVER table",
	"Only really usefull when you are debuging Neostats",
	"",
	"End of Help.",
	NULL
};


const char *ns_chandump_help[] = {
	"*** NeoStats: \2ChanDump\2 Help ***",
	"Usage: \2CHANDUMP\2",
	"",
	"When in Debug Mode, Neostats will send Coders its Entire CHANNEL table",
	"Only really usefull when you are debuging Neostats",
	"",
	"End of Help.",
	NULL
};

const char *ns_logs_help[] = {
	"*** NeoStats: \2LOGS\2 Help ***",
	"Usage: \2LOGS\2",
	"",
	"Sends today's logfile via PRIVMSG/NOTICE",
	"",
	"End of Help.",
	NULL
};

const char *ns_join_help[] = {
	"*** NeoStats: \2JOIN\2 Help ***",
	"Usage: \2JOIN <Channel>\2",
	"",
	"Get StatServ to Join a Channel",
	"It will then Echo Events as they happen to that Channel, So it Shouldn't Join Public Channels",
	"",
	"End of Help.",
	NULL
};
