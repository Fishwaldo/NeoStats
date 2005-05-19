/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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

const char *ls_about[] = 
{
	"\2TextServ\2 is a text messaging service",
	NULL
};

const char ls_help_add_oneline[] ="Add a channel";
const char ls_help_del_oneline[] ="Delete a channel";
const char ls_help_list_oneline[] ="List channels";

const char *ls_help_add[] = {
	"Syntax: \2ADD <channel>\2",
	"",
	"Register a channel with limitserv.",
	"<channel> is the name of the channel",
	NULL
};

const char *ls_help_del[] = {
	"Syntax: \2DEL <channel>\2",
	"",
	"Delete a channel.",
	NULL
};

const char *ls_help_list[] = {
	"Syntax: \2LIST\2",
	"",
	"Lists channels.",
	NULL
};
