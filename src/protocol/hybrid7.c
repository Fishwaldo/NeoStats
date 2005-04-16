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
#include "ircd.h"
#include "hybrid7.h"
#include "services.h"

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_topic( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );

/* buffer sizes */
const int proto_maxhost		=( 128 + 1 );
const int proto_maxpass		=( 32 + 1 );
const int proto_maxnick		=( 32 + 1 );
const int proto_maxuser		=( 10 + 1 );
const int proto_maxrealname	=( 50 + 1 );
const int proto_chanlen		=( 200 + 1 );
const int proto_topiclen	=( 512 + 1 );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_UNKLN,
	/* Features supported by this IRCd */
	0,
	"+o",
	"+o",
};

/* this is the command list and associated functions to run */
ircd_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{MSG_PRIVATE, 0, _m_private, 0},
	{MSG_NOTICE, 0, _m_notice, 0},
	{MSG_STATS, 0, _m_stats, 0},
	{MSG_VERSION, 0, _m_version, 0},
	{MSG_MOTD, 0, _m_motd, 0},
	{MSG_ADMIN, 0, _m_admin, 0},
	{MSG_CREDITS, 0, _m_credits, 0},
	{MSG_SERVER, 0, m_server, 0},
	{MSG_SQUIT, 0, _m_squit, 0},
	{MSG_QUIT, 0, _m_quit, 0},
	{MSG_MODE, 0, _m_mode, 0},
	{MSG_KILL, 0, _m_kill, 0},
	{MSG_PONG, 0, _m_pong, 0},
	{MSG_AWAY, 0, _m_away, 0},
	{MSG_NICK, 0, m_nick, 0},
	{MSG_TOPIC, 0, m_topic, 0},
	{MSG_KICK, 0, _m_kick, 0},
	{MSG_JOIN, 0, _m_join, 0},
	{MSG_PART, 0, _m_part, 0},
	{MSG_PING, 0, _m_ping, 0},
	{MSG_SVINFO, 0, _m_svinfo, 0},
	{MSG_PASS, 0, _m_pass, 0},
	{MSG_EOB, 0, m_burst, 0},
	{MSG_SJOIN, 0, m_sjoin, 0},
	{MSG_CAPAB, 0, _m_capab, 0},
	{MSG_WALLOPS,	0, _m_wallops, 0},
	{MSG_CHATOPS,	0, _m_chatops, 0},
	{MSG_ERROR, 0, _m_error, 0},
	{0, 0, 0, 0},
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{0, 0, 0},
};

mode_init chan_modes[] = 
{
	{'e', CMODE_EXCEPT, MODEPARAM},
	{'I', CMODE_INVEX, MODEPARAM},
	{'a', CMODE_HIDEOPS, 0},
	{0, 0, 0},
};

mode_init user_umodes[] = 
{
	{'d', UMODE_DEBUG},
	{'a', UMODE_ADMIN},
	{'l', UMODE_LOCOP},
	{'b', UMODE_BOTS},
	{'c', UMODE_CLIENT},
	{'f', UMODE_FULL},
	{'g', UMODE_CALLERID},
	{'k', UMODE_SKILL},
	{'n', UMODE_NCHANGE},
	{'r', UMODE_REJ},
	{'s', UMODE_SERVNOTICE},
	{'u', UMODE_UNAUTH},
	{'w', UMODE_WALLOP},
	{'x', UMODE_EXTERNAL},
	{'y', UMODE_SPY},
	{'z', UMODE_OPERWALL},
	{0, 0},
};

void send_eob( const char *server )
{
	send_cmd( ":%s %s", server, MSG_EOB );
}

void send_server( const char *source, const char *name, const int numeric, const char *infoline )
{
	send_cmd( ":%s %s %s %d :%s", source, MSG_SERVER, name, numeric, infoline );
}

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	send_cmd( "CAPAB :TS EX CHW IE EOB KLN GLN KNOCK HOPS HUB AOPS MX" );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
}

void send_burst( int b )
{
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, target );
}

void send_cmode( const char *source, const char *who, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", who, MSG_MODE, chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, ts, newmode, ident, host, server, realname );
}

void send_umode( const char *source, const char *target, const char *mode )
{
	send_cmd( ":%s %s %s :%s", source, MSG_MODE, target, mode );
}

void send_snetinfo( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSG_SNETINFO, ts, prot, cloak, netname );
}

void send_netinfo( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSG_NETINFO, ts, prot, cloak, netname );
}

void send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts )
{
	send_cmd( ":%s %s %s %lu", oldnick, MSG_NICK, newnick, ts );
}

void send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow )
{
	send_cmd( "%s %d %d 0 :%lu", MSG_SVINFO, tscurrent, tsmin, tsnow );
}

/* there isn't an akill on Hybrid, so we send a kline to all servers! */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s * %lu %s %s :%s", setby, MSG_KLINE, length, ident, host, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	if( ircd_srv.protocol&PROTOCOL_UNKLN ) {
		send_cmd( ":%s %s %s %s", source, MSG_UNKLINE, ident, host );
	} else {
		irc_chanalert( ns_botptr, "Please Manually remove KLINES using /unkline on each server" );
	}
}

/* source SJOIN unsigned long chan modes param:" */
static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1],( ( argc <= 2 ) ? argv[1] : argv[2] ), argv[4], argv, argc );
}

static void m_burst( char *origin, char **argv, int argc, int srv )
{
	send_eob( me.name );
	do_synch_neostats( );
}

static void m_server( char *origin, char **argv, int argc, int srv )
{
	do_server( argv[0], origin, argv[1], NULL, argv[2], srv );
}

/*  m_nick
 *    argv[0] = nickname
 *    argv[1] = hop count
 *    argv[2] = TS
 *    argv[3] = umode
 *    argv[4] = username
 *    argv[5] = hostname
 *    argv[6] = server
 *    argv[7] = ircname
 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			NULL, NULL, argv[3], NULL, argv[7], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], NULL );
	}
}

static void m_topic( char *origin, char **argv, int argc, int srv )
{
	/*
	** Hybrid uses two different formats for the topic change protocol... 
	** :user TOPIC channel :topic 
	** and 
	** :server TOPIC channel author topicts :topic 
	** Both forms must be accepted.
	** - Hwy
	*/	
	if( FindUser( origin ) ) {
		do_topic( argv[0], origin, NULL, argv[1] );
	} else if( FindServer( origin ) ) {
		do_topic( argv[0], argv[1], argv[2], argv[3] );
	} else {
		nlog( LOG_WARNING, "m_topic: can't find topic setter %s for topic %s", origin, argv[1] ); 
	}
}
