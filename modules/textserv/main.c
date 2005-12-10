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
 *  - Multiple output line support
 */

#include "neostats.h"
#include "textserv.h"

typedef struct botentry {
	char botname[MAXNICK];
	char botuser[MAXUSER];
	char bothost[MAXHOST];
	char dbname[MAXNICK];
	char channel[MAXCHANLEN];
	int public;
} botentry;

typedef struct dbbot {
	botentry tsbot;
	BotInfo botinfo;
	Bot *botptr;
	char *abouttext;
	char *creditstext;
	char *versiontext;
	char **stringlist;
	int stringcount;
	int helpcount;
	int commandcount;
	hash_t *chanhash;
}dbbot;

typedef struct botchanentry {
	char name[MAXNICK];
	char channel[MAXCHANLEN];
	char namechan[MAXNICK+MAXCHANLEN];
} botchanentry;

/** Bot command function prototypes */
static int ts_cmd_add( const CmdParams *cmdparams );
static int ts_cmd_list( const CmdParams *cmdparams );
static int ts_cmd_del( const CmdParams *cmdparams );

static int ts_cmd_msg( const CmdParams* cmdparams );
static int ts_cmd_about( const CmdParams* cmdparams );
static int ts_cmd_credits( const CmdParams* cmdparams );
static int ts_cmd_version( const CmdParams* cmdparams );
static int ts_cmd_add_chan( const CmdParams *cmdparams );
static int ts_cmd_del_chan( const CmdParams *cmdparams );

static char emptyline[] = "";

/** hash to store database and bot info */
static hash_t *tshash;

/** Bot pointer */
static Bot *ts_bot;

/** Copyright info */
static const char *ts_copyright[] = {
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
	0,
};

