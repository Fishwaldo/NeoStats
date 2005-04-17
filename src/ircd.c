/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

/*  TODO:
 *  - Extend akill system to send different ban types (e.g. zlines)
 */

#include "neostats.h"
#include "ircd.h"
#include "numerics.h"
#include "modes.h"
#include "modules.h"
#include "dl.h"
#include "bots.h"
#include "commands.h"
#include "sock.h"
#include "users.h"
#include "channels.h"
#include "services.h"
#include "servers.h"
#include "bans.h"
#include "auth.h"
#include "dns.h"
#include "base64.h"
#include "dcc.h"
#include "main.h"

#define MOTD_FILENAME	"neostats.motd"
#define ADMIN_FILENAME	"neostats.admin"

typedef struct ircd_sym 
{
	void **handler;
	void *defaulthandler;
	char *sym;
	unsigned int required;
	unsigned int feature;
} ircd_sym;

typedef struct msgtok_sym 
{
	void **handler;
	char **msgptr;
	char *msgsym;
	char **tokptr;
	char *toksym;
	void *defaulthandler;
	unsigned int feature;
} msgtok_sym;

typedef struct protocol_entry {
	char *token;
	unsigned int flag;
} protocol_entry;

ircd_server ircd_srv;

static char ircd_buf[BUFSIZE];
static char protocol_path[MAXPATH];

static ProtocolInfo *protocol_info;

static ircd_cmd *cmd_list;

static void *protocol_module_handle;

static void _send_numeric( const char *source, const int numeric, const char *target, const char *buf );
static void _send_privmsg( const char *source, const char *target, const char *buf );
static void _send_notice( const char *source, const char *target, const char *buf );
static void _send_wallops( const char *source, const char *buf );
static void _send_globops( const char *source, const char *buf );
static void _send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts );
static void _send_umode( const char *source, const char *target, const char *mode );
static void _send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts );
static void _send_join( const char *source, const char *chan, const char *key, const unsigned long ts );
static void _send_part( const char *source, const char *chan, const char *reason );
static void _send_kick( const char *source, const char *chan, const char *target, const char *reason );
static void _send_invite( const char *source, const char *target, const char *chan );
static void _send_quit( const char *source, const char *quitmsg );
static void _send_ping( const char *source, const char *reply, const char *target );
static void _send_pong( const char *reply );
static void _send_server( const char *source, const char *name, const int numeric, const char *infoline );
static void _send_squit( const char *server, const char *quitmsg );
static void _send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow );
static void _send_kill( const char *source, const char *target, const char *reason );
static void _send_setname( const char *nick, const char *realname );
static void _send_sethost( const char *nick, const char *host );
static void _send_setident( const char *nick, const char *ident );
static void _send_svsnick( const char *source, const char *target, const char *newnick, const unsigned long ts );
static void _send_svsjoin( const char *source, const char *target, const char *chan );
static void _send_svspart( const char *source, const char *target, const char *chan );
static void _send_svsmode( const char *source, const char *target, const char *modes );
static void _send_svskill( const char *source, const char *target, const char *reason );

static void( *irc_send_privmsg )( const char *source, const char *to, const char *buf );
static void( *irc_send_notice )( const char *source, const char *to, const char *buf );
static void( *irc_send_globops )( const char *source, const char *buf );
static void( *irc_send_wallops )( const char *source, const char *buf );
static void( *irc_send_numeric )( const char *source, const int numeric, const char *target, const char *buf );
static void( *irc_send_quit )( const char *source, const char *quitmsg );
static void( *irc_send_umode )( const char *source, const char *target, const char *mode );
static void( *irc_send_join )( const char *source, const char *chan, const char *key, const unsigned long ts );
static void( *irc_send_sjoin )( const char *source, const char *who, const char *chan, const unsigned long ts );
static void( *irc_send_part )( const char *source, const char *chan, const char *reason );
static void( *irc_send_nickchange )( const char *oldnick, const char *newnick, const unsigned long ts );
static void( *irc_send_cmode )( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, unsigned long ts );
static void( *irc_send_kill )( const char *source, const char *target, const char *reason );
static void( *irc_send_kick )( const char *source, const char *chan, const char *target, const char *reason );
static void( *irc_send_invite )( const char *source, const char *to, const char *chan );
static void( *irc_send_svskill )( const char *source, const char *target, const char *reason );
static void( *irc_send_svsmode )( const char *source, const char *target, const char *modes );
static void( *irc_send_svshost )( const char *source, const char *who, const char *vhost );
static void( *irc_send_svsjoin )( const char *source, const char *target, const char *chan );
static void( *irc_send_svspart )( const char *source, const char *target, const char *chan );
static void( *irc_send_svsnick )( const char *source, const char *target, const char *newnick, const unsigned long ts );
static void( *irc_send_swhois )( const char *source, const char *target, const char *swhois );
static void( *irc_send_smo )( const char *source, const char *umodetarget, const char *msg );
static void( *irc_send_akill )( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, unsigned long ts );
static void( *irc_send_rakill )( const char *source, const char *host, const char *ident );
static void( *irc_send_sqline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unsqline )( const char *source, const char *mask );
static void( *irc_send_sgline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unsgline )( const char *source, const char *mask );
static void( *irc_send_zline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unzline )( const char *source, const char *mask );
static void( *irc_send_ping )( const char *source, const char *reply, const char *to );
static void( *irc_send_pong )( const char *reply );
static void( *irc_send_server )( const char *source, const char *name, const int numeric, const char *infoline );
static void( *irc_send_squit )( const char *server, const char *quitmsg );
static void( *irc_send_nick )( const char *nick, const unsigned long ts, const char* newmode, const char *ident, const char *host, const char* server, const char *realname );
static void( *irc_send_server_connect )( const char *name, const int numeric, const char *infoline, const char *pass, unsigned long tsboot, unsigned long tslink );
static void( *irc_send_netinfo )( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts );
static void( *irc_send_snetinfo )( const char* source, const int prot, const char* cloak, const char* netname, const unsigned long ts );
static void( *irc_send_svinfo )( const int tscurrent, const int tsmin, const unsigned long tsnow );
static void( *irc_send_vctrl )( const int uprot, const int nicklen, const int modex, const int gc, const char* netname );
static void( *irc_send_burst )( int b );
static void( *irc_send_svstime )( const char *source, const unsigned long ts );
static void( *irc_send_setname )( const char* nick, const char* realname );
static void( *irc_send_sethost )( const char* nick, const char* host );
static void( *irc_send_setident )( const char* nick, const char* ident );
static void( *irc_send_serverrequptime )( const char *source, const char *target );
static void( *irc_send_serverreqversion )( const char *source, const char *target );
static void( *irc_send_cloakhost )( char* host );

