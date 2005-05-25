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
 *  - Parameter support
 *  - Multiple channel per bot support
 *  - Multiple output line support
 */

#include "neostats.h"
#include "textserv.h"

typedef struct dbentry {
	char name[MAXNICK];
	char channel[MAXCHANLEN];
} dbentry;

typedef struct dbbot {
	dbentry database;
	BotInfo botinfo;
	Bot *botptr;
	char *abouttext;
	char *creditstext;
	char *versiontext;
	char **stringlist;
	int stringcount;
	int helpcount;
	int commandcount;
}dbbot;

/** Bot command function prototypes */
static int ts_cmd_add( CmdParams *cmdparams );
static int ts_cmd_list( CmdParams *cmdparams );
static int ts_cmd_del( CmdParams *cmdparams );

static int ts_cmd_msg( CmdParams* cmdparams );
static int ts_cmd_about( CmdParams* cmdparams );
static int ts_cmd_credits( CmdParams* cmdparams );
static int ts_cmd_version( CmdParams* cmdparams );

static char emptyline[] = "";

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
	{"ADD",		ts_cmd_add,		1,	NS_ULEVEL_ADMIN,	ts_help_add},
	{"DEL",		ts_cmd_del,		1, 	NS_ULEVEL_ADMIN,	ts_help_del},
	{"LIST",	ts_cmd_list,	0, 	NS_ULEVEL_ADMIN,	ts_help_list},
	{NULL,		NULL,			0, 	0,					NULL}
};

/** Bot setting table */
static bot_setting ts_settings[]=
{
	{NULL,		NULL,			0,			0, 0, 		0,				 NULL,		NULL,			NULL	},
};

/** Sub bot comand table template */
const char *ts_help_about[] = {
	"Display about text",
	"Syntax: \2ABOUT\2",
	"",
	"Display information about the database",
	NULL
};

const char *ts_help_credits[] = {
	"Display credits",
	"Syntax: \2CREDITS\2",
	"",
	"Display credits",
	NULL
};

const char *ts_help_version[] = {
	"Display version",
	"Syntax: \2VERSION\2",
	"",
	"Display version",
	NULL
};

static bot_cmd ts_commandtemplate[]=
{
	{NULL,	ts_cmd_msg,	1, 	0,	NULL, CMD_FLAG_CHANONLY },
};

static bot_cmd ts_commandtemplateabout[]=
{
	{"ABOUT",	ts_cmd_about,	0, 	0,	ts_help_about},
};

static bot_cmd ts_commandtemplatecredits[]=
{
	{"CREDITS",	ts_cmd_credits,	0, 	0,	ts_help_credits},
};

