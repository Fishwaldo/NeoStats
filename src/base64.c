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
#include "neostats.h"
#include "ircd.h"
#include "servers.h"
#include "users.h"

/** @brief setserverbase64
 *
 *  @return none
 */
void setserverbase64 (const char *name, const char* num)
{
	Client *s;

	s = find_server(name);
	if (s) {
		dlog (DEBUG1, "setserverbase64: setting %s to %s", name, num);
		strlcpy(s->name64, num, 6);
	} else {
		dlog (DEBUG1, "setserverbase64: cannot find %s for %s", name, num);
	}
}

/** @brief servertobase64
 *
 *  @return base64
 */
char *servertobase64 (const char* name)
{
	Client *s;

	dlog (DEBUG1, "servertobase64: scanning for %s", name);
	s = find_server(name);
	if (s) {
		return s->name64;
	} else {
		dlog (DEBUG1, "servertobase64: cannot find %s", name);
	}
	return NULL;
}

/** @brief base64toserver
 *
 *  @return server name
 */
char *base64toserver (const char* num)
{
	Client *s;

	dlog (DEBUG1, "base64toserver: scanning for %s", num);
	s = findserverbase64(num);
	if (s) {
		return s->name;
	} else {
		dlog (DEBUG1, "base64toserver: cannot find %s", num);
	}
	return NULL;
}

/** @brief setnickbase64
 *
 *  @return none
 */
void setnickbase64 (const char *nick, const char* num)
{
	Client *u;

	u = find_user(nick);
	if (u) {
		dlog (DEBUG1, "setnickbase64: setting %s to %s", nick, num);
		strlcpy(u->name64, num, B64SIZE);
	} else {
		dlog (DEBUG1, "setnickbase64: cannot find %s for %s", nick, num);
	}
}

/** @brief nicktobase64
 *
 *  @return base64
 */
char *nicktobase64 (const char* nick)
{
	Client *u;

	dlog (DEBUG1, "nicktobase64: scanning for %s", nick);
	u = find_user(nick);
	if (u) {
		return u->name64;
	} else {
		dlog (DEBUG1, "nicktobase64: cannot find %s", nick);
	}
	return NULL;
}

/** @brief base64tonick
 *
 *  @return nickname
 */
char *base64tonick (const char* num)
{
	Client *u;

	dlog (DEBUG1, "base64tonick: scanning for %s", num);
	u = finduserbase64(num);
	if (u) {
		return u->name;
	} else {
		dlog (DEBUG1, "base64tonick: cannot find %s", num);
	}
	return NULL;
}
