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
#include "hostserv.h"

#define PAGESIZE	20

typedef struct vhostentry {
	char nick[MAXNICK];
	char host[MAXHOST];
	char vhost[MAXHOST];
	char passwd[MAXPASS];
	char added[MAXNICK];
	time_t tslastused;
} vhostentry;

typedef struct banentry {
	char host[MAXHOST];
	char who[MAXNICK];
	char reason[MAXREASON];
} banentry;

struct hs_cfg {
	int expire;
	int regnick;
	char vhostdom[MAXHOST];
	int operhosts;
	int verbose;
	int addlevel;
} hs_cfg;

static int hs_event_signon( CmdParams *cmdparams );
static int hs_event_umode( CmdParams *cmdparams );

static int hs_cmd_bans( CmdParams *cmdparams );
static int hs_cmd_login( CmdParams *cmdparams );
static int hs_cmd_chpass( CmdParams *cmdparams );
static int hs_cmd_add( CmdParams *cmdparams );
static int hs_cmd_list( CmdParams *cmdparams );
static int hs_cmd_list_limit( CmdParams *cmdparams );
static int hs_cmd_view( CmdParams *cmdparams );
static int hs_cmd_del( CmdParams *cmdparams );

static int hs_set_regnick_cb( CmdParams* cmdparams, SET_REASON reason );
static int hs_set_expire_cb( CmdParams* cmdparams, SET_REASON reason );

static list_t *vhost_list;
static hash_t *banhash;

/** Bot pointer */
static Bot *hs_bot;

/** Copyright info */
const char *hs_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"HostServ",
	"Network virtual host service",
	hs_copyright,
	hs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	FEATURE_SVSHOST,
};

/** Bot comand table */
static bot_cmd hs_commands[]=
{
	{"ADD",		hs_cmd_add,		4,	NS_ULEVEL_LOCOPER,	hs_help_add},
	{"DEL",		hs_cmd_del,		1, 	NS_ULEVEL_LOCOPER,	hs_help_del},
	{"LIST",	hs_cmd_list,	0, 	NS_ULEVEL_LOCOPER,	hs_help_list},
	{"BANS",	hs_cmd_bans,	1,  NS_ULEVEL_ADMIN,	hs_help_bans},
	{"VIEW",	hs_cmd_view,	1, 	NS_ULEVEL_OPER,		hs_help_view},
	{"LOGIN",	hs_cmd_login,	2, 	0,					hs_help_login},
	{"CHPASS",	hs_cmd_chpass,	3, 	0,					hs_help_chpass},
	{NULL,		NULL,			0, 	0,					NULL}
};

/** Bot setting table */
static bot_setting hs_settings[]=
{
	{"EXPIRE",	&hs_cfg.expire,		SET_TYPE_INT,		0, 99, 		NS_ULEVEL_ADMIN, "days",hs_help_set_expire,	hs_set_expire_cb,	( void* )60	},
	{"HIDDENHOST",	&hs_cfg.regnick,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_hiddenhost, hs_set_regnick_cb,	( void* )0	},
	{"HOSTNAME",	hs_cfg.vhostdom,	SET_TYPE_STRING,	0, MAXHOST,	NS_ULEVEL_ADMIN, NULL,	hs_help_set_hostname,	NULL,			( void* )""	},
	{"OPERHOSTS",	&hs_cfg.operhosts,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_operhosts,	NULL,			( void* )0	},
	{"VERBOSE",	&hs_cfg.verbose,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_verbose,	NULL,			( void* )1	},
	{"ADDLEVEL",	&hs_cfg.addlevel,	SET_TYPE_INT,		0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_addlevel,	NULL,			( void* )NS_ULEVEL_LOCOPER },
	{NULL,		NULL,			0,			0, 0, 		0,				 NULL,		NULL,			NULL	},
};

/** BotInfo */
static BotInfo hs_botinfo = 
{
	"HostServ", 
	"HostServ1", 
	"HS", 
	BOT_COMMON_HOST, 
	"Network virtual host service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	hs_commands, 
	hs_settings,
};

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_SIGNON,	hs_event_signon,	EVENT_FLAG_EXCLUDE_ME | EVENT_FLAG_USE_EXCLUDE},
	{EVENT_UMODE,	hs_event_umode,		EVENT_FLAG_EXCLUDE_ME | EVENT_FLAG_USE_EXCLUDE}, 
	{EVENT_NULL,	NULL}
};

