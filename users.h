/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _USERS_H_
#define _USERS_H_

void AddUser (const char *nick, const char *user, const char *host, const char *server, const unsigned long ip, const unsigned long TS);
void DelUser (const char *nick);
void AddRealName (const char *nick, const char *realname);
void Change_User (User *u, const char * newnick);
void UserDump (char *nick);
void part_u_chan (list_t *list, lnode_t *node, void *v);
void UserMode (const char *nick, const char *modes, int smode);
int init_user_hash (void);
void UserAway (User *u, const char *awaymsg);
void KillUser (const char *nick);

#endif /* _USERS_H_ */
