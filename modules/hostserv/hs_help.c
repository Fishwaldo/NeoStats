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

#include "neostats.h"

const char hs_help_login_oneline[] ="Login to HostServ";
const char hs_help_chpass_oneline[] ="Change password for a vhost";
const char hs_help_bans_oneline[] ="List banned vhosts";
const char hs_help_add_oneline[] ="Add a vhost";
const char hs_help_del_oneline[] ="Delete a vhost";
const char hs_help_list_oneline[] ="List vhosts";
const char hs_help_view_oneline[] ="Detailed vhost list";

const char *hs_about[] = {
	"\2HostServ\2 allows users to have a unique virtual host",
	"while on the network. Virtual hosts are set automatically",
	"when you join the network or by using the LOGIN command",
	"",
	"If you find your host is not working, it may have been",
	"removed due to abuse and you should contact an operator.",
	NULL
};

const char *hs_help_add[] = {
	"Syntax: \2ADD <NICK> <HOSTNAME> <VHOST> <PASSWORD>\2",
	"",
	"Register a vhost with hostserv. e.g. vhost.com.",
	"HOSTNAME is the host the user is connecting from",
	"such as realhost.com and can include wildcards",
	"such as *.realhost.com.",
	"Users can also get their vhost with the LOGIN",
	"command. This allows them use of the vhost from"
	"multiple hosts or multiple users to share a vhost.",
	NULL
};

const char *hs_help_del[] = {
	"Syntax: \2DEL <nick>\2",
	"",
	"Delete the vhost for <nick>.",
	NULL
};

const char *hs_help_view[] = {
	"Syntax: \2VIEW <nick>\2",
	"",
	"View detailed information about the vhost for <nick>.",
	NULL
};

const char *hs_help_list[] = {
	"Syntax: \2LIST\2",
	"        \2LIST <startpos>\2",
	"",
	"Lists the current vhosts stored in the database.",
	"For more descriptive info on a vhost see \2HELP VIEW\2",
	"",
	"A maximum 20 vhosts are displayed at one time, in order",
	"to view other vhosts, the <startpos> parameter allows",
	"listing from that position",
	NULL
};

const char *hs_help_login[] = {
	"Syntax: \2LOGIN <NICK> <PASSWORD>\2",
	"",
	"Sets your vhost if it was not set when you connect to IRC,",
	"or you are connecting from a different host, or you share a.",
	"vhost with other users.",
	NULL
};

const char *hs_help_chpass[] = {
	"Syntax: \2CHPASS <NICK> <OLDPASS> <NEWPASS>\2",
	"",
	"Change the password for your vhost.",
	NULL
};

const char *hs_help_bans[] = {
	"Syntax: \2BANS LIST\2",
	"        \2BANS ADD <hostname> <reason>\2",
	"        \2BANS DEL <hostname>\2",
	"",
	"Maintain the list of banned vhosts.",
	"",
	"\2LIST\2 list banned vhosts.",
	"",
	"\2ADD\2 add a banned vhost.",
	"Wildcards, like *host* are permitted.",
	"",
	"\2DEL\2 delete a banned vhost.",
	"hostname must match the one used to add the vhost.",
	NULL
};

const char *hs_help_set_expire[] = {
	"\2EXPIRE <TIME>\2",
	"How long before unused HostServ entries should be",
	"automatically deleted. A value of 0 makes all vhosts",
	"permanent",
	NULL
};

const char *hs_help_set_hiddenhost[] = {
	"\2HIDDENHOST <ON/OFF>\2",
	"Turns on undernet style hidden hosts when users identify to nickserv.",
	"You will also need to set HOSTNAME to the host you want to use",
	NULL
};

const char *hs_help_set_hostname[] = {
	"\2HOSTNAME <hostname>\2",
	"The hidden host you want to set on users.",
	"Users will then be set to nick.<hostname>.",
	NULL
};

const char *hs_help_set_operhosts[] = {
	"\2OPERHOSTS <ON/OFF>\2",
	"Whether HostServ will set oper vhosts or not. If your IRCd does",
	"not provide oper hosts, you might want to use this option.",
	NULL
};

const char *hs_help_set_verbose[] = {
	"\2VERBOSE <ON/OFF>\2",
	"Control HostServ verbosity",
	NULL
};
