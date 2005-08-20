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

/* @file global exclusion handling functions
 */

/*  TODO:
 *  - nothing at present
 *  TOCONSIDER:
 *  - Real time exclusions??? possibly optional.
 *  - move excludelists and bot_cmd_lists arrays into Module struct???
 */

#include "neostats.h"
#include "exclude.h"
#include "services.h"
#include "ircstring.h"
#include "helpstrings.h"

/* Prototype for module exclude command handler */
static int mod_cmd_exclude( const CmdParams *cmdparams );

/* Exclude types */
typedef enum NS_EXCLUDE
{
	NS_EXCLUDE_HOST	= 0,
	NS_EXCLUDE_SERVER,
	NS_EXCLUDE_CHANNEL,
	NS_EXCLUDE_USERHOST,
	NS_EXCLUDE_MAX,
} NS_EXCLUDE;

/* Maximum size of module exclude lists */
#define MAX_MOD_EXCLUDES		100

/* Exclude struct */
typedef struct Exclude
{
	NS_EXCLUDE type;
	char pattern[USERHOSTLEN];
	char addedby[MAXNICK];
	char reason[MAXREASON];
	time_t addedon;
} Exclude;

/* Global exclusion list */
static list_t *exclude_list;
/* Module exclusion list array */
static list_t *excludelists[NUM_MODULES];
/* Module exclusion command array */
static bot_cmd *bot_cmd_lists[NUM_MODULES];

/* String descriptions of exclude types */
const char* ExcludeDesc[NS_EXCLUDE_MAX] =
{
	"Host",
	"Server",
	"Channel",
	"Userhost"
};

/* Template bot exclude command struture */
bot_cmd mod_exclude_commands[] =
{
	{"EXCLUDE",	mod_cmd_exclude,1,	NS_ULEVEL_ADMIN,ns_help_exclude},
	NS_CMD_END()
};

/** @brief new_exclude
 *
 *  Add an exclude to the selected exclude list
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclude list to add to
 *  @param data exclude data
 *
 *  @return none
 */

static void new_exclude( list_t *exclude_list, const void *data )
{
	Exclude *exclude;

	exclude = ns_calloc( sizeof( Exclude ) );
	os_memcpy( exclude, data, sizeof( Exclude ) );
	lnode_create_append( exclude_list, exclude );
	dlog( DEBUG2, "Added exclusion %s (%d) by %s on %d", exclude->pattern, exclude->type, exclude->addedby,( int )exclude->addedon );
}

/** @brief new_global_exclude
 *
 *  Table load handler
 *  Database row handler to load global exclude data
 *  Exclusion sub system use only
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int new_global_exclude( const void *data, int size )
{
	new_exclude( exclude_list, data );
	return NS_FALSE;
}

/** @brief new_mod_exclude
 *
 *  Table load handler
 *  Database row handler to load module exclude data
 *  Exclusion sub system use only
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int new_mod_exclude( const void *data, int size )
{
	new_exclude( excludelists[GET_CUR_MODNUM()], data );
	return NS_FALSE;
}

/** @brief InitExcludes
 *
 *  Initialise global exclusion system and load existing exclusions
 *  NeoStats core use only
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitExcludes( void ) 
{
	exclude_list = list_create( -1 );
	DBAFetchRows( "exclusions", new_global_exclude );
	return NS_SUCCESS;
} 

/** @brief InitModExcludes
 *
 *  Initialise module exclusion system and load existing exclusions
 *  NeoStats core use only
 *
 *  @param mod_ptr pointer to module to initialise
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitModExcludes( Module *mod_ptr )
{
	SET_SEGV_LOCATION();
	SET_RUN_LEVEL( mod_ptr );
	excludelists[mod_ptr->modnum] = list_create( MAX_MOD_EXCLUDES );
	bot_cmd_lists[mod_ptr->modnum] = ns_malloc( sizeof( mod_exclude_commands ) );
	os_memcpy( bot_cmd_lists[mod_ptr->modnum], mod_exclude_commands, sizeof( mod_exclude_commands ) );
	DBAFetchRows( "exclusions", new_mod_exclude );
	RESET_RUN_LEVEL();
	return NS_SUCCESS;
}

/** @brief FiniExcludes
 *
 *  Finish global exclusion system
 *  NeoStats core use only
 *
 *  @param none
 *
 *  @return none
 */