/** @brief findnick
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of strcmp
 */

int findnick( const void *key1, const void *key2 )
{
	const vhostentry *vhost = key1;
	return( ircstrcasecmp( vhost->nick,( char * )key2 ) );
}

/** @brief del_vhost
 *
 *  Delete a vhost entry
 *
 *  @param pointer to vhost entry to delete
 *
 *  @return none
 */

static void del_vhost( vhostentry *vhost ) 
{
	DBADelete( "vhosts", vhost->nick );
	ns_free( vhost );
}

/** @brief ExpireOldHosts
 *
 *  Timer function to expire old hosts
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ExpireOldHosts( void )
{
	lnode_t *hn, *hn2;
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	hn = list_first( vhost_list );
	while( hn != NULL ) {
		vhe = lnode_get( hn );
		if( vhe->tslastused < ( me.now -( hs_cfg.expire * 86400 ) ) ) {
			nlog( LOG_NOTICE, "Expiring old vhost: %s for %s", vhe->vhost, vhe->nick );
			del_vhost( vhe );
			hn2 = list_next( vhost_list, hn );
			list_delete( vhost_list, hn );
			lnode_destroy( hn );
			hn = hn2;
		} else {
			hn = list_next( vhost_list, hn );
		}
	}
	return NS_SUCCESS;
}

int new_dbvhost( void *data, int size )
{
	vhostentry *vhe;

	vhe = ns_calloc( sizeof( vhostentry ) );
	os_memcpy( vhe, data, sizeof( vhostentry ) );
	lnode_create_append( vhost_list, vhe );
	return NS_FALSE;
}

/** @brief LoadHosts
 *
 *  Load vhosts
 *
 *  @param none
 *
 *  @return none
 */

static void LoadHosts( void )
{
	DBAFetchRows( "vhosts", new_dbvhost );
	list_sort( vhost_list, findnick );
}

/** @brief SaveVhost
 *
 *  Save a vhost entry
 *
 *  @param pointer to vhost entry to save
 *
 *  @return none
 */

static void SaveVhost( vhostentry *vhe ) 
{
	vhe->tslastused = me.now;
	DBAStore( "vhosts", vhe->nick,( void * )vhe, sizeof( vhostentry ) );
	list_sort( vhost_list, findnick );
}

/** @brief SaveBan
 *
 *  Save banned vhost
 *
 *  @param pointer to ban to save
 *
 *  @return none
 */

static void SaveBan( banentry *ban )
{
	DBAStore( "bans", ban->host,( void * )ban, sizeof( ban ) );
}

/** @brief LoadBans
 *
 *  Load banned vhosts
 *
 *  @param none
 *
 *  @return none
 */

static int new_ban( void *data, int size )
{
	banentry *ban;

	ban = ns_calloc( sizeof( banentry ) );
	os_memcpy( ban, data, sizeof( banentry ) );
	hnode_create_insert( banhash, ban, ban->host );
	return NS_FALSE;
}

static void LoadBans( void )
{
	DBAFetchRows( "ban", new_ban );
}

/** @brief hs_set_regnick_cb
 *
 *  SET REGNICK callback
 *
 *  @param cmdparams
 *  @param reason
 *
 *  @return NS_SUCCESS 
 */

