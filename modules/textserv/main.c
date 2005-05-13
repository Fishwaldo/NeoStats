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
#include "textserv.h"

typedef struct dbentry {
	char name[MAXNICK];
	char nick[MAXNICK];
	char channel[MAXCHANLEN];
} dbentry;

typedef struct dbbot {
	dbentry database;
	BotInfo botinfo;
	Bot *botptr;
	char **stringlist;
	int stringcount;
}dbbot;

/** Bot command function prototypes */
static int ts_cmd_add( CmdParams *cmdparams );
static int ts_cmd_list( CmdParams *cmdparams );
static int ts_cmd_del( CmdParams *cmdparams );

/** hash to store database and bot info */
static hash_t *tshash;

/** Bot pointer */
static Bot *ts_bot;

/** Copyright info */
const char *ts_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"TextServ",
	"Network text message service",
	ts_copyright,
	ts_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Bot comand table */
static bot_cmd ts_commands[]=
{
	{"ADD",		ts_cmd_add,		1,	NS_ULEVEL_ADMIN,	ts_help_add,	ts_help_add_oneline },
	{"DEL",		ts_cmd_del,		1, 	NS_ULEVEL_ADMIN,	ts_help_del,	ts_help_del_oneline },
	{"LIST",	ts_cmd_list,	0, 	NS_ULEVEL_ADMIN,	ts_help_list,	ts_help_list_oneline },
	{NULL,		NULL,			0, 	0,					NULL, 			NULL}
};

/** Bot setting table */
static bot_setting ts_settings[]=
{
	{NULL,		NULL,			0,			0, 0, 		0,				 NULL,		NULL,			NULL	},
};

/** Sub bot comand table template */
const char ts_help_cmd_oneline[] ="oneline";

const char *ts_help_cmd[] = {
	"Syntax: \2CMD\2",
	"",
	"command...",
	NULL
};


static bot_cmd ts_commandtemplate[]=
{
	{NULL,	NULL,	0, 	0,	ts_help_cmd,	ts_help_cmd_oneline },
};

/** Sub bot setting table template */
static bot_setting ts_settingstemplate[]=
{
	{NULL,		NULL,			0,			0, 0, 		0,				 NULL,		NULL,			NULL	},
};

/** TextServ BotInfo */
static BotInfo ts_botinfo = 
{
	"TextServ", 
	"TextServ1", 
	"TS", 
	BOT_COMMON_HOST, 
	"Network text message service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF, 
	ts_commands, 
	NULL,
};

/** @brief tsprintf
 *
 *  printf style message function 
 *
 *  @param buf Storage location for output. 
 *  @param fmt Format specification. 
 *  @param ... to list of arguments. 
 *
 *  @return number of characters written excluding terminating null
 */

int tsprintf( char *botname, char *from, char *target, char *buf, const size_t size, const char *fmt, ... )
{
	static char nullstring[] = "(null)";
	size_t len = 0;
    char *str;
    char c;
    va_list args;

	va_start( args, fmt );
	while( ( c = *fmt++ ) != 0 && ( len < size ) )
	{
		/* Is it a format string character? */
	    if( c == '%' ) 
		{
			switch( *fmt ) 
			{
				/* handle %B (botname) */
				case 'B': 
					str = botname;
					/* If NULL string point to our null output string */
					if( str == NULL ) {
						str = nullstring;
					}
					/* copy string to output observing limit */
					while( *str && len < size ) {
						buf[len++] = *str++;
					}
					/* next char... */
					fmt++;
					break;
				/* handle %F (from) */
				case 'F': 
					str = from;
					/* If NULL string point to our null output string */
					if( str == NULL ) {
						str = nullstring;
					}
					/* copy string to output observing limit */
					while( *str && len < size ) {
						buf[len++] = *str++;
					}
					/* next char... */
					fmt++;
					break;
				/* handle %T (target) */
				case 'T': 
					str = target;
					/* If NULL string point to our null output string */
					if( str == NULL ) {
						str = nullstring;
					}
					/* copy string to output observing limit */
					while( *str && len < size ) {
						buf[len++] = *str++;
					}
					/* next char... */
					fmt++;
					break;
				default:
					buf[len++] = c;
					break;
			}
		}
		/* just copy char from src */
		else 
		{
			buf[len++] = c;
	    }
	}
	/* NULL terminate */
	if( len < size ) 
		buf[len] = 0;
	else
		buf[size -1] = 0;
	/* return count chars written */
    va_end( args );
    return len;
}

