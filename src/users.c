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

#include "neostats.h"
#include "protocol.h"
#include "ircprotocol.h"
#include "modes.h"
#include "users.h"
#include "channels.h"
#include "exclude.h"
#include "nsevents.h"
#include "bots.h"
#include "auth.h"
#include "services.h"
#include "ctcp.h"
#include "base64.h"

#define USER_TABLE_SIZE	-1
#define MAXJOINCHANS	-1

/** List of online users */
static hash_t *userhash;

/** @brief new_user
 *
 *  Create a new user Client struct
 *  NeoStats core use only.
 *
 *  @param nick of user to create
 *
 *  @return pointer to Client or NULL if fails
 */

static Client *new_user( const char *nick )
{
	Client *u;

	SET_SEGV_LOCATION();
	if( hash_isfull( userhash ) )
	{
		nlog( LOG_CRITICAL, "new_user: user hash is full" );
		return NULL;
	}
	dlog( DEBUG2, "new_user: %s", nick );
	u = ns_calloc( sizeof( Client ) );
	strlcpy( u->name, nick, MAXNICK );
	u->user = ns_calloc( sizeof( User ) );
	hnode_create_insert( userhash, u, u->name );
	me.usercount++;
	return u;
}

/** @brief lookupnickip
 *
 *  DNS callback for IP lookups
 *  NeoStats core use only.
 *
 *  @param data
 *  @param a
 *
 *  @return none
 */

static void lookupnickip( void *data, adns_answer *a ) 
{
	CmdParams *cmdparams;
	Client *u;
	
	u = FindUser( ( char * )data );
	if( a && a->nrrs > 0 && u && a->status == adns_s_ok )
	{
		u->ip.s_addr = a->rrs.addr->addr.inet.sin_addr.s_addr;
		strlcpy( u->hostip, inet_ntoa( u->ip ), HOSTIPLEN );
		if( u->ip.s_addr > 0 )
		{
			cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
			cmdparams->source = u;	
			SendAllModuleEvent( EVENT_NICKIP, cmdparams );
			ns_free( cmdparams );
		}
	} 
}

/** @brief process_ip
 *
 *  Setup DNS callback for IP lookups
 *  NeoStats core use only.
 *
 *  @param nick
 *  @param host
 *
 *  @return IP address
 */

static int process_ip( const char *nick, const char *host )
{
	unsigned long ipaddress = 0;
	struct in_addr *ipad;
	int res;

	/* first, if the u->host is a ip address, just convert it */
	ipad = ns_calloc( sizeof( struct in_addr ) );
	res = inet_aton( host, ipad );
	if( res > 0 )
	{
		/* its valid */
		ipaddress = htonl( ipad->s_addr );
		ns_free( ipad );
	}
	else
	{		
		/* kick of a dns reverse lookup for this host */
		dns_lookup( ( char * )host, adns_r_addr, lookupnickip,( void * )nick );
		ipaddress = 0;
	}		
	return ipaddress;
}

/** @brief AddUser
 *
 *  Add a user to NeoStats
 *  NeoStats core use only.
 *
 *  @param nick
 *  @param user
 *  @param host
 *  @param realname
 *  @param server
 *  @param ip
 *  @param TS
 *  @param numeric
 *
 *  @return pointer to Client or NULL if fails
 */

