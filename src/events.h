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

/*  Events sent to modules. Each event has 2 parameters:
 *    av is a list of parameters.
 *    ac is the number of parameters
 */

/*  EVENT_ONLINE
 *    Called when NeoStats connects to the Network. 
 *    Can be used to init bots, and so on. 
 *    parameters:
 *      av[0] server connected to
 */
#define EVENT_ONLINE "ONLINE"

/*  EVENT_SIGNON 
 *    Called when a new user signs onto the network. the user 
 *    nickname is passed as a parameter, so you can see who signed on.
 *    parameters:
 *      av[0] nick
 */
#define EVENT_SIGNON "SIGNON"

/*  EVENT_SIGNOFF 
 *    Called when a user quits the network. The user 
 *    nickname is passed as a parameter, so you can see who signed off.
 *    parameters:
 *      av[0] nick
 *      av[1] reason
 */
#define EVENT_SIGNOFF "SIGNOFF"

/*  EVENT_KILL 
 *    Called when a user is killed on the network. the user 
 *    nick is passed as a parameter, so you can see who
 *    was killed (You would have to use the recbuf global 
 *    variable to see who killed them though)
 *    parameters:
 *      av[0] nick
 */
#define EVENT_KILL "KILL"

/*  EVENT_BOTKILL 
 *    is called if one of the NeoStats bots gets killed. You would 
 *    use it to reinitialize the bot.
 *    parameters:
 *      av[0] botnick
 */
#define EVENT_BOTKILL "BOTKILL"

/*  EVENT_SERVER 
 *    Called when a server connects to the network. 
 *    parameters:
 *      av[0] server name
 *      av[1] uplink
 *      av[2] hops
 *      av[3] numeric
 *      av[4] infoline
 */
#define EVENT_SERVER "SERVER"

/*  EVENT_SQUIT 
 *    Called when a server squits the network 
 *    parameters:
 *      av[0] server name
 *      av[1] reason
 */
#define EVENT_SQUIT "SQUIT"

/*  EVENT_NETINFO 
 *    Called when the connection to the network is synced. 
 *    parameters:
 *      av[0] none
 */
#define EVENT_NETINFO "NETINFO"

/*  EVENT_UMODE 
 *    Called when a user changes Umodes. (e.g., /mode +o fish) 
 *    parameters:
 *      av[0] nick
 *      av[1] mode string
 */
#define EVENT_UMODE "UMODE"

/*  EVENT_SMODE 
 *    Called when a user changes Smodes.
 *    parameters:
 *      av[0] nick
 *      av[1] mode string
 */
#define EVENT_SMODE "SMODE"

/*  EVENT_NICKCHANGE 
 *    Called when a user changes nick
 *    parameters:
 *      av[0] old nick
 *      av[1] new nick
 */
#define EVENT_NICKCHANGE "NICKCHANGE"

/*  EVENT_PONG 
 *    parameters:
 *      av[0] server name
 */
#define EVENT_PONG "PONG"

/*  EVENT_AWAY 
 *    parameters:
 *      av[0] nick
 *      av[1] away message if setting away, NULL if cancel away
 */
#define EVENT_AWAY "AWAY"

/*  EVENT_NEWCHAN 
 *    parameters:
 *      av[0] channel name
 */
#define EVENT_NEWCHAN "NEWCHAN"

/*  EVENT_DELCHAN 
 *    parameters:
 *      av[0] channel name
 */
#define EVENT_DELCHAN "DELCHAN"

/*  EVENT_JOINCHAN 
 *    parameters:
 *      av[0] channel name
 *      av[1] user nick
 */
#define EVENT_JOINCHAN "JOINCHAN"

/*  EVENT_PARTCHAN 
 *    parameters:
 *      av[0] channel name
 *      av[1] user nick
 *      av[2] reason
 */
#define EVENT_PARTCHAN "PARTCHAN"

/*  EVENT_KICK 
 *    parameters:
 *      av[0] channel name
 *      av[1] nick of user who made the kick
 *      av[2] nick of user who was kick
 *      av[3] reason
 */
#define EVENT_KICK "KICK"

/*  EVENT_KICKBOT 
 *    parameters:
 *      av[0] channel name
 *      av[1] nick of user who made the kick
 *      av[2] nick of user who was kick
 *      av[3] reason
 */
#define EVENT_KICKBOT "KICKBOT"

/*  EVENT_PARTBOT 
 *    parameters:
 *      av[0] channel name
 *      av[1] user nick
 *      av[2] reason
 */
#define EVENT_PARTBOT "PARTBOT"

/*  EVENT_TOPICCHANGE 
 *    parameters:
 *      av[0] channel name
 *      av[1] owner
 *      av[2] topic
 */
#define EVENT_TOPICCHANGE "TOPICCHANGE"

/*  EVENT_CLIENTVERSION 
 *    parameters:
 *      av[0] user nick
 *      av[1] client version string
 */
#define EVENT_CLIENTVERSION "CLIENTVERSION"

/*  EVENT_CHANMODE
 *    parameters:
 *      av[0] channel name
 *      av[1] mode string
 */
#define EVENT_CHANMODE "CHANMODE"

/*  EVENT_PRIVATE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to nick 
 *      av[2] message
 */
#define EVENT_PRIVATE "PRIVATE"

/*  EVENT_NOTICE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to nick 
 *      av[2] message
 */
#define EVENT_NOTICE "NOTICE"

/*  EVENT_CPRIVATE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to channel
 *      av[2] message
 */
#define EVENT_CPRIVATE "CPRIVATE"

/*  EVENT_CNOTICE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to channel
 *      av[2] message
 */
#define EVENT_CNOTICE "CNOTICE"

/*  EVENT_ADDBAN
 *    parameters:
 *      av[0] type
 *      av[1] user
 *      av[2] host
 *      av[3] mask
 *      av[4] reason
 *      av[5] setby
 *      av[6] tsset
 *      av[7] tsexpires
 */
#define EVENT_ADDBAN "ADDBAN"

/*  EVENT_DELBAN
 *    parameters:
 *      av[0] type
 *      av[1] user
 *      av[2] host
 *      av[3] mask
 *      av[4] reason
 *      av[5] setby
 *      av[6] tsset
 *      av[7] tsexpires
 */
#define EVENT_DELBAN "DELBAN"

/* EVENT_GOTNICKIP
 * fired when we get the IP address of a user
 * only fired if me.want_nickip = 1 and:
 * the ircd sends the nickip as part of the connect message
 * or
 * a dns lookup completes and is successfull
 * 
 * Parameters:
 *	av[0] nick
 */
 
#define EVENT_GOTNICKIP "NICKIP"
