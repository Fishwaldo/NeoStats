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

/*  ONLINE parameters are:
 *	server connected to
 */
#define EVENT_ONLINE	    "ONLINE"
/*  SIGNON parameters are:
 *	user nick
 */
#define EVENT_SIGNON		"SIGNON"
/*  SIGNOFF parameters are:
 *	user nick
 */
#define EVENT_SIGNOFF		"SIGNOFF"
/*  KILL parameters are:
 *	user nick
 */
#define EVENT_KILL			"KILL"
/*  BOTKILL parameters are:
 *	
 */
#define EVENT_BOTKILL		"BOTKILL"
/*  NEWSERVER parameters are:
 *	server name
 */
#define EVENT_NEWSERVER		"NEWSERVER"
/*  DELSERVER parameters are:
 *	
 */
#define EVENT_DELSERVER		"DELSERVER"
/*  SQUIT parameters are:
 *	server name
 */
#define EVENT_SQUIT			"SQUIT"
/*  NETINFO parameters are:
 *	none
 */
#define EVENT_NETINFO		"NETINFO"
/*  UMODE parameters are:
 *	user nick
 *	mode string
 */
#define EVENT_UMODE			"UMODE"
/*  SMODE parameters are:
 *	user nick
 *	mode string
 */
#define EVENT_SMODE			"SMODE"
/*  NICKCHANGE parameters are:
 *	old nick
 *	new nick
 */
#define EVENT_NICKCHANGE	"NICKCHANGE"
/*  PONG parameters are:
 *	server name
 */
#define EVENT_PONG			"PONG"
/*  AWAY parameters are:
 *	user nick
 *	away message if setting away, NULL if cancel away
 */
#define EVENT_AWAY			"AWAY"
/*  NEWCHAN parameters are:
 *	channel name
 */
#define EVENT_NEWCHAN		"NEWCHAN"
/*  DELCHAN parameters are:
 *	channel name
 */
#define EVENT_DELCHAN		"DELCHAN"
/*  JOINCHAN parameters are:
 *	channel name
 *	user nick
 */
#define EVENT_JOINCHAN		"JOINCHAN"
/*  PARTCHAN parameters are:
 *	channel name
 *	user nick
 */
#define EVENT_PARTCHAN		"PARTCHAN"
/*  KICK parameters are:
 *	channel name
 *	nick of user who made the kick
 *	nick of user who was kick
 */
#define EVENT_KICK			"KICK"
/*  KICKBOT parameters are:
 *	channel name
 *	nick of user who made the kick
 *	nick of user who was kick
 */
#define EVENT_KICKBOT		"KICKBOT"
/*  PARTBOT parameters are:
 *	channel name
 *	user nick
 */
#define EVENT_PARTBOT		"PARTBOT"
/*  TOPICCHANGE parameters are:
 *	channel name
 *	owner
 *	topic
 */
#define EVENT_TOPICCHANGE	"TOPICCHANGE"
/*  CLIENTVERSION parameters are:
 *	user nick
 *	client version string
 */
#define EVENT_CLIENTVERSION	"CLIENTVERSION"
