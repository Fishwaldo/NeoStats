/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
** $Id: auth.h 1964 2004-07-27 12:44:20Z Mark $
*/

#include "neostats.h"
#include "rta.h"

COLDEF neo_banscols[] = {
	{
		"bans",
		"type",
		RTA_STR,
		8,
		offsetof(struct Ban, type),
		RTA_READONLY,
		NULL,
		NULL,
		"type"
	},
	{
		"bans",
		"user",
		RTA_STR,
		MAXUSER,
		offsetof(struct Ban, user),
		RTA_READONLY,
		NULL,
		NULL,
		"user"
	},
	{
		"bans",
		"host",
		RTA_STR,
		MAXHOST,
		offsetof(struct Ban, host),
		RTA_READONLY,
		NULL,
		NULL,
		"host"
	},
	{
		"bans",
		"mask",
		RTA_STR,
		MAXHOST,
		offsetof(struct Ban, mask),
		RTA_READONLY,
		NULL,
		NULL,
		"mask"
	},
	{
		"bans",
		"reason",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Ban, reason),
		RTA_READONLY,
		NULL,
		NULL,
		"reason"
	},
	{
		"bans",
		"setby",
		RTA_STR,
		MAXHOST,
		offsetof(struct Ban, setby),
		RTA_READONLY,
		NULL,
		NULL,
		"setby"
	},
	{
		"bans",
		"tsset",
		RTA_INT,
		sizeof(int),
		offsetof(struct Ban, tsset),
		RTA_READONLY,
		NULL,
		NULL,
		"tsset"
	},
	{
		"bans",
		"tsexpire",
		RTA_INT,
		sizeof(int),
		offsetof(struct Ban, tsexpires),
		RTA_READONLY,
		NULL,
		NULL,
		"tsexpire"
	},
};

TBLDEF neo_bans = {
	"bans",
	NULL, 	/* for now */
	sizeof(struct Ban),
	0,
	TBL_HASH,
	neo_banscols,
	sizeof(neo_banscols) / sizeof(COLDEF),
	"",
	"The list of bans on the IRC network"
};