Client *AddUser( const char *nick, const char *user, const char *host, 
	const char *realname, const char *server, const char *ip, const char *TS, 
	const char *numeric )
{
	CmdParams *cmdparams;
	unsigned long ipaddress = 0;
	Client *u;

	SET_SEGV_LOCATION();
	u = FindUser( nick );
	if( u )
	{
		nlog( LOG_WARNING, "AddUser: trying to add a user that already exists %s", nick );
		return NULL;
	}
	dlog( DEBUG2, "AddUser: %s (%s@%s) %s (%d) -> %s at %s", nick, user, host, realname,( int )htonl( ipaddress ), server, TS );
	u = new_user( nick );
	if( !u )
		return NULL;
	if( ip )
		ipaddress = strtoul( ip, NULL, 10 );
	else if( !( ircd_srv.protocol&PROTOCOL_NICKIP ) && me.want_nickip == 1 )
		ipaddress = process_ip( u->name, host );
	u->tsconnect = TS ? strtoul( TS, NULL, 10 ) : me.now;
	if( ( time( NULL ) - u->tsconnect ) > nsconfig.splittime )
		u->flags |= NS_FLAGS_NETJOIN;
	strlcpy( u->user->hostname, host, MAXHOST );
	strlcpy( u->user->vhost, host, MAXHOST );
	ircsnprintf( u->user->userhostmask, USERHOSTLEN, "%s!%s@%s", nick, user, host );
	strlcpy( u->user->uservhostmask, u->user->userhostmask, USERHOSTLEN );
	strlcpy( u->user->username, user, MAXUSER );
	strlcpy( u->info, realname, MAXREALNAME );
	u->user->ulevel = -1;
	u->uplink = FindServer( server );
	u->uplink->server->users++;
	u->user->tslastmsg = me.now;
	u->user->chans = list_create( MAXJOINCHANS );
	u->ip.s_addr = htonl( ipaddress );
	strlcpy( u->hostip, inet_ntoa( u->ip ), HOSTIPLEN );
	if( IsMe( u->uplink ) )
		u->flags |= CLIENT_FLAG_ME;
	/* check if the user is excluded */
	ns_do_exclude_user( u );
	if( ( ircd_srv.protocol & PROTOCOL_B64NICK ) && numeric )
		set_nick_base64( u->name, numeric );
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;	
	SendAllModuleEvent( EVENT_SIGNON, cmdparams );
	/* Send EVENT_NICKIP if we have it and a module wants it */
	if( me.want_nickip == 1 && ipaddress != 0 )
		SendAllModuleEvent( EVENT_NICKIP, cmdparams );
	ns_free( cmdparams );
	/* Send CTCP VERSION request if we are configured to do so */
	if( IsNeoStatsSynched() && me.versionscan && !IsExcluded( u ) && !IsMe( u ) )
		irc_ctcp_version_req( ns_botptr, u );
	return u;
}

/** @brief deluser
 *
 *  Remove a user from NeoStats
 *  NeoStats core use only.
 *
 *  @param u pointer to Client to remove
 *
 *  @return none
 */

static void deluser( Client *u )
{
	hnode_t *un;

	un = hash_lookup( userhash, u->name );
	if( !un )
	{
		nlog( LOG_WARNING, "deluser: %s failed!", u->name );
		return;
	}
	/* if its one of our bots, remove it from the modlist */
	if( IsMe( u ) )
		DelBot( u->name );
	hash_delete_destroy_node( userhash, un );
	list_destroy( u->user->chans );
	if( u->uplink )
		u->uplink->server->users--;
	if( IsAway( u ) )
	{
		me.awaycount--;
		if( u->uplink )
			u->uplink->server->awaycount--;
	}
	ns_free( u->user );
	ns_free( u );
	me.usercount--;
}

/** @brief KillUser
 *
 *  Process IRC KILL
 *  NeoStats core use only.
 *
 *  @param source of kill
 *  @param nick to kill
 *  @param reason for kill
 *
 *  @return none
 */

