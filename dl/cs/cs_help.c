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
** $Id: cs_help.c,v 1.8 2003/05/26 09:18:29 fishwaldo Exp $
*/

const char *cs_help[] = {
"\2ConnectServ HELP\2",
"",
"COMMANDS:",
"     SIGNWATCH     KILLWATCH     MODEWATCH      NICKWATCH",
"",
"     STATUS     ABOUT",
"",
NULL
};

const char *cs_help_about[] = {
"\2About ConnectServ\2",
"",
"ConnectServ is designed to echo the signing on/off of users,",
"killing of users and the modes that the operators are using.",
"These three echo types can be customised to be echoed to the",
"services channel or NOT to be echoed at all. Technical Admins",
"and Network Admins can turn these three settings on and off",
"",
NULL
};

const char *cs_help_status[] = {
"\2ConnectServ Help : STATUS\2",
"\2Usage:\2 STATUS",
"",
"This will tell you the current status of the settings. eg: MODEWATCH is enabled ot disabled",
"",
NULL
};

const char *cs_help_signwatch[] = {
"\2ConnectServ Help : SIGNWATCH",
"\2Usage:\2 SIGNWATCH",
"",
"Will enable or disable SIGNWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};

const char *cs_help_killwatch[] = {
"\2ConnectServ Help : KILLWATCH",
"\2Usage:\2 KILLWATCH",
"",
"Will enable or disable KILLWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};

const char *cs_help_modewatch[] = {
"\2ConnectServ Help : MODEWATCH",
"\2Usage:\2 MODEWATCH",
"",
"Will enable or disable MODEWATCH. It is reccomended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};

const char *cs_help_nickwatch[] = {
"\2ConnectServ Help : NICKWATCH",
"\2Usage:\2 NICKWATCH",
"",
"Will enable or disable NICKWATCH. It is recommended you use the STATUS",
"command to check if this setting is enabled or disabled before you use",
"this command.",
"",
NULL
};
