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

#include "stats.h"

const char *hs_help[] = {
	"The following commands can be used with HostServ",
	"",
	"    ABOUT       About HostServ",
	"    LOGIN       Login to HostServ",
	"    CHPASS      Change password for a vhost",
	"    BANS        List banned vhosts",
	NULL
};

const char *hs_user_help[] = {
	"",
	"Additional commands for Service Roots",
	"",
	"    ADD         Add a vhost",
	"    DEL         Delete a vhost",
	"    LIST        List vhosts",
	"    VIEW        Detailed vhost list",
	"    SET         HostServ configuration",
	"    LEVELS      Levels to manage HostServ",
	NULL
};

const char *hs_help_on_help[] = {
	"",
	"To use a command, type:",
	"    \2/msg HostServ command\2",
	"For more information on a command, type:", 
	"    \2/msg HostServ HELP command\2.",
	NULL
};

const char *hs_help_about[] = {
	"\2HostServ\2 allows users to use their own unique host",
	"while on the Network. IRC Operators add them to the",
	"database and upon connection the user gets their vhost.",
	"",
	"If you find your host is not working, it could have been",
	"removed due to abuse, or the fact you are connecting from",
	"a different ISP or have a numerical address.",
	"Contact an admin.",
	NULL
};

const char *hs_help_add[] = {
	"Syntax: \2ADD <NICK> <HOSTNAME> <VHOST NAME> <PASSWORD>\2",
	"",
	"Register a host name. e.g. my-host.com DO NOT INCLUDE AN @.",
	"HOSTNAME must be where the user is connecting from WITHOUT",
	"THE @. HOSTNAME can include wildcards e.g. *.myhost.com",
	"Users can also get their VHOST by typing the command:",
	"    \2/msg HostServ LOGIN nick password\2",
	"This allows them to use the VHOST from any host and for",
	"multiple users to share one VHOST",
	NULL
};

const char *hs_help_del[] = {
	"Syntax: \2DEL <ACCESS LIST #>\2",
	"",
	"Delete a VHOST.",
	"Use \2LIST\2 to find the number to use in this command",
	NULL
};

const char *hs_help_view[] = {
	"Syntax: \2VIEW <ACCESS LIST #>\2",
	"",
	"View Detailed information about a VHOST.",
	"Use \2LIST\2 to find the number to use in this command",
	NULL
};

const char *hs_help_list[] = {
	"Syntax: \2LIST <startpos>\2",
	"",
	"Lists the people and vhosts in the database",
	"For more descriptive info on a vhost see \2HELP VIEW\2",
	"",
	"If you supply a value for startpos (optional) the list",
	"will start at that position",
	NULL
};

const char *hs_help_login[] = {
	"Syntax: \2LOGIN <NICK> <PASSWORD>\2",
	"",
	"Login to HostServ with your NICK and PASSWORD and your",
	"vhost will be assigned to your nick on successful login.",
	NULL
};

const char *hs_help_chpass[] = {
	"Syntax: \2CHPASS <NICK> <OLDPASS> <NEWPASS>\2",
	"",
	"Use this command to change the password assigned to your",
	"vhost account. You must supply your current password and",
	"a valid account for it to be successful",
	NULL
};

const char *hs_help_listban[] = {
	"Syntax: \2BANS\2",
	"        \2BANS ADD <hostname>\2",
	"        \2BANS DEL <index>\2",
	"",
	"Maintain the list of banned vhosts.",
	"\2BANS\2 lists current vhosts that are banned by network",
	"administration.",
	"",
	"Service Roots may also add a banned vhost to the list using"
	"ADD. Wildcards, like *fbi* are permitted.",
	"",
	"DEL will delete a banned vhost with ID number index",
	"Use \2BANS\2 to find the index",
	NULL
};

const char *hs_help_levels[] = {
	"Syntax: \2LEVELS\2",
	"        \2LEVELS ADD <level>\2",
	"        \2LEVELS LIST <level>\2",
	"        \2LEVELS DEL <level>\2",
	"        \2LEVELS VIEW <level>\2",
	"        \2LEVELS RESET\2",
	"",
	"\2LEVELS\2 without any parameters will lists the levels",
	"required to perform certain HostServ functions.",
	"",
	"Service Roots can modify these levels.",
	"LEVEL must be between 1 and 200",
	"RESET will restore the levels to original settings.",
	NULL
};

const char *hs_help_set[] = {
	"Syntax: \2SET LIST\2",
	"        \2SET EXPIRE <TIME>\2",
	"",
	"\2SET LIST\2",
	"This lists the current settings for HostServ.",
	"",
	"\2SET EXPIRE <TIME>\2",
	"How long before unused HostServ entries should be",
	"automatically deleted.",
	NULL
};