void KillUser( const char *source, const char *nick, const char *reason )
{
	char *killbuf;
	char *killreason;
	char** av;
	int ac = 0;
	CmdParams *cmdparams;
	Client *u;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "KillUser: %s", nick );
	u = FindUser( nick );
	if( !u )
	{
		nlog( LOG_WARNING, "KillUser: %s failed!", nick );
		return;
	}
	PartAllChannels( u, reason );
	/* run the event to delete a user */
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->target = u;
	killbuf = sstrdup( reason );
	ac = split_buf( killbuf, &av, 0 );
	killreason = joinbuf( av, ac, 1 );
	cmdparams->param = killreason;
	cmdparams->source = FindUser( source );
	if( cmdparams->source )
	{
		SendAllModuleEvent( EVENT_KILL, cmdparams );
		SendAllModuleEvent( EVENT_GLOBALKILL, cmdparams );
	}
	else
	{
		cmdparams->source = FindServer( source );
		SendAllModuleEvent( EVENT_KILL, cmdparams );
		SendAllModuleEvent( EVENT_SERVERKILL, cmdparams ); 		
	}
	/* if its one of our bots inform the module */
	if( IsMe( u ) )
	{
		cmdparams->bot = u->user->bot;
		nlog( LOG_NOTICE, "KillUser: deleting bot %s as it was killed", u->name );
		SendModuleEvent( EVENT_BOTKILL, cmdparams, u->user->bot->moduleptr );
	}
	deluser( u );
	ns_free( killbuf );
	ns_free( killreason );
	ns_free( av );
	ns_free( cmdparams );
}

/** @brief QuitUser
 *
 *  Process IRC QUIT
 *  NeoStats core use only.
 *
 *  @param nick which quit
 *  @param reason for quit
 *
 *  @return none
 */

void QuitUser( const char *nick, const char *reason )
{
	CmdParams *cmdparams;
	Client *u;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "QuitUser: %s", nick );
	u = FindUser( nick );
	if( !u )
	{
		nlog( LOG_WARNING, "QuitUser: %s failed!", nick );
		return;
	}
	PartAllChannels( u, reason );
	/* run the event to delete a user */
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;
	if( reason )
		cmdparams->param = ( char *)reason;
	SendAllModuleEvent( EVENT_QUIT, cmdparams );
	/* RX: :m , :[irc.foonet.com] Local kill by Mark( testing ) */
	if( strstr( reason, "Local kill by" ) && 
		strstr( reason, "[" ) && 
		strstr( reason, "]" ) )
	{
		char *killbuf;
		char *killreason;
		char** av;
		int ac = 0;

		killbuf = sstrdup( cmdparams->param );
		ac = split_buf( killbuf, &av, 0 );
		killreason = joinbuf( av, ac, 5 );
		cmdparams->source = FindUser( av[4] );
		cmdparams->target = u;
		cmdparams->param = killreason;
		SendAllModuleEvent( EVENT_LOCALKILL, cmdparams );
		ns_free( killbuf );
		ns_free( killreason );
		ns_free( av );
	}
	deluser( u );
	ns_free( cmdparams );
}

/** @brief UserAway
 *
 *  Process IRC AWAY
 *  NeoStats core use only.
 *
 *  @param nick which quit
 *  @param awaymsg
 *
 *  @return none
 */

void UserAway( const char *nick, const char *awaymsg )
{
	CmdParams *cmdparams;
	Client *u;

	u = FindUser( nick );
	if( !u )
	{
		nlog( LOG_WARNING, "UserAway: unable to find user %s for away", nick );
		return;
	}
	if( awaymsg )
		strlcpy( u->user->awaymsg, awaymsg, MAXHOST );
	else
		u->user->awaymsg[0] = 0;
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;
	if( IsAway( u ) && ( !awaymsg ) )
	{
		u->user->is_away = 0;
		me.awaycount--;
		u->uplink->server->awaycount--;
	}
	else if( !IsAway( u ) && ( awaymsg ) )
	{
		u->user->is_away = 1;
		me.awaycount++;
		u->uplink->server->awaycount++;
	}
	SendAllModuleEvent( EVENT_AWAY, cmdparams );
	ns_free( cmdparams );
}

/** @brief UserNickChange
 *
 *  Process IRC NICK
 *  NeoStats core use only.
 *
 *  @param oldnick
 *  @param newnick
 *  @param ts
 *
 *  @return none
 */