static char *MSG_PRIVATE;
static char *TOK_PRIVATE;
static char *MSG_NOTICE;
static char *TOK_NOTICE;
static char *MSG_WALLOPS;
static char *TOK_WALLOPS;
static char *MSG_GLOBOPS;
static char *TOK_GLOBOPS;
static char *MSG_STATS;
static char *TOK_STATS;
static char *MSG_VERSION;
static char *TOK_VERSION;
static char *MSG_MOTD;
static char *TOK_MOTD;
static char *MSG_ADMIN;
static char *TOK_ADMIN;
static char *MSG_CREDITS;
static char *TOK_CREDITS;
static char *MSG_NICK;
static char *TOK_NICK;
static char *MSG_MODE;
static char *TOK_MODE;
static char *MSG_QUIT;
static char *TOK_QUIT;
static char *MSG_JOIN;
static char *TOK_JOIN;
static char *MSG_PART;
static char *TOK_PART;
static char *MSG_KICK;
static char *TOK_KICK;
static char *MSG_INVITE;
static char *TOK_INVITE;
static char *MSG_PING;
static char *TOK_PING;
static char *MSG_PONG;
static char *TOK_PONG;
static char *MSG_SERVER;
static char *TOK_SERVER;
static char *MSG_SQUIT;
static char *TOK_SQUIT;
static char *MSG_SVINFO;
static char *TOK_SVINFO;
static char *MSG_KILL;
static char *TOK_KILL;
static char *MSG_SETNAME;
static char *TOK_SETNAME;
static char *MSG_SETHOST;
static char *TOK_SETHOST;
static char *MSG_SETIDENT;
static char *TOK_SETIDENT;
static char *MSG_SVSNICK;
static char *TOK_SVSNICK;
static char *MSG_SVSJOIN;
static char *TOK_SVSJOIN;
static char *MSG_SVSPART;
static char *TOK_SVSPART;
static char *MSG_SVSMODE;
static char *TOK_SVSMODE;
static char *MSG_SVSKILL;
static char *TOK_SVSKILL;

msgtok_sym msgtok_sym_table[] =
{
	{( void * )&irc_send_privmsg, &MSG_PRIVATE, "MSG_PRIVATE", &TOK_PRIVATE, "TOK_PRIVATE", _send_privmsg, 0 },
	{( void * )&irc_send_notice, &MSG_NOTICE, "MSG_NOTICE", &TOK_NOTICE, "TOK_NOTICE", _send_notice, 0 },
	{( void * )&irc_send_wallops, &MSG_WALLOPS, "MSG_WALLOPS", &TOK_WALLOPS, "TOK_WALLOPS", _send_wallops, 0 },
	{( void * )&irc_send_globops, &MSG_GLOBOPS, "MSG_GLOBOPS", &TOK_GLOBOPS, "TOK_GLOBOPS", _send_globops, 0 },
	{( void * )NULL, &MSG_STATS, "MSG_STATS", &TOK_STATS, "TOK_STATS", NULL, 0 },
	{( void * )NULL, &MSG_VERSION, "MSG_VERSION", &TOK_VERSION, "TOK_VERSION", NULL, 0 },
	{( void * )NULL, &MSG_MOTD, "MSG_MOTD", &TOK_MOTD, "TOK_MOTD", NULL, 0 },
	{( void * )NULL, &MSG_ADMIN, "MSG_ADMIN", &TOK_ADMIN, "TOK_ADMIN", NULL, 0 },
	{( void * )NULL, &MSG_CREDITS, "MSG_CREDITS", &TOK_CREDITS, "TOK_CREDITS", NULL, 0 },
	{( void * )&irc_send_nickchange, &MSG_NICK, "MSG_NICK", &TOK_NICK, "TOK_NICK", _send_nickchange, 0 },
	{( void * )&irc_send_umode, &MSG_MODE, "MSG_MODE", &TOK_MODE, "TOK_MODE", _send_umode, 0 },
	{( void * )&irc_send_cmode, &MSG_MODE, "MSG_MODE", &TOK_MODE, "TOK_MODE", _send_cmode, 0 },
	{( void * )&irc_send_quit, &MSG_QUIT, "MSG_QUIT", &TOK_QUIT, "TOK_QUIT", _send_quit, 0 },
	{( void * )&irc_send_join, &MSG_JOIN, "MSG_JOIN", &TOK_JOIN, "TOK_JOIN", _send_join, 0 },
	{( void * )&irc_send_part, &MSG_PART, "MSG_PART", &TOK_PART, "TOK_PART", _send_part, 0 },
	{( void * )&irc_send_kick, &MSG_KICK, "MSG_KICK", &TOK_KICK, "TOK_KICK", _send_kick, 0 },
	{( void * )&irc_send_invite, &MSG_INVITE, "MSG_INVITE", &TOK_INVITE, "TOK_INVITE", _send_invite, 0 },
	{( void * )&irc_send_ping, &MSG_PING, "MSG_PING", &TOK_PING, "TOK_PING", _send_ping, 0 },
	{( void * )&irc_send_pong, &MSG_PONG, "MSG_PONG", &TOK_PONG, "TOK_PONG", _send_pong, 0 },
	{( void * )&irc_send_server, &MSG_SERVER, "MSG_SERVER", &TOK_SERVER, "TOK_SERVER", _send_server, 0 },
	{( void * )&irc_send_squit, &MSG_SQUIT, "MSG_SQUIT", &TOK_SQUIT, "TOK_SQUIT", _send_squit, 0 },
	{( void * )&irc_send_svinfo, &MSG_SVINFO, "MSG_SVINFO", &TOK_SVINFO, "TOK_SVINFO", _send_svinfo, 0 },
	{( void * )&irc_send_kill, &MSG_KILL, "MSG_KILL", &TOK_KILL, "TOK_KILL", _send_kill, 0 },
	{( void * )&irc_send_setname, &MSG_SETNAME, "MSG_SETNAME", &TOK_SETNAME, "TOK_SETNAME", _send_setname, 0 },
	{( void * )&irc_send_sethost, &MSG_SETHOST, "MSG_SETHOST", &TOK_SETHOST, "TOK_SETHOST", _send_sethost, 0 },
	{( void * )&irc_send_setident, &MSG_SETIDENT, "MSG_SETIDENT", &TOK_SETIDENT, "TOK_SETIDENT", _send_setident, 0 },
	{( void * )&irc_send_svsnick, &MSG_SVSNICK, "MSG_SVSNICK", &TOK_SVSNICK, "TOK_SVSNICK", _send_svsnick, 0 },
	{( void * )&irc_send_svsjoin, &MSG_SVSJOIN, "MSG_SVSJOIN", &TOK_SVSJOIN, "TOK_SVSJOIN", _send_svsjoin, 0 },
	{( void * )&irc_send_svspart, &MSG_SVSPART, "MSG_SVSPART", &TOK_SVSPART, "TOK_SVSPART", _send_svspart, 0 },
	{( void * )&irc_send_svsmode, &MSG_SVSMODE, "MSG_SVSMODE", &TOK_SVSMODE, "TOK_SVSMODE", _send_svsmode, 0 },
	{( void * )&irc_send_svskill, &MSG_SVSKILL, "MSG_SVSKILL", &TOK_SVSKILL, "TOK_SVSKILL", _send_svskill, 0 },
	{NULL, NULL, NULL, NULL, NULL, NULL, 0},
};

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