static int hs_set_regnick_cb( CmdParams* cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( hs_cfg.regnick ) {
			EnableEvent( EVENT_UMODE );
		} else {
			DisableEvent( EVENT_UMODE );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_set_expire_cb
 *
 *  SET EXPIRE callback
 *
 *  @param cmdparams
 *  @param reason
 *
 *  @return NS_SUCCESS 
 */

static int hs_set_expire_cb( CmdParams* cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
	{
		if( hs_cfg.expire ) {
			AddTimer( TIMER_TYPE_INTERVAL, ExpireOldHosts, "ExpireOldHosts", 7200 );
		} else {
			DelTimer( "ExpireOldHosts" );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_event_signon
 *
 *  Event handler for signon to automatically set a user's host
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_event_signon( CmdParams *cmdparams )
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	/* Check HostName Against Data Contained in vhosts.data */
	vhe = lnode_find( vhost_list, cmdparams->source->name, findnick );
	if( vhe ) {
		dlog( DEBUG1, "Checking %s against %s", vhe->host, cmdparams->source->user->hostname );
		if( match( vhe->host, cmdparams->source->user->hostname ) ) {
			irc_svshost( hs_bot, cmdparams->source, vhe->vhost );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"Automatically setting your hidden host to %s", vhe->vhost );
			SaveVhost( vhe );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_event_umode
 *
 *  Event handler for umode to set a user's host on regnick
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_event_umode( CmdParams *cmdparams ) 
{
	int add = 0;
	char *modes;
	char vhost[MAXHOST];

	SET_SEGV_LOCATION();
	if( IsOper( cmdparams->source ) && hs_cfg.operhosts == 0 ) 
		return NS_SUCCESS;
	/* first, find if its a regnick mode */
	modes = cmdparams->param;
	while( *modes ) {
		switch( *modes ) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			if( *modes == UmodeChRegNick ) {
				if( add ) {
					if( IsUserSetHosted( cmdparams->source ) ) {
						dlog( DEBUG2, "not setting hidden host on %s", cmdparams->av[0] );
						return -1;
					}
					dlog( DEBUG2, "Regnick Mode on %s", cmdparams->av[0] );
					ircsnprintf( vhost, MAXHOST, "%s.%s", cmdparams->av[0], hs_cfg.vhostdom );
					irc_svshost( hs_bot, cmdparams->source, vhost );
					irc_prefmsg( hs_bot, cmdparams->source, "Setting your host to %s", vhost );
					if( hs_cfg.verbose ) {
						irc_chanalert( hs_bot, "\2VHOST\2 registered nick %s now using vhost %s", 
							cmdparams->source->name, vhost );
					}

				}
			}
			break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	vhost_list = list_create( -1 );
	if( !vhost_list ) {
		nlog( LOG_CRITICAL, "Unable to create vhost list" );
		return -1;
	}
	banhash = hash_create( -1, 0, 0 );
	if( !banhash ) {
		nlog( LOG_CRITICAL, "Unable to create ban hash" );
		return -1;
	}
	ModuleConfig( hs_settings );
	LoadBans();
	LoadHosts();
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();
	hs_bot = AddBot( &hs_botinfo );
	if( !hs_bot ) {
		return NS_FAILURE;
	}
	if( hs_cfg.expire ) {
		AddTimer( TIMER_TYPE_INTERVAL, ExpireOldHosts, "ExpireOldHosts", 7200 );
	}
	if( !HaveUmodeRegNick() ) 
	{
		DisableEvent( EVENT_UMODE );
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, banhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		ban =( ( banentry * )hnode_get( hn ) );
		hash_delete( banhash, hn );
		hnode_destroy( hn );
		ns_free( ban );
	}
	hash_destroy( banhash );
	list_destroy_auto( vhost_list );
	return NS_SUCCESS;
}

/** @brief new_vhost
 *
 *  Allocate new vhost
 *
 *  @param nick
 *  @param host
 *  @param vhost
 *  @param pass
 *  @param who set it
 *
 *  @return none
 */

static void new_vhost( char *nick, char *host, char *vhost, char *pass, char *who )
{
	vhostentry *vhe;

	vhe = ns_calloc( sizeof( vhostentry ) );
	strlcpy( vhe->nick, nick, MAXNICK );
	strlcpy( vhe->host, host, MAXHOST );
	strlcpy( vhe->vhost, vhost, MAXHOST );
	strlcpy( vhe->passwd, pass, MAXPASS );
	strlcpy( vhe->added, who, MAXNICK );
	lnode_create_append( vhost_list, vhe );
	SaveVhost( vhe );
}

/** @brief hs_cmd_bans_list
 *
 *  Command handler for BANS LIST
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans_list( CmdParams *cmdparams )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	SET_SEGV_LOCATION();
	if( hash_count( banhash ) == 0 ) {
		irc_prefmsg( hs_bot, cmdparams->source, "No bans are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, banhash );
	irc_prefmsg( hs_bot, cmdparams->source, "Banned vhosts" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		ban =( ( banentry * )hnode_get( hn ) );
		irc_prefmsg( hs_bot, cmdparams->source, "%d - %s added by %s for %s", i, ban->host, ban->who, ban->reason );
		i++;
	}
	irc_prefmsg( hs_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans_add
 *
 *  Command handler for BANS ADD
 *
 *  @param cmdparams
 *    cmdparams->av[1] = ban host mask
 *    cmdparams->av[2 - cmdparams->ac-1] = reason
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans_add( CmdParams *cmdparams )
{
	banentry *ban;
	char *buf;

	SET_SEGV_LOCATION();
	if( hash_lookup( banhash, cmdparams->av[1] ) != NULL ) {
		irc_prefmsg( hs_bot, cmdparams->source, 
			"%s already exists in the banned vhost list", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	ban = ns_calloc( sizeof( banentry ) );
	strlcpy( ban->host, cmdparams->av[1], MAXHOST );
	strlcpy( ban->who, cmdparams->source->name, MAXNICK );
	buf = joinbuf( cmdparams->av, cmdparams->ac, 2 );
	strlcpy( ban->reason, buf, MAXREASON );
	ns_free( buf );

	hnode_create_insert( banhash, ban, ban->host );
	irc_prefmsg( hs_bot, cmdparams->source, 
		"%s added to the banned vhosts list", cmdparams->av[1] );
	CommandReport( hs_bot, "%s added %s to the banned vhosts list",
		  cmdparams->source->name, cmdparams->av[1] );
	SaveBan( ban );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans_del
 *
 *  Command handler for BANS DEL
 *
 *  @param cmdparams
 *    cmdparams->av[1] = ban host mask
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans_del( CmdParams *cmdparams )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, banhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		ban =( banentry * )hnode_get( hn );
		if( ircstrcasecmp( ban->host, cmdparams->av[1] ) == 0 ) {
			hash_scan_delete( banhash, hn );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"Deleted %s from the banned vhost list", cmdparams->av[1] );
			CommandReport( hs_bot, "%s deleted %s from the banned vhost list",
				cmdparams->source->name, cmdparams->av[1] );
			nlog( LOG_NOTICE, "%s deleted %s from the banned vhost list",
				cmdparams->source->name, cmdparams->av[1] );
			hnode_destroy( hn );
			DBADelete( "bans", ban->host );
			ns_free( ban );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( hs_bot, cmdparams->source, "No entry for %s", cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans
 *
 *  Command handler for BANS
 *
 *  @param cmdparams
 *    cmdparams->av[0] = sub command
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( !ircstrcasecmp( cmdparams->av[0], "LIST" ) ) {
		return hs_cmd_bans_list( cmdparams );
	} else if( !ircstrcasecmp( cmdparams->av[0], "ADD" ) ) {
		if( cmdparams->ac < 3 ) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		return hs_cmd_bans_add( cmdparams );
	} else if( !ircstrcasecmp( cmdparams->av[0], "DEL" ) ) {
		if( cmdparams->ac < 2 ) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		return hs_cmd_bans_del( cmdparams );
	}
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief hs_cmd_chpass
 *
 *  Command handler for CHPASS
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login
 *    cmdparams->av[1] = old password
 *    cmdparams->av[2] = new password
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_chpass( CmdParams *cmdparams )
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	vhe = lnode_find( vhost_list, cmdparams->av[0], findnick );
	if( !vhe ) {
		irc_prefmsg( hs_bot, cmdparams->source, "No vhost for that user." );
		irc_chanalert( hs_bot, "%s tried to change the password for %s, but there is no user with that name",
			cmdparams->source->name, cmdparams->av[0] );
		return NS_SUCCESS;
	}
	if( ( match( vhe->host, cmdparams->source->user->hostname ) )
		||( UserLevel( cmdparams->source ) >= 100 ) ) {
		if( !ircstrcasecmp( vhe->passwd, cmdparams->av[1] ) ) {
			strlcpy( vhe->passwd, cmdparams->av[2], MAXPASS );
			irc_prefmsg( hs_bot, cmdparams->source, "Password changed" );
			CommandReport( hs_bot, "%s changed the password for %s",
					cmdparams->source->name, vhe->nick );
			SaveVhost( vhe );
		}
		return NS_SUCCESS;
	}
	irc_prefmsg( hs_bot, cmdparams->source, "Error, hostname mismatch" );
	irc_chanalert( hs_bot, "%s tried to change the password for %s, but the hosts do not match( %s -> %s )",
			cmdparams->source->name, vhe->nick, cmdparams->source->user->hostname, vhe->host );
	nlog( LOG_WARNING, "%s tried to change the password for %s but the hosts do not match( %s -> %s )",
			cmdparams->source->name, vhe->nick, cmdparams->source->user->hostname, vhe->host );
	return NS_SUCCESS;
}

/** @brief hs_cmd_add
 *
 *  Command handler for ADD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login
 *    cmdparams->av[1] = real host mask
 *    cmdparams->av[2] = vhost
 *    cmdparams->av[3] = password
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_add( CmdParams *cmdparams )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;
	Client *u;

	SET_SEGV_LOCATION();
	if (cmdparams->source->user->ulevel < hs_cfg.addlevel && ircstrcasecmp(cmdparams->source->name, cmdparams->av[0])) {
		irc_prefmsg( hs_bot, cmdparams->source, "VHOST may be added for current nick only (%s)", cmdparams->source->name );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, banhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		ban =( banentry * ) hnode_get( hn );
		if( match( ban->host, cmdparams->av[2] ) ) {
			irc_prefmsg( hs_bot, cmdparams->source, 
				"%s has been matched against the vhost ban %s",
				cmdparams->av[2], ban->host );
			irc_chanalert( hs_bot, "%s tried to add a banned vhost %s",
				  cmdparams->source->name, cmdparams->av[2] );
			return NS_SUCCESS;
		}
	}
	if( ValidateHost( cmdparams->av[2] ) == NS_FAILURE || !index( cmdparams->av[2], '.' ) ) {
		irc_prefmsg( hs_bot, cmdparams->source, 
			"%s is an invalid host", cmdparams->av[2] );
		return NS_SUCCESS;
	}
	if( !ircstrcasecmp( cmdparams->av[1], "*" ) ) {
		irc_prefmsg( hs_bot, cmdparams->source, "* is too general a wildcard for realhost" );
		return NS_SUCCESS;
	}
	if( list_find( vhost_list, cmdparams->av[0], findnick ) ) {
		irc_prefmsg( hs_bot, cmdparams->source, 
			"%s already has a vhost entry", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	new_vhost( cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3], cmdparams->source->name );
	nlog( LOG_NOTICE, "%s added a vhost for %s with realhost %s vhost %s and password %s",
	    cmdparams->source->name, cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3] );
	irc_prefmsg( hs_bot, cmdparams->source, 
		"%s has successfully been registered under realhost: %s vhost: %s and password: %s",
		cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3] );
	CommandReport( hs_bot, "%s added a vhost %s for %s with realhost %s",
		cmdparams->source->name, cmdparams->av[2], cmdparams->av[0], cmdparams->av[1] );
	/* Apply The New Hostname If The User Is Online */
	u = FindUser( cmdparams->av[0] );
	if( u && !IsMe( u ) ) {
		if( match( cmdparams->av[1], u->user->hostname ) ) {
			irc_svshost( hs_bot, u, cmdparams->av[2] );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"%s is online now, setting vhost to %s",
				cmdparams->av[0], cmdparams->av[2] );
			irc_prefmsg( hs_bot, u, 
				"Your vhost has been created with hostmask of %s and username %s with password %s",
				cmdparams->av[1], cmdparams->av[0], cmdparams->av[3] );
			irc_prefmsg( hs_bot, u, 
				"For security, you should change your vhost password. See /msg %s help chpass",
				hs_bot->name );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_cmd_list
 *
 *  Command handler for LIST
 *
 *  @param cmdparams
 *    cmdparams->av[0] = Optional Number to start display after
 *   OR
 *    cmdparams->av[0] = Limit Type (NICK|HOST|VHOST)
 *    cmdparams->av[1] = wildcard match for Limit Type
 *	Calls hs_cmd_list_limit if limit type specified
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_list( CmdParams *cmdparams )
{
	int i;
	lnode_t *hn;
	vhostentry *vhe;
	int start = 0;
	int vhostcount;

	SET_SEGV_LOCATION();
	if( cmdparams->ac == 2 ) {
		if( !ircstrcasecmp(cmdparams->av[0], "nick") || !ircstrcasecmp(cmdparams->av[0], "host") || !ircstrcasecmp(cmdparams->av[0], "vhost")) {
			return hs_cmd_list_limit(cmdparams);
		}
	}
	if( cmdparams->ac == 1 ) {
		start = atoi( cmdparams->av[0] );
	}
	vhostcount = list_count( vhost_list );
	if( vhostcount == 0 ) {
		irc_prefmsg( hs_bot, cmdparams->source, "No vhosts are defined." );
		return NS_SUCCESS;
	}
	if( start >= vhostcount ) {
		irc_prefmsg( hs_bot, cmdparams->source, "Value out of range. There are only %d entries",( int )vhostcount );
		return NS_SUCCESS;
	}		
	i = 1;
	irc_prefmsg( hs_bot, cmdparams->source, "Current vhost list: " );
	irc_prefmsg( hs_bot, cmdparams->source, "Showing %d to %d entries of %d vhosts", start+1, start+PAGESIZE,( int )vhostcount );
	irc_prefmsg( hs_bot, cmdparams->source, "%-5s %-12s %-30s", "Num", "Nick", "Vhost" );
	hn = list_first( vhost_list );
	while( hn != NULL ) {
		if( i <= start ) {
			i++;
			hn = list_next( vhost_list, hn );
			continue;
		}
		vhe = lnode_get( hn );
		irc_prefmsg( hs_bot, cmdparams->source, "%-5d %-12s %-30s", i,
			vhe->nick, vhe->vhost );
		i++;
		/* limit to PAGESIZE entries per screen */
		if( i > start + PAGESIZE ) 
			break;
		hn = list_next( vhost_list, hn );
	}
	irc_prefmsg( hs_bot, cmdparams->source, 
		"For detailed information on a vhost use /msg %s VIEW <nick>",
		hs_bot->name );
	irc_prefmsg( hs_bot, cmdparams->source, "End of list." );
	if( vhostcount >= i ) {
		irc_prefmsg( hs_bot, cmdparams->source, "Type \2/msg %s list %d\2 to see next %d", hs_bot->name, i-1, PAGESIZE );
	}
	return NS_SUCCESS;
}

static int hs_cmd_list_limit( CmdParams *cmdparams )
{
	int i;
	int wm;
	lnode_t *hn;
	vhostentry *vhe;
	int vhostcount;

	if( !ircstrcasecmp(cmdparams->av[1], "*") ) {
		irc_prefmsg( hs_bot, cmdparams->source, "%s wildcard too broad, Refine wildcard limit (%s).", cmdparams->av[0], cmdparams->av[1] );
		return NS_SUCCESS;
	}
	vhostcount = list_count( vhost_list );
	if( vhostcount == 0 ) {
		irc_prefmsg( hs_bot, cmdparams->source, "No vhosts are defined." );
		return NS_SUCCESS;
	}
	i = 1;
	wm = 0;
	irc_prefmsg( hs_bot, cmdparams->source, "Current vhost list: " );
	irc_prefmsg( hs_bot, cmdparams->source, "Showing entries matching %s of %s", cmdparams->av[0], cmdparams->av[1]);
	irc_prefmsg( hs_bot, cmdparams->source, "%-5s %-12s %-30s", "Num", "Nick", "Vhost" );
	hn = list_first( vhost_list );
	while( hn != NULL ) {
		vhe = lnode_get( hn );
		if ( (!ircstrcasecmp(cmdparams->av[0], "nick") && match(cmdparams->av[1], vhe->nick)) || (!ircstrcasecmp(cmdparams->av[0], "host") && match(cmdparams->av[1], vhe->host)) || (!ircstrcasecmp(cmdparams->av[0], "vhost") && match(cmdparams->av[1], vhe->vhost)) ) {
			wm++;
			/* limit to PAGESIZE entries per screen */
			if ( wm <= PAGESIZE ) {
				irc_prefmsg( hs_bot, cmdparams->source, "%-5d %-12s %-30s", i, vhe->nick, vhe->vhost );
			} else {
				break;
			}
		}
		i++;
		hn = list_next( vhost_list, hn );
	}
	irc_prefmsg( hs_bot, cmdparams->source, 
		"For detailed information on a vhost use /msg %s VIEW <nick>",
		hs_bot->name );
	irc_prefmsg( hs_bot, cmdparams->source, "End of list." );
	if( wm > PAGESIZE ) {
		irc_prefmsg( hs_bot, cmdparams->source, "Not all matching entries shown, refine match to limit display");
	}
	return NS_SUCCESS;
}

/** @brief hs_cmd_view
 *
 *  Command handler for VIEW
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login to view
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_view( CmdParams *cmdparams )
{
	vhostentry *vhe;
	char ltime[80];

	SET_SEGV_LOCATION();
	vhe = lnode_find( vhost_list, cmdparams->av[0], findnick );
	if( !vhe ) {
		irc_prefmsg( hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	irc_prefmsg( hs_bot, cmdparams->source, "Vhost information:" );
	irc_prefmsg( hs_bot, cmdparams->source, "Nick:      %s", vhe->nick );
	irc_prefmsg( hs_bot, cmdparams->source, "Real host: %s", vhe->host );
	irc_prefmsg( hs_bot, cmdparams->source, "Vhost:     %s", vhe->vhost );
	irc_prefmsg( hs_bot, cmdparams->source, "Password:  %s", vhe->passwd );
	irc_prefmsg( hs_bot, cmdparams->source, "Added by:  %s",
		vhe->added ? vhe->added : "<unknown>" );
	strftime( ltime, 80, "%d/%m/%Y[%H:%M]", localtime( &vhe->tslastused ) );
	irc_prefmsg( hs_bot, cmdparams->source, "Last used: %s", ltime );
	irc_prefmsg( hs_bot, cmdparams->source, "--- End of information ---" );
	return NS_SUCCESS;
}

/** @brief hs_cmd_del
 *
 *  Command handler for DEL
 *    cmdparams->av[0] = login to delete
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_del( CmdParams *cmdparams )
{
	lnode_t *hn;
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	hn = list_find( vhost_list, cmdparams->av[0], findnick );
	if( !hn ) {
		irc_prefmsg( hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	vhe =( vhostentry * ) lnode_get( hn );
	irc_prefmsg( hs_bot, cmdparams->source, "removed vhost %s for %s",
		vhe->nick, vhe->vhost );
	nlog( LOG_NOTICE, "%s removed the vhost %s for %s", 
			cmdparams->source->name, vhe->vhost, vhe->nick );
	CommandReport( hs_bot, "%s removed vhost %s for %s",
			cmdparams->source->name, vhe->vhost, vhe->nick );
	del_vhost( vhe );
	list_delete( vhost_list, hn );
	lnode_destroy( hn );
	return NS_SUCCESS;
}

/** @brief hs_cmd_login
 *
 *  Command handler for LOGIN
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login
 *    cmdparams->av[1] = password
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_login( CmdParams *cmdparams )
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	/* Check HostName Against Data Contained in vhosts.data */
	vhe = lnode_find( vhost_list, cmdparams->av[0], findnick );
	if( vhe ) {
		if( !ircstrcasecmp( vhe->passwd, cmdparams->av[1] ) ) {
			irc_svshost( hs_bot, cmdparams->source, vhe->vhost );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"Your vhost has been set to %s", vhe->vhost );
			nlog( LOG_NORMAL, "%s used LOGIN to obtain vhost of %s",
			    cmdparams->source->name, vhe->vhost );
			if( hs_cfg.verbose ) {
				irc_chanalert( hs_bot, "\2VHOST\2 %s login to vhost %s", 
					cmdparams->source->name, vhe->vhost );
			}
			SaveVhost( vhe );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( hs_bot, cmdparams->source, 
		"Incorrect login or password. Do you have a vhost added?" );
	return NS_SUCCESS;
}