void UserNickChange( const char *oldnick, const char *newnick, const char *ts )
{
	CmdParams *cmdparams;
	hnode_t *un;
	Client *u;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "UserNickChange: %s -> %s", oldnick, newnick );
	un = hash_lookup( userhash, oldnick );
	if( !un )
	{
		nlog( LOG_WARNING, "UserNickChange: can't find user %s", oldnick );
		return;
	}
	u = ( Client * ) hnode_get( un );
	hash_delete( userhash, un );
	strlcpy( u->name, newnick, MAXNICK );
	ircsnprintf( u->user->userhostmask, USERHOSTLEN, "%s!%s@%s", u->name, u->user->username, u->user->hostname );
	ircsnprintf( u->user->uservhostmask, USERHOSTLEN, "%s!%s@%s", u->name, u->user->username, u->user->vhost );
	u->tsconnect = ts ? atoi( ts ) : me.now;
	ns_do_exclude_user( u );
	hash_insert( userhash, un, u->name );
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;
	cmdparams->param = ( char * )oldnick;
	SendAllModuleEvent( EVENT_NICK, cmdparams );
	ns_free( cmdparams );
	if( IsMe( u ) )
		BotNickChange( u->user->bot, newnick );
	return;
}

/** @brief find_user_base64
 *
 *  Find user based on base 64 representation
 *  NeoStats core use only.
 *
 *  @param num numeric to find
 *
 *  @return pointer to Client or NULL if fails
 */

Client *find_user_base64( const char *num )
{
	Client *u;
	hnode_t *un;
	hscan_t us;

	hash_scan_begin( &us, userhash );
	while( ( un = hash_scan_next( &us ) ) != NULL )
	{
		u = hnode_get( un );
		if( strncmp( u->name64, num, BASE64NICKSIZE ) == 0 )
		{
			dlog( DEBUG1, "find_user_base64: %s -> %s", num, u->name );
			return u;
		}
	}
	dlog( DEBUG3, "find_user_base64: %s not found", num );
	return NULL;
}

/** @brief FindUser
 *
 *  Find user based on nick
 *  NeoStats core use only.
 *
 *  @param nick to find
 *
 *  @return pointer to Client or NULL if fails
 */

Client *FindUser( const char *nick )
{
	Client *u;

	u = ( Client * )hnode_find( userhash, nick );
	if( !u )
		dlog( DEBUG3, "FindUser: %s not found", nick );
	return u;
}

/** @brief InitUsers
 *
 *  Init user subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitUsers( void )
{
	userhash = hash_create( USER_TABLE_SIZE, 0, 0 );
	if( !userhash )
	{
		nlog( LOG_CRITICAL, "Unable to create user hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief dumpuser
 *
 *  Report user information
 *  NeoStats core use only.
 *
 *  @param u pointer to client to report
 *  @param v pointer to cmdparams
 *
 *  @return NS_FALSE
 */

static int dumpuser( Client *u, void* v )
{
	CmdParams *cmdparams;
	lnode_t *cm;
	int i = 0;

	cmdparams = ( CmdParams * ) v;
	if( ircd_srv.protocol & PROTOCOL_B64NICK )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "User:     %s!%s@%s (%s)", cmdparams->source ), u->name, u->user->username, u->user->hostname, u->name64 );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, __( "User:     %s!%s@%s", cmdparams->source ), u->name, u->user->username, u->user->hostname );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "IP:       %s", cmdparams->source ), u->hostip );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Vhost:    %s", cmdparams->source ), u->user->vhost );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Flags:    0x%x", cmdparams->source ), u->flags );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Modes:    %s( 0x%x )", cmdparams->source ), UmodeMaskToString( u->user->Umode ), u->user->Umode );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Smodes:   %s( 0x%x )", cmdparams->source ), SmodeMaskToString( u->user->Smode ), u->user->Smode );
	if( IsAway( u ) )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Away:     %s", cmdparams->source ), u->user->awaymsg );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Version:  %s", cmdparams->source ), u->version );

	cm = list_first( u->user->chans );
	while( cm )
	{
		if( i==0 )
			irc_prefmsg( ns_botptr, cmdparams->source, __( "Channels: %s", cmdparams->source ),( char * ) lnode_get( cm ) );
		else
			irc_prefmsg( ns_botptr, cmdparams->source, "          %s",( char * ) lnode_get( cm ) );
		cm = list_next( u->user->chans, cm );
		i++;
	}
	irc_prefmsg( ns_botptr, cmdparams->source, "========================================" );
	return NS_FALSE;
}