static ircd_sym ircd_sym_table[] = 
{
	{( void * )&irc_send_privmsg, NULL, "send_privmsg", 1, 0},
	{( void * )&irc_send_notice, NULL, "send_notice", 1, 0},
	{( void * )&irc_send_globops, NULL, "send_globops", 0, 0},
	{( void * )&irc_send_wallops, NULL, "send_wallops", 0, 0},
	{( void * )&irc_send_numeric, _send_numeric, "send_numeric", 0, 0},
	{( void * )&irc_send_umode, NULL, "send_umode", 1, 0},
	{( void * )&irc_send_join, NULL, "send_join", 1, 0},
	{( void * )&irc_send_sjoin, NULL, "send_sjoin", 0, 0},
	{( void * )&irc_send_part, NULL, "send_part", 1, 0},
	{( void * )&irc_send_nickchange, NULL, "send_nickchange", 1, 0},
	{( void * )&irc_send_cmode, NULL, "send_cmode", 1, 0},
	{( void * )&irc_send_quit, NULL, "send_quit", 1, 0},
	{( void * )&irc_send_kill, NULL, "send_kill", 0, 0},
	{( void * )&irc_send_kick, NULL, "send_kick", 0, 0},
	{( void * )&irc_send_invite, NULL, "send_invite", 0, 0},
	{( void * )&irc_send_svskill, NULL, "send_svskill", 0, FEATURE_SVSKILL},
	{( void * )&irc_send_svsmode, NULL, "send_svsmode", 0, FEATURE_SVSMODE},
	{( void * )&irc_send_svshost, NULL, "send_svshost", 0, FEATURE_SVSHOST},
	{( void * )&irc_send_svsjoin, NULL, "send_svsjoin", 0, FEATURE_SVSJOIN},
	{( void * )&irc_send_svspart, NULL, "send_svspart", 0, FEATURE_SVSPART},
	{( void * )&irc_send_svsnick, NULL, "send_svsnick", 0, FEATURE_SVSNICK},
	{( void * )&irc_send_swhois, NULL, "send_swhois", 0, FEATURE_SWHOIS},
	{( void * )&irc_send_smo, NULL, "send_smo", 0, FEATURE_SMO},
	{( void * )&irc_send_svstime, NULL, "send_svstime", 0, FEATURE_SVSTIME},
	{( void * )&irc_send_akill, NULL, "send_akill", 0, 0},
	{( void * )&irc_send_sqline, NULL, "send_sqline", 0, 0},
	{( void * )&irc_send_unsqline, NULL, "send_unsqline", 0, 0},
	{( void * )&irc_send_zline, NULL, "send_zline", 0, 0},
	{( void * )&irc_send_unzline, NULL, "send_unzline", 0, 0},
	{( void * )&irc_send_rakill, NULL, "send_rakill", 0, 0},
	{( void * )&irc_send_ping, NULL, "send_ping", 0, 0},
	{( void * )&irc_send_pong, NULL, "send_pong", 0, 0},
	{( void * )&irc_send_server, NULL, "send_server", 0, 0},
	{( void * )&irc_send_squit, NULL, "send_squit", 0, 0},
	{( void * )&irc_send_nick, NULL, "send_nick", 1, 0},
	{( void * )&irc_send_server_connect, NULL, "send_server_connect", 1, 0},
	{( void * )&irc_send_netinfo, NULL, "send_netinfo", 0, 0},
	{( void * )&irc_send_snetinfo, NULL, "send_snetinfo", 0, 0},
	{( void * )&irc_send_svinfo, NULL, "send_svinfo", 0, 0},
	{( void * )&irc_send_vctrl, NULL, "send_vctrl", 0, 0},
	{( void * )&irc_send_burst, NULL, "send_burst", 0, 0},
	{( void * )&irc_send_setname, NULL, "send_setname", 0, 0},
	{( void * )&irc_send_sethost, NULL, "send_sethost", 0, 0},
	{( void * )&irc_send_setident, NULL, "send_setident", 0, 0},
	{( void * )&irc_send_cloakhost, NULL, "cloakhost", 0, 0},
	{( void * )&irc_send_serverrequptime, NULL, "send_serverrequptime", 0, 0},
	{( void * )&irc_send_serverreqversion, NULL, "send_serverreqversion", 0, 0},
	{NULL, NULL, NULL, 0, 0},
};

typedef struct ircd_cmd_intrinsic {
	const char **name;
	const char **token;
	ircd_cmd_handler handler;
	unsigned int usage;
}ircd_cmd_intrinsic;

ircd_cmd_intrinsic intrinsic_cmd_list[] = 
{
	{&MSG_PRIVATE, &TOK_PRIVATE, _m_private, 0},
	{&MSG_NOTICE, &TOK_NOTICE, _m_notice, 0},
	{&MSG_STATS, &TOK_STATS, _m_stats, 0},
	{&MSG_VERSION, &TOK_VERSION, _m_version, 0},
	{&MSG_MOTD, &TOK_MOTD, _m_motd, 0},
	{&MSG_ADMIN, &TOK_ADMIN, _m_admin, 0},
	{&MSG_CREDITS, &TOK_CREDITS, _m_credits, 0},
	{&MSG_SQUIT, &TOK_SQUIT, _m_squit, 0},
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
	{0, 0, 0, 0},
};


static void IrcdError( char* err )
{
	nlog( LOG_CRITICAL, "Unable to find %s in selected IRCd module", err );
}

/** @brief InitIrcdProtocol
 *
 *  Init protocol info
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitIrcdProtocol( void )
{
	protocol_info = ns_dlsym( protocol_module_handle, "protocol_info" );
	if( !protocol_info ) {
		nlog( LOG_CRITICAL, "Unable to find protocol_info in protocol module %s", protocol_path );
		return NS_FAILURE;	
	}
	if( protocol_info->minprotocol & PROTOCOL_CLIENTMODE ) {
		nsconfig.singlebotmode = 1;
	}
	strlcpy( me.servicescmode, protocol_info->services_cmode, MODESIZE );
	strlcpy( me.servicesumode, protocol_info->services_umode, MODESIZE );
	/* set min protocol */
	ircd_srv.protocol = protocol_info->minprotocol;
	/* Allow protocol module to "override" the parser */
	irc_parse = ns_dlsym( protocol_module_handle, "parse" );
	if( irc_parse == NULL ) {
		irc_parse = parse;
	}
	cmd_list = ns_dlsym( protocol_module_handle, "cmd_list" );
	if( !cmd_list ) {
		IrcdError( "command list" );
		return NS_FAILURE;	
	}
	return NS_SUCCESS;
}

/** @brief InitIrcdSymbols
 *
 *  Map protocol module pointers to core pointers and check for minimum 
 *  requirements
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitIrcdSymbols( void )
{
	void **protocol_handler;
	msgtok_sym *pmsgtok_sym;
	ircd_sym *pircd_sym;

	/* Build up core supported message and function table */
	pmsgtok_sym = msgtok_sym_table;
	while( pmsgtok_sym->msgptr )
	{
		*pmsgtok_sym->msgptr = ns_dlsym( protocol_module_handle, pmsgtok_sym->msgsym );
		if( pmsgtok_sym->tokptr )
			*pmsgtok_sym->tokptr = ns_dlsym( protocol_module_handle, pmsgtok_sym->toksym );
		if( pmsgtok_sym->msgptr && pmsgtok_sym->handler )
			*pmsgtok_sym->handler = pmsgtok_sym->defaulthandler;
		pmsgtok_sym ++;
	}
	/* Build up protocol module overrides */
	pircd_sym = ircd_sym_table;
	while( pircd_sym->handler )
	{
		protocol_handler = ns_dlsym( protocol_module_handle, pircd_sym->sym );
		if( protocol_handler )
			*pircd_sym->handler = protocol_handler;
		if( !*pircd_sym->handler ) 
			*pircd_sym->handler = pircd_sym->defaulthandler;
		if( pircd_sym->required && !*pircd_sym->handler ) 
		{
			IrcdError( pircd_sym->sym );
			return NS_FAILURE;	
		}
		ircd_srv.features |= pircd_sym->feature;
		pircd_sym ++;
	}
	return NS_SUCCESS;
}