/** @brief ts_read_database
 *
 *  Read a database file
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_read_database( dbbot *db )
{
	static char buf[BUFSIZE];
	FILE *fp;
	int commandcount;
	int commandreadcount = 0;
	int i;

	fp = os_fopen( db->database.name, "rt" );
	if( !fp )
		return NS_SUCCESS;
	/* Get command count */
	os_fgets(buf, BUFSIZE, fp);
	commandcount = atoi(buf);	
	while( os_fgets( buf, BUFSIZE, fp ) != NULL )
	{
		char *ptr;
		char *ptr2;
		
		/* comment char */
		if( buf[0] == '#' )
			continue;
		/* Get command text */
		ptr = strtok( buf, "|" );
		if( !ptr )
			break;
		dlog( DEBUG1, "command %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) );
		AddStringToList( &db->stringlist, ptr2, &db->stringcount );
		/* Get param text */
		ptr = strtok( NULL, "|" );
		if( !ptr )
			break;
		dlog( DEBUG1, "params %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) );
		AddStringToList( &db->stringlist, ptr2, &db->stringcount );
		/* Get help text */
		ptr = strtok( NULL, "|" );
		if( !ptr )
			break;
		dlog( DEBUG1, "desc %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) );
		AddStringToList( &db->stringlist, ptr2, &db->stringcount );
		/* Get output text */
		ptr = strtok( NULL, "|" );
		if( !ptr )
			break;
		ptr2 = ns_malloc( strlen( ptr ) );
		AddStringToList( &db->stringlist, ptr2, &db->stringcount );
		dlog( DEBUG1, "text %s", ptr );
		commandreadcount++;
	}	
	os_fclose( fp );
	if( commandreadcount != commandcount )
	{
		for( i = 0; i < db->stringcount; i++ )
		{
			ns_free( db->stringlist[i] );
		}
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief BuildBot
 *
 *  populate botinfo structure
 *
 *  @param none
 *
 *  @return none
 */
void BuildBot( dbbot *db )
{
	strlcpy( db->botinfo.nick, db->database.nick, MAXNICK );
	strlcpy( db->botinfo.user, "ts", MAXUSER );
	strlcpy( db->botinfo.realname, db->database.name, MAXREALNAME );
	db->botinfo.bot_setting_list = ns_calloc( sizeof (ts_settingstemplate) );
	os_memcpy( db->botinfo.bot_setting_list, ts_settingstemplate, sizeof (ts_settingstemplate) );
	db->botinfo.flags = BOT_FLAG_SERVICEBOT;
	ts_read_database( db );
}

/** @brief JoinBot
 *
 *  Join bot to IRC
 *
 *  @param none
 *
 *  @return none
 */

void JoinBot( dbbot *db )
{
	db->botptr = AddBot( &db->botinfo );
	if( *db->database.channel )
		irc_join( db->botptr, db->database.channel, NULL );
}

/** @brief PartBot
 *
 *  Part bot from IRC
 *
 *  @param none
 *
 *  @return none
 */

void PartBot( dbbot *db )
{
	int i;

	if( db->botptr )
	{
		if( *db->database.channel )
			irc_part( db->botptr, db->database.channel, "" );
		irc_quit( db->botptr, "" );
	}
	for( i = 0; i < db->stringcount; i++ )
	{
		ns_free( db->stringlist[i] );
	}
}

/** @brief load_dbentry
 *
 *  load dbentry
 *
 *  @param none
 *
 *  @return none
 */

static int load_dbentry( void *data, int size )
{
	dbbot *db;

	db = ns_calloc( sizeof( dbbot ) );
	os_memcpy( &db->database, data, sizeof( dbentry ) );
	hnode_create_insert( tshash, db, db->database.name );
	BuildBot( db );
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
	tshash = hash_create( -1, 0, 0 );
	if( !tshash ) {
		nlog( LOG_CRITICAL, "Unable to create database hash" );
		return -1;
	}
	DBAFetchRows( "databases", load_dbentry );
	ModuleConfig( ts_settings );
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
	dbbot *db;
	hnode_t *hn;
	hscan_t hs;

	ts_bot = AddBot( &ts_botinfo );
	if( !ts_bot ) 
	{
		return NS_FAILURE;
	}
	hash_scan_begin( &hs, tshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( dbbot * )hnode_get( hn ) );
		JoinBot( db );
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
	dbbot *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, tshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( dbbot * )hnode_get( hn ) );
		PartBot( db );
		hash_delete( tshash, hn );
		hnode_destroy( hn );
		ns_free( db );
	}
	hash_destroy( tshash );
	return NS_SUCCESS;
}

/** @brief ts_cmd_add
 *
 *  Command handler for ADD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = database
 *    cmdparams->av[1] = nick
 *    cmdparams->av[2] = optional channel
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_cmd_add( CmdParams *cmdparams )
{
	FILE *fp;
	dbbot *db;

	SET_SEGV_LOCATION();
	if( hash_lookup( tshash, cmdparams->av[0] ) != NULL ) 
	{
		irc_prefmsg( ts_bot, cmdparams->source, 
			"%s already exists in the database list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	fp = os_fopen( cmdparams->av[0], "rt" );
	if( !fp )
	{
		irc_prefmsg( ts_bot, cmdparams->source, 
			"%s not found", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	os_fclose( fp );
	if( cmdparams->ac > 1 && ValidateNick( cmdparams->av[1] ) != NS_SUCCESS )
	{
		irc_prefmsg( ts_bot, cmdparams->source, 
			"%s is an invalid nick", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	if( cmdparams->ac > 2 && ValidateChannel( cmdparams->av[2] ) != NS_SUCCESS )
	{
		irc_prefmsg( ts_bot, cmdparams->source, 
			"%s is an invalid channel", cmdparams->av[2] );
		return NS_SUCCESS;
	}
	db = ns_calloc( sizeof( dbbot ) );
	strlcpy( db->database.name, cmdparams->av[0], MAXNICK );
	if( cmdparams->ac > 1 )
		strlcpy( db->database.nick, cmdparams->av[1], MAXNICK );
	else
		strlcpy( db->database.nick, cmdparams->av[0], MAXNICK );
	if( cmdparams->ac > 2 )
		strlcpy( db->database.channel, cmdparams->av[2], MAXCHANLEN );
	hnode_create_insert( tshash, db, db->database.name );
	DBAStore( "databases", db->database.name,( void * )db, sizeof( dbentry ) );
	BuildBot( db );
	JoinBot( db );
	return NS_SUCCESS;
}

/** @brief ts_cmd_list
 *
 *  Command handler for LIST
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_cmd_list( CmdParams *cmdparams )
{
	dbbot *db;
	hnode_t *hn;
	hscan_t hs;
	int i = 1;

	SET_SEGV_LOCATION();
	if( hash_count( tshash ) == 0 ) {
		irc_prefmsg( ts_bot, cmdparams->source, "No databases are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, tshash );
	irc_prefmsg( ts_bot, cmdparams->source, "Databases" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( dbbot * )hnode_get( hn ) );
		irc_prefmsg( ts_bot, cmdparams->source, "%d - %s %s %s", i, db->database.name, db->database.nick, db->database.channel );
		i++;
	}
	irc_prefmsg( ts_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ts_cmd_del
 *
 *  Command handler for DEL
 *    cmdparams->av[0] = database to delete
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_cmd_del( CmdParams *cmdparams )
{
	dbbot *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, tshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( dbbot * )hnode_get( hn );
		if( ircstrcasecmp( db->database.name, cmdparams->av[0] ) == 0 ) {
			PartBot( db );
			hash_scan_delete( tshash, hn );
			irc_prefmsg( ts_bot, cmdparams->source, 
				"Deleted %s from the database list", cmdparams->av[0] );
			CommandReport( ts_bot, "%s deleted %s from the database list",
				cmdparams->source->name, cmdparams->av[0] );
			nlog( LOG_NOTICE, "%s deleted %s from the database list",
				cmdparams->source->name, cmdparams->av[0] );
			hnode_destroy( hn );
			DBADelete( "databases", db->database.name );
			ns_free( db );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( ts_bot, cmdparams->source, "No entry for %s", cmdparams->av[0] );
	return NS_SUCCESS;
}
