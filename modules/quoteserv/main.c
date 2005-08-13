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

/*  TODO:
 *  - Database sanity checking
 *  - Free allocs made during database load
 */

#include "neostats.h"
#include "quoteserv.h"

typedef struct database {
	char name[MAXNICK];
	char *prefixstring;
	char *suffixstring;
	char **stringlist;
	int stringcount;
}database;

/** Bot command function prototypes */
static int qs_cmd_add( CmdParams *cmdparams );
static int qs_cmd_list( CmdParams *cmdparams );
static int qs_cmd_del( CmdParams *cmdparams );
static int qs_cmd_quote( CmdParams* cmdparams );

/** Set callbacks */
static int qs_set_exclusions_cb( CmdParams *cmdparams, SET_REASON reason );
static int qs_set_signonquote_cb( CmdParams *cmdparams, SET_REASON reason );

/** Event function prototypes */
static int event_signon( CmdParams *cmdparams );

/** Configuration variables */
int signonquote = 0;
int useexclusions = 0;

/** hash to store database and bot info */
static hash_t *qshash;

/** Bot pointer */
static Bot *qs_bot;

/** Copyright info */
const char *qs_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"QuoteServ",
	"Quote service",
	qs_copyright,
	qs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Bot comand table */
static bot_cmd qs_commands[]=
{
	{"ADD",		qs_cmd_add,	1,	NS_ULEVEL_ADMIN,	qs_help_add},
	{"DEL",		qs_cmd_del,	1, 	NS_ULEVEL_ADMIN,	qs_help_del},
	{"LIST",		qs_cmd_list,	0, 	NS_ULEVEL_ADMIN,	qs_help_list},
	{"QUOTE",	qs_cmd_quote,	0, 	0,		qs_help_quote},
	{NULL,		NULL,		0, 	0,		NULL}
};

/** Bot setting table */
static bot_setting qs_settings[]=
{
	{ "SIGNONQUOTE",	&signonquote,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	help_set_signonquote,	qs_set_signonquote_cb,	( void* )0	},
	{ "EXCLUSIONS",		&useexclusions,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	help_set_exclusions,	qs_set_exclusions_cb,	( void* )0	},
	{ NULL,				NULL,			0,					0, 0, 	0,				 NULL,	NULL,			NULL		},
};

/** BotInfo */
static BotInfo qs_botinfo = 
{
	"QuoteServ", 
	"QuoteServ1", 
	"QS", 
	BOT_COMMON_HOST, 
	"Quote service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	qs_commands, 
	qs_settings,
};

/** Module Events */
ModuleEvent module_events[] = 
{
	{EVENT_SIGNON,	event_signon},
	{EVENT_NULL,	NULL}
};

/** @brief qs_read_database
 *
 *  Read a database file
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int qs_read_database( database *db )
{
	static char filename[MAXPATH];
	static char buf[BUFSIZE*4];
	FILE *fp;
	char *ptr;

	strlcpy( filename, "data/", MAXPATH );
	strlcat( filename, db->name, MAXPATH );
	fp = os_fopen( filename, "rt" );
	if( !fp )
		return NS_SUCCESS;
	while( os_fgets( buf, BUFSIZE*4, fp ) != NULL )
	{
		/* comment char */
		if( buf[0] == '#' )
			continue;
		ptr = strdup(buf);
		strip(ptr);
		dlog( DEBUG1, "read %s", ptr );
		if( ircstrncasecmp( buf, "PREFIX:", 7 ) == 0 )
			db->prefixstring = strdup(ptr + 7);
		else if( ircstrncasecmp( buf, "SUFFIX:", 7 ) == 0 )
			db->suffixstring = strdup(ptr + 7);
		else
			AddStringToList( &db->stringlist, ptr, &db->stringcount );
	}	
	os_fclose( fp );
	return NS_SUCCESS;
}

/** @brief load_database
 *
 *  load database
 *
 *  @param none
 *
 *  @return none
 */

static int load_database( void *data, int size )
{
	database *db;

	db = ns_calloc( sizeof( database ) );
	os_memcpy( &db->name, data, MAXNICK );
	hnode_create_insert( qshash, db, db->name );
	qs_read_database( db );
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
		nlog( LOG_CRITICAL, "Unable to create database hash" );
		return NS_FAILURE;
	}
	DBAFetchRows( "databases", load_database );
	ModuleConfig( qs_settings );
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
	qs_bot = AddBot( &qs_botinfo );
	if( !qs_bot ) 
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
	database *db;
	hnode_t *hn;
	hscan_t hs;
	int i;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( database * )hnode_get( hn ) );
		hash_delete( qshash, hn );
		hnode_destroy( hn );
		ns_free(db->prefixstring);
		ns_free(db->suffixstring);
		for (i = 0; i < db->stringcount; i++) {
			ns_free(db->stringlist[i]);
		}
		ns_free(db->stringlist);
		ns_free( db );
	}
	hash_destroy( qshash );
	return NS_SUCCESS;
}