/** @brief InitIrcdModes
 *
 *  Lookup mode tables and init neostats mode lookup tables
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitIrcdModes( void )
{
	mode_init *chan_umodes;
	mode_init *chan_modes;
	mode_init *user_umodes;
	mode_init *user_smodes;

	chan_umodes = ns_dlsym( protocol_module_handle, "chan_umodes" );
	chan_modes  = ns_dlsym( protocol_module_handle, "chan_modes" );
	user_umodes = ns_dlsym( protocol_module_handle, "user_umodes" );
	/* Not required */
	user_smodes = ns_dlsym( protocol_module_handle, "user_smodes" );
	if( user_smodes ) {
		ircd_srv.features |= FEATURE_USERSMODES;
	}
	if( InitModeTables( chan_umodes, chan_modes, user_umodes, user_smodes ) != NS_SUCCESS ) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief InitIrcd
 *
 *  ircd initialisation
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitIrcd( void )
{
	/* Clear IRCD info */
	os_memset( &ircd_srv, 0, sizeof( ircd_srv ) );
	/* Open protocol module */
	ircsnprintf( protocol_path, 255, "%s/%s%s", MOD_PATH, me.protocol,MOD_EXT );
	nlog( LOG_NORMAL, "Using protocol module %s", protocol_path );
	protocol_module_handle = ns_dlopen( protocol_path, RTLD_NOW || RTLD_GLOBAL );
	if( !protocol_module_handle ) {
		nlog( LOG_CRITICAL, "Unable to load protocol module %s", protocol_path );
		return NS_FAILURE;	
	}
	/* Setup protocol options */
	if( InitIrcdProtocol() != NS_SUCCESS ) {
		return NS_FAILURE;	
	}
	/* Setup IRCD function calls */
	if( InitIrcdSymbols() != NS_SUCCESS ) 
		return NS_FAILURE;
	/* Build mode tables */
	if( InitIrcdModes() != NS_SUCCESS ) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

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

void _m_globops( char* origin, char **av, int ac, int cmdptr )
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

void _m_wallops( char* origin, char **av, int ac, int cmdptr )
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

void _m_chatops( char* origin, char **av, int ac, int cmdptr )
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

void _m_error( char* origin, char **av, int ac, int cmdptr )
{
	fprintf(stderr, "IRCD reported error: %s", av[0] );
	nlog (LOG_ERROR, "IRCD reported error: %s", av[0] );
	do_exit (NS_EXIT_ERROR, av[0] );
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

void _m_pass( char* origin, char **av, int ac, int cmdptr )
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

void _m_notice( char* origin, char **argv, int argc, int cmdptr )
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

void _m_private( char* origin, char **argv, int argc, int cmdptr )
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

/** @brief process ircd commands
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

EXPORTFUNC void process_ircd_cmd( int cmdptr, char *cmd, char* origin, char **av, int ac )
{
	ircd_cmd *ircd_cmd_ptr;
	ircd_cmd_intrinsic *intrinsic_cmd_ptr;

	SET_SEGV_LOCATION();
	ircd_cmd_ptr = cmd_list;
	while( ircd_cmd_ptr->name ) {
		if( !ircstrcasecmp( ircd_cmd_ptr->name, cmd ) || 
		 ( ( ircd_srv.protocol & PROTOCOL_TOKEN ) && ircd_cmd_ptr->token && !ircstrcasecmp( ircd_cmd_ptr->token, cmd ) ) ) {
			if( ircd_cmd_ptr->handler ) {
				dlog( DEBUG3, "process_ircd_cmd: running command %s", ircd_cmd_ptr->name );
				ircd_cmd_ptr->handler( origin, av, ac, cmdptr );
			} else {
				dlog( DEBUG3, "process_ircd_cmd: ignoring command %s", cmd );
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}
	intrinsic_cmd_ptr = intrinsic_cmd_list;
	while( intrinsic_cmd_ptr->handler ) {
		if( !ircstrcasecmp( *intrinsic_cmd_ptr->name, cmd ) || 
		 ( ( ircd_srv.protocol & PROTOCOL_TOKEN ) && *intrinsic_cmd_ptr->token && !ircstrcasecmp( *intrinsic_cmd_ptr->token, cmd ) ) ) {
			dlog( DEBUG3, "process_ircd_cmd: running command %s", *intrinsic_cmd_ptr->name );
			intrinsic_cmd_ptr->handler( origin, av, ac, cmdptr );
			intrinsic_cmd_ptr->usage++;
			return;
		}
		intrinsic_cmd_ptr ++;
	}
	ircd_cmd_ptr = numeric_cmd_list;	
	/* Process numeric replies */
	while( ircd_cmd_ptr->name ) {
		if( !ircstrcasecmp( ircd_cmd_ptr->name, cmd ) ) {
			if( ircd_cmd_ptr->handler ) {
				dlog( DEBUG3, "process_ircd_cmd: running command %s", ircd_cmd_ptr->name );
				ircd_cmd_ptr->handler( origin, av, ac, cmdptr );
			} else {
				dlog( DEBUG3, "process_ircd_cmd: ignoring command %s", cmd );
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}

	nlog( LOG_INFO, "No support for %s", cmd );
}

/** @brief parse
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

int parse(void *arg,  void *rline, size_t len )
{
	char origin[64], cmd[64], *coreLine;
	char *line = (char *)rline;
	int cmdptr = 0;
	int ac = 0;
	char **av = NULL;

	SET_SEGV_LOCATION();
	if( !( *line ) )
		return NS_FAILURE;
	dlog( DEBUG1, "------------------------BEGIN PARSE-------------------------" );
	dlog( DEBUGRX, "RX: %s", line );
	if( *line == ':' ) {
		coreLine = strpbrk( line, " " );
		if( !coreLine )
			return NS_FAILURE;
		*coreLine = 0;
		while( isspace( *++coreLine ) );
		strlcpy( origin, line + 1, sizeof( origin ) );
		memmove( line, coreLine, strnlen( coreLine, BUFSIZE ) + 1 );
		cmdptr = 1;
	} else {
		cmdptr = 0;
		*origin = 0;
	}
	if( !*line )
		return NS_FAILURE;
	coreLine = strpbrk( line, " " );
	if( coreLine ) {
		*coreLine = 0;
		while( isspace( *++coreLine ) );
	} else {
		coreLine = line + strlen( line );
	}
	strlcpy( cmd, line, sizeof( cmd ) ); 
	dlog( DEBUG1, "origin: %s", origin );
	dlog( DEBUG1, "cmd   : %s", cmd );
	dlog( DEBUG1, "args  : %s", coreLine );
	ac = ircsplitbuf( coreLine, &av, 1 );
	process_ircd_cmd( cmdptr, cmd, origin, av, ac );
	ns_free( av );
	dlog( DEBUG1, "-------------------------END PARSE--------------------------" );
	return NS_SUCCESS;
}

/** @brief unsupported_cmd
 *
 *  report attempts to use a feature not supported by the loaded protocol
 *
 *  @param none
 *
 *  @return none
 */

static void unsupported_cmd( const char* cmd )
{
	irc_chanalert( ns_botptr, _( "Warning, %s tried to %s which is not supported" ), GET_CUR_MODNAME(), cmd );
	nlog( LOG_NOTICE, "Warning, %s tried to %s, which is not supported", GET_CUR_MODNAME(), cmd );
}

/** @brief irc_connect
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	irc_send_server_connect( name, numeric, infoline, pass, tsboot, tslink );
	return NS_SUCCESS;
}

/** @brief irc_prefmsg_list 
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_prefmsg_list( const Bot *botptr, const Client * target, const char **text )
{
	if( IsMe( target ) ) {
		nlog( LOG_NOTICE, "Dropping irc_prefmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	while( *text ) {
		if( **text ) {
			irc_prefmsg( botptr, target,( char* )*text );
		} else {
			irc_prefmsg( botptr, target, " " );
		}
		text++;
	}
	return NS_SUCCESS;
}

/** @brief irc_privmsg_list
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_privmsg_list( const Bot *botptr, const Client * target, const char **text )
{
	if( IsMe( target ) ) {
		nlog( LOG_NOTICE, "Dropping irc_privmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	while( *text ) {
		if( **text ) {
			irc_privmsg( botptr, target,( char* )*text );
		} else {
			irc_privmsg( botptr, target, " " );
		}
		text++;
	}
	return NS_SUCCESS;
}

/** @brief irc_chanalert
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_chanalert( const Bot *botptr, const char *fmt, ... )
{
	va_list ap;

	if( !is_synched || !botptr )
		return NS_SUCCESS;
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_privmsg( botptr->name, me.serviceschan, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_prefmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_prefmsg( const Bot *botptr, const Client *target, const char *fmt, ... )
{
	va_list ap;

	if( IsMe( target ) ) {
		nlog( LOG_NOTICE, "Dropping irc_prefmsg from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( target->flags & CLIENT_FLAG_DCC ) {
		dcc_send_msg( target, ircd_buf );
	} else if( nsconfig.want_privmsg ) {
		irc_send_privmsg( botptr->u->name, target->name, ircd_buf );
	} else {
		irc_send_notice( botptr ? botptr->u->name : ns_botptr->u->name, target->name, ircd_buf );
	}
	return NS_SUCCESS;
}

/** @brief irc_privmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_privmsg( const Bot *botptr, const Client *target, const char *fmt, ... )
{
	va_list ap;

	if( IsMe( target ) ) {
		nlog( LOG_NOTICE, "Dropping privmsg from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_privmsg( botptr->u->name, target->name, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_notice
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_notice( const Bot *botptr, const Client *target, const char *fmt, ... )
{
	va_list ap;

	if( IsMe( target ) ) {
		nlog( LOG_NOTICE, "Dropping notice from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_notice( botptr->u->name, target->name, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_chanprivmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_chanprivmsg( const Bot *botptr, const char *chan, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_privmsg( botptr->u->name, chan, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_channotice
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_channotice( const Bot *botptr, const char *chan, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_notice( botptr->u->name, chan, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_globops
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_globops( const Bot *botptr, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( is_synched ) {
		if( !irc_send_globops ) {
			unsupported_cmd( "GLOBOPS" );
			nlog( LOG_NOTICE, "Dropping unhandled globops: %s", ircd_buf );
			return NS_FAILURE;
		}
		irc_send_globops( ( botptr?botptr->u->name:me.name ), ircd_buf );
	} else {
		nlog( LOG_NOTICE, "globops before sync: %s", ircd_buf );
	}
	return NS_SUCCESS;
}

/** @brief irc_wallops
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_wallops( const Bot *botptr, const char *fmt, ... )
{
	va_list ap;

	if( !irc_send_wallops ) {
		unsupported_cmd( "WALLOPS" );
		nlog( LOG_NOTICE, "Dropping unhandled wallops: %s", ircd_buf );
		return NS_FAILURE;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_wallops( ( botptr?botptr->name:me.name ), ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_numeric
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_numeric( const int numeric, const char *target, const char *data, ... )
{
	va_list ap;

	if( !irc_send_numeric ) {
		unsupported_cmd( "NUMERIC" );
		return NS_FAILURE;
	}
	va_start( ap, data );
	ircvsnprintf( ircd_buf, BUFSIZE, data, ap );
	va_end( ap );
	irc_send_numeric( me.name, numeric, target, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_nick( const char *nick, const char *user, const char *host, const char *realname, const char *modes )
{
	irc_send_nick( nick,( unsigned long )me.now, modes, user, host, me.name, realname );
	return NS_SUCCESS;
}

/** @brief irc_cloakhost
 *
 *  Create a hidden hostmask for the bot 
 *  Support is currently just via UMODE auto cloaking
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_cloakhost( const Bot *botptr )
{
	if( ircd_srv.features&FEATURE_UMODECLOAK ) {
		irc_umode( botptr, botptr->name, UMODE_HIDE );
		return NS_SUCCESS;	
	}
	if( irc_send_cloakhost ) {
		irc_send_cloakhost( botptr->u->user->vhost );
		return NS_SUCCESS;	
	}
	return NS_FAILURE;	
}

/** @brief irc_umode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_umode( const Bot *botptr, const char *target, long mode )
{
	char* newmode;
	
	newmode = UmodeMaskToString( mode );
	irc_send_umode( botptr->u->name, target, newmode );
	UserMode( target, newmode );
	return NS_SUCCESS;
}

/** @brief irc_join
 *
 *  @param none
 *
 *  @return none
 */

int irc_join( const Bot *botptr, const char *chan, const char *mode )
{
	time_t ts;
	Channel *c;

	c = FindChannel( chan );
	ts =( !c ) ? me.now : c->creationtime;
	/* Use sjoin if available */
	if( ( ircd_srv.protocol & PROTOCOL_SJOIN ) && irc_send_sjoin ) 
	{
		if( mode == NULL ) 
		{
			irc_send_sjoin( me.name, botptr->u->name, chan,( unsigned long )ts );
			JoinChannel( botptr->u->name, chan );
		} 
		else 
		{
			ircsnprintf( ircd_buf, BUFSIZE, "%c%s", CmodeCharToPrefix( mode[1] ), botptr->u->name );
			irc_send_sjoin( me.name, ircd_buf, chan,( unsigned long )ts );
			JoinChannel( botptr->u->name, chan );
			ChanUserMode( chan, botptr->u->name, 1, CmodeStringToMask( mode ) );
		}
	}
	else
	{
		/* sjoin not available so use normal join */	
		irc_send_join( botptr->u->name, chan, NULL, ( unsigned long )me.now );
		JoinChannel( botptr->u->name, chan );
		if( mode ) {
			irc_chanusermode( botptr, chan, mode, botptr->u->name );
		}
	}
	/* Increment number of persistent users if needed */
	if( botptr->flags & BOT_FLAG_PERSIST ) {
		if( !c )
			c = FindChannel( chan );
		c->persistentusers ++;
	}
	return NS_SUCCESS;
}

/** @brief irc_part
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_part( const Bot *botptr, const char *chan, const char *quitmsg )
{
	Channel *c;

	if( !irc_send_part ) {
		unsupported_cmd( "PART" );
		return NS_FAILURE;
	}
	c = FindChannel( chan );
	/* Decrement number of persistent users if needed 
	 * Must be BEFORE we part the channel in order to trigger
	 * empty channel processing for other bots
	 */
	if( botptr->flags & BOT_FLAG_PERSIST ) {
		c->persistentusers --;
	}
	irc_send_part( botptr->u->name, chan, quitmsg ? quitmsg : "" );
	PartChannel( botptr->u, ( char * ) chan, quitmsg );
	return NS_SUCCESS;
}

/** @brief irc_nickchange
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_nickchange( const Bot *botptr, const char *newnick )
{
	if( !botptr ) {
		nlog( LOG_WARNING, "Unknown bot tried to change nick to %s", newnick );
		return NS_FAILURE;
	}
	/* Check newnick is not in use */
	if( FindUser( newnick ) ) {
		nlog( LOG_WARNING, "Bot %s tried to change nick to one that already exists %s", botptr->name, newnick );
		return NS_FAILURE;
	}
	irc_send_nickchange( botptr->name, newnick, ( unsigned long )me.now );
	UserNickChange( botptr->name, newnick, NULL );
	return NS_SUCCESS;
}

/** @brief irc_setname
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_setname( const Bot *botptr, const char* realname )
{
	if( !irc_send_setname ) {
		unsupported_cmd( "SETNAME" );
		return NS_FAILURE;
	}
	irc_send_setname( botptr->name, realname );
	strlcpy( botptr->u->info,( char* )realname, MAXHOST );
	return NS_SUCCESS;
}

/** @brief irc_sethost
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_sethost( const Bot *botptr, const char* host )
{
	if( !irc_send_sethost ) {
		unsupported_cmd( "SETNAME" );
		return NS_FAILURE;
	}
	irc_send_sethost( botptr->name, host );
	strlcpy( botptr->u->user->hostname,( char* )host, MAXHOST );
	return NS_SUCCESS;
}
 
/** @brief irc_setident
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_setident( const Bot *botptr, const char* ident )
{
	if( !irc_send_setident ) {
		unsupported_cmd( "SETNAME" );
		return NS_FAILURE;
	}
	irc_send_setident( botptr->name, ident );
	strlcpy( botptr->u->user->username,( char* )ident, MAXHOST );
	return NS_SUCCESS;
}

/** @brief irc_cmode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_cmode( const Bot *botptr, const char *chan, const char *mode, const char *args )
{
	char **av;
	int ac;

	irc_send_cmode( me.name, botptr->u->name, chan, mode, args, ( unsigned long )me.now );
	ircsnprintf( ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args );
	ac = split_buf( ircd_buf, &av, 0 );
	ChanMode( me.name, av, ac );
	ns_free( av );
	return NS_SUCCESS;
}

/** @brief irc_chanusermode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_chanusermode( const Bot *botptr, const char *chan, const char *mode, const char *target )
{
	if( ( ircd_srv.protocol & PROTOCOL_B64NICK ) ) {
		irc_send_cmode( me.name, botptr->u->name, chan, mode, nick_to_base64( target ), ( unsigned long )me.now );
	} else {
		irc_send_cmode( me.name, botptr->u->name, chan, mode, target, ( unsigned long )me.now );
	}
	ChanUserMode( chan, target, 1, CmodeStringToMask( mode ) );
	return NS_SUCCESS;
}

/** @brief irc_quit
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_quit( const Bot * botptr, const char *quitmsg )
{
	if( !botptr ) {
		return NS_FAILURE;
	}
	irc_send_quit( botptr->u->name, quitmsg );
	do_quit( botptr->u->name, quitmsg );
	return NS_SUCCESS;
}

/** @brief irc_kill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_kill( const Bot *botptr, const char *target, const char *reason, ... )
{
	va_list ap;

	if( !irc_send_kill ) {
		unsupported_cmd( "KILL" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_kill( botptr->u->name, target, ircd_buf );
	do_quit( target, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_kick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_kick( const Bot *botptr, const char *chan, const char *target, const char *reason )
{
	if( !irc_send_kick ) {
		unsupported_cmd( "KICK" );
		return NS_FAILURE;
	}
	irc_send_kick( botptr->u->name, chan, target, reason );
	PartChannel( FindUser( target ), ( char * ) chan, reason[0] != 0 ?( char * )reason : NULL );
	return NS_SUCCESS;
}

/** @brief irc_invite
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_invite( const Bot *botptr, const Client *target, const char *chan ) 
{
	if( !irc_send_invite ) {
		unsupported_cmd( "INVITE" );
		return NS_FAILURE;
	}
	irc_send_invite( botptr->u->name, target->name, chan );
	return NS_SUCCESS;
}

/** @brief irc_svstime
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svstime( const Bot *botptr, Client *target, const time_t ts )
{
	if( !irc_send_svstime ) {
		unsupported_cmd( "SVSTIME" );
		return NS_FAILURE;
	}
	irc_send_svstime( me.name,( unsigned long )ts );
	nlog( LOG_NOTICE, "irc_svstime: synching server times to %lu", ts );
	return NS_SUCCESS;
}

/** @brief irc_svskill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svskill( const Bot *botptr, Client *target, const char *reason, ... )
{
	va_list ap;

	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	if( irc_send_svskill ) {
		irc_send_svskill( me.name, target->name, ircd_buf );
	} else if( irc_send_kill ) {
		irc_send_kill( me.name, target->name, ircd_buf );
		do_quit( target->name, ircd_buf );
	} else {
		unsupported_cmd( "SVSKILL" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief irc_svsmode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svsmode( const Bot *botptr, Client *target, const char *modes )
{
	if( !irc_send_svsmode ) {
		unsupported_cmd( "SVSMODE" );
		return NS_FAILURE;
	}
	irc_send_svsmode( me.name, target->name, modes );
	UserMode( target->name, modes );
	return NS_SUCCESS;
}

/** @brief irc_svshost
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svshost( const Bot *botptr, Client *target, const char *vhost )
{
	if( !irc_send_svshost ) {
		unsupported_cmd( "SVSHOST" );
		return NS_FAILURE;
	}
	strlcpy( target->user->vhost, vhost, MAXHOST );
	target->flags |= CLIENT_FLAG_SETHOST;
	irc_send_svshost( me.name, target->name, vhost );
	return NS_SUCCESS;
}

/** @brief irc_svsjoin
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svsjoin( const Bot *botptr, Client *target, const char *chan )
{
	if( !irc_send_svsjoin ) {
		unsupported_cmd( "SVSJOIN" );
		return irc_invite( botptr, target, chan );
	}
	irc_send_svsjoin( me.name, target->name, chan );
	return NS_SUCCESS;
}

/** @brief irc_svspart
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svspart( const Bot *botptr, Client *target, const char *chan )
{
	if( !irc_send_svspart ) {
		unsupported_cmd( "SVSPART" );
		return NS_FAILURE;
	}
	irc_send_svspart( me.name, target->name, chan );
	return NS_SUCCESS;
}

/** @brief irc_swhois
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_swhois( const char *target, const char *swhois )
{
	if( !irc_send_swhois ) {
		unsupported_cmd( "SWHOIS" );
		return NS_FAILURE;
	}
	irc_send_swhois( me.name, target, swhois );
	return NS_SUCCESS;
}

/** @brief irc_svsnick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svsnick( const Bot *botptr, Client *target, const char *newnick )
{
	if( !irc_send_svsnick ) {
		unsupported_cmd( "SVSNICK" );
		return NS_FAILURE;
	}
	irc_send_svsnick( me.name, target->name, newnick, ( unsigned long )me.now );
	return NS_SUCCESS;
}

/** @brief irc_smo
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_smo( const char *from, const char *umodetarget, const char *msg )
{
	if( !irc_send_smo ) {
		unsupported_cmd( "SMO" );
		return NS_FAILURE;
	}
	irc_send_smo( from, umodetarget, msg );
	return NS_SUCCESS;
}

/** @brief irc_akill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_akill( const Bot *botptr, const char *host, const char *ident, const unsigned long length, const char *reason, ... )
{
	va_list ap;

	if( !irc_send_akill ) {
		unsupported_cmd( "AKILL" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_akill( me.name, host, ident, botptr->name, length, ircd_buf, ( unsigned long )me.now );
	return NS_SUCCESS;
}

/** @brief irc_rakill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_rakill( const Bot *botptr, const char *host, const char *ident )
{
	if( !irc_send_rakill ) {
		unsupported_cmd( "RAKILL" );
		return NS_FAILURE;
	}
	irc_send_rakill( me.name, host, ident );
	return NS_SUCCESS;
}

/** @brief irc_sqline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_sqline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( !irc_send_sqline) {
		unsupported_cmd( "SQLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_sqline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unsqline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unsqline( const Bot *botptr, const char *mask )
{
	if( !irc_send_unsqline) {
		unsupported_cmd( "UNSQLINE" );
		return NS_FAILURE;
	}
	irc_send_unsqline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_sgline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_sgline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( !irc_send_sgline) {
		unsupported_cmd( "SGLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_sgline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unsgline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unsgline( const Bot *botptr, const char *mask )
{
	if( !irc_send_unsgline) {
		unsupported_cmd( "UNSGLINE" );
		return NS_FAILURE;
	}
	irc_send_unsgline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_zline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_zline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( !irc_send_sqline) {
		unsupported_cmd( "ZLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_zline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unzline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unzline( const Bot *botptr, const char *mask )
{
	if( !irc_send_unsqline) {
		unsupported_cmd( "UNZLINE" );
		return NS_FAILURE;
	}
	irc_send_unzline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_ping
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ping( const char *from, const char *reply, const char *to )
{
	if( !irc_send_ping ) {
		unsupported_cmd( "PING" );
		return NS_FAILURE;
	}
	irc_send_ping( from, reply, to );
	return NS_SUCCESS;
}

/** @brief irc_pong
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_pong( const char *reply )
{
	if( !irc_send_pong ) {
		unsupported_cmd( "PONG" );
		return NS_FAILURE;
	}
	irc_send_pong( reply );
	return NS_SUCCESS;
}

/** @brief irc_server
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_server( const char *name, const int numeric, const char *infoline )
{
	if( !irc_send_server ) {
		unsupported_cmd( "SERVER" );
		return NS_FAILURE;
	}
	irc_send_server( me.name, name, numeric, infoline );
	return NS_SUCCESS;
}

/** @brief irc_serverrequptime
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_serverrequptime( const char *source, const char *target )
{
	if( irc_send_serverrequptime )
		irc_send_serverrequptime( source, target );
	else
		send_cmd(":%s STATS u %s", source, target );
	return NS_SUCCESS;
}

/** @brief irc_serverreqversion
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_serverreqversion( const char *source, const char *target )
{
	if( irc_send_serverreqversion )
		irc_send_serverreqversion( source, target );
	else
		send_cmd( ":%s VERSION %s", source, target );
	return NS_SUCCESS;
}

/** @brief irc_squit
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_squit( const char *server, const char *quitmsg )
{
	if( !irc_send_squit ) {
		unsupported_cmd( "SQUIT" );
		return NS_FAILURE;
	}
	irc_send_squit( server, quitmsg );
	return NS_SUCCESS;
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

void do_globops( char *origin, char *message )
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
			cmdparams->param = message;
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

void do_wallops( char *origin, char *message )
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
			cmdparams->param = message;
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

void do_chatops( char *origin, char *message )
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
			cmdparams->param = message;
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

void do_ping( const char* origin, const char* destination )
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

void do_pong( const char* origin, const char* destination )
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

void do_version( const char* nick, const char *remoteserver )
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

void do_motd( const char* nick, const char *remoteserver )
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

void do_admin( const char* nick, const char *remoteserver )
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

void do_credits( const char* nick, const char *remoteserver )
{
	SET_SEGV_LOCATION();
	irc_numeric( RPL_VERSION, nick, ":- NeoStats %s Credits ", me.version );
	irc_numeric( RPL_VERSION, nick, ":- Now Maintained by Fish (fish@dynam.ac) and Mark (mark@ctcp.net)" );
	irc_numeric( RPL_VERSION, nick, ":- Previous Authors: Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)" );
	irc_numeric( RPL_VERSION, nick, ":- For Support, you can find us at" );
	irc_numeric( RPL_VERSION, nick, ":- irc.irc-chat.net #NeoStats" );
	irc_numeric( RPL_VERSION, nick, ":- Thanks to:" );
	irc_numeric( RPL_VERSION, nick, ":- Enigma for being part of the dev team" );
	irc_numeric( RPL_VERSION, nick, ":- Stskeeps for writing the best IRCD ever!" );
	irc_numeric( RPL_VERSION, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)" );
	irc_numeric( RPL_VERSION, nick, ":- monkeyIRCD for the Module Segv Catching code" );
	irc_numeric( RPL_VERSION, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!" );
	irc_numeric( RPL_VERSION, nick, ":- Andy For Ideas" );
	irc_numeric( RPL_VERSION, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies" );
	irc_numeric( RPL_VERSION, nick, ":- sre and Jacob for development systems and access" );
	irc_numeric( RPL_VERSION, nick, ":- Error51 for Translating our FAQ and README files" );
	irc_numeric( RPL_VERSION, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!" );
	irc_numeric( RPL_VERSION, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't( and all the bug reports! )" );
	irc_numeric( RPL_VERSION, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips : )" );
	irc_numeric( RPL_VERSION, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback" );
	irc_numeric( RPL_VERSION, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd" );
	irc_numeric( RPL_VERSION, nick, ":- Blud - Giving us patches for Mystic IRCd" );
	irc_numeric( RPL_VERSION, nick, ":- herrohr - Giving us patches for Liquid IRCd support" );
	irc_numeric( RPL_VERSION, nick, ":- OvErRiTe - Giving us patches for Viagra IRCd support" );
	irc_numeric( RPL_VERSION, nick, ":- Reed Loden - Contributions to IRCu support" );
	irc_numeric( RPL_VERSION, nick, ":- Adam Rutter (Shmad) - Developer from the 1.0 days to 2.0 Days");
	irc_numeric( RPL_VERSION, nick, ":- DeadNotBuried - early testing of 3.0, providing patches and feedback and his NeoStats modules" );
}

/** @brief 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_stats( const char* nick, const char *what )
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
				if( protocol_info->optprotocol&protocol_ptr->flag ) {
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

void do_sjoin( char* tstime, char* channame, char *modes, char *sjoinnick, char **argv, int argc )
{
	char nick[MAXNICK];
	char* nicklist;
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

void do_netinfo( const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname )
{
	ircd_srv.maxglobalcnt = atoi( maxglobalcnt );
	ircd_srv.tsendsync = atoi( tsendsync );
	ircd_srv.uprot = atoi( prot );
	strlcpy( ircd_srv.cloak, cloak, CLOAKKEYLEN );
	strlcpy( me.netname, netname, MAXPASS );
	irc_send_netinfo( me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, ( unsigned long )me.now );
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

void do_snetinfo( const char* maxglobalcnt, const char* tsendsync, const char* prot, const char* cloak, const char* netname )
{
	ircd_srv.uprot = atoi( prot );
	strlcpy( ircd_srv.cloak, cloak, CLOAKKEYLEN );
	strlcpy( me.netname, netname, MAXPASS );
	irc_send_snetinfo( me.name, ircd_srv.uprot, ircd_srv.cloak, me.netname, ( unsigned long )me.now );
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

void do_join( const char* nick, const char* chanlist, const char* keys )
{
	char *s, *t;
	t =( char* )chanlist;
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

void do_part( const char* nick, const char* chan, const char* reason )
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

void do_nick( const char *nick, const char *hopcount, const char* TS, 
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

void do_squit( const char *name, const char* reason )
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
	irc_send_svinfo( TS_CURRENT, TS_MIN,( unsigned long )me.now );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_vctrl( const char* uprot, const char* nicklen, const char* modex, const char* gc, const char* netname )
{
	ircd_srv.uprot = atoi( uprot );
	ircd_srv.nicklen = atoi( nicklen );
	ircd_srv.modex = atoi( modex );
	ircd_srv.gc = atoi( gc );
	strlcpy( me.netname, netname, MAXPASS );
	if( irc_send_vctrl ) {
		irc_send_vctrl( ircd_srv.uprot, ircd_srv.nicklen, ircd_srv.modex, ircd_srv.gc, me.netname );
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

void do_smode( const char* targetnick, const char* modes )
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

void do_mode_user( const char* targetnick, const char* modes )
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

void do_svsmode_user( const char* targetnick, const char* modes, const char* ts )
{
	char modebuf[MODESIZE];
	
	if( ts && isdigit( *ts ) ) {
		const char* pModes;	
		char* pNewModes;	

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

void do_away( const char* nick, const char *reason )
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

void do_vhost( const char* nick, const char *vhost )
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

void do_nickchange( const char * oldnick, const char *newnick, const char * ts )
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

void do_topic( const char* chan, const char *owner, const char* ts, const char *topic )
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

void do_server( const char *name, const char *uplink, const char* hops, const char *numeric, const char *infoline, int srv )
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
			if( irc_send_burst ) {
				irc_send_burst( 0 );
			}
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

void do_setname( const char* nick, const char* realname )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_setname: setting realname of user %s to %s", nick, realname );
		strlcpy( u->info,( char* )realname, MAXHOST );
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

void do_sethost( const char* nick, const char* host )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_sethost: setting host of user %s to %s", nick, host );
		strlcpy( u->user->hostname,( char* )host, MAXHOST );
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

void do_setident( const char* nick, const char* ident )
{
	Client *u;

	u = FindUser( nick );
	if( u ) {
		dlog( DEBUG1, "do_setident: setting ident of user %s to %s", nick, ident );
		strlcpy( u->user->username,( char* )ident, MAXHOST );
	} else {
		nlog( LOG_WARNING, "do_setident: user %s not found", nick );
	}
}

static void _send_numeric( const char *source, const int numeric, const char *target, const char *buf )
{
	send_cmd( ":%s %d %s :%s", source, numeric, target, buf );
}

static void _send_privmsg( const char *source, const char *target, const char *buf )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( PRIVATE ), target, buf );
}

static void _send_notice( const char *source, const char *target, const char *buf )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( NOTICE ), target, buf );
}

static void _send_wallops( const char *source, const char *buf )
{
	send_cmd( ":%s %s :%s", source, MSGTOK( WALLOPS ), buf );
}

static void _send_globops( const char *source, const char *buf )
{
	send_cmd( ":%s %s :%s", source, MSGTOK( GLOBOPS ), buf );
}

static void _send_join( const char *source, const char *chan, const char *key, const unsigned long ts )
{
	send_cmd( ":%s %s %s", source, MSGTOK( JOIN ), chan );
}

static void _send_part( const char *source, const char *chan, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( PART ), chan, reason );
}

static void _send_kick( const char *source, const char *chan, const char *target, const char *reason )
{
	send_cmd( ":%s %s %s %s :%s", source, MSGTOK( KICK ), chan, target, ( reason ? reason : "No Reason Given" ) );
}

static void _send_invite( const char *source, const char *target, const char *chan )
{
	send_cmd( ":%s %s %s %s", source, MSGTOK( INVITE ), target, chan );
}

static void _send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts )
{
	send_cmd( ":%s %s %s %lu", oldnick, MSG_NICK, newnick, ts );
}

static void _send_umode( const char *source, const char *target, const char *mode )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( MODE ), target, mode );
}

static void _send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceserver, MSGTOK( MODE ), chan, mode, args, ts );
}

static void _send_quit( const char *source, const char *quitmsg )
{
	send_cmd( ":%s %s :%s", source, MSGTOK( QUIT ), quitmsg );
}

static void _send_ping( const char *source, const char *reply, const char *target )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( PING ), reply, target );
}

static void _send_pong( const char *reply )
{
	send_cmd( "%s %s", MSGTOK( PONG ), reply );
}

static void _send_server( const char *source, const char *name, const int numeric, const char *infoline )
{
	send_cmd( ":%s %s %s %d :%s", source, MSGTOK( SERVER ), name, numeric, infoline );
}

static void _send_squit( const char *server, const char *quitmsg )
{
	send_cmd( "%s %s :%s", MSGTOK( SQUIT ), server, quitmsg );
}

static void _send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow )
{
	send_cmd( "%s %d %d 0 :%lu", MSGTOK( SVINFO ), tscurrent, tsmin, tsnow );
}

static void _send_kill( const char *source, const char *target, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK(KILL), target, reason );
}

static void _send_setname( const char *nick, const char *realname )
{
	send_cmd( ":%s %s :%s", nick, MSGTOK( SETNAME ), realname );
}

static void _send_sethost( const char *nick, const char *host )
{
	send_cmd( ":%s %s :%s", nick, MSGTOK( SETHOST ), host );
}

static void _send_setident( const char *nick, const char *ident )
{
	send_cmd( ":%s %s :%s", nick, MSGTOK( SETIDENT ), ident );
}

static void _send_svsnick( const char *source, const char *target, const char *newnick, const unsigned long ts )
{
	send_cmd( "%s %s %s :%lu", MSGTOK( SVSNICK ), target, newnick, ts );
}

static void _send_svsjoin( const char *source, const char *target, const char *chan )
{
	send_cmd( "%s %s %s", MSGTOK( SVSJOIN ), target, chan);
}

static void _send_svspart( const char *source, const char *target, const char *chan )
{
	send_cmd( "%s %s %s", MSGTOK( SVSPART ), target, chan );
}

static void _send_svsmode( const char *source, const char *target, const char *modes )
{
	send_cmd( ":%s %s %s %s", source, MSGTOK( SVSMODE ), target, modes );
}

static void _send_svskill( const char *source, const char *target, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( SVSKILL ), target, reason );
}

/** @brief send_cmd
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
void send_cmd( char *fmt, ... )
{
	va_list ap;
	char buf[BUFSIZE];
	int buflen;
	
	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );

	dlog( DEBUGTX, "%s", buf );
	if( strnlen( buf, BUFSIZE ) < BUFSIZE - 2 ) {
		strlcat( buf, "\n", BUFSIZE );
	} else {
		buf[BUFSIZE - 1] = 0;
		buf[BUFSIZE - 2] = '\n';
	}
	buflen = strnlen( buf, BUFSIZE );
	if (send_to_sock(me.servsock, buf, buflen) == NS_FAILURE) {
		do_exit(NS_EXIT_ERROR, NULL);
	}
}

/** @brief HaveFeature
 *
 *  @return 1 if have else 0
 */
int HaveFeature( int mask )
{
	return( ircd_srv.features&mask );
}
