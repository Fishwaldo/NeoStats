/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include <stdio.h>

const char hs_help_login_oneline[] ="Login to HostServ";
const char hs_help_chpass_oneline[] ="Change password for a vhost";
const char hs_help_bans_oneline[] ="List banned vhosts";
const char hs_help_add_oneline[] ="Add a vhost";
const char hs_help_del_oneline[] ="Delete a vhost";
const char hs_help_list_oneline[] ="List vhosts";
const char hs_help_view_oneline[] ="Detailed vhost list";
const char hs_help_levels_oneline[] ="Levels to manage HostServ";

const char *hs_about[] = {
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
	"Syntax: \2ADD <NICK> <HOSTNAME> <VHOST> <PASSWORD>\2",
	"",
	"Register a vhost with hostserv. e.g. my-host.com.",
	"HOSTNAME must be where the user is connecting from",
	"\2without the @\2",
	"HOSTNAME can include wildcards e.g. *.myhost.com",
	"Users can also get their vhost by typing the command:",
	"    \2/msg HostServ LOGIN nick password\2",
	"This allows them to use the vhost from any host and for",
	"multiple users to share one vhost",
	NULL
};

const char *hs_help_del[] = {
	"Syntax: \2DEL <ACCESS LIST #>\2",
	"",
	"Delete a vhost.",
	"Use \2LIST\2 to find the number to use in this command",
	NULL
};

const char *hs_help_view[] = {
	"Syntax: \2VIEW <ACCESS LIST #>\2",
	"",
	"View Detailed information about a vhost.",
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

const char *hs_help_bans[] = {
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

const char *hs_help_set_expire[] = {
	"\2SET EXPIRE <TIME>\2",
	"How long before unused HostServ entries should be",
	"automatically deleted. A value of 0 makes all vhosts",
	"permanent",
	NULL
};

const char *hs_help_set_hiddenhost[] = {
	"\2SET HIDDENHOST <ON/OFF>\2",
	"Turns on undernet style hidden hosts when users identify to nickserv.",
	"You will also need to set HOSTNAME to the host you want to use",
	NULL
};

const char *hs_help_set_hostname[] = {
	"\2SET HOSTNAME <hostname>\2",
	"The hidden host you want to set on users.",
	"Users will then be set to nick.<hostname>.",
	NULL
};

const char *hs_help_set_operhosts[] = {
	"\2SET OPERHOSTS <ON/OFF>\2",
	"Whether HostServ will set oper vhosts or not. If your IRCd does",
	"not provide oper hosts, you might want to use this option.",
	NULL
};
