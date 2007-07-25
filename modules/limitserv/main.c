/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2007 Adam Rutter, Justin Hammond, Mark Hetherington
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

static unsigned int lsjoin = 0;
static unsigned int lsbuffer = 1;
static unsigned int lstimer = 10;

/** Bot command function prototypes */
static int cmd_add( const CmdParams *cmdparams );
static int cmd_list( const CmdParams *cmdparams );
static int cmd_del( const CmdParams *cmdparams );

/** Event function prototypes */
static int event_join( const CmdParams *cmdparams );

/** Timer function prototypes */
int limitservtimer(void *userptr);

/** Setting callback prototypes */
static int set_join_cb( const CmdParams *cmdparams, SET_REASON reason );
static int set_timer_cb( const CmdParams *cmdparams, SET_REASON reason );

/** hash to store ls_channel and bot info */
static hash_t *qshash;

/** Bot pointer */
static Bot *ls_bot;

/** Copyright info */
static const char *ls_copyright[] = {
	"Copyright (c) 1999-2007, NeoStats",
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
	0,
};

/** Bot command table */
static bot_cmd ls_commands[]=
{
	{"ADD",		cmd_add,	1,	NS_ULEVEL_ADMIN,	help_add, 0, NULL, NULL},
	{"DEL",		cmd_del,	1, 	NS_ULEVEL_ADMIN,	help_del, 0, NULL, NULL},
	{"LIST",	cmd_list,	0, 	NS_ULEVEL_ADMIN,	help_list, 0, NULL, NULL},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting ls_settings[]=
{
	{"JOIN",	&lsjoin,	SET_TYPE_BOOLEAN,	0, 0,	NS_ULEVEL_ADMIN, NULL,	help_set_join,		set_join_cb,	(void *)0	},
	{"BUFFER", 	&lsbuffer,	SET_TYPE_INT,		0, 100,	NS_ULEVEL_ADMIN, NULL,	help_set_buffer,	NULL,		(void *)1	},
	{"TIMER", 	&lstimer,	SET_TYPE_INT,		1, 100,	NS_ULEVEL_ADMIN, NULL,	help_set_timer,		set_timer_cb,	(void *)10	},
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
	{EVENT_JOIN, event_join, 0},
	NS_EVENT_END()
};

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
	ls_channel *ls_chan;
	hnode_t *node;
	hscan_t scan;

	hash_scan_begin( &scan, qshash );
	while( ( node = hash_scan_next( &scan ) ) != NULL ) 
	{
		Channel *c;

		ls_chan = ( ( ls_channel * )hnode_get( node ) );
		c = FindChannel( ls_chan->name );
		if( c )
		{
			if( lsjoin )
				irc_join( ls_bot, ls_chan->name, "+o" );
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
	ls_channel *ls_chan;
	hnode_t *node;
	hscan_t scan;

	hash_scan_begin( &scan, qshash );
	while( ( node = hash_scan_next( &scan ) ) != NULL ) 
	{
		ls_chan = ( ( ls_channel * )hnode_get( node ) );
		if( IsChannelMember( FindChannel( ls_chan->name ), ls_bot->u ) )
			irc_part( ls_bot, ls_chan->name, NULL);
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
	ls_channel *ls_chan;

	ls_chan = ns_calloc( sizeof( ls_channel ) );
	os_memcpy( ls_chan, data, MAXCHANLEN );
	hnode_create_insert( qshash, ls_chan, ls_chan->name );
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
	qshash = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	if( !qshash ) {
		nlog( LOG_CRITICAL, "Unable to create ls_channel hash" );
		return -1;
	}
	DBAFetchRows( "channels", LoadChannel );
	ModuleConfig( ls_settings );
	AddTimer (TIMER_TYPE_INTERVAL, limitservtimer, "limitservtimer", lstimer, NULL);
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
	ls_channel *ls_chan;
	hnode_t *node;
	hscan_t scan;

	SET_SEGV_LOCATION();
	DelTimer ("limitservtimer");
	hash_scan_begin( &scan, qshash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ls_chan = ( ( ls_channel * )hnode_get( node ) );
		hash_scan_delete_destroy_node( qshash, node );
		ns_free( ls_chan );
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

static int cmd_add( const CmdParams *cmdparams )
{
	ls_channel *ls_chan;

	SET_SEGV_LOCATION();
	if( hash_lookup( qshash, cmdparams->av[0] ) != NULL ) 
	{
		irc_prefmsg( ls_bot, cmdparams->source, 
			"%s already exists in the channel list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	ls_chan = ns_calloc( sizeof( ls_channel ) );
	strlcpy( ls_chan->name, cmdparams->av[0], MAXCHANLEN );
	hnode_create_insert( qshash, ls_chan, ls_chan->name );
	DBAStore( "channels", ls_chan->name, ( void * )ls_chan->name, MAXCHANLEN );
	CommandReport( ls_bot, "%s added %s to the channel list",
		cmdparams->source->name, cmdparams->av[0] );
	if( lsjoin )
		if(!IsChannelMember( FindChannel( ls_chan->name ), ls_bot->u ) )
			irc_join( ls_bot, ls_chan->name, NULL);
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

static int cmd_list( const CmdParams *cmdparams )
{
	ls_channel *ls_chan;
	hnode_t *node;
	hscan_t scan;

	SET_SEGV_LOCATION();
	if( hash_count( qshash ) == 0 )
	{
		irc_prefmsg( ls_bot, cmdparams->source, "No channels are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &scan, qshash );
	irc_prefmsg( ls_bot, cmdparams->source, "channels" );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ls_chan = ( ( ls_channel * )hnode_get( node ) );
		irc_prefmsg( ls_bot, cmdparams->source, "%s", ls_chan->name );
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

static int cmd_del( const CmdParams *cmdparams )
{
	ls_channel *ls_chan;
	hnode_t *node;
	hscan_t scan;

	SET_SEGV_LOCATION();
	hash_scan_begin( &scan, qshash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ls_chan = ( ls_channel * )hnode_get( node );
		if( ircstrcasecmp( ls_chan->name, cmdparams->av[0] ) == 0 )
		{
			if( lsjoin )
				if( IsChannelMember( FindChannel( ls_chan->name ), ls_bot->u ) )
					irc_part( ls_bot, ls_chan->name, NULL);
			irc_prefmsg( ls_bot, cmdparams->source, 
				"Deleted %s from the channel list", cmdparams->av[0] );
			CommandReport( ls_bot, "%s deleted %s from the channel list",
				cmdparams->source->name, cmdparams->av[0] );
			hash_scan_delete_destroy_node( qshash, node );
			DBADelete( "channels", ls_chan->name );
			ns_free( ls_chan );
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

static int event_join( const CmdParams *cmdparams )
{
	ls_channel *ls_chan;

	SET_SEGV_LOCATION();
	ls_chan = (ls_channel *)hnode_find( qshash, cmdparams->channel->name );
	if( ls_chan )
	{
		/* Join channel if we are not a member */
		if( lsjoin && !IsChannelMember( cmdparams->channel, ls_bot->u ) )
			irc_join( ls_bot, ls_chan->name, "+o" );
	}
	return NS_SUCCESS;
}

/** @brief limitservtimer
 *
 *  update channel user limit
 *
 *  @param none
 *
 *  @return none
 */
int limitservtimer(void *userptr) 
{
	static char limitsize[10];
	unsigned int limit;
	ls_channel *ls_chan;
	hnode_t *node;
	hscan_t scan;
	Channel *c;

	SET_SEGV_LOCATION();
	hash_scan_begin( &scan, qshash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ls_chan = ( ( ls_channel * )hnode_get( node ) );
		c = FindChannel(ls_chan->name);
		if( c != NULL )
		{
			if( c->users != ( c->limit - lsbuffer ) )
			{
				limit = ( c->users + lsbuffer );
				ircsnprintf( limitsize, 10, "%d", limit );	
				irc_cmode( ls_bot, ls_chan->name, "+l", limitsize );
			}
		}		
	}
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

static int set_join_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
	{
		if( lsjoin )
			JoinChannels();
		else
			PartChannels();
	}
	return NS_SUCCESS;
}

/** @brief set_timer_cb
 *
 *  SET TIMER callback
 *
 *  @param cmdparams
 *  @param reason
 *
 *  @return NS_SUCCESS 
 */

static int set_timer_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
		SetTimerInterval( "limitservtimer", lstimer );
	return NS_SUCCESS;
}
