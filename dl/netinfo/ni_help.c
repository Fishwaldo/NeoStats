/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: ni_help.c,v 1.4 2002/09/04 08:40:28 fishwaldo Exp $
*/

const char *ni_help[] = {
"*** Netinfo Help ***",
"Commands:",
"\2Set\2       Set Specific Options up",
"\2Show\2      Show Specific Settings",
"\2Version\2   Shows you the current NetInfo Version.",
"End of Help.",
NULL
};

const char *ni_help_set[] = {
"*** Netinfo \2SET\2 Help ***",
"Commands:",
"\2AUTOJOIN <ON/OFF>\2:",
"       Toggles the Setting of your AutoJoin.",
"       The Administrator may setup Default Channels when you join",
"       Turning this off means that you will not be autojoined when you connect",
"       to the Network",
NULL
};
const char *ni_help_seto[] = {
"*** Administrator Commands:",
"\2MAP <ADD/DEL> <TLD> <CHANNEL> <MESSAGE>\2:",
"       Allows you to add or Delete AutoJoin TLD to Channel Mapping",
"       <TLD>           :either a IP, or Hostmask. e.g: *.au or 203.202.181.*",
"       <CHANNEL>       :The Channel to Join the User to",
"       <MESSAGE>       :The Message to Send to the User when they are AutoJoined",
NULL,
};

const char *ni_help_show[] = {
"*** Netinfo \2SHOW\2 Help ***",
"\2Show\2",
"       Shows your Current Settings,",
NULL
};	
const char *ni_help_showo[] = {
"*** Administrator Options:",
"\2Show map\2",
"       Shows you the Current TLD to Channel Mapping",
"\2Show ignore\2",
"       Shows you the Current Ignore List",
NULL
};