void FiniExcludes( void )
{
	DBACloseTable( "exclusions" );
	list_destroy_auto( exclude_list );
}

/** @brief FiniModExcludes
 *
 *  Finish global exclusion system
 *  NeoStats core use only
 *
 *  @param mod_ptr pointer to module to initialise
 *
 *  @return none
 */

void FiniModExcludes( Module *mod_ptr )
{
	ns_free( bot_cmd_lists[mod_ptr->modnum] );
	list_destroy_auto( excludelists[mod_ptr->modnum] );
}

/** @brief AddBotExcludeCommands
 *
 *  Add exclude command list to bot that uses module exclusions
 *  NeoStats core use only
 *
 *  @param botptr pointer to bot to add comands to
 *
 *  @return none
 */

void AddBotExcludeCommands( Bot *botptr )
{
	add_bot_cmd_list( botptr, bot_cmd_lists[botptr->moduleptr->modnum] );
}

/** @brief do_exclude_add
 *
 *  EXCLUDE ADD command handler
 *  Adds exclude to exclusion list
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *    cmdparams->av[1] = type( one of HOST, CHANNEL, SERVER, USERHOST )
 *    cmdparams->av[2] = mask
 *    cmdparams->av[3..cmdparams->ac] = reason
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int do_exclude_add( list_t *exclude_list, const CmdParams *cmdparams )
{
	NS_EXCLUDE type;
	char *buf;
	Exclude *exclude, *etst;
	lnode_t *ln;
	
	if( cmdparams->ac < 4 )
		return NS_ERR_NEED_MORE_PARAMS;
	if( list_isfull( exclude_list ) )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "Error, Exception list is full" );
		return NS_SUCCESS;
	}
	if( !ircstrcasecmp( "HOST", cmdparams->av[1] ) )
	{
		if( !index( cmdparams->av[2], '.' ) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid host name" );
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_HOST;
	} 
	else if( !ircstrcasecmp( "CHANNEL", cmdparams->av[1] ) )
	{
		if( cmdparams->av[2][0] != '#' )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid channel name" );
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_CHANNEL;
	} 
	else if( !ircstrcasecmp( "SERVER", cmdparams->av[1] ) )
	{
		if( !index( cmdparams->av[2], '.' ) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid host name" );
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_SERVER;
	} 
	else if( !ircstrcasecmp( "USERHOST", cmdparams->av[1] ) )
	{
		if( !index( cmdparams->av[2], '!' ) || !index( cmdparams->av[2], '@' ) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid userhost mask" );
			return NS_SUCCESS;
		}
		type = NS_EXCLUDE_USERHOST;
	} 
	else
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid exclude type" );
		return NS_SUCCESS;
	}
	ln = list_first( exclude_list );
	while( ln != NULL ) 
	{
		etst = lnode_get( ln );
		if( etst->type == type )
		{
			if( match( etst->pattern, cmdparams->av[2] ) )
			{
				irc_prefmsg( cmdparams->bot, cmdparams->source, "Mask already matched by %s", etst->pattern );
				return NS_SUCCESS;
			}
		}
		ln = list_next( exclude_list, ln );
	}
	exclude = ns_calloc( sizeof( Exclude ) );
	exclude->type = type;
	exclude->addedon = me.now;
	strlcpy( exclude->pattern, collapse( cmdparams->av[2] ), MAXHOST );
	strlcpy( exclude->addedby, cmdparams->source->name, MAXNICK );
	buf = joinbuf( cmdparams->av, cmdparams->ac, 3 );
	strlcpy( exclude->reason, buf, MAXREASON );
	ns_free( buf );
	/* if we get here, then exclude is valid */
	lnode_create_append( exclude_list, exclude );
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Added %s (%s) to exclusion list", cmdparams->source ), exclude->pattern, cmdparams->av[1] );
	if( nsconfig.cmdreport )
		irc_chanalert( cmdparams->bot, _( "%s added %s (%s) to the exclusion list" ), cmdparams->source->name, exclude->pattern, cmdparams->av[1] );
	/* now save the exclusion list */
	DBAStore( "exclusions", exclude->pattern,( void * )exclude, sizeof( Exclude ) );
	return NS_SUCCESS;
} 

