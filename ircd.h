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
** $Id: ircd.h,v 1.2 2003/06/26 05:14:16 fishwaldo Exp $
*/
#ifndef IRCD_H
#define IRCD_H

struct int_cmds {
	char *name;
	void (*function)(char *origin, char **argv, int argc);
	int srvmsg; /* Should this be a Server Message(1), or a User Message?(0) */
	int usage; 
} int_cmds;

typedef struct int_cmds IntCommands;




void ShowMOTD(char *);
void ShowADMIN(char *);
void Showcredits(char *);
void ShowStats(char *, User *);
void dopong(Server *);



#endif
