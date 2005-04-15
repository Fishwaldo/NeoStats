/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
** ( at your option)any later version.
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

/** ExtAuth Module
 *
 *  User authentication based on nick!user@host masking
 */

static int ea_cmd_access( CmdParams* cmdparams );

/** Access list struct */
typedef struct AccessEntry
{
	char nick[MAXNICK];
	char mask[USERHOSTLEN];
	int level;
}AccessEntry;

/** Copyright info */
static const char *extauth_copyright[] = 
{
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** About info */
static const char *extauth_about[] = 
{
	"\2ExtAuth\2 authorises users based on an access list.",
	NULL
};

/** Help text */
const char *ea_help_access[] = 
{
	"Syntax: \2ACCESS ADD <nick> <mask> <level>\2",
	"        \2ACCESS DEL <nick>\2",
	"        \2ACCESS LIST\2",
	"",
	"Manage the list of users having access to NeoStats",
	"<mask> must be of the form user@host",
	"<level> must be between 0 and 200",
	NULL
};

const char ea_help_access_oneline[] = "Manage NeoStats user access list";

/** Module info */
ModuleInfo module_info = 
{
	"ExtAuth",
	"Access List Authentication Module",
	extauth_copyright,
	extauth_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
};

/** hash for storing access list */
static hash_t *accesshash;

/** Bot comand table */
bot_cmd extauth_commands[] =
{
	{"ACCESS",	ea_cmd_access,	0,	NS_ULEVEL_ROOT, ea_help_access,	ea_help_access_oneline},
	{NULL,		NULL,			0, 	0,				NULL, 			NULL}
};

/** @brief dbaccesslisthandler
 *
 *  Table load handler
 *
 *  @param pointer to table row data
 *
 *  @return none
 */

static int dbaccesslisthandler( void *data, int size )
{
	AccessEntry *access;
	
	access = ns_calloc( sizeof( AccessEntry ) );
	os_memcpy( access, data, sizeof( AccessEntry ) );
	hnode_create_insert( accesshash, access, access->nick );
	return NS_FALSE;
}

/** @brief LoadAccessList
 *
 *  Load access list 
 *
 *  @param none
 *
 *  @return none
 */

static void LoadAccessList( void )
{
	accesshash = hash_create( -1, 0, 0 );
	DBAFetchRows( "AccessList", dbaccesslisthandler );
}

/** @brief AccessAdd
 *
 *  Access ADD sub-command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int AccessAdd( CmdParams* cmdparams )
{
	int level = 0;
	AccessEntry *access;
	
	SET_SEGV_LOCATION();
	if( cmdparams->ac < 3 ) 
	{
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if( hash_lookup( accesshash, cmdparams->av[1] ) ) 
	{
		irc_prefmsg( NULL, cmdparams->source, "Entry for %s already exists", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	if( !strstr( cmdparams->av[2], "@" ) ) 
	{
		irc_prefmsg( NULL, cmdparams->source, "Invalid format for hostmask. Must be of the form user@host." );
		return NS_ERR_SYNTAX_ERROR;
	}
	level = atoi( cmdparams->av[3] );
	if( level < 0 || level > NS_ULEVEL_ROOT) 
	{
		irc_prefmsg( NULL, cmdparams->source, "Level out of range. Valid values range from 0 to 200." );
		return NS_ERR_PARAM_OUT_OF_RANGE;
	}
	access = ns_calloc( sizeof( AccessEntry) );
	strlcpy( access->nick, cmdparams->av[1], MAXNICK );
	strlcpy( access->mask, cmdparams->av[2], USERHOSTLEN );
	access->level = level;
 	hnode_create_insert( accesshash, access, access->nick );
	/* save the entry */
	DBAStore( "AccessList", access->nick,( void *)access, sizeof( AccessEntry) );
	irc_prefmsg( NULL, cmdparams->source, "Successfully added %s for host %s with level %d to access list", access->nick, access->mask, access->level );
	return NS_SUCCESS;
}

/** @brief AccessDel
 *
 *  Access DEL sub-command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int AccessDel( CmdParams *cmdparams )
{
	hnode_t *node;

	SET_SEGV_LOCATION();
	if( cmdparams->ac < 1 ) 
	{
		return NS_ERR_SYNTAX_ERROR;
	}
	node = hash_lookup( accesshash, cmdparams->av[1] );
	if( node ) 
	{
		AccessEntry *access =( AccessEntry * )hnode_get( node );
		hash_delete( accesshash, node );
		hnode_destroy( node );
		ns_free( access );
		DBADelete( "AccessList", cmdparams->av[1] );
		irc_prefmsg( NULL, cmdparams->source, "Deleted %s from access list", cmdparams->av[1] );
	} 
	else 
	{
		irc_prefmsg( NULL, cmdparams->source, "Error, %s not found in access list.", cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief AccessList
 *
 *  Access LIST command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int AccessList( CmdParams *cmdparams )
{
	hscan_t accessscan;
	hnode_t *node;
	AccessEntry *access;

	SET_SEGV_LOCATION();	
	irc_prefmsg( NULL, cmdparams->source, "Access List (%d):",( int )hash_count( accesshash ) );
	hash_scan_begin( &accessscan, accesshash );
	while( ( node = hash_scan_next( &accessscan ) )!= NULL) 
	{
		access = hnode_get( node );
		irc_prefmsg( NULL, cmdparams->source, "%s %s (%d)", access->nick, access->mask, access->level );
	}
	irc_prefmsg( NULL, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ea_cmd_access
 *
 *  Access command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int ea_cmd_access( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( !ircstrcasecmp( cmdparams->av[0], "ADD" ) ) 
	{
		return AccessAdd( cmdparams );
	} 
	else if( !ircstrcasecmp( cmdparams->av[0], "DEL" ) ) 
	{
		return AccessDel( cmdparams );
	} 
	else if( !ircstrcasecmp( cmdparams->av[0], "LIST" ) ) 
	{
		return AccessList( cmdparams );
	}
	return NS_ERR_SYNTAX_ERROR;
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
	LoadAccessList();
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	if( add_services_cmd_list( extauth_commands ) != NS_SUCCESS ) 
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

int ModFini( void)
{
	del_services_cmd_list( extauth_commands );
	return NS_SUCCESS;
}

/** @brief ModAuthUser
 *
 *  Lookup authentication level for user
 *
 *  @param pointer to user
 *
 *  @return authentication level for user
 */

int ModAuthUser( Client *u )
{
	static char hostmask[USERHOSTLEN];
	AccessEntry *access;

	dlog( DEBUG2, "ModAuthUser for %s", u->name );
	access =( AccessEntry *)hnode_find( accesshash, u->name );
	if( access) 
	{
		ircsnprintf( hostmask, USERHOSTLEN, "%s@%s", u->user->username, u->user->hostname );
		if( match( access->mask, hostmask ) ) 
		{
			return access->level;		
		}
	}		
	return 0;
}
