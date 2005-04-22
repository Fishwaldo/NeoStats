/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "ircprotocol.h"
#include "protocol.h"
#include "ircsend.h"
#include "ircrecv.h"
#include "main.h"
#include "bots.h"
#include "modules.h"
#include "modes.h"
#include "users.h"
#include "servers.h"
#include "channels.h"
#include "services.h"
#include "bans.h"
#include "dns.h"




#define MOTD_FILENAME	"neostats.motd"
#define ADMIN_FILENAME	"neostats.admin"

typedef struct protocol_entry {
	char *token;
	unsigned int flag;
} protocol_entry;

extern ProtocolInfo *protocol_info;

extern ircd_cmd *cmd_list;

protocol_entry protocol_list[] =
{
	{"TOKEN",	PROTOCOL_TOKEN},
	{"CLIENT",	PROTOCOL_CLIENT},
	{"UNKLN",	PROTOCOL_UNKLN},
	{"NOQUIT",	PROTOCOL_NOQUIT},
	{"NICKIP",	PROTOCOL_NICKIP},
	{"NICKv2",	PROTOCOL_NICKv2},
	{"SJ3",		PROTOCOL_SJ3},	
	{NULL, 0}
};

ircd_cmd_intrinsic intrinsic_cmd_list[] = 
{
	{&MSG_PRIVATE, &TOK_PRIVATE, _m_private, 0},
	{&MSG_NOTICE, &TOK_NOTICE, _m_notice, 0},
	{&MSG_STATS, &TOK_STATS, _m_stats, 0},
	{&MSG_VERSION, &TOK_VERSION, _m_version, 0},
	{&MSG_MOTD, &TOK_MOTD, _m_motd, 0},
	{&MSG_ADMIN, &TOK_ADMIN, _m_admin, 0},
	{&MSG_CREDITS, &TOK_CREDITS, _m_credits, 0},
	{&MSG_INFO, &TOK_INFO, _m_info, 0},
	{&MSG_SQUIT, &TOK_SQUIT, _m_squit, 0},
	{&MSG_AWAY, &TOK_AWAY, _m_away, 0},
	{&MSG_QUIT, &TOK_QUIT, _m_quit, 0},
	{&MSG_MODE, &TOK_MODE, _m_mode, 0},
	{&MSG_SVSJOIN, &TOK_SVSJOIN, _m_svsjoin, 0},
	{&MSG_SVSPART, &TOK_SVSPART, _m_svspart, 0},
	{&MSG_KILL, &TOK_KILL, _m_kill, 0},
	{&MSG_PING, &TOK_PING, _m_ping, 0},
	{&MSG_PONG, &TOK_PONG, _m_pong, 0},
	{&MSG_JOIN, &TOK_JOIN, _m_join, 0},
	{&MSG_PART, &TOK_PART, _m_part, 0},
	{&MSG_KICK, &TOK_KICK, _m_kick, 0},
	{&MSG_GLOBOPS, &TOK_GLOBOPS, _m_globops, 0},
	{&MSG_WALLOPS, &TOK_WALLOPS, _m_wallops, 0},
	{&MSG_SVINFO, &TOK_SVINFO, _m_svinfo, 0},
	{&MSG_NETINFO, &TOK_NETINFO, _m_netinfo, 0},
	{&MSG_SNETINFO, &TOK_SNETINFO, _m_snetinfo, 0},
	{&MSG_EOB, &TOK_EOB, _m_eob, 0},
	{&MSG_PROTOCTL, &TOK_PROTOCTL, _m_protoctl, 0},
	{&MSG_CAPAB, &TOK_CAPAB, _m_capab, 0},
	{&MSG_PASS, &TOK_PASS, _m_pass, 0},
	{&MSG_TOPIC, &TOK_TOPIC, _m_topic, 0},
	{&MSG_SVSNICK, &TOK_SVSNICK, _m_svsnick, 0},
	{&MSG_SETNAME, &TOK_SETNAME, _m_setname, 0},
	{&MSG_SETHOST, &TOK_SETHOST, _m_sethost, 0},
	{&MSG_SETIDENT, &TOK_SETIDENT, _m_setident, 0},
	{&MSG_CHGHOST, &TOK_CHGHOST, _m_chghost, 0},
	{&MSG_CHATOPS, &TOK_CHATOPS, _m_chatops, 0},
	{&MSG_ERROR, &TOK_ERROR, _m_error, 0},
	{0, 0, 0, 0},
};

/*  _m_command functions are highly generic support functions for 
 *  use in protocol modules. If a protocol differs at all from 
 *  the RFC, they must provide their own local version of this 
 *  function. These	are purely to avoid protocol module bloat for
 *  the more common forms of these commands and allow protocol module
 *  coders to concentrate on the areas that need it.
 */

