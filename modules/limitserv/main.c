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
#include "limitserv.h"

typedef struct ls_channel {
	char name[MAXCHANLEN];
}ls_channel;

int joinchannels = 0;
int limitbuffer = 1;

/** Bot command function prototypes */
static int cmd_add( CmdParams *cmdparams );
static int cmd_list( CmdParams *cmdparams );
static int cmd_del( CmdParams *cmdparams );

/** Event function prototypes */
static int event_join( CmdParams *cmdparams );
static int event_part( CmdParams *cmdparams );

/** Setting callback prototypes */
static int set_join_cb( CmdParams* cmdparams, SET_REASON reason );
static int set_limitbuffer_cb( CmdParams* cmdparams, SET_REASON reason );


/** hash to store ls_channel and bot info */
static hash_t *qshash;

/** Bot pointer */
static Bot *ls_bot;

/** Copyright info */
const char *ls_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"LimitServ",
	"Channel limit service",
	ls_copyright,
	ls_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Bot comand table */
static bot_cmd ls_commands[]=
{
	{"ADD",		cmd_add,	1,	NS_ULEVEL_ADMIN,	help_add},
	{"DEL",		cmd_del,	1, 	NS_ULEVEL_ADMIN,	help_del},
	{"LIST",	cmd_list,	0, 	NS_ULEVEL_ADMIN,	help_list},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting ls_settings[]=
{
	{"JOIN",		&joinchannels,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	help_set_join,	set_join_cb,	( void* )0	},
	{"LIMITBUFFER", 	&limitbuffer, 	SET_TYPE_INT,	0, 100,		NS_ULEVEL_ADMIN, NULL,	help_set_limitbuffer, set_limitbuffer_cb, (void *)1	},
	NS_SETTING_END()
};

/** Bot info */
static BotInfo ls_botinfo = 
{
	"LimitServ", 
	"LimitServ1", 
	"TS", 
	BOT_COMMON_HOST, 
	"Limit service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	ls_commands, 
	ls_settings,
};

/** Module Events */
ModuleEvent module_events[] = 
{
	{EVENT_JOIN,	event_join},
	{EVENT_PART,	event_part},
	NS_EVENT_END()
};

/** @brief ManageLimit
 *
 *  manage channel limit
 *
 *  @param none
 *
 *  @return none
 */

static void ManageLimit( char *name, int users, int curlimit, int add )
{
	static char limitsize[10];
	int limit;

	if (curlimit > 0) {
		limit = curlimit;
	} else {
		limit = users;
	}
	if( add )
		limit = users + limitbuffer;
	else
		limit = users - limitbuffer;

	if( limit <= users )
		limit = ( users + limitbuffer );
	ircsnprintf( limitsize, 10, "%d", limit );	
	irc_cmode( ls_bot, name, "+l", limitsize );
}

/** @brief JoinChannels
 *
 *  join channels
 *
 *  @param none
 *
 *  @return none
 */

static void JoinChannels( void )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) 
	{
		Channel *c;

		db = ( ( ls_channel * )hnode_get( hn ) );
		c = FindChannel( db->name );
		if( c )
		{
			if( joinchannels )
				irc_join (ls_bot, db->name, "+o");
			ManageLimit( db->name, c->users, 0, 1 );
		}
	}
}

/** @brief PartChannels
 *
 *  part channels
 *
 *  @param none
 *
 *  @return none
 */

static void PartChannels( void )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) 
	{
		db = ( ( ls_channel * )hnode_get( hn ) );
		if( FindChannel( db->name ) )
			irc_part( ls_bot, db->name, NULL);
	}
}

/** @brief LoadChannel
 *
 *  load ls_channel
 *
 *  @param none
 *
 *  @return none
 */

static int LoadChannel( void *data, int size )
{
	ls_channel *db;

	db = ns_calloc( sizeof( ls_channel ) );
	os_memcpy( &db->name, data, MAXCHANLEN );
	hnode_create_insert( qshash, db, db->name );
	return NS_FALSE;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	qshash = hash_create( -1, 0, 0 );
	if( !qshash ) {
		nlog( LOG_CRITICAL, "Unable to create ls_channel hash" );
		return -1;
	}
	DBAFetchRows( "channels", LoadChannel );
	ModuleConfig( ls_settings );
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	ls_bot = AddBot( &ls_botinfo );
	if( !ls_bot ) 
		return NS_FAILURE;
	JoinChannels();
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
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		db = ( ( ls_channel * )hnode_get( hn ) );
		hash_delete_destroy_node( qshash, hn );
		ns_free( db );
	}
	hash_destroy( qshash );
	return NS_SUCCESS;
}

