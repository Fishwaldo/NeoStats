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
** $Id: hs_help.c,v 1.5 2002/09/04 08:40:28 fishwaldo Exp $
*/

#include "stats.h"

const char *hs_help[] = {
"\2HostServ HELP\2",
"",
"COMMANDS:",
"     ABOUT     ADD     DEL     LIST     VIEW",
"",
"Only Network Admins can use the ADD, DEL & LIST functions",
"",
NULL
};

const char *hs_help_about[] = {
"\2About HostServ\2",
"",
"HostServ is designed to let users use their own unique host",
"While on the Network. IRC Operators add them to the database",
"And upon connection the user gets their unique host.",
"",
"If you find your host is not working, it could have been removed",
"Due to abuse, or the fact you are connecting from a different",
"Internet Provider or have a numerical address. Contact an admin",
"",
NULL
};

const char *hs_help_add[] = {
"\2HostServ Help : ADD\2",
"\2Usage:\2 ADD <NICK> <HOST NAME> <VIRTUAL HOST NAME>",
"",
"Register a host name to be set. eg: my-host.com 4DO NOT INCLUDE AN @",
"The <HOST NAME> must be where the user is connecting from 4WITHOUT THE @",
"HostServ supports wildcards such as *.myhost.com in the <HOST NAME> setting", 
"",
NULL
};

const char *hs_help_del[] = {
"\2HostServ Help : DEL\2",
"\2Usage:\2 DEL <ACCESS LIST #>",
"",
"The information needed for this is in the LIST command.  To delete Number 3",
"use DEL 3 ... its that easy!",
"",
NULL
};

const char *hs_help_view[] = {
"\2HostServ Help : VIEW",
"\2Usage:\2 VIEW <ACCESS LIST #>",
"",
"View Detailed information about the user on the access list # you selected",
"",
NULL
};

const char *hs_help_list[] = {
"\2HostServ Help : LIST",
"\2Usage:\2 LIST",
"",
"Lists the people and vhosts in the Database",
"For more descriptive info on a SINGLE vhost see HELP VIEW",
"",
NULL
};
