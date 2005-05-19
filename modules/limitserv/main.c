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

/** Bot command function prototypes */
static int ls_cmd_add( CmdParams *cmdparams );
static int ls_cmd_list( CmdParams *cmdparams );
static int ls_cmd_del( CmdParams *cmdparams );

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
	{"ADD",		ls_cmd_add,		1,	NS_ULEVEL_ADMIN,	ls_help_add,	ls_help_add_oneline },
	{"DEL",		ls_cmd_del,		1, 	NS_ULEVEL_ADMIN,	ls_help_del,	ls_help_del_oneline },
	{"LIST",	ls_cmd_list,	0, 	NS_ULEVEL_ADMIN,	ls_help_list,	ls_help_list_oneline },
	{NULL,		NULL,			0, 	0,					NULL, 			NULL}
};

/** Bot setting table */
static bot_setting ls_settings[]=
{
	{NULL,		NULL,			0,			0, 0, 		0,				 NULL,		NULL,			NULL	},
};

/** TextServ BotInfo */
static BotInfo ls_botinfo = 
{
	"LimitServ", 
	"LimitServ1", 
	"TS", 
	BOT_COMMON_HOST, 
	"Limit service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	ls_commands, 
	NULL,
};

/** @brief load_ls_channel
 *
 *  load ls_channel
 *
 *  @param none
 *
 *  @return none
 */

static int load_ls_channel( void *data, int size )
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
	DBAFetchRows( "channels", load_ls_channel );
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
	{
		return NS_FAILURE;
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
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( ls_channel * )hnode_get( hn ) );
		hash_delete( qshash, hn );
		hnode_destroy( hn );
		ns_free( db );
	}
	hash_destroy( qshash );
	return NS_SUCCESS;
}

/** @brief ls_cmd_add
 *
 *  Command handler for ADD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = channel
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ls_cmd_add( CmdParams *cmdparams )
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
	return NS_SUCCESS;
}

/** @brief ls_cmd_list
 *
 *  Command handler for LIST
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ls_cmd_list( CmdParams *cmdparams )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	SET_SEGV_LOCATION();
	if( hash_count( qshash ) == 0 ) {
		irc_prefmsg( ls_bot, cmdparams->source, "No channels are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, qshash );
	irc_prefmsg( ls_bot, cmdparams->source, "channels" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( ls_channel * )hnode_get( hn ) );
		irc_prefmsg( ls_bot, cmdparams->source, "%d - %s", i, db->name );
		i++;
	}
	irc_prefmsg( ls_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ls_cmd_del
 *
 *  Command handler for DEL
 *    cmdparams->av[0] = ls_channel to delete
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ls_cmd_del( CmdParams *cmdparams )
{
	ls_channel *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ls_channel * )hnode_get( hn );
		if( ircstrcasecmp( db->name, cmdparams->av[0] ) == 0 ) {
			hash_scan_delete( qshash, hn );
			irc_prefmsg( ls_bot, cmdparams->source, 
				"Deleted %s from the channel list", cmdparams->av[0] );
			CommandReport( ls_bot, "%s deleted %s from the channel list",
				cmdparams->source->name, cmdparams->av[0] );
			nlog( LOG_NOTICE, "%s deleted %s from the channel list",
				cmdparams->source->name, cmdparams->av[0] );
			hnode_destroy( hn );
			DBADelete( "channels", db->name );
			ns_free( db );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( ls_bot, cmdparams->source, "No entry for %s", cmdparams->av[0] );
	return NS_SUCCESS;
}