/** @brief ListUsers
 *
 *  Report current user list
 *  NeoStats core use only.
 *
 *  @param cmdparams
 *  @param nick
 *
 *  @return none
 */

void ListUsers( CmdParams *cmdparams, const char *nick )
{
	Client *u;

	if( !nsconfig.debug )
		return;
	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "================USERLIST================", cmdparams->source ) );
	if( !nick )
	{
		ProcessUserList( dumpuser, cmdparams );
		return;
	}
	u = FindUser( nick );
	if( u )
		dumpuser( u,( void * )cmdparams );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, __( "ListUsers: can't find user %s", cmdparams->source ), nick );
}

/** @brief UserLevel
 *
 *  Calculate user authentication level
 *  NeoStats core use only.
 *
 *  @param u pointer to client to authenticate
 *
 *  @return user level
 */

int UserLevel( Client *u )
{
	/* Have we already calculated the user level? */
	if( u->user->ulevel != -1 )
		return u->user->ulevel;
	u->user->ulevel = AuthUser( u );
	/* Set user level so we no longer need to calculate */
	dlog( DEBUG1, "UserLevel for %s set to %d", u->name, u->user->ulevel );
	return u->user->ulevel;
}

/** @brief SetUserVhost
 *
 *  Process IRC VHOST
 *  NeoStats core use only.
 *
 *  @param nick to set vhost for
 *  @param vhost to set
 *
 *  @return none
 */

void SetUserVhost( const char *nick, const char *vhost ) 
{
	Client *u;

	u = FindUser( nick );
	dlog( DEBUG1, "Vhost %s", vhost );
	if( u )
	{
		strlcpy( u->user->vhost, vhost, MAXHOST );
		ircsnprintf( u->user->uservhostmask, USERHOSTLEN, "%s!%s@%s", nick, u->user->username, vhost );
		if( ircd_srv.features & FEATURE_UMODECLOAK )
			u->user->Umode |= UMODE_HIDE;
	}
}

/** @brief UserMode
 *
 *  Process IRC MODE
 *  NeoStats core use only.
 *
 *  @param nick to change mode for
 *  @param modes to set
 *
 *  @return none
 */

void UserMode( const char *nick, const char *modes )
{
	CmdParams *cmdparams;
	Client *u;
	long oldmode;

	SET_SEGV_LOCATION();
	dlog( DEBUG1, "UserMode: user %s modes %s", nick, modes );
	u = FindUser( nick );
	if( !u )
	{
		nlog( LOG_WARNING, "UserMode: mode change for unknown user %s %s", nick, modes );
		return;
	}
	/* Reset user level so it will be recalculated */
	u->user->ulevel = -1;
	strlcpy( u->user->modes, modes, MODESIZE );
	oldmode = u->user->Umode;
	u->user->Umode |= UmodeStringToMask( modes );
	if( ircd_srv.features&FEATURE_UMODECLOAK )
	{
		/* Do we have a hidden host any more? */
		if( ( oldmode & UMODE_HIDE ) && ( !( u->user->Umode & UMODE_HIDE ) ) )
			strlcpy( u->user->vhost, u->user->hostname, MAXHOST );
	}
	dlog( DEBUG1, "UserMode: modes for %s now %x", u->name, u->user->Umode );
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;	
	cmdparams->param = ( char *)modes;
	SendAllModuleEvent( EVENT_UMODE, cmdparams );
	ns_free( cmdparams );
}

/** @brief UserSMode
 *
 *  Process IRC SMODE
 *  NeoStats core use only.
 *
 *  @param nick to change smode for
 *  @param smodes to set
 *
 *  @return none
 */

