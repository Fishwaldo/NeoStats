/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

const char ns_help_oneline[]="Provides Help on Commands";
const char ns_help_level_oneline[]="Show permission level for NeoStats.";
const char ns_help_status_oneline[]="Stats info on NeoStats.";
const char ns_help_version_oneline[]="Show NeoStats version information.";
const char ns_help_shutdown_oneline[]="Shutdown NeoStats";
const char ns_help_reload_oneline[]="Force NeoStats to reload";
const char ns_help_logs_oneline[]="View logfiles";
const char ns_help_load_oneline[]="Load a module";
const char ns_help_unload_oneline[]="Unload a module";
const char ns_help_jupe_oneline[]="Jupiter a Server";
const char ns_help_exclude_oneline[]="Maintain Global Exclusion lists";
#ifdef USE_RAW
const char ns_help_raw_oneline[]="Send a raw command from this Server";
#endif
const char ns_help_debug_oneline[]="Toggles debug mode";
const char ns_help_botlist_oneline[]="List current module bots";
const char ns_help_socklist_oneline[]="List current module sockets";
const char ns_help_timerlist_oneline[]="List current module timers";
const char ns_help_botchanlist_oneline[]="List current module bot channels";
const char ns_help_modlist_oneline[]="List loaded modules";
const char ns_help_userdump_oneline[]="Debug user table";
const char ns_help_chandump_oneline[]="Debug channel table";
const char ns_help_serverdump_oneline[]="Debug server table";

const char *ns_help_level[] = {
	"Syntax: \2LEVEL [nick]\2",
	"",
	"Show your permission level for NeoStats.",
	"This may range from 0 (lowest) to 200 (highest).",
	"Optional nick parameter allows you to see the level",
	"for another user",
	NULL
};

const char *ns_help_jupe[] = {
	"Syntax: \2JUPE \37Server\37\2",
	"",
	"Cause NeoStats to jupiter a server; i.e. create a fake",
	"\"server\" connected to the NeoStats host which prevents",
	"any real server of that name from connecting.",
	"To remove the jupe use the IRCD \2/SQUIT\2 command.",
	NULL
};

const char *ns_help_exclude[] = {
	"Syntax: \2EXCLUDE \37<ADD> <HOST/SERVER/CHAN> <pattern>\37\2",
	"        \2EXCLUDE \37<DEL> <position>\37\2",
	"        \2EXCLUDE \37<LIST>\37\2",
	"",
	"This command is to maintain the global exclusion lists, which the NeoStats",
	"modules can take advantage of, and means you only have to maintain",
	"one exclusion list, rather than many exclusion lists",
	"The Syntax is:",
	"\2EXCLUDE ADD <HOST/SERVER/CHAN> <pattern>\2",
	"Add a new exclusion to the list, matching either a hostname of a client",
	"or a servername, or a channel name",
	"\2EXCLUDE DEL <position>\2",
	"Delete a entry from the exclusion list.",
	"Deleting a entry will only take effect for newly connected clients or created channels",
	"\2EXCLUDE LIST\2",
	"List the current Exclusions that are active",
	"",
	"\2Please Note:\2",
	"Not all modules may take advantage of the global exclusion lists. Currently, all core",
	"modules do, but you should consult the documentation of any 3rd party module you use",
	"to see if it supports the global exlusions",
	NULL
};

