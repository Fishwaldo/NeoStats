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

const char *qs_about[] = 
{
	"\2QuoteServ\2 is a Quote messaging service",
	NULL
};

const char *qs_help_add[] = {
	"Add a database",
	"Syntax: \2ADD <database>\2",
	"",
	"Register a database with quoteserv.",
	"<database> is the name of the database to load",
	NULL
};

const char *qs_help_del[] = {
	"Delete a database",
	"Syntax: \2DEL <database>\2",
	"",
	"Delete a database.",
	NULL
};

const char *qs_help_list[] = {
	"List databases",
	"Syntax: \2LIST\2",
	"",
	"Lists loaded databases.",
	NULL
};

const char *qs_help_quote[] = {
	"Fetch quote",
	"Syntax: \2QUOTE\2",
	"",
	"Get quote.",
	NULL
};

const char *help_set_signonquote[] = {
	"\2SIGNONQUOTE <ON|OFF>\2",
	"Whether quoteserv gives a quote to users when the join the network",
	NULL
};