/** @brief ns_cmd_exclude_add
 *
 *  EXCLUDE ADD command handler for global exclusions
 *  Adds exclude to global exclusion list
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_exclude_add( const CmdParams *cmdparams ) 
{
	return do_exclude_add( exclude_list, cmdparams );
} 

/** @brief mod_cmd_exclude_add
 *
 *  EXCLUDE ADD command handler for module exclusions
 *  Adds exclude to module exclusion list
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int mod_cmd_exclude_add( const CmdParams *cmdparams )
{
	return do_exclude_add( excludelists[cmdparams->bot->moduleptr->modnum], cmdparams );
}

/** @brief do_exclude_del
 *
 *  EXCLUDE DEL command handler
 *  Deletes exclusion from exclusion list
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *    cmdparams->av[1] = mask
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int do_exclude_del( list_t *exclude_list, const CmdParams *cmdparams ) 
{
	lnode_t *node;
	Exclude *exclude;
	
	if( cmdparams->ac < 2 )
		return NS_ERR_NEED_MORE_PARAMS;
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		if( ircstrcasecmp( exclude->pattern, cmdparams->av[1] ) == 0 )
		{
			list_delete( exclude_list, node );
			lnode_destroy( node );
			DBADelete( "exclusions", exclude->pattern );
			irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s delete from exclusion list",cmdparams->source ), exclude->pattern );
			ns_free( exclude );
			return NS_SUCCESS;
		}
		node = list_next( exclude_list, node );
	}
	/* if we get here, means that we never got a match */
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s not found in the exclusion list",cmdparams->source ), cmdparams->av[1] );
	return NS_SUCCESS;
} 

/** @brief ns_cmd_exclude_del
 *
 *  EXCLUDE DEL global command handler
 *  Deletes exclusion from global exclusion list
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_exclude_del( const CmdParams *cmdparams ) 
{
	return do_exclude_del( exclude_list, cmdparams );
} 

/** @brief mod_cmd_exclude_del
 *
 *  EXCLUDE DEL module command handler
 *  Deletes exclusion from module exclusion list
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int mod_cmd_exclude_del( const CmdParams *cmdparams )
{
	return do_exclude_del( excludelists[cmdparams->bot->moduleptr->modnum], cmdparams );
}

/** @brief do_exclude_list
 *
 *  EXCLUDE LIST command handler
 *  List exclusions
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int do_exclude_list( list_t *exclude_list, const CmdParams *cmdparams ) 
{
	lnode_t *node;
	Exclude *exclude;
	
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Exclusion list:", cmdparams->source ) );
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );			
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s (%s) Added by %s on %s for %s", cmdparams->source ), exclude->pattern, ExcludeDesc[exclude->type], exclude->addedby, sftime( exclude->addedon ), exclude->reason );
		node = list_next( exclude_list, node );
	}
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "End of list.", cmdparams->source ) );
	return NS_SUCCESS;
} 

/** @brief ns_cmd_exclude_list
 *
 *  EXCLUDE LIST command handler
 *  List global exclusions
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_exclude_list( const CmdParams *cmdparams ) 
{
	return do_exclude_list( exclude_list, cmdparams );
} 

/** @brief mod_cmd_exclude_list
 *
 *  EXCLUDE LIST command handler
 *  List module exclusions
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int mod_cmd_exclude_list( const CmdParams *cmdparams )
{
	return do_exclude_list( excludelists[cmdparams->bot->moduleptr->modnum], cmdparams );
}

/** @brief ns_cmd_exclude
 *
 *  EXCLUDE command handler
 *  Manage global exclusions
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *    cmdparams->av[0] = subcommand( one of ADD, DEL, LIST )
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ns_cmd_exclude( CmdParams *cmdparams ) 
{
	SET_SEGV_LOCATION();
	if( !ircstrcasecmp( cmdparams->av[0], "ADD" ) )
		return ns_cmd_exclude_add( cmdparams );
	if( !ircstrcasecmp( cmdparams->av[0], "DEL" ) )
		return ns_cmd_exclude_del( cmdparams );
	if( !ircstrcasecmp( cmdparams->av[0], "LIST" ) )
		return ns_cmd_exclude_list( cmdparams );
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief mod_cmd_exclude
 *
 *  EXCLUDE command handler
 *  Manage module exclusions
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *    cmdparams->av[0] = subcommand( one of ADD, DEL, LIST )
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int mod_cmd_exclude( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( !ircstrcasecmp( cmdparams->av[0], "ADD" ) )
		return ns_cmd_exclude_add( cmdparams );
	if( !ircstrcasecmp( cmdparams->av[0], "DEL" ) )
		return ns_cmd_exclude_del( cmdparams );
	if( !ircstrcasecmp( cmdparams->av[0], "LIST" ) )
		return ns_cmd_exclude_list( cmdparams );
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief ns_do_exclude_user
 *
 *  Check user against global exclusion list and set appropriate flags 
 *  on user connect
 *  NeoStats core use only 
 *
 *  @param u pointer to Client struct of user to check
 *
 *  @return none
 */

