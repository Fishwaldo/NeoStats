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
** $Id$
*/

/* its huge, because we can have a *LOT* of users in a channel */

#include "neostats.h"
#include "rta.h"

static char chanusers[BUFSIZE*10];
void *display_chanusers (void *tbl, char *col, char *sql, void *row) 
{
	Channel *c = row;
	lnode_t *cmn;
	char final[BUFSIZE*2];
 	Chanmem *cm;
	
	chanusers[0] = '\0';
	cmn = list_first (c->chanmembers);
	while (cmn) {
		cm = lnode_get (cmn);
		ircsnprintf(final, BUFSIZE*2, "+%s %s%s,", CmodeMaskToString (cm->flags), CmodeMaskToPrefixString (cm->flags), cm->nick);
		strlcat(chanusers, final, BUFSIZE*10);
		cmn = list_next (c->chanmembers, cmn);
	}
	return chanusers;
}

/* display the channel modes */
/* BUFSIZE is probably too small.. oh well */
static char chanmodes[BUFSIZE];

void *display_chanmodes (void *tbl, char *col, char *sql, void *row) 
{
	Channel *c = row;
	lnode_t *cmn;
	char tmp[BUFSIZE];
	ModesParm *m;
	
	strlcpy (chanmodes, CmodeMaskToString (c->modes), BUFSIZE);
	cmn = list_first (c->modeparms);
	while (cmn) {
		m = lnode_get (cmn);	
		ircsnprintf(tmp, BUFSIZE, " +%c %s", CmodeMaskToChar (m->mask), m->param);
		strlcat(chanmodes, tmp, BUFSIZE);
		cmn = list_next (c->modeparms, cmn);
	}
	return chanmodes;
}

COLDEF neo_chanscols[] = {
	{
		"chans",
		"name",
		RTA_STR,
		MAXCHANLEN,
		offsetof(struct Channel, name),
		RTA_READONLY,
		NULL,
		NULL,
		"The name of the channel"
	},
	{
		"chans",
		"nomems",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, users),
		RTA_READONLY,
		NULL, 
		NULL,
		"The no of users in the channel"
	},
	{
		"chans",
		"modes",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Channel, modes),
		RTA_READONLY,
		display_chanmodes,
		NULL,
		"The modes of the channel"
	},
	{
		"chans",
		"users",
		RTA_STR,
		BUFSIZE*10,
		offsetof(struct Channel, chanmembers),
		RTA_READONLY,
		display_chanusers,
		NULL,
		"The users of the channel"
	},
	{
		"chans",
		"topic",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Channel, topic),
		RTA_READONLY,
		NULL,
		NULL,
		"The topic of the channel"
	},
	{	
		"chans",
		"topicowner",
		RTA_STR,
		MAXHOST,
		offsetof(struct Channel, topicowner),
		RTA_READONLY,
		NULL,
		NULL,
		"Who set the topic"
	},
	{	
		"chans",
		"topictime",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, topictime),
		RTA_READONLY,
		NULL,
		NULL,
		"When the topic was set"
	},
	{	
		"chans",
		"created",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, creationtime),
		RTA_READONLY,
		NULL,
		NULL,
		"when the channel was created"
	},
	{	
		"chans",
		"flags",
		RTA_INT,
		sizeof(int),
		offsetof(struct Channel, flags),
		RTA_READONLY,
		NULL,
		NULL,
		"Flags for this channel"
	},

};

TBLDEF neo_chans = {
	"chans",
	NULL, 	/* for now */
	sizeof(struct Channel),
	0,
	TBL_HASH,
	neo_chanscols,
	sizeof(neo_chanscols) / sizeof(COLDEF),
	"",
	"The list of Channels on the IRC network"
};

