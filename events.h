/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

/*  ONLINE is called when NeoStats connects to the Network. 
 *  Can be used to init bots, and so on. 
 *  ONLINE parameters are:
 *	server connected to
 */
#define EVENT_ONLINE	    "ONLINE"

/*  SIGNON is called when a new user signs onto the network. the user 
 *  nickname is passed as a parameter, so you can see who signed on.
 *  SIGNON parameters are:
 *	nick
 */
#define EVENT_SIGNON		"SIGNON"

/*  SIGNOFF is called when a user quits the network. The user 
 *  nickname is passed as a parameter, so you can see who signed off.
 *  SIGNOFF parameters are:
 *	nick
 *	reason
 */
#define EVENT_SIGNOFF		"SIGNOFF"

/*  KILL is called when a user is killed on the network. the user 
 *  nick is passed as a parameter, so you can see who
 *  was killed (You would have to use the recbuf global 
 *  variable to see who killed them though)
 *  KILL parameters are:
 *	nick
 */
#define EVENT_KILL			"KILL"

/*  BOTKILL	is called if one of the NeoStats bots gets killed. You would 
 *  use it to reinitialize the bot.
 *  BOTKILL parameters are:
 *	botnick
 */
#define EVENT_BOTKILL		"BOTKILL"

/*  SERVER is called when a server connects to the network. the server 
 *  structure Server is passed as a parameter so you can see 
 *  details about the new server
 *  SERVER parameters are:
 *	server name
 *	uplink
 *	hops
 *	numeric
 *	infoline
 */
#define EVENT_SERVER		"SERVER"

/*  SQUIT is called when a server squits the network 
 *  SQUIT parameters are:
 *	server name
 *	reason
 */
#define EVENT_SQUIT			"SQUIT"

/*  NETINFO	is called when the connection to the network is synced. 
 *  NETINFO parameters are:
 *	none
 */
#define EVENT_NETINFO		"NETINFO"

/*  UMODE is called when a user changes Umodes. (e.g., /mode +o fish) 
 *  the user structure User is passed as a parameter.
 *  UMODE parameters are:
 *	nick
 *	mode string
 */
#define EVENT_UMODE			"UMODE"

/*  SMODE is called when a user changes Smodes.
 *  the user structure User is passed as a parameter.
 *  SMODE parameters are:
 *	nick
 *	mode string
 */
#define EVENT_SMODE			"SMODE"

/*  NICKCHANGE is called when a user changes nick
 *  the user structure User is passed as a parameter.
 *  NICKCHANGE parameters are:
 *	old nick
 *	new nick
 */
#define EVENT_NICKCHANGE	"NICKCHANGE"

/*  PONG parameters are:
 *	server name
 */
#define EVENT_PONG			"PONG"

/*  AWAY parameters are:
 *	nick
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
 *	reason
 */
#define EVENT_PARTCHAN		"PARTCHAN"

/*  KICK parameters are:
 *	channel name
 *	nick of user who made the kick
 *	nick of user who was kick
 *	reason
 */
#define EVENT_KICK			"KICK"

/*  KICKBOT parameters are:
 *	channel name
 *	nick of user who made the kick
 *	nick of user who was kick
 *	reason
 */
#define EVENT_KICKBOT		"KICKBOT"

/*  PARTBOT parameters are:
 *	channel name
 *	user nick
 *	reason
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

/* CHANMODE params are:
 *	channel name
 *	mode string
 */
#define EVENT_CHANMODE		"CHANMODE"

/* PRIVATE params are:
 *	from nick 
 *	to nick 
 *	message
 */
#define EVENT_PRIVATE		"PRIVATE"

/* NOTICE params are:
 *	from nick 
 *	to nick 
 *	message
 */
#define EVENT_NOTICE		"NOTICE"

/* CPRIVATE params are:
 *	from nick 
 *	to channel
 *	message
 */
#define EVENT_CPRIVATE		"CPRIVATE"

/* CNOTICE params are:
 *	from nick 
 *	to channel
 *	message
 */
#define EVENT_CNOTICE		"CNOTICE"

/* ADDBAN params are:
 */
#define EVENT_ADDBAN		"ADDBAN"

/* DELBAN params are:
 */
#define EVENT_DELBAN		"DELBAN"