void ns_do_exclude_user( Client *u ) 
{
	lnode_t *node;
	Exclude *exclude;
	
	/* if the server is excluded, user is excluded as well */
	if( u->uplink->flags & NS_FLAG_EXCLUDED )
	{
	 	u->flags |= NS_FLAG_EXCLUDED;
		return;
	}	
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		switch( exclude->type )
		{
			case NS_EXCLUDE_HOST:
				if( match( exclude->pattern, u->user->hostname ) )
				{
					u->flags |= NS_FLAG_EXCLUDED;
					return;
				}
				break;
			case NS_EXCLUDE_USERHOST:
				if( match( exclude->pattern, u->user->userhostmask ) )
				{
					u->flags |= NS_FLAG_EXCLUDED;
					return;
				}
				break;
		}
		node = list_next( exclude_list, node );
	}
	/* if we are here, there is no match */
	u->flags &= ~NS_FLAG_EXCLUDED;
}

/** @brief ns_do_exclude_server
 *
 *  Check server against global exclusion list and set appropriate flags
 *  on server connect
 *  NeoStats core use only
 *
 *  @param s pointer to Client struct of server to check
 *
 *  @return none
 */

void ns_do_exclude_server( Client *s ) 
{
	lnode_t *node;
	Exclude *exclude;
	
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		if( exclude->type == NS_EXCLUDE_SERVER )
		{
			if( match( exclude->pattern, s->name ) )
			{
				s->flags |= NS_FLAG_EXCLUDED;
				return;
			}
		}
		node = list_next( exclude_list, node );
	}
	/* if we are here, there is no match */
	s->flags &= ~NS_FLAG_EXCLUDED;
}

/** @brief ns_do_exclude_chan
 *
 *  Check channel against global exclusion list and set appropriate flags
 *  on channel creation
 *  NeoStats core use only
 *
 *  @param c pointer to Channel struct of channel to check
 *
 *  @return none
 */

void ns_do_exclude_chan( Channel *c ) 
{
	lnode_t *node;
	Exclude *exclude;
	
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		if( exclude->type == NS_EXCLUDE_CHANNEL )
		{
			if( match( exclude->pattern, c->name ) )
			{
				c->flags |= NS_FLAG_EXCLUDED;
				return;
			}
		}
		node = list_next( exclude_list, node );
	}
	/* if we are here, there is no match */
	c->flags &= ~NS_FLAG_EXCLUDED;
}