#ifdef USE_RAW
const char *ns_help_raw[] = {
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

const char *ns_help_load[] = {
	"Syntax: \2LOAD \37module file name\37\2",
	"",
	"Load a module while NeoStats is running. Some modules",
	"cannot be loaded at runtime, and will return an error.",
	NULL
};

const char *ns_help_unload[] = {
	"Syntax: \2UNLOAD \37module name\37\2",
	"",
	"Unload a module while NeoStats is running. Some modules",
	"cannot be unloaded at runtime, and will return an error.",
	NULL
};

const char *ns_help_modlist[] = {
	"Syntax: \2MODLIST\2",
	"",
	"Display module names and descriptions of all loaded",
	"modules.",
	NULL
};

const char *ns_help_debug[] = {
	"Syntax: \2DEBUG <ON|OFF>\2",
	"",
	"Toggles debug mode. When enabled, debugging information is",
	"sent to the services channel.",
	"",
	"Beware, on a large network, this command will generate a",
	"large amount of information.",
	NULL
};

const char *ns_help_version[] = {
	"Syntax: \2VERSION\2",
	"",
	"Shows the current NeoStats version.",
	NULL
};

const char *ns_help_shutdown[] = {
	"Syntax: \2SHUTDOWN <reason>\2",
	"",
	"Cause NeoStats to save data files and exit immediately.",
	"The reason provided will be broadcast to the services",
	"channel and other operators on the network.",
	"This command should be used wisely.",
	NULL
};

const char *ns_help_reload[] = {
	"Syntax: \2RELOAD <reason>\2",
	"",
	"Cause NeoStats to leave the network, reload datafiles,",
	"then reconnect to the network.",
	"The reason provided will be broadcast to the services",
	"channel and other operators on the network.",
	"This command SHOULD be used wisely.",
	NULL
};

const char *ns_help_userdump[] = {
	"Syntax: \2USERDUMP\2",
	"Syntax: \2USERDUMP <nick>\2",
	"",
	"When in debug mode, Neostats will echo its user table to",
	"the services channel. Only useful for debugging Neostats",
	"If nick is passed, only the information of that nick is",
	"returned, otherwise the entire user list is dumped.",
	NULL
};

const char *ns_help_serverdump[] = {
	"Syntax: \2SERVERDUMP\2",
	"Syntax: \2SERVERDUMP <name>\2",
	"",
	"When in debug mode, Neostats will echo its server table to",
	"the services channel. Only useful for debugging Neostats",
	"If name is passed, only the information for that server is",
	"returned, otherwise the entire server list is dumped.",
	NULL
};

const char *ns_help_chandump[] = {
	"Syntax: \2CHANDUMP\2",
	"Syntax: \2CHANDUMP <channel>\2",
	"",
	"When in debug mode, Neostats will echo its channel table to",
	"the services channel. Only useful for debugging Neostats",
	"If channel is passed, only the information of that nick is",
	"returned, otherwise the entire channel list is dumped.",
	NULL
};

const char *ns_help_logs[] = {
	"Syntax: \2LOGS\2",
	"",
	"Sends today's logfile via PRIVMSG/NOTICE",
	NULL
};

const char *ns_help_botlist[] = {
	"Syntax: \2BOTLIST\2",
	"",
	"NeoStats will send you by notice a list of the current bots",
	"being used on the network for each module.",
	NULL
};

const char *ns_help_socklist[] = {
	"Syntax: \2SOCKLIST\2",
	"",
	"NeoStats will send you by notice a list of the current",
	"sockets being used on the network for each module.",	
	NULL
};

const char *ns_help_timerlist[] = {
	"Syntax: \2TIMERLIST\2",
	"",
	"NeoStats will send you by notice a list of the current",
	"timer functions being used on the network by each module.",	
	NULL
};

const char *ns_help_botchanlist[] = {
	"Syntax: \2BOTCHANLIST\2",
	"",
	"NeoStats will send you by notice a list of the current bots",
	"and the channels they are using for each module.",
	NULL
};

const char *ns_help_status[] = {
	"Syntax: \2STATUS\2",
	"",
	"Display info about NeoStats uptime and other stats.",
	NULL
};

const char *ns_help_set_nick[] = {
	"NICK <newnick> Change bot nickname",
	"(requires restart to take effect).",
	NULL
};

const char *ns_help_set_user[] = {
	"USER <username> Change bot username",
	"(requires restart to take effect).",
	NULL
};

const char *ns_help_set_host[] = {
	"HOST <host> Change bot host",
	"(requires restart to take effect).",
	NULL
};

const char *ns_help_set_realname[] = {
	"REALNAME <realname> Change bot realname",
	"(requires restart to take effect).",
	NULL
};