/** @brief _m_glopops
 *
 *  process GLOBOPS command
 *  RX: :Mark GLOBOPS :test globops
 *  GLOBOPS :message
 *	argv[0] = message
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_globops( char *origin, char **av, int ac, int cmdptr )
{
	do_globops( origin, av[0] );	
}

/** @brief _m_wallops
 *
 *  process WALLOPS command
 *  RX: :Mark WALLOPS :test wallops
 *  WALLOPS :message
 *	argv[0] = message
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_wallops( char *origin, char **av, int ac, int cmdptr )
{
	do_wallops( origin, av[0] );	
}

/** @brief _m_chatops
 *
 *  process CHATOPS command
 *  RX: :Mark CHATOPS :test chatops
 *  CHATOPS :message
 *	argv[0] = message
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_chatops( char *origin, char **av, int ac, int cmdptr )
{
	do_chatops( origin, av[0] );	
}

/** @brief _m_error
 *
 *  process ERROR command
 *  RX: :Mark ERROR :message
 *  ERROR :message
 *	argv[0] = message
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_error( char *origin, char **av, int ac, int cmdptr )
{
	fprintf(stderr, "IRCD reported error: %s", av[0] );
	nlog (LOG_ERROR, "IRCD reported error: %s", av[0] );
	do_exit (NS_EXIT_ERROR, av[0] );
}

/** @brief _m_ignorecommand
 *
 *  silently drop command
 *  RX: 
 *  cmd anything else
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_ignorecommand( char *origin, char **argv, int argc, int srv )
{
}

/** @brief _m_pass
 *
 *  process PASS command
 *  RX: 
 *  PASS :password
 *	argv[0] = password
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_pass( char *origin, char **av, int ac, int cmdptr )
{
	
}

/** @brief _m_protoctl
 *
 *  process PROTOCTL command
 *  RX: 
 *  PROTOCTL <token list>
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_protoctl( char *origin, char **argv, int argc, int cmdptr )
{
	do_protocol( origin, argv, argc );
}

/** @brief _m_version
 *
 *  process VERSION command
 *  origin VERSION :stats.neostats.net
 *	argv[0] = remote server
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_version( char *origin, char **argv, int argc, int srv )
{
	do_version( origin, argv[0] );
}

/** @brief _m_motd
 *
 *  process MOTD command
 *  origin MOTD :stats.neostats.net
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_motd( char *origin, char **argv, int argc, int srv )
{
	do_motd( origin, argv[0] );
}

/** @brief _m_admin
 *
 *  process ADMIN command
 *  origin ADMIN :stats.neostats.net
 *	argv[0] = servername
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_admin( char *origin, char **argv, int argc, int srv )
{
	do_admin( origin, argv[0] );
}

/** @brief _m_credits
 *
 *  process CREDITS command
 *  origin CREDITS :stats.neostats.net
 *	argv[0] = servername
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_credits( char *origin, char **argv, int argc, int srv )
{
	do_credits( origin, argv[0] );
}

/** @brief _m_info
 *
 *  process INFO command
 *  origin INFO :stats.neostats.net
 *	argv[0] = servername
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_info( char *origin, char **argv, int argc, int srv )
{
	/* For now */
	do_credits( origin, argv[0] );
}

