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

/*  Events sent to modules. Each event has 2 parameters:
 *    av is a list of parameters.
 *    ac is the number of parameters
 */
typedef enum Event {
/*  EVENT_NULL
 *    dummy value to end lists
 */
	EVENT_NULL = -1,

/*  EVENT_ONLINE
 *    Called when NeoStats connects to the Network. 
 *    Can be used to init bots, and so on. 
 *    parameters:
 *      none
 */
	EVENT_ONLINE = 0,

/*  EVENT_SIGNON 
 *    Called when a new user signs onto the network. the user 
 *    nickname is passed as a parameter, so you can see who signed on.
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_SIGNON,

/*  EVENT_QUIT
 *    Called when a user quits the network. The user 
 *    nickname is passed as a parameter, so you can see who signed off.
 *    parameters:
 *      user in cmdparams->source
 *      reason in cmdparams->message
 */
	EVENT_QUIT,

/*  EVENT_KILL 
 *    Called when a user is killed on the network. the user 
 *    nick is passed as a parameter, so you can see who
 *    was killed (You would have to use the recbuf global 
 *    variable to see who killed them though)
 *    parameters:
 *      av[0] nick
 */
	EVENT_KILL,

/*  EVENT_BOTKILL 
 *    is called if one of the NeoStats bots gets killed. You would 
 *    use it to reinitialize the bot.
 *    parameters:
 *      av[0] botnick
 */
	EVENT_BOTKILL,

/*  EVENT_SERVER 
 *    Called when a server connects to the network. 
 *    parameters:
 *      av[0] server name
 *      av[1] uplink
 *      av[2] hops
 *      av[3] numeric
 *      av[4] infoline
 */
	EVENT_SERVER,

/*  EVENT_SQUIT 
 *    Called when a server squits the network 
 *    parameters:
 *      av[0] server name
 *      av[1] reason
 */
	EVENT_SQUIT,

/*  EVENT_NETINFO 
 *    Called when the connection to the network is synched. 
 *    parameters:
 *      av[0] none
 */
	EVENT_NETINFO,

/*  EVENT_UMODE 
 *    Called when a user changes Umodes. (e.g., /mode +o fish) 
 *    parameters:
 *      av[0] nick
 *      av[1] mode string
 */
	EVENT_UMODE,

/*  EVENT_SMODE 
 *    Called when a user changes Smodes.
 *    parameters:
 *      av[0] nick
 *      av[1] mode string
 */
	EVENT_SMODE,

/*  EVENT_NICK
 *    Called when a user changes nick
 *    parameters:
 *      av[0] old nick
 *      av[1] new nick
 */
	EVENT_NICK,

/*  EVENT_PONG 
 *    parameters:
 *      av[0] server name
 */
	EVENT_PONG,

/*  EVENT_AWAY 
 *    parameters:
 *      av[0] nick
 *      av[1] away message if setting away, NULL if cancel away
 */
	EVENT_AWAY,

/*  EVENT_NEWCHAN 
 *    parameters:
 *      av[0] channel name
 */
	EVENT_NEWCHAN,

/*  EVENT_DELCHAN 
 *    parameters:
 *      av[0] channel name
 */
	EVENT_DELCHAN,

/*  EVENT_JOIN
 *    parameters:
 *      av[0] channel name
 *      av[1] user nick
 */
	EVENT_JOIN,

/*  EVENT_PART
 *    parameters:
 *      av[0] channel name
 *      av[1] user nick
 *      av[2] reason
 */
	EVENT_PART,

/*  EVENT_KICK 
 *    parameters:
 *      av[0] channel name
 *      av[1] nick of user who made the kick
 *      av[2] nick of user who was kick
 *      av[3] reason
 */
	EVENT_KICK,

/*  EVENT_KICKBOT 
 *    parameters:
 *      av[0] channel name
 *      av[1] nick of user who made the kick
 *      av[2] nick of user who was kick
 *      av[3] reason
 */
	EVENT_KICKBOT,

/*  EVENT_PARTBOT 
 *    parameters:
 *      av[0] channel name
 *      av[1] user nick
 *      av[2] reason
 */
	EVENT_PARTBOT,

/*  EVENT_TOPIC 
 *    parameters:
 *      av[0] channel name
 *      av[1] owner
 *      av[2] topic
 */
	EVENT_TOPIC,

/*  EVENT_CTCPVERSION 
 *    parameters:
 *      av[0] user nick
 *      av[1] client version string
 */
	EVENT_CTCPVERSION,

/*  EVENT_CHANMODE
 *    parameters:
 *      av[0] channel name
 *      av[1] mode string
 */
	EVENT_CHANMODE,

/*  EVENT_PRIVATE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to nick 
 *      av[2] message
 */
	EVENT_PRIVATE,

/*  EVENT_NOTICE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to nick 
 *      av[2] message
 */
	EVENT_NOTICE,

/*  EVENT_CPRIVATE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to channel
 *      av[2] message
 */
	EVENT_CPRIVATE,

/*  EVENT_CNOTICE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to channel
 *      av[2] message
 */
	EVENT_CNOTICE,

/*  EVENT_CTCPPRIVATE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to nick
 *      av[2] message
 */
	EVENT_CTCPPRIVATE,

/*  EVENT_CTCPNOTICE
 *    parameters:
 *      av[0] from nick 
 *      av[1] to nick
 *      av[2] message
 */
	EVENT_CTCPNOTICE,

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
	EVENT_ADDBAN,

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
	EVENT_DELBAN,

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
 
	EVENT_GOTNICKIP,

} Event;
