/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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

/* List of events available to be sent to modules 
 * Once all modules are ported to use these defines,
 * events should be tokenised to improve performance
 */

#define EVENT_ONLINE	    "ONLINE"
#define EVENT_SIGNON		"SIGNON"
#define EVENT_SIGNOFF		"SIGNOFF"
#define EVENT_KILL			"KILL"
#define EVENT_BOTKILL		"BOTKILL"
#define EVENT_NEWSERVER		"NEWSERVER"
#define EVENT_DELSERVER		"DELSERVER"
#define EVENT_SQUIT			"SQUIT"
#define EVENT_NETINFO		"NETINFO"
#define EVENT_UMODE			"UMODE"
#define EVENT_SMODE			"SMODE"
#define EVENT_NICKCHANGE	"NICK_CHANGE"
#define EVENT_PONG			"PONG"
#define EVENT_AWAY			"AWAY"
#define EVENT_NEWCHAN		"NEWCHAN"
#define EVENT_DELCHAN		"DELCHAN"
#define EVENT_JOINCHAN		"JOINCHAN"
#define EVENT_PARTCHAN		"PARTCHAN"
#define EVENT_KICK			"KICK"
#define EVENT_KICKBOT		"KICKBOT"
#define EVENT_PARTBOT		"PARTBOT"
#define EVENT_TOPICCHANGE	"TOPICCHANGE"
#define EVENT_CLIENTVERSION	"CLIENTVERSION"