void UserSMode( const char *nick, const char *modes )
{
	CmdParams *cmdparams;
	Client *u;

	SET_SEGV_LOCATION();
	dlog( DEBUG1, "UserSMode: user %s smodes %s", nick, modes );
	u = FindUser( nick );
	if( !u )
	{
		nlog( LOG_WARNING, "UserSMode: smode change for unknown user %s %s", nick, modes );
		return;
	}
	/* Reset user level so it will be recalculated */
	u->user->ulevel = -1;
	u->user->Smode |= SmodeStringToMask( modes );
	dlog( DEBUG1, "UserSMode: smode for %s is now %x", u->name, u->user->Smode );
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = u;	
	cmdparams->param = ( char *)modes;
	SendAllModuleEvent( EVENT_SMODE, cmdparams );
	ns_free( cmdparams );
}

/** @brief SetUserServicesTS
 *
 *  Process services time stamp
 *  NeoStats core use only.
 *
 *  @param nick
 *  @param ts
 *
 *  @return none
 */

void SetUserServicesTS( const char *nick, const char *ts ) 
{
	Client *u;

	u = FindUser( nick );
	if( u )
		u->user->servicestamp = strtoul( ts, NULL, 10 );
}

/** @brief FiniUsers
 *
 *  Fini user subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniUsers( void )
{
	Client *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, userhash );
	while( ( un = hash_scan_next( &hs ) ) != NULL )
	{
		u = hnode_get( un );
		PartAllChannels( u, NULL );
		/* something is wrong if its our bots */
		if( IsMe( u ) )
			nlog( LOG_NOTICE, "FiniUsers called with a neostats bot online: %s", u->name );
		hash_scan_delete_destroy_node( userhash, un );
		list_destroy( u->user->chans );
		ns_free( u->user );
		ns_free( u );
	}
	hash_destroy( userhash );
}

/** @brief QuitServerUsers
 *
 *  Remove all users from a given server for use with NOQUIT protocols
 *  NeoStats core use only.
 *
 *  @param s pointer to server to quit users of
 *
 *  @return none
 */

void QuitServerUsers( Client *s )
{
	Client *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, userhash );
	while( ( un = hash_scan_next( &hs ) ) != NULL )
	{
		u = hnode_get( un );
		if( u->uplink == s ) 
		{
			dlog( DEBUG1, "QuitServerUsers: deleting %s from %s", u->name, s->name );
			QuitUser( u->name, s->name );
		}
	}
}

/** @brief ProcessUserList
 *
 *  Walk user list and call handler for each user
 *  NeoStats core use only.
 *
 *  @param handler to call
 *  @param v optional pointer
 *
 *  @return NS_SUCCESS
 */

int ProcessUserList( UserListHandler handler, void *v )
{
	Client *u;
	hscan_t scan;
	hnode_t *node;

	SET_SEGV_LOCATION();
	hash_scan_begin( &scan, userhash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		u = hnode_get( node );
		if( handler( u, v ) == NS_TRUE )
			break;
	}
	return NS_SUCCESS;
}

/** @brief AddFakeUser
 *
 *  Adds fake user to NeoStats
 *  NeoStats core use only.
 *
 *  @param mask of fake user
 *
 *  @return none
 */

void AddFakeUser( const char *mask )
{
	char maskcopy[MAXHOST];
	char *nick;
	char *user;
	char *host;
	Client *u;

	SET_SEGV_LOCATION();
	strlcpy( maskcopy, mask, MAXHOST );
	nick = strtok( maskcopy, "!" );
	user = strtok( NULL, "@" );
	host = strtok( NULL, "" );
	u = FindUser( nick );
	if( u )
	{
		nlog( LOG_WARNING, "AddUser: trying to add a user that already exists %s", nick );
		return;
	}
	u = new_user( nick );
	if( !u )
		return;
	u->tsconnect = me.now;
	strlcpy( u->user->hostname, host, MAXHOST );
	strlcpy( u->user->vhost, host, MAXHOST );
	strlcpy( u->user->username, user, MAXUSER );
	strlcpy( u->info, "fake user", MAXREALNAME );
	ircsnprintf( u->user->userhostmask, USERHOSTLEN, "%s!%s@%s", nick, user, host );
	strlcpy( u->user->uservhostmask, u->user->userhostmask, USERHOSTLEN );
	u->user->tslastmsg = me.now;
	u->user->chans = list_create( MAXJOINCHANS );
}