/** Bot command table */
static bot_cmd ts_commands[]=
{
	{"ADD",		ts_cmd_add,	3,	NS_ULEVEL_ADMIN,	ts_help_add, 0, NULL, NULL},
	{"DEL",		ts_cmd_del,	1, 	NS_ULEVEL_ADMIN,	ts_help_del, 0, NULL, NULL},
	{"LIST",	ts_cmd_list,	0, 	0,				ts_help_list, 0, NULL, NULL},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting ts_settings[]=
{
	NS_SETTING_END()
};

/** Sub bot command table template */
static const char *ts_help_about[] = {
	"Display about text",
	"Syntax: \2ABOUT\2",
	"",
	"Display information about the database",
	NULL
};

static const char *ts_help_credits[] = {
	"Display credits",
	"Syntax: \2CREDITS\2",
	"",
	"Display credits",
	NULL
};

static const char *ts_help_version[] = {
	"Display version",
	"Syntax: \2VERSION\2",
	"",
	"Display version",
	NULL
};

static const char *ts_help_addchan[] = {
	"Add Channel to Client",
	"Syntax: \2ADD <#channel>\2",
	"",
	"Adds the channel to the clients list",
	NULL
};

static const char *ts_help_delchan[] = {
	"Remove Channel from Client",
	"Syntax: \2DEL <#channel>\2",
	"",
	"Removes the channel from the clients list",
	NULL
};

static bot_cmd ts_commandtemplate =
{
	NULL,	ts_cmd_msg,	1, 	0,	NULL, CMD_FLAG_CHANONLY, NULL, NULL
};

static bot_cmd ts_commandtemplateabout =
{
	"ABOUT",	ts_cmd_about,	0, 	0,	ts_help_about, 0, NULL, NULL
};

static bot_cmd ts_commandtemplatecredits =
{
	"CREDITS",	ts_cmd_credits,	0, 	0,	ts_help_credits, 0, NULL, NULL
};

static bot_cmd ts_commandtemplateversion =
{
	"VERSION",	ts_cmd_version,	0, 	0,	ts_help_version, 0, NULL, NULL
};

static bot_cmd ts_commandtemplateaddchanpublic =
{
	"ADD",	ts_cmd_add_chan,	1, 	0,	ts_help_addchan, 0, NULL, NULL
};

static bot_cmd ts_commandtemplatedelchanpublic =
{
	"DEL",	ts_cmd_del_chan,	1, 	0,	ts_help_delchan, 0, NULL, NULL
};

static bot_cmd ts_commandtemplateaddchanprivate =
{
	"ADD",	ts_cmd_add_chan,	1, 	NS_ULEVEL_ADMIN,	ts_help_addchan, 0, NULL, NULL
};

static bot_cmd ts_commandtemplatedelchanprivate =
{
	"DEL",	ts_cmd_del_chan,	1, 	NS_ULEVEL_ADMIN,	ts_help_delchan, 0, NULL, NULL
};

/** Sub bot setting table template */
static bot_setting ts_settingstemplate[]=
{
	NS_SETTING_END()
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

static int tsprintf( char *botname, char *from, char *target, char *buf, const size_t size, const char *fmt, ... )
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
					while( *str != '\0' && len < size ) {
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
					while( *str != '\0' && len < size ) {
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
					while( *str != '\0' && len < size ) {
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
					while( *str != '\0' && len < size ) {
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
 *  @return none
 */
static void parse_line( dbbot *db, char *buf, int *commandreadcount )
{
	int readcount = 0;
	char *ptr;
	char *ptr2;

	/* Get command text */
	ptr = strtok( buf, "|" );
	if( !ptr )
		return;
	if( ircstrcasecmp( ptr, "ABOUT" ) == 0 )
	{
		ptr = strtok( NULL, "|" );
		if( !ptr )
			return;
		dlog( DEBUG1, "about %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		db->abouttext = ptr2;
		return;		
	}
	if( ircstrcasecmp( ptr, "CREDITS" ) == 0 )
	{
		ptr = strtok( NULL, "|" );
		if( !ptr )
			return;
		dlog( DEBUG1, "credits %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		db->creditstext = ptr2;
		return;		
	}
	if( ircstrcasecmp( ptr, "VERSION" ) == 0 )
	{
		ptr = strtok( NULL, "|" );
		if( !ptr )
			return;
		dlog( DEBUG1, "credits %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		db->versiontext = ptr2;
		return;		
	}
	while( ptr != NULL )
	{
		readcount++;
		dlog( DEBUG1, "read %s", ptr );
		ptr2 = ns_malloc( strlen( ptr ) + 1 );
		strcpy( ptr2, ptr );
		AddStringToList( &db->stringlist, ptr2, &db->helpcount );
		ptr = strtok( NULL, "|" );
	}
	if( readcount != 4 )
		return;
	(*commandreadcount)++;
}

/** @brief ts_read_database
 *
 *  Read a database file
 *
 *  @param none
 *
 *  @return none
 */

static void ts_read_database( dbbot *db )
{
	static char filename[MAXPATH];
	static char buf[BUFSIZE*4];
	static char helpbuf[128];
	FILE *fp;
	int commandreadcount = 0;
	int i;

	strlcpy( filename, "data/TSDB/", MAXPATH );
	strlcat( filename, db->tsbot.dbname, MAXPATH );
	fp = os_fopen( filename, "rt" );
	if( !fp )
		return;
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
	db->botinfo.bot_cmd_list = ns_calloc( ( commandreadcount + 6 ) * sizeof( bot_cmd ) );
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
	if( db->tsbot.public == 1 )
	{
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateaddchanpublic, sizeof( bot_cmd ) );
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplatedelchanpublic, sizeof( bot_cmd ) );
	} else {
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateaddchanprivate, sizeof( bot_cmd ) );
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplatedelchanprivate, sizeof( bot_cmd ) );
	}
}

/** @brief BuildBot
 *
 *  populate botinfo structure
 *
 *  @param none
 *
 *  @return none
 */

static void BuildBot( dbbot *db )
{
	strlcpy( db->botinfo.nick, db->tsbot.botname, MAXNICK );
	strlcpy( db->botinfo.altnick, db->tsbot.botname, MAXNICK );
	strlcpy( db->botinfo.user, (db->tsbot.botuser[0] != '\0') ? db->tsbot.botuser : "ts", MAXUSER );
	strlcpy( db->botinfo.host, db->tsbot.bothost, MAXHOST );
	strlcat( db->botinfo.realname, db->tsbot.dbname, MAXREALNAME );
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

static void JoinBot( dbbot *db )
{
	hnode_t *hn;
	hscan_t hs;
	char *channame;
	
	db->botptr = AddBot( &db->botinfo );
	SetBotModValue( db->botptr, (void *) db );
	if( *db->tsbot.channel )
		irc_join( db->botptr, db->tsbot.channel, "+o" );
	hash_scan_begin( &hs, db->chanhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		channame = ( ( char * )hnode_get( hn ) );
		irc_join( db->botptr, channame, "+o" );
	}
}

/** @brief PartBot
 *
 *  Part bot from IRC
 *
 *  @param none
 *
 *  @return none
 */

static void PartBot( dbbot *db )
{
	hnode_t *hn;
	hscan_t hs;
	char *channame;
	int i;

	if( db->botptr )
	{
		if( *db->tsbot.channel )
			irc_part( db->botptr, db->tsbot.channel, "" );
		hash_scan_begin( &hs, db->chanhash );
		while( ( hn = hash_scan_next( &hs ) ) != NULL )
		{
			channame = ( ( char * )hnode_get( hn ) );
			irc_part( db->botptr, channame, "" );
			hash_scan_delete_destroy_node( db->chanhash, hn );
			ns_free( channame );
		}
		irc_quit( db->botptr, "" );
		hash_destroy( db->chanhash );
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

/** @brief load_botentry
 *
 *  load botentry
 *
 *  @param none
 *
 *  @return none
 */

static int load_botentry( void *data, int size )
{
	dbbot *db;

	db = ns_calloc( sizeof( dbbot ) );
	os_memcpy( &db->tsbot, data, sizeof( botentry ) );
	db->chanhash = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	hnode_create_insert( tshash, db, db->tsbot.botname );
	BuildBot( db );
	return NS_FALSE;
}

/** @brief load_botchanentry
 *
 *  load botchanentry
 *
 *  @param none
 *
 *  @return none
 */

static int load_botchanentry( void *data, int size )
{
	dbbot *db;
	botchanentry *bce;
	char *channame;
	hnode_t *hn;

	bce = (botchanentry *)data;
	hn = hash_lookup( tshash, bce->name );
	if( hn != NULL )
	{
		db = ( ( dbbot * )hnode_get( hn ) );
		if( hash_lookup( db->chanhash, bce->channel) == NULL ) 
		{
			channame = ns_calloc( MAXCHANLEN );
			strlcpy( channame, bce->channel, MAXCHANLEN);
			hnode_create_insert( db->chanhash, channame, channame );
		}
	}
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
	tshash = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	if( !tshash ) {
		nlog( LOG_CRITICAL, "Unable to create database hash" );
		return NS_FAILURE;
	}
	DBAFetchRows( "Bots", load_botentry );
	DBAFetchRows( "BotChans", load_botchanentry );
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
		db = ( ( dbbot * )hnode_get( hn ) );
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
		db = ( ( dbbot * )hnode_get( hn ) );
		PartBot( db );
		hash_scan_delete_destroy_node( tshash, hn );
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
 *    cmdparams->av[0] = nick
 *    cmdparams->av[1] = database
 *    cmdparams->av[2] = main channel
 *    cmdparams->av[3] = optional public access on/off (default off)
 *    cmdparams->av[4] = optional user
 *    cmdparams->av[5] = optional host
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_cmd_add( const CmdParams *cmdparams )
{
	static char filename[MAXPATH];
	FILE *fp;
	dbbot *db;

	SET_SEGV_LOCATION();
	if( ValidateNick( cmdparams->av[0] ) != NS_SUCCESS )
	{
		irc_prefmsg( ts_bot, cmdparams->source, "%s is an invalid Nickname", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	if( hash_lookup( tshash, cmdparams->av[0] ) != NULL ) 
	{
		irc_prefmsg( ts_bot, cmdparams->source, 
			"%s already exists in the bot list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	strlcpy( filename, "data/TSDB/", MAXPATH );
	strlcat( filename, cmdparams->av[1], MAXPATH );
	fp = os_fopen( filename, "rt" );
	if( !fp )
	{
		irc_prefmsg( ts_bot, cmdparams->source, "database %s not found", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	os_fclose( fp );
	if( ValidateChannel( cmdparams->av[2] ) != NS_SUCCESS )
	{
		irc_prefmsg( ts_bot, cmdparams->source, "%s is an invalid channel", cmdparams->av[2] );
		return NS_SUCCESS;
	}
	db = ns_calloc( sizeof( dbbot ) );
	strlcpy( db->tsbot.botname, cmdparams->av[0], MAXNICK );
	strlcpy( db->tsbot.dbname, cmdparams->av[1], MAXNICK );
	strlcpy( db->tsbot.channel, cmdparams->av[2], MAXCHANLEN );
	if( cmdparams->ac > 3 )
		if( ircstrcasecmp( cmdparams->av[3], "public" ) == 0 )
			db->tsbot.public = 1;
	if( cmdparams->ac > 4 )
		if( ValidateUser( cmdparams->av[4] ) == NS_SUCCESS )
			strlcpy( db->tsbot.botuser, cmdparams->av[4], MAXUSER );
	if( cmdparams->ac > 5 )
		if( ValidateHost( cmdparams->av[5] ) == NS_SUCCESS )
			strlcpy( db->tsbot.bothost, cmdparams->av[5], MAXHOST );
	db->chanhash = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	if( !db->chanhash ) {
		nlog( LOG_CRITICAL, "Unable to create bots channel hash" );
		irc_prefmsg( ts_bot, cmdparams->source, "Error creating channel list, %s not added as a bot", cmdparams->av[0] );
		ns_free( db );
		return NS_SUCCESS;
	}
	hnode_create_insert( tshash, db, db->tsbot.botname );
	DBAStore( "Bots", db->tsbot.botname, ( void * )db, sizeof( botentry ) );
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

static int ts_cmd_list( const CmdParams *cmdparams )
{
	dbbot *db;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	if( hash_count( tshash ) == 0 ) {
		irc_prefmsg( ts_bot, cmdparams->source, "No bots are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, tshash );
	irc_prefmsg( ts_bot, cmdparams->source, "Bots" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db = ( ( dbbot * )hnode_get( hn ) );
		irc_prefmsg( ts_bot, cmdparams->source, "%s (%s@%s), %s, %s, %s", db->tsbot.botname, db->tsbot.botuser, db->tsbot.bothost, db->tsbot.public ? "Public" : "Private", db->tsbot.dbname, db->tsbot.channel );
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

static int ts_cmd_del( const CmdParams *cmdparams )
{
	dbbot *db;
	hnode_t *hn, *hn2;
	hscan_t hs, hs2;
	char *botchan;
	char *channame;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, tshash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		db = ( dbbot * )hnode_get( hn );
		if( ircstrcasecmp( db->tsbot.botname, cmdparams->av[0] ) == 0 ) {
			hash_scan_begin( &hs2, db->chanhash );
			while( ( hn2 = hash_scan_next( &hs2 ) ) != NULL )
			{
				channame = ( ( char * )hnode_get( hn2 ) );
				botchan = ns_calloc( MAXNICK+MAXCHANLEN );
				strlcpy( botchan, db->tsbot.botname, MAXNICK+MAXCHANLEN );
				strlcat( botchan, channame, MAXNICK+MAXCHANLEN );
				DBADelete( "BotChans", botchan );
				ns_free( botchan );
			}
			PartBot( db );
			irc_prefmsg( ts_bot, cmdparams->source, 
				"Deleted %s from the Bot list", cmdparams->av[0] );
			CommandReport( ts_bot, "%s deleted %s from the bot list",
				cmdparams->source->name, cmdparams->av[0] );
			hash_scan_delete_destroy_node( tshash, hn );
			DBADelete( "Bots", db->tsbot.botname );
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
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ts_cmd_msg( const CmdParams* cmdparams )
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

static int ts_cmd_about( const CmdParams* cmdparams )
{
	dbbot *db;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	irc_prefmsg( cmdparams->bot, cmdparams->source, db->abouttext );
	return NS_SUCCESS;
}

static int ts_cmd_credits( const CmdParams* cmdparams )
{
	dbbot *db;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	irc_prefmsg( cmdparams->bot, cmdparams->source, db->creditstext );
	return NS_SUCCESS;
}

static int ts_cmd_version( const CmdParams* cmdparams )
{
	dbbot *db;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	irc_prefmsg( cmdparams->bot, cmdparams->source, db->versiontext );
	return NS_SUCCESS;
}

/** @brief ts_cmd_add_chan
 *
 *  Command handler for bots ADD chan
 *
 *  @param cmdparams
 *    cmdparams->av[0] = channel
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_cmd_add_chan( const CmdParams *cmdparams )
{
	dbbot *db;
	char *channame;
	botchanentry *bce;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	if( ValidateChannel( cmdparams->av[0] ) != NS_SUCCESS )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "invalid channel name specified - %s ", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	if ((db->tsbot.public == 0) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
		return NS_FAILURE;
	if ((db->tsbot.public == 1) && (!IsChanOp(FindChannel(cmdparams->av[0]), cmdparams->source)) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
		return NS_FAILURE;
	if( hash_lookup( db->chanhash, cmdparams->av[0] ) != NULL )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "%s is already in the channel list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	channame = ns_calloc( MAXCHANLEN );
	strlcpy( channame, cmdparams->av[0], MAXCHANLEN );
	hnode_create_insert( db->chanhash, channame, channame );
	bce = ns_calloc( sizeof ( botchanentry ));
	strlcpy( bce->name, db->tsbot.botname, MAXCHANLEN );
	strlcpy( bce->channel, channame, MAXCHANLEN );
	strlcpy( bce->namechan, db->tsbot.botname, MAXNICK+MAXCHANLEN );
	strlcat( bce->namechan, channame, MAXNICK+MAXCHANLEN );
	DBAStore( "BotChans", bce->namechan, ( void * )bce, sizeof( botchanentry ) );
	irc_join( db->botptr, channame, "+o" );
	ns_free( bce );
	return NS_SUCCESS;
}

/** @brief ts_cmd_del_chan
 *
 *  Command handler for bots DEL chan
 *
 *  @param cmdparams
 *    cmdparams->av[0] = channel
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ts_cmd_del_chan( const CmdParams *cmdparams )
{
	dbbot *db;
	char *channame, *botchan;
	hnode_t *hn;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	if( ValidateChannel( cmdparams->av[0] ) != NS_SUCCESS )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "invalid channel name specified - %s ", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	if ((db->tsbot.public == 0) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
		return NS_FAILURE;
	if ((db->tsbot.public == 1) && (!IsChanOp(FindChannel(cmdparams->av[0]), cmdparams->source)) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
		return NS_FAILURE;
	hn = hash_lookup( db->chanhash, cmdparams->av[0] );
	if( hn == NULL )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "%s is not in the channel list", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	channame = ( ( char * )hnode_get( hn ) );
	irc_part( db->botptr, channame, "" );
	botchan = ns_calloc( MAXNICK+MAXCHANLEN );
	strlcpy( botchan, db->tsbot.botname, MAXNICK+MAXCHANLEN );
	strlcat( botchan, channame, MAXNICK+MAXCHANLEN );
	DBADelete( "BotChans", botchan );
	hash_delete_destroy_node( db->chanhash, hn );
	ns_free( channame );
	ns_free( botchan );
	return NS_SUCCESS;
}