/** @brief qs_cmd_add
 *
 *  Command handler for ADD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = database
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int qs_cmd_add( CmdParams *cmdparams )
{
	static char filename[MAXPATH];
	FILE *fp;
	database *db;

	SET_SEGV_LOCATION();
	if( hash_lookup( qshash, cmdparams->av[0] ) != NULL ) 
	{
		irc_prefmsg( qs_bot, cmdparams->source, 
			"%s already exists in the database list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	strlcpy( filename, "data/", MAXPATH );
	strlcat( filename, cmdparams->av[0], MAXPATH );
	fp = os_fopen( filename, "rt" );
	if( !fp )
	{
		irc_prefmsg( qs_bot, cmdparams->source, "%s not found", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	os_fclose( fp );
	db = ns_calloc( sizeof( database ) );
	strlcpy( db->name, cmdparams->av[0], MAXNICK );
	hnode_create_insert( qshash, db, db->name );
	qs_read_database( db );
	DBAStore( "databases", db->name,( void * )db->name, MAXNICK );
	return NS_SUCCESS;
}

/** @brief qs_cmd_list
 *
 *  Command handler for LIST
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int qs_cmd_list( CmdParams *cmdparams )
{
	database *db;
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	SET_SEGV_LOCATION();
	if( hash_count( qshash ) == 0 ) {
		irc_prefmsg( qs_bot, cmdparams->source, "No databases are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, qshash );
	irc_prefmsg( qs_bot, cmdparams->source, "Databases" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( database * )hnode_get( hn ) );
		irc_prefmsg( qs_bot, cmdparams->source, "%d - %s", i, db->name );
		i++;
	}
	irc_prefmsg( qs_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief qs_cmd_del
 *
 *  Command handler for DEL
 *
 *  @param cmdparams
 *    cmdparams->av[0] = database to delete
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int qs_cmd_del( CmdParams *cmdparams )
{
	database *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, qshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( database * )hnode_get( hn );
		if( ircstrcasecmp( db->name, cmdparams->av[0] ) == 0 ) {
			hash_scan_delete( qshash, hn );
			irc_prefmsg( qs_bot, cmdparams->source, 
				"Deleted %s from the database list", cmdparams->av[0] );
			CommandReport( qs_bot, "%s deleted %s from the database list",
				cmdparams->source->name, cmdparams->av[0] );
			nlog( LOG_NOTICE, "%s deleted %s from the database list",
				cmdparams->source->name, cmdparams->av[0] );
			hnode_destroy( hn );
			DBADelete( "databases", db->name );
			ns_free( db );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( qs_bot, cmdparams->source, "No entry for %s", cmdparams->av[0] );
	return NS_SUCCESS;
}

/** @brief do_quote
 *
 *  Send a quote to a client
 *
 *  @param target to send quote
 *  @param which quote type
 *  @param reporterror flag whether to report errors to target
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int do_quote( Client *target, char *which, int reporterror )
{
	int flag = 0;
	database *db = NULL;
	int randno;
	
	SET_SEGV_LOCATION();
	if((which == NULL) || ( ircstrcasecmp( which, "random" ) == 0 ))
	{
		hnode_t *hn;
		hscan_t hs;
		int randdb;
		int i = 0;
		
		/* return if no databases defined */
		if (hash_count( qshash ) < 1)
			return NS_FAILURE;
		randdb = hrand( hash_count( qshash ) , 1 );	
		hash_scan_begin( &hs, qshash );
		while( ( hn = hash_scan_next( &hs ) ) != NULL )
		{
			i++;
			db =( database * )hnode_get( hn );
			if( i == randdb )
				break;
		}
		
	}
	else
	{
		db = (database *)hnode_find( qshash, which );
	}
	if (!db) {
		if( reporterror )
			irc_prefmsg( qs_bot, target, "%s not available", which );
		return NS_SUCCESS;
	}
	/* return if no records in selected database */
	/* TODO: This should be checked at DB load time, not during command execution! */
	if( db->stringcount < 1 )
		return NS_FAILURE;
	randno = hrand( db->stringcount, 1 );	
	if( db->prefixstring )
		flag |= 1 << 0;
	if( db->suffixstring )
		flag |= 1 << 1;
	switch( flag )
	{
		case 3: /* have prefix and suffix */
			irc_prefmsg( qs_bot, target, "%s %s %s", db->prefixstring, db->stringlist[randno], db->suffixstring );
			break;
		case 2: /* have suffix */
			irc_prefmsg( qs_bot, target, "%s %s", db->stringlist[randno], db->suffixstring );
			break;
		case 1: /* have suffix */
			irc_prefmsg( qs_bot, target, "%s %s", db->prefixstring, db->stringlist[randno] );
			break;
		case 0: /* no prefix or suffix */
		default:
			irc_prefmsg( qs_bot, target, db->stringlist[randno] );
	}
	return NS_SUCCESS;
}

/** @brief qs_cmd_quote
 *
 *  QUOTE command handler
 *
 *  @cmdparams pointer to commands param struct
 *    cmdparams->av[0] = target nick
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int qs_cmd_quote( CmdParams* cmdparams )
{
	return do_quote( cmdparams->source, cmdparams->ac >= 1 ? cmdparams->av[0] : NULL, 1 );
}

/** @brief event_signon
 *
 *  signon event handler
 *  Send quote on signon if enabled
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int event_signon( CmdParams *cmdparams )
{
	return do_quote( cmdparams->source, "random", 0 );
}

/** @brief cs_set_exclusions_cb
 *
 *  Set callback for exclusions
 *  Enable or disable exclude event flag
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int qs_set_exclusions_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		SetAllEventFlags( EVENT_FLAG_USE_EXCLUDE, useexclusions );
	}
	return NS_SUCCESS;
}

/** @brief qs_set_signonquote_cb
 *
 *  Set callback for signonquote
 *  Enable or disable events associated with signonquote
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int qs_set_signonquote_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( signonquote )
		{
			EnableEvent( EVENT_SIGNON );
		}
		else
		{
			DisableEvent( EVENT_SIGNON );
		}
	}
	return NS_SUCCESS;
}
