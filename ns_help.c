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
** $Id$
*/

#include "stats.h"

const char *ns_help[] = {
	"The following commands can be used with NeoStats",
	"",
	"    INFO           Stats info on NeoStats.",
	"    VERSION        Show NeoStats version information.",
	"    LEVEL          Show your permission level for NeoStats.",
	NULL
};

const char *ns_myuser_help[] = {
	"",
	"Additional commands for Service Roots",
	"",
	"    SHUTDOWN       Shutdown NeoStats",
	"    RELOAD         Force NeoStats to reload",
	"    LOAD           Load a module",
	"    UNLOAD         Unload a module",
	"    MODLIST        List loaded modules",
	"    LOGS           View logfiles",
#ifdef USE_RAW
	"    RAW            Send a raw command from this Server",
#endif
	"    JUPE           Jupiter a Server",
	"    DEBUG          Toggles debug mode",
	"    USERDUMP       Debug user table",
	"    CHANDUMP       Debug channel table",
	"    SERVERDUMP     Debug server table",
	"    MODBOTLIST     List of current module bots",
	"    MODSOCKLIST    List of current module sockets",
	"    MODTIMERLIST   List of current module timers",
	"    MODBOTCHANLIST List of current module bot channels",
	NULL
};

const char *ns_help_on_help[] = {
	"",
	"To use a command, type",
	"    \2/msg ConnectServ command\2",
	"For for more information on a command, type", 
	"    \2/msg ConnectServ HELP command\2.",
	NULL
};

const char *ns_level_help[] = {
	"Syntax: \2LEVEL\2",
	"",
	"Show your permission level for NeoStats.",
	"This may range from 0 (lowest) to 200 (highest).",
	NULL
};

const char *ns_jupe_help[] = {
	"Syntax: \2JUPE \37Server\37\2",
	"",
	"Cause NeoStats to jupiter a server; i.e. create a fake",
	"\"server\" connected to the NeoStats host which prevents",
	"any real server of that name from connecting.",
	"To remove the jupe use the IRCD \2/SQUIT\2 command.",
	NULL
};

#ifdef USE_RAW
const char *ns_raw_help[] = {
	"Syntax: \2RAW \37Command\37\2",
	"",
	"Sends a string of raw text directly to the server to which",
	"NeoStats is connected. Nothing is returned to the user",
	"after a raw command.",
	"Raw can cause a number of problems on the network and is",
	"used at your own risk. No support of any kind is provided",
	"for this command.",
	NULL
};
#endif

const char *ns_load_help[] = {
	"Syntax: \2LOAD \37module file name\37\2",
	"",
	"Load a module while NeoStats is running. Some modules",
	"cannot be loaded at runtime, and will return an error.",
	NULL
};

const char *ns_unload_help[] = {
	"Syntax: \2UNLOAD \37module name\37\2",
	"",
	"Unload a module while NeoStats is running. Some modules",
	"cannot be unloaded at runtime, and will return an error.",
	NULL
};

const char *ns_modlist_help[] = {
	"Syntax: \2MODLIST\2",
	"",
	"Display module names and descriptions of all loaded",
	"modules.",
	NULL
};

const char *ns_debug_help[] = {
	"Syntax: \2DEBUG\2",
	"",
	"Toggles debug mode. When enabled, debugging information is",
	"sent to the services channel.",
	"",
	"Beware, on a large network, this command will generate a",
	"large amount of information.",
	NULL
};

const char *ns_version_help[] = {
	"Syntax: \2VERSION\2",
	"",
	"Shows the current NeoStats version.",
	NULL
};

const char *ns_shutdown_help[] = {
	"Syntax: \2SHUTDOWN <reason>\2",
	"",
	"Cause NeoStats to save data files and exit immediately.",
	"The reason provided will be broadcast to the services",
	"channel and other operators on the network.",
	"This command should be used wisely.",
	NULL
};

const char *ns_reload_help[] = {
	"Syntax: \2RELOAD <reason>\2",
	"",
	"Cause NeoStats to leave the network, reload datafiles,",
	"then reconnect to the network.",
	"The reason provided will be broadcast to the services",
	"channel and other operators on the network.",
	"This command SHOULD be used wisely.",
	NULL
};

const char *ns_userdump_help[] = {
	"Syntax: \2USERDUMP\2",
	"Syntax: \2USERDUMP <nick>\2",
	"",
	"When in debug mode, Neostats will echo its user table to",
	"the services channel. Only useful for debugging Neostats",
	"If nick is passed, only the information of that nick is",
	"returned, otherwise the entire user list is dumped.",
	NULL
};

const char *ns_serverdump_help[] = {
	"Syntax: \2SERVERDUMP\2",
	"",
	"When in debug mode, Neostats will echo its server table to",
	"the services channel. Only useful for debugging Neostats",
	NULL
};

const char *ns_chandump_help[] = {
	"Syntax: \2CHANDUMP <channel>\2",
	"",
	"When in debug mode, Neostats will echo its channel table to",
	"the services channel. Only useful for debugging Neostats",
	"If channel is passed, only the information of that nick is",
	"returned, otherwise the entire channel list is dumped.",
	NULL
};

const char *ns_logs_help[] = {
	"Syntax: \2LOGS\2",
	"",
	"Sends today's logfile via PRIVMSG/NOTICE",
	NULL
};

const char *ns_modbotlist_help[] = {
	"Syntax: \2MODBOTLIST\2",
	"",
	"NeoStats will send you by notice a list of the current bots",
	"being used on the network for each module.",
	NULL
};

const char *ns_modsocklist_help[] = {
	"Syntax: \2MODSOCKLIST\2",
	"",
	"NeoStats will send you by notice a list of the current",
	"sockets being used on the network for each module.",	
	NULL
};

const char *ns_modtimerlist_help[] = {
	"Syntax: \2MODTIMERLIST\2",
	"",
	"NeoStats will send you by notice a list of the current",
	"timer functions being used on the network by each module.",	
	NULL
};

const char *ns_modbotchanlist_help[] = {
	"Syntax: \2MODBOTCHANLIST\2",
	"",
	"NeoStats will send you by notice a list of the current bots",
	"and the channels they are using for each module.",
	NULL
};

const char *ns_info_help[] = {
	"Syntax: \2INFO\2",
	"",
	"Display info about NeoStats uptime and other stats.",
	NULL
};