/** @brief ModIsServerExcluded
 *
 *  Check whether server is excluded by module exclusion list
 *  Module use
 *
 *  @param s pointer to Client struct of server to check
 *
 *  @return NS_TRUE if excluded else NS_FALSE if not
 */

int ModIsServerExcluded( const Client *s )
{
	lnode_t *node;
	Exclude *exclude;

	node = list_first( excludelists[GET_CUR_MODNUM()] );
	while( node )
	{
		exclude = lnode_get( node );
		if( exclude->type == NS_EXCLUDE_SERVER )
		{
			if( match( exclude->pattern, s->name ) )
			{
				dlog( DEBUG1, "Matched server entry %s in exclusions", exclude->pattern );
				return NS_TRUE;
			}
		}
		node = list_next( excludelists[GET_CUR_MODNUM()], node );
	}
	return NS_FALSE;
}

/** @brief ModIsUserExcluded
 *
 *  Check whether user is excluded by module exclusion list
 *  Module use
 *
 *  @param u pointer to Client struct of user to check
 *
 *  @return NS_TRUE if excluded else NS_FALSE if not
 */

int ModIsUserExcluded( const Client *u ) 
{
	lnode_t *node;
	Exclude *exclude;

	SET_SEGV_LOCATION();
	if( !ircstrcasecmp( u->uplink->name, me.name ) )
	{
		dlog( DEBUG1, "User %s Exclude. its Me!", u->name );
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first( excludelists[GET_CUR_MODNUM()] );
	while( node )
	{
		exclude = lnode_get( node );
		switch( exclude->type )
		{
			case NS_EXCLUDE_SERVER:
				dlog( DEBUG4, "Testing %s against server %s", u->uplink->name, exclude->pattern );
				if( match( exclude->pattern, u->uplink->name ) )
				{
					dlog( DEBUG1, "User %s excluded. Matched server entry %s in exclusions", u->name, exclude->pattern );
					return NS_TRUE;
				}
				break;
			case NS_EXCLUDE_HOST:
				dlog( DEBUG4, "Testing %s against host %s", u->user->hostname, exclude->pattern );
				if( match( exclude->pattern, u->user->hostname ) )
				{
					dlog( DEBUG1, "User %s is excluded. Matched host entry %s in exclusions", u->name, exclude->pattern );
					return NS_TRUE;
				}
				break;
			case NS_EXCLUDE_USERHOST:
				dlog( DEBUG4, "Testing %s against userhost %s", u->user->userhostmask, exclude->pattern );
				if( match( exclude->pattern, u->user->userhostmask ) )
				{
					dlog( DEBUG1, "User %s is excluded. Matched userhost entry %s in exclusions", u->name, exclude->pattern );
					return NS_TRUE;
				}
				break;
		}
		node = list_next( excludelists[GET_CUR_MODNUM()], node );
	}
	return NS_FALSE;
}

/** @brief ModIsChannelExcluded
 *
 *  Check whether channel is excluded by module exclusion list
 *  Module use
 *
 *  @param u pointer to Channel struct of channel to check
 *
 *  @return NS_TRUE if excluded else NS_FALSE if not
 */

int ModIsChannelExcluded( const Channel *c ) 
{
	lnode_t *node;
	Exclude *exclude;

	SET_SEGV_LOCATION();
	if( IsServicesChannel( c ) )
	{
		dlog( DEBUG1, "Services channel %s is exclude.", c->name );
		return NS_TRUE;
	}
	/* don't scan users from a server that is excluded */
	node = list_first( excludelists[GET_CUR_MODNUM()] );
	while( node )
	{
		exclude = lnode_get( node );
		if( exclude->type == NS_EXCLUDE_CHANNEL )
		{
			if( match( exclude->pattern, c->name ) )
			{
				dlog( DEBUG1, "Channel %s exclude. Matched Channel entry %s in Excludeions", c->name, exclude->pattern );
				return NS_TRUE;
			}
		}				
		node = list_next( excludelists[GET_CUR_MODNUM()], node );
	}
	return NS_FALSE;
}
