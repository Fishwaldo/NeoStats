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

/*  Events. 
 *    An event handler is passed a structure pointer of type
 *    CmdParams.
 */
typedef enum Event {
/*  EVENT_NULL
 *    dummy value to end lists
 */
	EVENT_NULL = -1,

/*  Network events 
 *    Generated in response to network events
 */

/*  EVENT_ONLINE
 *    Called when NeoStats connects to the Network. 
 *    Can be used to init bots, and so on. 
 *    parameters:
 *      none
 */
	EVENT_ONLINE = 0,

/*  EVENT_NETINFO 
 *    Called when the connection to the network is synched. 
 *    parameters:
 *      none
 */
	EVENT_NETINFO,

/*  Network events 
 *    Generated in response to server events
 */

/*  EVENT_SERVER 
 *    Called when a server connects to the network. 
 *    parameters:
 *      server in cmdparams->source
 */
	EVENT_SERVER,

/*  EVENT_SQUIT 
 *    Called when a server squits the network 
 *    parameters:
 *      server in cmdparams->source
 *      reason in cmdparams->param
 */
	EVENT_SQUIT,

/*  EVENT_PING 
 *    parameters:
 *      server in cmdparams->source
 */
	EVENT_PING,

/*  EVENT_PONG 
 *    parameters:
 *      server in cmdparams->source
 */
	EVENT_PONG,

/*  User events 
 *    Generated in response to user events
 */

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
 *      reason in cmdparams->param
 */
	EVENT_QUIT,

/*	EVENT_GOTNICKIP
 *	fired when we get the IP address of a user
 *	only fired if me.want_nickip = 1 and:
 *	the ircd sends the nickip as part of the connect message
 *	or a dns lookup completes and is successfull
 * 
 *	  parameters:
 *		user in cmdparams->source
 */ 
	EVENT_GOTNICKIP,

/*  EVENT_KILL 
 *    Called when a user is killed on the network. the user 
 *    nick is passed as a parameter, so you can see who
 *    was killed (You would have to use the recbuf global 
 *    variable to see who killed them though)
 *    parameters:
 *      user in cmdparams->source
 *      reason in cmdparams->param
 */
	EVENT_KILL,

/*  EVENT_BOTKILL 
 *    is called if one of the NeoStats bots gets killed. You would 
 *    use it to reinitialize the bot.
 *    parameters:
 *      user in cmdparams->source
 *      reason in cmdparams->param
 */
	EVENT_BOTKILL,

/*  EVENT_NICK
 *    Called when a user changes nick
 *    parameters:
 *      user in cmdparams->source
 *      oldnick in cmdparams->param
 */
	EVENT_NICK,

/*  EVENT_AWAY 
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_AWAY,

/*  EVENT_UMODE 
 *    Called when a user changes Umodes. (e.g., /mode +o fish) 
 *    parameters:
 *      user in cmdparams->source
 *      modes in cmdparams->param
 */
	EVENT_UMODE,

/*  EVENT_SMODE 
 *    Called when a user changes Smodes.
 *    parameters:
 *      user in cmdparams->source
 *      modes in cmdparams->param
 */
	EVENT_SMODE,

/*  Channel events 
 *    Generated in response to channel events
 */

/*  EVENT_NEWCHAN 
 *    parameters:
 *      channel in cmdparams->channel
 */
	EVENT_NEWCHAN,

/*  EVENT_DELCHAN 
 *    parameters:
 *      channel in cmdparams->channel
 */
	EVENT_DELCHAN,

/*  EVENT_JOIN
 *    parameters:
 *      user in cmdparams->source
 *      channel in cmdparams->channel
 */
	EVENT_JOIN,

/*  EVENT_PART
 *    parameters:
 *      user in cmdparams->source
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_PART,

/*  EVENT_PARTBOT 
 *    parameters:
 *      user in cmdparams->source
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_PARTBOT,

/*  EVENT_KICK 
 *    parameters:
 *      user in cmdparams->target
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_KICK,

/*  EVENT_KICKBOT 
 *    parameters:
 *      user in cmdparams->target
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_KICKBOT,

/*  EVENT_TOPIC 
 *    parameters:
 *      channel in cmdparams->channel
 *      owner in cmdparams->av[0]
 *      topic in cmdparams->av[1]
 */
	EVENT_TOPIC,

/*  EVENT_CHANMODE
 *    parameters:
 *      channel in cmdparams->channel
 *      modes in cmdparams->av[0]..av[cmdparams->ac-1]
 */
	EVENT_CHANMODE,

/*  Messages event
 *    Generated in response to messages received 
 */

/*  EVENT_PRIVATE
 *    parameters:
 *      from in cmdparams->source
 *      to in cmdparams->bot
 *      message in cmdparams->param
 */
	EVENT_PRIVATE,

/*  EVENT_NOTICE
 *    parameters:
 *      from in cmdparams->source
 *      to in cmdparams->bot
 *      message in cmdparams->param
 */
	EVENT_NOTICE,

/*  EVENT_CPRIVATE
 *    parameters:
 *      from in cmdparams->source
 *      channel in cmdparams->channel
 *      message in cmdparams->param
 */
	EVENT_CPRIVATE,

/*  EVENT_CNOTICE
 *    parameters:
 *      from in cmdparams->source
 *      channel in cmdparams->channel
 *      message in cmdparams->param
 */
	EVENT_CNOTICE,

/*  CTCP events 
 *    Generated in response to ctcp events
 */

/*  EVENT_CTCPVERSIONRPL 
 *    parameters:
 *      user in cmdparams->source
 *      version in cmdparams->param
 */
	EVENT_CTCPVERSIONRPL,

/*  EVENT_CTCPVERSIONREQ
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_CTCPVERSIONREQ,

/*  DCC events 
 *    Generated in response to ctcp dcc events
 */

/*  EVENT_DCCSEND 
 *    parameters:
 *      user in cmdparams->source
 *      parameters in cmdparams->param
 */
	EVENT_DCCSEND,

/*  Ban events 
 *    Generated in response to ban events
 */

/*  EVENT_ADDBAN
 *    parameters:
 *      cmdparams->av[0] type
 *      cmdparams->av[1] user
 *      cmdparams->av[2] host
 *      cmdparams->av[3] mask
 *      cmdparams->av[4] reason
 *      cmdparams->av[5] setby
 *      cmdparams->av[6] tsset
 *      cmdparams->av[7] tsexpires
 */
	EVENT_ADDBAN,

/*  EVENT_DELBAN
 *    parameters:
 *      cmdparams->av[0] type
 *      cmdparams->av[1] user
 *      cmdparams->av[2] host
 *      cmdparams->av[3] mask
 *      cmdparams->av[4] reason
 *      cmdparams->av[5] setby
 *      cmdparams->av[6] tsset
 *      cmdparams->av[7] tsexpires
 */
	EVENT_DELBAN,

/*  EVENT_COUNT
 *    dummy value to provide last event number
 */
	EVENT_COUNT,

} Event;
