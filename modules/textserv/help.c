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

const char *ts_about[] = 
{
	"\2TextServ\2 is a text messaging service",
	NULL
};

const char ts_help_add_oneline[] ="Add a database";
const char ts_help_del_oneline[] ="Delete a database";
const char ts_help_list_oneline[] ="List databases";

const char *ts_help_add[] = {
	"Syntax: \2ADD <database> <nick> <channel>\2",
	"",
	"Register a database with textserv.",
	"<database> is the name of the database to load",
	"<nick> is the optional name of the bot you want to use. If not",
	"specified, the database name is used as the nick",
	"<channel> is the optional channel you want the bot to join. If not",
	"specified, the bot will not join a channel",
	NULL
};

const char *ts_help_del[] = {
	"Syntax: \2DEL <database>\2",
	"",
	"Delete a database.",
	NULL
};

const char *ts_help_list[] = {
	"Syntax: \2LIST\2",
	"",
	"Lists loaded databases.",
	NULL
};