/** @brief _m_stats
 *
 *  process STATS command
 *  RX: 
 *  :origin STATS u :stats.neostats.net
 *	argv[0] = stats type
 *	argv[1] = destination
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_stats( char *origin, char **argv, int argc, int srv )
{
	do_stats( origin, argv[0] );
}

/** @brief _m_ping
 *
 *  process PING command
 *  RX: 
 *  PING :irc.foonet.com
 *	argv[0] = origin
 *	argv[1] = destination
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_ping( char *origin, char **argv, int argc, int srv )
{
	do_ping( argv[0], argc > 1 ? argv[1] : NULL );
}

/** @brief _m_pong
 *
 *  process PONG command
 *  irc.foonet.com PONG irc.foonet.com :stats.neostats.net
 *  argv[0] = origin
 *  argv[1] = destination
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_pong( char *origin, char **argv, int argc, int srv )
{
	do_pong( argv[0], argv[1] );
}

/** @brief _m_quit
 *
 *  process QUIT command
 *  origin QUIT :comment
 *  argv[0] = comment
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_quit( char *origin, char **argv, int argc, int srv )
{
	do_quit( origin, argv[0] );
}

/** @brief m_join
 *
 *  process JOIN command
 *	argv[0] = channel
 *	argv[1] = channel password( key )
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_join( char *origin, char **argv, int argc, int srv )
{
	do_join( origin, argv[0], argv[1] );
}

/** @brief m_part
 *
 *  process PART command
 *	argv[0] = channel
 *	argv[1] = comment
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_part( char *origin, char **argv, int argc, int srv )
{
	do_part( origin, argv[0], argv[1] );
}

/** @brief _m_kick
 *
 *  process KICK command
 *	argv[0] = channel
 *	argv[1] = client to kick
 *	argv[2] = kick comment
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_kick( char *origin, char **argv, int argc, int srv )
{
	do_kick( origin, argv[0], argv[1], argv[2] );
}

/** @brief _m_topic
 *
 *  process TOPIC command
 *  origin TOPIC #channel owner TS :topic
 *  argv[0] = topic text
 * For servers using TS:
 *  argv[0] = channel name
 *  argv[1] = topic nickname
 *  argv[2] = topic time
 *  argv[3] = topic text
 *
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_topic( char *origin, char **argv, int argc, int srv )
{
	do_topic( argv[0], argv[1], argv[2], argv[3] );
}

/** @brief _m_away
 *
 *  process AWAY command
 *	argv[0] = away message
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_away( char *origin, char **argv, int argc, int srv )
{
	do_away( origin,( argc > 0 ) ? argv[0] : NULL );
}

/** @brief _m_kill
 *
 *  process KILL command
 *	argv[0] = kill victim(s) - comma separated list
 *	argv[1] = kill path
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_kill( char *origin, char **argv, int argc, int srv )
{
	do_kill( origin, argv[0], argv[1] );
}

/** @brief _m_squit
 *
 *  process SQUIT command
 *	argv[0] = server name
 *	argv[argc-1] = comment
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_squit( char *origin, char **argv, int argc, int srv )
{
	do_squit( argv[0], argv[argc-1] );
}

/** @brief _m_netinfo
 *
 *  process NETINFO command
 *  argv[0] = max global count
 *  argv[1] = time of end sync
 *  argv[2] = protocol
 *  argv[3] = cloak
 *  argv[4] = free( ** )
 *  argv[5] = free( ** )
 *  argv[6] = free( ** )
 *  argv[7] = ircnet
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_netinfo( char *origin, char **argv, int argc, int srv )
{
	do_netinfo( argv[0], argv[1], argv[2], argv[3], argv[7] );
}

/** @brief _m_snetinfo
 *
 *  process NETINFO command
 *  argv[0] = max global count
 *  argv[1] = time of end sync
 *  argv[2] = protocol
 *  argv[3] = cloak
 *  argv[4] = free( ** )
 *  argv[5] = free( ** )
 *  argv[6] = free( ** )
 *  argv[7] = ircnet
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_snetinfo( char *origin, char **argv, int argc, int srv )
{
	do_snetinfo( argv[0], argv[1], argv[2], argv[3], argv[7] );
}

/** @brief _m_mode
 *
 *  process MODE command
 *	 argv[0] - channel
 *
 *  m_umode
 *   argv[0] - username to change mode for
 *   argv[1] - modes to change
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */
/*  MODE
 *  :nick MODE nick :+modestring 
 *  :servername MODE #channel +modes parameter list TS 
 */

void _m_mode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' )
	{
		do_mode_channel( origin, argv, argc );
	}
	else
	{
		do_mode_user( argv[0], argv[1] );
	}
}

