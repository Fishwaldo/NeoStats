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

const char *cs_help[] = {
	"The following commands can be used with ConnectServ",
	"",
	"    ABOUT       About ConnectServ",
	"    VERSION     Display version info",
	"    SET         Configure ConnectServ",
	NULL
};

const char cs_help_about_oneline[] = "About ConnectServ";
const char cs_help_version_oneline[] = "Display version info";
const char cs_help_status_oneline[] = "Configure ConnectServ";


const char *cs_help_on_help[] = {
	"",
	"To use a command, type",
	"    \2/msg ConnectServ command\2",
	"For for more information on a command, type", 
	"    \2/msg ConnectServ HELP command\2.",
	NULL
};

const char *cs_help_about[] = {
	"\2ConnectServ\2 is designed to track users signing on and",
	"off, killing of users, modes that the operators are using",
	"and nickname changes. These events can be optionally",
	"echoed to the services channel.",
	NULL
};

const char *cs_help_version[] = {
	"Syntax: \2VERSION\2",
	"",
	"Show ConnectServ version information",
	NULL
};

const char *cs_help_status[] = {
	"Syntax: \2SET LIST\2",
	"        \2SET SIGNWATCH <ON|OFF>\2",
	"        \2SET KILLWATCH <ON|OFF>\2",
	"        \2SET MODEWATCH <ON|OFF>\2",
	"        \2SET NICKWATCH <ON|OFF>\2",
	"",
	"LIST will show the current settings.",
	"SIGNWATCH configures whether to echo signon/signoff",
	"events to the services channel.",
	"KILLWATCH configures whether to echo kill",
	"events to the services channel.",
	"MODEWATCH configures whether to echo operator modes",
	"events to the services channel.",
	"NICKWATCH configures whether to echo nick changes",
	"events to the services channel.",
	NULL
};
