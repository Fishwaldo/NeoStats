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

#ifndef _USERS_H_
#define _USERS_H_

void AddUser (const char *nick, const char *user, const char *host, const char *realname, const char *server, const char*ip, const char* TS);
int UserNick (const char * oldnick, const char * newnick, const char * ts);
void UserDump (const char *nick);
void UserPart (list_t *list, lnode_t *node, void *v);
void SetUserVhost (const char* nick, const char* vhost);
void SetUserServicesTS (const char* nick, const char* ts);
void UserMode (const char *nick, const char *modes);
#ifdef GOTUSERSMODES
void UserSMode (const char *nick, const char *modes);
#endif
int init_user_hash (void);
void UserAway (const char *nick, const char *awaymsg);
void DelUser (const char *nick, int killflag, const char *reason);
int InitExtAuth(void);
void FreeUsers();
#endif /* _USERS_H_ */