/** @brief _m_svsnick
 *
 *  process SVSNICK command
 *  argv[0] = old nickname
 *  argv[1] = new nickname
 *  argv[2] = timestamp
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_svsnick( char *origin, char **argv, int argc, int srv )
{
	do_svsnick( argv[0], argv[1], ( argc > 2 ) ? argv[2] : NULL );
}

/** @brief _m_setname
 *
 *  process SETNAME command
 *	argv[0] = name
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_setname( char *origin, char **argv, int argc, int srv )
{
	do_setname( origin, argv[0] );
}

/** @brief _m_sethost
 *
 *  process SETHOST command
 *	argv[0] = host
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_sethost( char *origin, char **argv, int argc, int srv )
{
	do_sethost( origin, argv[0] );
}

/** @brief _m_chghost
 *
 *  process CHGHOST command
 *	argv[0] = nick
 *	argv[1] = host
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_chghost( char *origin, char **argv, int argc, int srv )
{
	do_chghost( argv[0], argv[1] );
}

/** @brief _m_setident
 *
 *  process SETIDENT command
 *	argv[0] = ident
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_setident( char *origin, char **argv, int argc, int srv )
{
	do_setident( origin, argv[0] );
}

/** @brief _m_svsjoin
 *
 *  process SVSJOIN command
 *	argv[0] = nick
 *	argv[1] = channel
 *	argv[2] = key
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_svsjoin( char *origin, char **argv, int argc, int srv )
{
	do_join( argv[0], argv[1], argv[2] );
}

/** @brief _m_svspart
 *
 *  process SVSPART command
 *	argv[0] = nick
 *	argv[1] = channel
 *	argv[2] = reason
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_svspart( char *origin, char **argv, int argc, int srv )
{
	do_part( argv[0], argv[1], argv[2] );
}

/** @brief _m_svinfo
 *
 *  process SVINFO command
 *  RX: 
 *  SVINFO
 *	argv[0]
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_svinfo( char *origin, char **argv, int argc, int srv )
{
	do_svinfo();
}

/** @brief _m_eob
 *
 *  process EOB command
 *  RX: 
 *  EOB
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_eob( char *origin, char **argv, int argc, int srv )
{
	irc_eob( me.name );
	do_synch_neostats( );
}

/** @brief _m_notice
 *
 *  process NOTICE command
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_notice( char *origin, char **argv, int argc, int cmdptr )
{
	SET_SEGV_LOCATION();
	if( argv[0] == NULL ) {
		dlog( DEBUG1, "_m_notice: dropping notice from %s to NULL: %s", origin, argv[argc-1] );
		return;
	}
	dlog( DEBUG1, "_m_notice: from %s, to %s : %s", origin, argv[0], argv[argc-1] );
	/* who to */
	if( argv[0][0] == '#' ) {
		bot_chan_notice( origin, argv, argc );
		return;
	}
#if 0
	if( ircstrcasecmp( argv[0], "AUTH" ) ) {
		dlog( DEBUG1, "_m_notice: dropping server notice from %s, to %s : %s", origin, argv[0], argv[argc-1] );
		return;
	}
#endif
	bot_notice( origin, argv, argc );
}

/** @brief _m_private
 *
 *  process PRIVATE command
 *
 *  @param origin source of message (user/server)
 *  @param av list of message parameters
 *  @param ac parameter count
 *  @param cmdptr command flag
 *
 *  @return none
 */

void _m_private( char *origin, char **argv, int argc, int cmdptr )
{
	char target[64];

	SET_SEGV_LOCATION();
	if( argv[0] == NULL ) {
		dlog( DEBUG1, "_m_private: dropping privmsg from %s to NULL: %s", origin, argv[argc-1] );
		return;
	}
	dlog( DEBUG1, "_m_private: from %s, to %s : %s", origin, argv[0], argv[argc-1] );
	/* who to */
	if( argv[0][0] == '#' ) {
		bot_chan_private( origin, argv, argc );
		return;
	}
	if( strstr( argv[0], "!" ) ) {
		strlcpy( target, argv[0], 64 );
		argv[0] = strtok( target, "!" );
	} else if( strstr( argv[0], "@" ) ) {
		strlcpy( target, argv[0], 64 );
		argv[0] = strtok( target, "@" );
	}
	bot_private( origin, argv, argc );
}

/** @brief do_globops
 *
 * 
 *
 *  @param origin
 *  @param message
 *
 *  @return none
 */

void do_globops( const char *origin, const char *message )
{
	Client *c;
	CmdParams * cmdparams;

	dlog( DEBUG1, "GLOBOPS: %s %s", origin, message );
	c = FindServer( origin );
	if( !c ) 
	{
		c = FindUser( origin );
		if( c )
		{
			cmdparams =( CmdParams* )ns_calloc( sizeof( CmdParams ) );
			cmdparams->source = c;
			cmdparams->param = ( char * )message;
			SendAllModuleEvent( EVENT_GLOBOPS, cmdparams );
			ns_free( cmdparams );
		}
	}
}

/** @brief do_wallops
 *
 * 
 *
 *  @param origin
 *  @param message
 *
 *  @return none
 */

void do_wallops( const char *origin, const char *message )
{
	Client *c;
	CmdParams * cmdparams;

	dlog( DEBUG1, "WALLOPS: %s %s", origin, message );
	c = FindServer( origin );
	if( !c ) 
	{
		c = FindUser( origin );
		if( c )
		{
			cmdparams =( CmdParams* )ns_calloc( sizeof( CmdParams ) );
			cmdparams->source = c;
			cmdparams->param = ( char * )message;
			SendAllModuleEvent( EVENT_WALLOPS, cmdparams );
			ns_free( cmdparams );
		}
	}
}

/** @brief do_chatops
 *
 * 
 *
 *  @param origin
 *  @param message
 *
 *  @return none
 */