static bot_cmd ts_commandtemplateversion[]=
{
	{"VERSION",	ts_cmd_version,	0, 	0,	ts_help_version},
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
				/* handle %M (message) */
				case 'M': 
					str = va_arg( args, char * );
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

/** @brief parse_line
 *
 *  parse line of database file
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */
static int parse_line( dbbot *db, char *buf, int *commandreadcount )
{
	int readcount = 0;
	char *ptr;
	char *ptr2;

	/* Get command text */
	ptr = strtok( buf, "|" );
	if( !ptr )
		return NS_FAILURE;
	if( ircstrcasecmp( ptr, "ABOUT" ) == 0 )
	{
		ptr = strtok( NULL, "|" );
		if( !ptr )
			return NS_FAILURE;
		dlog( DEBUG1, "about %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		db->abouttext = ptr2;
		return NS_SUCCESS;		
	}
	if( ircstrcasecmp( ptr, "CREDITS" ) == 0 )
	{
		ptr = strtok( NULL, "|" );
		if( !ptr )
			return NS_FAILURE;
		dlog( DEBUG1, "credits %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		db->creditstext = ptr2;
		return NS_SUCCESS;		
	}
	if( ircstrcasecmp( ptr, "VERSION" ) == 0 )
	{
		ptr = strtok( NULL, "|" );
		if( !ptr )
			return NS_FAILURE;
		dlog( DEBUG1, "credits %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		db->versiontext = ptr2;
		return NS_SUCCESS;		
	}
	while( ptr )
	{
		readcount++;
		dlog( DEBUG1, "read %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		AddStringToList( &db->stringlist, ptr2, &db->helpcount );
		ptr = strtok( NULL, "|" );
	}
	if( readcount != 4 )
		return NS_FAILURE;
	(*commandreadcount)++;
	return NS_SUCCESS;
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
	static char filename[MAXPATH];
	static char buf[BUFSIZE*4];
	static char helpbuf[128];
	FILE *fp;
	int commandreadcount = 0;
	int i;

	strlcpy( filename, "data/", MAXPATH );
	strlcat( filename, db->database.name, MAXPATH );
	fp = os_fopen( filename, "rt" );
	if( !fp )
		return NS_SUCCESS;
	while( os_fgets( buf, BUFSIZE*4, fp ) != NULL )
	{
		/* comment char */
		if( buf[0] == '#' )
			continue;
		parse_line( db, buf, &commandreadcount );
	}	
	os_fclose( fp );
	db->commandcount = commandreadcount;
	/* Allocate command structure */
	db->botinfo.bot_cmd_list = ns_calloc( ( commandreadcount + 4 ) * sizeof( bot_cmd ) );
	for( i = 0; i < commandreadcount; i ++ )
	{		
		char *ptr;

		/* Fill in command structure defaults */
		os_memcpy( &db->botinfo.bot_cmd_list[i], &ts_commandtemplate, sizeof( bot_cmd ) );
		/* Assign command */
		db->botinfo.bot_cmd_list[i].cmd = db->stringlist[( i * 4 ) + 0];
		/* Allocate and build help text structures */
		db->botinfo.bot_cmd_list[i].helptext = ns_malloc( 5 * sizeof( char * ) );
		db->botinfo.bot_cmd_list[i].helptext[0] = db->stringlist[( i * 4 ) + 2];
		ptr = ns_malloc( ircsnprintf( helpbuf, 128, "Syntax: \2%s\2", db->stringlist[( i * 4 ) + 0] ) + 1 );
		strcpy( ptr, helpbuf );
		db->botinfo.bot_cmd_list[i].helptext[1] = ptr;
		db->botinfo.bot_cmd_list[i].helptext[2] = emptyline ;
		db->botinfo.bot_cmd_list[i].helptext[3] = db->stringlist[( i * 4 ) + 2];
		db->botinfo.bot_cmd_list[i].helptext[4] = NULL;
		/* Pointer to output format string */
		db->botinfo.bot_cmd_list[i].moddata = ( void * )db->stringlist[( i * 4 ) + 3];
	}
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateabout, sizeof( bot_cmd ) );
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplatecredits, sizeof( bot_cmd ) );
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateversion, sizeof( bot_cmd ) );
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
	strlcpy( db->botinfo.nick, db->database.name, MAXNICK );
	strlcpy( db->botinfo.user, "ts", MAXUSER );
	strlcat( db->botinfo.realname, db->database.name, MAXREALNAME );
	db->botinfo.bot_setting_list = ns_calloc( sizeof (ts_settingstemplate) );
	os_memcpy( db->botinfo.bot_setting_list, ts_settingstemplate, sizeof (ts_settingstemplate) );
	db->botinfo.flags = BOT_FLAG_SERVICEBOT|BOT_FLAG_NOINTRINSICLEVELS;
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
	SetBotModValue( db->botptr, (void *) db );
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
	for( i = 0; i < db->commandcount; i++ )
	{
		ns_free( db->botinfo.bot_cmd_list[i].helptext[1] );
		ns_free( db->botinfo.bot_cmd_list[i].helptext );
	}
	for( i = 0; i < db->stringcount; i++ )
	{
		ns_free( db->stringlist[i] );
	}
	ns_free( db->stringlist );
	ns_free( db->abouttext );
	ns_free( db->creditstext );	
	ns_free( db->versiontext );	
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
		return NS_FAILURE;
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
		return NS_FAILURE;
	hash_scan_begin( &hs, tshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
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
	static char filename[MAXPATH];
	FILE *fp;
	dbbot *db;

	SET_SEGV_LOCATION();
	if( hash_lookup( tshash, cmdparams->av[0] ) != NULL ) 
	{
		irc_prefmsg( ts_bot, cmdparams->source, 
			"%s already exists in the database list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	strlcpy( filename, "data/", MAXPATH );
	strlcat( filename, cmdparams->av[0], MAXPATH );
	fp = os_fopen( filename, "rt" );
	if( !fp )
	{
		irc_prefmsg( ts_bot, cmdparams->source, "%s not found", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	os_fclose( fp );
	if( cmdparams->ac > 1 && ValidateChannel( cmdparams->av[1] ) != NS_SUCCESS )
	{
		irc_prefmsg( ts_bot, cmdparams->source, "%s is an invalid channel", cmdparams->av[2] );
		return NS_SUCCESS;
	}
	db = ns_calloc( sizeof( dbbot ) );
	strlcpy( db->database.name, cmdparams->av[0], MAXNICK );
	if( cmdparams->ac > 1 )
		strlcpy( db->database.channel, cmdparams->av[1], MAXCHANLEN );
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

	SET_SEGV_LOCATION();
	if( hash_count( tshash ) == 0 ) {
		irc_prefmsg( ts_bot, cmdparams->source, "No databases are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, tshash );
	irc_prefmsg( ts_bot, cmdparams->source, "Databases" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db =( ( dbbot * )hnode_get( hn ) );
		irc_prefmsg( ts_bot, cmdparams->source, "%s %s", db->database.name, db->database.channel );
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

/** @brief ts_cmd_msg
 *
 *  ts_cmd_msg
 *    cmdparams->av[0] = target nick
 *    cmdparams->av[1 - cmdparams->ac] = message
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ts_cmd_msg( CmdParams* cmdparams )
{
	static char buf[BUFSIZE];
	char *fmt;
	dbbot *db;
	Client *target;
	int isaction = 0;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	target = FindValidUser( cmdparams->bot, cmdparams->source, cmdparams->av[0] );
	if( !target ) 
	{
		return NS_FAILURE;
	}
	irc_prefmsg( cmdparams->bot, cmdparams->source, "%s has been sent to %s", cmdparams->cmd, target->name );
	fmt = ( char * ) cmdparams->cmd_ptr->moddata;
	if( ircstrncasecmp( fmt, "ACTION ", 7 ) == 0 )
	{
		isaction = 1;
		fmt += 7;
	}
	if( cmdparams->ac > 1)
	{
		char *message;
		
		message = joinbuf( cmdparams->av, cmdparams->ac, 1 );
		tsprintf( cmdparams->bot->u->name, cmdparams->source->name, target->name, buf, BUFSIZE, fmt, message );
		ns_free( message );
	}
	else
	{
		tsprintf( cmdparams->bot->u->name, cmdparams->source->name, target->name, buf, BUFSIZE, fmt );
	}
	if( isaction )
		irc_ctcp_action_req_channel( cmdparams->bot, cmdparams->channel, buf );
	else
		irc_chanprivmsg( cmdparams->bot, cmdparams->channel->name, buf );
	return NS_SUCCESS;
}

static int ts_cmd_about( CmdParams* cmdparams )
{
	dbbot *db;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	irc_prefmsg( cmdparams->bot, cmdparams->source, db->abouttext );
	return NS_SUCCESS;
}

static int ts_cmd_credits( CmdParams* cmdparams )
{
	dbbot *db;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	irc_prefmsg( cmdparams->bot, cmdparams->source, db->creditstext );
	return NS_SUCCESS;
}

static int ts_cmd_version( CmdParams* cmdparams )
{
	dbbot *db;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	irc_prefmsg( cmdparams->bot, cmdparams->source, db->versiontext );
	return NS_SUCCESS;
}