/** @brief DelFakeUser
 *
 *  Delete fake user from NeoStats
 *  NeoStats core use only.
 *
 *  @param mask of fake user
 *
 *  @return none
 */

void DelFakeUser( const char *mask )
{
	char maskcopy[MAXHOST];
	char *nick;
	char *user;
	char *host;
	Client *u;

	SET_SEGV_LOCATION();
	strlcpy( maskcopy, mask, MAXHOST );
	nick = strtok( maskcopy, "!" );
	user = strtok( NULL, "@" );
	host = strtok( NULL, "" );
	u = FindUser( nick );
	deluser( u );
}

/** @brief AllocUserModPtr
 *
 *  Allocate memory for a module pointer for a user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to add pointer for
 *  @param size to allocate
 *
 *  @return pointer to allocated memory
 */

void *AllocUserModPtr( Client* u, int size )
{
	void *ptr;
	ptr = ns_calloc( size );
	u->modptr[GET_CUR_MODULE_INDEX()] = ptr;
	GET_CUR_MODULE()->userdatacnt++;
	return ptr;
}

/** @brief FreeUserModPtr
 *
 *  Free memory for a module pointer for a user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to free pointer for
 *
 *  @return none
 */

void FreeUserModPtr( Client* u )
{
	ns_free( u->modptr[GET_CUR_MODULE_INDEX()] );
	GET_CUR_MODULE()->userdatacnt--;
}

/** @brief GetUserModPtr
 *
 *  Retrieve module pointer for a user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to lookup pointer for
 *
 *  @return none
 */

void* GetUserModPtr( const Client* u )
{
	return u->modptr[GET_CUR_MODULE_INDEX()];
}

/** @brief ClearUserModValue
 *
 *  Clear module value for a user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to clear
 *
 *  @return none
 */

void ClearUserModValue( Client *u )
{
	if( u )
	{
		u->modvalue[GET_CUR_MODULE_INDEX()] = NULL;
		GET_CUR_MODULE()->userdatacnt--;
	}
}

/** @brief SetUserModValue
 *
 *  Set module value for a user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to set
 *  @param data pointer to set
 *
 *  @return none
 */

void SetUserModValue( Client *u, void *data )
{
	if( u )
	{
		u->modvalue[GET_CUR_MODULE_INDEX()] = data;
		GET_CUR_MODULE()->userdatacnt++;
	}
}

/** @brief GetUserModValue
 *
 *  Retrieve module value for a user
 *  NeoStats core use only.
 *
 *  @param u pointer to client to lookup pointer for
 *
 *  @return none
 */

void *GetUserModValue( const Client *u )
{
	if( u )
		return u->modvalue[GET_CUR_MODULE_INDEX()];
	return NULL;	
}

/** @brief CleanupUserModdataHandler
 *
 *  Cleanup user moddata
 *
 *  @param u pointer to user
 *  @param v not used
 *
 *  @return none
 */

static int CleanupUserModdataHandler( Client *u, void *v )
{
	if( u->modptr[GET_CUR_MODULE_INDEX()] )
		ns_free( u->modptr[GET_CUR_MODULE_INDEX()] );		
	u->modvalue[GET_CUR_MODULE_INDEX()] = NULL;
	return NS_FALSE;
}

/** @brief CleanupUserModdata
 *
 *  Clear module data values and pointer left set by an unloaded module
 *  NeoStats core use only.
 *
 *  @param index of module to clear
 *
 *  @return none
 */

void CleanupUserModdata( int index )
{
	SET_SEGV_LOCATION();
	if( GET_CUR_MODULE()->userdatacnt > 0 )
	{
		nlog( LOG_WARNING, "Cleaning up users after dirty module!" );
		ProcessServerList( CleanupUserModdataHandler, NULL );
	}
	GET_CUR_MODULE()->userdatacnt = 0;
}