/** @brief cmd_add
 *
 *  Command handler for ADD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = channel
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int cmd_add( CmdParams *cmdparams )
{
	ls_channel *db;

	SET_SEGV_LOCATION();
	if( hash_lookup( qshash, cmdparams->av[0] ) != NULL ) 
	{
		irc_prefmsg( ls_bot, cmdparams->source, 
			"%s already exists in the channel list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	db = ns_calloc( sizeof( ls_channel ) );
	strlcpy( db->name, cmdparams->av[0], MAXCHANLEN );
	hnode_create_insert( qshash, db, db->name );
	DBAStore( "channels", db->name,( void * )db->name, MAXCHANLEN );
	CommandReport( ls_bot, "%s added %s to the channel list",
		cmdparams->source->name, cmdparams->av[0] );
	nlog( LOG_NOTICE, "%s added %s to the channel list",
		cmdparams->source->name, cmdparams->av[0] );
	return NS_SUCCESS;
}

/** @brief cmd_list
 *
 *  Command handler for LIST
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int cmd_list( CmdParams *cmdparams )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	if( hash_count( qshash ) == 0 )
	{
		irc_prefmsg( ls_bot, cmdparams->source, "No channels are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, qshash );
	irc_prefmsg( ls_bot, cmdparams->source, "channels" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		db = ( ( ls_channel * )hnode_get( hn ) );
		irc_prefmsg( ls_bot, cmdparams->source, "%s", db->name );
	}
	irc_prefmsg( ls_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief cmd_del
 *
 *  Command handler for DEL
 *    cmdparams->av[0] = ls_channel to delete
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int cmd_del( CmdParams *cmdparams )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		db = ( ls_channel * )hnode_get( hn );
		if( ircstrcasecmp( db->name, cmdparams->av[0] ) == 0 )
		{
			irc_prefmsg( ls_bot, cmdparams->source, 
				"Deleted %s from the channel list", cmdparams->av[0] );
			CommandReport( ls_bot, "%s deleted %s from the channel list",
				cmdparams->source->name, cmdparams->av[0] );
			nlog( LOG_NOTICE, "%s deleted %s from the channel list",
				cmdparams->source->name, cmdparams->av[0] );
			hash_scan_delete_destroy_node( qshash, hn );
			DBADelete( "channels", db->name );
			ns_free( db );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( ls_bot, cmdparams->source, "No entry for %s", cmdparams->av[0] );
	return NS_SUCCESS;
}

/** @brief event_join
 *
 *  join event handler
 *  join channels if we need to and manage limit on channels
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int event_join( CmdParams *cmdparams )
{
	ls_channel *db;

	SET_SEGV_LOCATION();
	db = (ls_channel *)hnode_find( qshash, cmdparams->channel->name );
	if( db )
	{
		/* Join channel if we are not a member */
		if( joinchannels && !IsChannelMember( cmdparams->channel, ls_bot->u ) )
			irc_join( ls_bot, db->name, "+o" );
		ManageLimit( cmdparams->channel->name, cmdparams->channel->users, cmdparams->channel->limit, 1 );
	}
	return NS_SUCCESS;
}

/** @brief event_part
 *
 *  part event handler
 *  manage limit on channels
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int event_part( CmdParams *cmdparams )
{
	ls_channel *db;

	SET_SEGV_LOCATION();
	db = (ls_channel *)hnode_find( qshash, cmdparams->channel->name );
	if( db )
		ManageLimit( cmdparams->channel->name, cmdparams->channel->users, cmdparams->channel->limit, 0 );
	return NS_SUCCESS;
}

/** @brief set_join_cb
 *
 *  SET JOIN callback
 *
 *  @param cmdparams
 *  @param reason
 *
 *  @return NS_SUCCESS 
 */

static int set_join_cb( CmdParams* cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
	{
		if( joinchannels )
			JoinChannels();
		else
			PartChannels();
	}
	return NS_SUCCESS;
}

static int set_limitbuffer_cb( CmdParams* cmdparams, SET_REASON reason )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	if( reason == SET_CHANGE )
	{
		hash_scan_begin( &hs, qshash );
		while( ( hn = hash_scan_next( &hs ) ) != NULL ) 
		{
			Channel *c;
			db = ( ( ls_channel * )hnode_get( hn ) );
			c = FindChannel( db->name );
			if( c )
				ManageLimit( db->name, c->users, cmdparams->channel->limit, 1 );
		}
	}
	return NS_SUCCESS;
}
