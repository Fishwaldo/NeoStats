/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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
#ifndef IRCD_H
#define IRCD_H

#define MAX_CMD_LINE_LENGTH		350

typedef struct {
	char *name;
	void (*function) (char *origin, char **argv, int argc);
	int srvmsg;		/* Should this be a Server Message(1), or a User Message?(0) */
	int usage;
}IntCommands;

extern IntCommands cmd_list[];

void init_ServBot (void);
void ShowMOTD (char *nick);
void ShowADMIN (char *nick);
void Showcredits (char *nick);
void ShowStats (char * what, User *u);
void dopong (Server *s);

/* Defined in ircd specific files but common to all */
int SignOn_NewBot (const char *nick, const char *user, const char *host, const char *rname, long Umode);

#endif
