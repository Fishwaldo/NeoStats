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
	"\2ConnectServ HELP\2",
	"",
	"COMMANDS:",
	"HELP       - Online command help",
	"VERSION    - Shows the current ConnectServ Version",
	"SET        - Adjust settings for ConnectServ",
	"",
	NULL
};

const char *cs_help_about[] = {
	"\2About ConnectServ\2",
	"",
	"ConnectServ is designed to echo the signing on/off of users,",
	"killing of users, modes that the operators are using and",
	"nickname changes. These echo types can be customised to be",
	"echoed to the services channel or NOT to be echoed at all.",
	"",
	NULL
};

const char *cs_help_status[] = {
	"\2ConnectServ Help : SET \2",
	"\2Usage:\2 SET <OPTION>",
	"",
	"LIST",
	"This will tell you the current status of the settings.",
	"eg: MODEWATCH is enabled.",
	"SIGNWATCH <ON|OFF>",
	"echo signon/signoff events.",
	"KILLWATCH <ON|OFF>",
	"echo kill events.",
	"MODEWATCH <ON|OFF>",
	"echo operator mode changes.",
	"NICKWATCH <ON|OFF>",
	"echo nickname changes.",
	"",
	NULL
};