void do_chatops( const char *origin, const char *message )
{
	Client *c;
	CmdParams * cmdparams;

	dlog( DEBUG1, "CHATOPS: %s %s", origin, message );
	c = FindServer( origin );
	if( !c ) 
	{
		c = FindUser( origin );
		if( c )
		{
			cmdparams =( CmdParams* )ns_calloc( sizeof( CmdParams ) );
			cmdparams->source = c;
			cmdparams->param = ( char * )message;
			SendAllModuleEvent( EVENT_CHATOPS, cmdparams );
			ns_free( cmdparams );
		}
	}
}

/** @brief do_synch_neostats
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_synch_neostats( void )
{
	init_services_bot();
	irc_globops( NULL, _( "Link with Network \2Complete!\2" ) );
}

/** @brief do_ping
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_ping( const char *origin, const char *destination )
{
	irc_pong( origin );
	if( ircd_srv.burst ) {
		irc_ping( me.name, origin, origin );
	}
}

/** @brief do_pong
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_pong( const char *origin, const char *destination )
{
	Client *s;
	CmdParams * cmdparams;

	s = FindServer( origin );
	if( s ) {
		s->server->ping = me.now - me.tslastping;
		if( me.ulag > 1 )
			s->server->ping -= me.ulag;
		if( IsMe( s ) )
			me.ulag = me.s->server->ping;
		cmdparams =( CmdParams* )ns_calloc( sizeof( CmdParams ) );
		cmdparams->source = s;
		SendAllModuleEvent( EVENT_PONG, cmdparams );
		ns_free( cmdparams );
		return;
	}
	nlog( LOG_NOTICE, "Received PONG from unknown server: %s", origin );
}

/** @brief Display NeoStats version info
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_version( const char *nick, const char *remoteserver )
{
	SET_SEGV_LOCATION();
	irc_numeric( RPL_VERSION, nick, "%s :%s -> %s %s", me.version, me.name, ns_module_info.build_date, ns_module_info.build_time );
	ModulesVersion( nick, remoteserver );
}

/** @brief Display our MOTD Message of the Day from the external neostats.motd file 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_motd( const char *nick, const char *remoteserver )
{
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	fp = fopen( MOTD_FILENAME, "rt" );
	if( !fp ) {
		irc_numeric( ERR_NOMOTD, nick, _( ":- MOTD file Missing" ) );
	} else {
		irc_numeric( RPL_MOTDSTART, nick, _( ":- %s Message of the Day -" ), me.name );
		irc_numeric( RPL_MOTD, nick, _( ":- %s. Copyright (c) 1999 - 2005 The NeoStats Group" ), me.version );
		irc_numeric( RPL_MOTD, nick, ":-" );

		while( fgets( buf, sizeof( buf ), fp ) ) {
			buf[strnlen( buf, BUFSIZE ) - 1] = 0;
			irc_numeric( RPL_MOTD, nick, ":- %s", buf );
		}
		fclose( fp );
		irc_numeric( RPL_ENDOFMOTD, nick, _( ":End of MOTD command." ) );
	}
}

/** @brief Display the ADMIN Message from the external stats.admin file
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_admin( const char *nick, const char *remoteserver )
{
	FILE *fp;
	char buf[BUFSIZE];
	SET_SEGV_LOCATION();

	fp = fopen( ADMIN_FILENAME, "rt" );
	if( !fp ) {
		irc_numeric( ERR_NOADMININFO, nick, _( "%s :No administrative info available" ), me.name );
	} else {
		irc_numeric( RPL_ADMINME, nick, _( ":%s :Administrative info" ), me.name );
		irc_numeric( RPL_ADMINME, nick, _( ":%s.  Copyright (c) 1999 - 2005 The NeoStats Group" ), me.version );
		while( fgets( buf, sizeof( buf ), fp ) ) {
			buf[strnlen( buf, BUFSIZE ) - 1] = 0;
			irc_numeric( RPL_ADMINLOC1, nick, ":- %s", buf );
		}
		fclose( fp );
		irc_numeric( RPL_ADMINLOC2, nick, _( "End of /ADMIN command." ) );
	}
}

/** @brief 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_credits( const char *nick, const char *remoteserver )
{
	SET_SEGV_LOCATION();
	irc_numeric( RPL_INFO, nick, ":- NeoStats %s Credits ", me.version );
	irc_numeric( RPL_INFO, nick, ":- Now Maintained by Fish (fish@dynam.ac) and Mark (mark@ctcp.net)" );
	irc_numeric( RPL_INFO, nick, ":- Previous Authors: Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)" );
	irc_numeric( RPL_INFO, nick, ":- For Support, you can find us at" );
	irc_numeric( RPL_INFO, nick, ":- irc.irc-chat.net #NeoStats" );
	irc_numeric( RPL_INFO, nick, ":- Thanks to:" );
	irc_numeric( RPL_INFO, nick, ":- Enigma for being part of the dev team" );
	irc_numeric( RPL_INFO, nick, ":- Stskeeps for writing the best IRCD ever!" );
	irc_numeric( RPL_INFO, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)" );
	irc_numeric( RPL_INFO, nick, ":- monkeyIRCD for the Module Segv Catching code" );
	irc_numeric( RPL_INFO, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!" );
	irc_numeric( RPL_INFO, nick, ":- Andy For Ideas" );
	irc_numeric( RPL_INFO, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies" );
	irc_numeric( RPL_INFO, nick, ":- sre and Jacob for development systems and access" );
	irc_numeric( RPL_INFO, nick, ":- Error51 for Translating our FAQ and README files" );
	irc_numeric( RPL_INFO, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!" );
	irc_numeric( RPL_INFO, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't( and all the bug reports! )" );
	irc_numeric( RPL_INFO, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips : )" );
	irc_numeric( RPL_INFO, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback" );
	irc_numeric( RPL_INFO, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd" );
	irc_numeric( RPL_INFO, nick, ":- Blud - Giving us patches for Mystic IRCd" );
	irc_numeric( RPL_INFO, nick, ":- herrohr - Giving us patches for Liquid IRCd support" );
	irc_numeric( RPL_INFO, nick, ":- OvErRiTe - Giving us patches for Viagra IRCd support" );
	irc_numeric( RPL_INFO, nick, ":- Reed Loden - Contributions to IRCu support" );
	irc_numeric( RPL_INFO, nick, ":- Adam Rutter (Shmad) - Developer from the 1.0 days to 2.0 Days");
	irc_numeric( RPL_INFO, nick, ":- DeadNotBuried - early testing of 3.0, providing patches and feedback and his NeoStats modules" );
	irc_numeric( RPL_ENDOFINFO, nick, ":End of /CREDITS." );
}

/** @brief 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_stats( const char *nick, const char *what )
{
	ircd_cmd* ircd_cmd_ptr;
	time_t tmp;
	time_t tmp2;
	Client *u;

	SET_SEGV_LOCATION();
	u = FindUser( nick );
	if( !u ) {
		nlog( LOG_WARNING, "do_stats: message from unknown user %s", nick );
		return;
	}
	if( !ircstrcasecmp( what, "u" ) ) {
		/* server uptime - Shmad */
		time_t uptime = me.now - me.ts_boot;
		irc_numeric( RPL_STATSUPTIME, u->name, __( "Statistical Server up %ld days, %ld:%02ld:%02ld", u ), uptime / 86400,( uptime / 3600 ) % 24,( uptime / 60 ) % 60, uptime % 60 );
	} else if( !ircstrcasecmp( what, "c" ) ) {
		/* Connections */
		irc_numeric( RPL_STATSNLINE, u->name, "N *@%s * * %d 50", me.uplink, me.port );
		irc_numeric( RPL_STATSCLINE, u->name, "C *@%s * * %d 50", me.uplink, me.port );
	} else if( !ircstrcasecmp( what, "o" ) ) {
		/* Operators */
	} else if( !ircstrcasecmp( what, "l" ) ) {
		/* Port Lists */
		tmp = me.now - me.lastmsg;
		tmp2 = me.now - me.ts_boot;
		irc_numeric( RPL_STATSLINKINFO, u->name, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE" );
		irc_numeric( RPL_STATSLLINE, u->name, "%s 0 %d %d %d %d %d 0 :%d", me.uplink,( int )me.SendM,( int )me.SendBytes,( int )me.RcveM,( int )me.RcveBytes,( int )tmp2,( int )tmp );
        } else if( !ircstrcasecmp( what, "Z" ) ) {
                if( UserLevel( u ) >= NS_ULEVEL_ADMIN ) {
                        do_dns_stats_Z( u );
                }
	} else if( !ircstrcasecmp( what, "M" ) ) {
		ircd_cmd_ptr = cmd_list;
		while( ircd_cmd_ptr->name ) {
			if( ircd_cmd_ptr->usage > 0 ) {
				irc_numeric( RPL_STATSCOMMANDS, u->name, "Command %s Usage %d", ircd_cmd_ptr->name, ircd_cmd_ptr->usage );
			}
			ircd_cmd_ptr ++;
		}
	}
	irc_numeric( RPL_ENDOFSTATS, u->name, __( "%s :End of /STATS report", u ), what );
	irc_chanalert( ns_botptr, _( "%s Requested Stats %s" ), u->name, what );
};

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_protocol( char *origin, char **argv, int argc )
{
	protocol_entry *protocol_ptr;
	int i;

	for( i = 0; i < argc; i++ ) {
		protocol_ptr = protocol_list;
		while( protocol_ptr->token )
		{
			if( !ircstrcasecmp( protocol_ptr->token, argv[i] ) ) {
				if( protocol_info->options&protocol_ptr->flag ) {
					ircd_srv.protocol |= protocol_ptr->flag;
					break;
				}
			}
			protocol_ptr ++;
		}
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

/* SJOIN <TS> #<channel> <modes> :[@][+]<nick_1> ...  [@][+]<nick_n> */

void do_sjoin( char *tstime, char *channame, char *modes, char *sjoinnick, char **argv, int argc )
{
	char nick[MAXNICK];
	char *nicklist;
	long mask = 0;
	int ok = 1, j = 3;
	Channel *c;
	char **param;
	int paramcnt = 0;
	int paramidx = 0;

	if( *modes == '#' ) {
		JoinChannel( sjoinnick, modes );
		return;
	}

	paramcnt = split_buf( argv[argc-1], &param, 0 );
		   
	while( paramcnt > paramidx ) {
		nicklist = param[paramidx];
		if( ircd_srv.protocol & PROTOCOL_SJ3 ) {
			/* Unreal passes +b( & ) and +e( " ) via SJ3 so skip them for now */	
			if( *nicklist == '&' || *nicklist == '"' ) {
				dlog( DEBUG1, "Skipping %s", nicklist );
				paramidx++;
				continue;
			}
		}
		mask = 0;
		while( CmodePrefixToMask( *nicklist ) ) {
			mask |= CmodePrefixToMask( *nicklist );
			nicklist ++;
		}
		strlcpy( nick, nicklist, MAXNICK );
		JoinChannel( nick, channame ); 
		ChanUserMode( channame, nick, 1, mask );
		paramidx++;
		ok = 1;
	}
	c = FindChannel( channame );
	if (c) {
		/* update the TS time */
		c->creationtime = atoi( tstime );
		j = ChanModeHandler( c, modes, j, argv, argc );
	}
	ns_free( param );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_netinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname )
{
	ircd_srv.maxglobalcnt = atoi( maxglobalcnt );
	ircd_srv.tsendsync = atoi( tsendsync );
	ircd_srv.uprot = atoi( prot );
	strlcpy( ircd_srv.cloak, cloak, CLOAKKEYLEN );
	strlcpy( me.netname, netname, MAXPASS );
	irc_netinfo( me.name, maxglobalcnt, ( unsigned long )me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname );
	do_synch_neostats();
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_snetinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname )
{
	ircd_srv.uprot = atoi( prot );
	strlcpy( ircd_srv.cloak, cloak, CLOAKKEYLEN );
	strlcpy( me.netname, netname, MAXPASS );
	irc_snetinfo( me.name, maxglobalcnt, ( unsigned long )me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname );
	do_synch_neostats();
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_join( const char *nick, const char *chanlist, const char *keys )
{
	char *s, *t;
	t =( char *)chanlist;
	while( *( s = t ) ) {
		t = s + strcspn( s, "," );
		if( *t )
			*t++ = 0;
		JoinChannel( nick, s );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_part( const char *nick, const char *chan, const char *reason )
{
	PartChannel( FindUser( nick ), chan, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_nick( const char *nick, const char *hopcount, const char *TS, 
		 const char *user, const char *host, const char *server, 
		 const char *ip, const char *servicestamp, const char *modes, 
		 const char *vhost, const char *realname, const char *numeric, 
		 const char *smodes )
{
	if( !nick ) {
		nlog( LOG_CRITICAL, "do_nick: trying to add user with NULL nickname" );
		return;
	}
	AddUser( nick, user, host, realname, server, ip, TS, numeric );
	if( modes ) {
		UserMode( nick, modes );
	}
	if( vhost ) {
		SetUserVhost( nick, vhost );
	}
	if( smodes ) {
		UserSMode( nick, smodes );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_client( const char *nick, const char *hopcount, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *servicestamp, 
		const char *ip, const char *realname )
{
	do_nick( nick, hopcount, TS, user, host, server, ip, servicestamp, 
		modes, vhost, realname, NULL, smodes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_kill( const char *source, const char *nick, const char *reason )
{
	KillUser( source, nick, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_quit( const char *nick, const char *quitmsg )
{
	QuitUser( nick, quitmsg );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_squit( const char *name, const char *reason )
{
	DelServer( name, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_kick( const char *kickby, const char *chan, const char *kicked, const char *kickreason )
{
	KickChannel( kickby, chan, kicked, kickreason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_svinfo( void )
{
	irc_svinfo( TS_CURRENT, TS_MIN,( unsigned long )me.now );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_vctrl( const char *uprot, const char *nicklen, const char *modex, const char *gc, const char *netname )
{
	ircd_srv.uprot = atoi( uprot );
	ircd_srv.nicklen = atoi( nicklen );
	ircd_srv.modex = atoi( modex );
	ircd_srv.gc = atoi( gc );
	strlcpy( me.netname, netname, MAXPASS );
	irc_vctrl( ircd_srv.uprot, ircd_srv.nicklen, ircd_srv.modex, ircd_srv.gc, me.netname );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_smode( const char *targetnick, const char *modes )
{
	UserSMode( targetnick, modes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_mode_user( const char *targetnick, const char *modes )
{
	UserMode( targetnick, modes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_svsmode_user( const char *targetnick, const char *modes, const char *ts )
{
	char modebuf[MODESIZE];
	
	if( ts && isdigit( *ts ) ) {
		const char *pModes;	
		char *pNewModes;	

		SetUserServicesTS( targetnick, ts );
		/* If only setting TS, we do not need further mode processing */
		if( ircstrcasecmp( modes, "+d" ) == 0 ) {
			dlog( DEBUG3, "dropping modes since this is a services TS %s", modes );
			return;
		}
		/* We need to strip the d from the mode string */
		pNewModes = modebuf;
		pModes = modes;
		while( *pModes ) {
			if( *pModes != 'd' ) {
				*pNewModes = *pModes;
			}
			pModes++;
			pNewModes++;			
		}
		/* NULL terminate */
		*pNewModes = 0;
		UserMode( targetnick, modebuf );
	} else {
		UserMode( targetnick, modes );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_mode_channel( char *origin, char **argv, int argc )
{
	ChanMode( origin, argv, argc );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_away( const char *nick, const char *reason )
{
	UserAway( nick, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_vhost( const char *nick, const char *vhost )
{
	SetUserVhost( nick, vhost );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_nickchange( const char *oldnick, const char *newnick, const char *ts )
{
	UserNickChange( oldnick, newnick, ts );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_topic( const char *chan, const char *owner, const char *ts, const char *topic )
{
	ChannelTopic( chan, owner, ts, topic );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_server( const char *name, const char *uplink, const char *hops, const char *numeric, const char *infoline, int srv )
{
	if( !srv ) {
		if( uplink == NULL || *uplink == 0 ) {
			me.s = AddServer( name, me.name, hops, numeric, infoline );
		} else {
			me.s = AddServer( name, uplink, hops, numeric, infoline );
		}
	} else {
		AddServer( name, uplink, hops, numeric, infoline );
	}
	irc_serverreqversion( me.name, name );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_burst( char *origin, char **argv, int argc )
{
	if( argc > 0 ) {
		if( ircd_srv.burst == 1 ) {
			irc_burst( 0 );
			ircd_srv.burst = 0;
			do_synch_neostats();
		}
	} else {
		ircd_srv.burst = 1;
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_swhois( char *who, char *swhois )
{
	Client * u;
	u = FindUser( who );
	if( u ) {
		strlcpy( u->user->swhois, swhois, MAXHOST );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_tkl( const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason )
{
	static char mask[MAXHOST];

	ircsnprintf( mask, MAXHOST, "%s@%s", user, host );
	if( add[0] == '+' ) {
		AddBan( type, user, host, mask, reason, setby, tsset, tsexpire );
	} else {
		DelBan( type, user, host, mask, reason, setby, tsset, tsexpire );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_eos( const char *name )
{
	Client *s;

	s = FindServer( name );
	if( s ) {
		SynchServer( s );
		dlog( DEBUG1, "do_eos: server %s is now synched", name );
	} else {
		nlog( LOG_WARNING, "do_eos: server %s not found", name );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_setname( const char *nick, const char *realname )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_setname: setting realname of user %s to %s", nick, realname );
		strlcpy( u->info,( char *)realname, MAXHOST );
	} else {
		nlog( LOG_WARNING, "do_setname: user %s not found", nick );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_svsnick( const char *oldnick, const char *newnick, const char *ts )
{
	do_nickchange( oldnick, newnick, ts );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_sethost( const char *nick, const char *host )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_sethost: setting host of user %s to %s", nick, host );
		strlcpy( u->user->hostname,( char *)host, MAXHOST );
	} else {
		nlog( LOG_WARNING, "do_sethost: user %s not found", nick );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_setident( const char *nick, const char *ident )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_setident: setting ident of user %s to %s", nick, ident );
		strlcpy( u->user->username,( char *)ident, MAXHOST );
	} else {
		nlog( LOG_WARNING, "do_setident: user %s not found", nick );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_chghost( const char *nick, const char *host )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_chghost: setting host of user %s to %s", nick, host );
		strlcpy( u->user->hostname,( char *)host, MAXHOST );
	} else {
		nlog( LOG_WARNING, "do_chghost: user %s not found", nick );
	}
}

