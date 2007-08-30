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

/*  TODO:
 *  - Database sanity checking
 *  - Parameter support
 *  - Multiple output line support
 */

#include "neostats.h"
#include "textserv.h"
#include "confuse.h"

typedef struct botentry {
	char botname[MAXNICK];
	char botuser[MAXUSER];
	char bothost[MAXHOST];
	char dbname[MAXNICK];
	char channel[MAXCHANLEN];
	int ispublic;
} botentry;


typedef struct output {
	char **outputstring;
	int action;
	int target;
} output;

typedef struct dbbot {
	botentry tsbot;
	BotInfo botinfo;
	Bot *botptr;
	char *abouttext;
	char *creditstext;
	char *versiontext;
	output **outstr;
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
	"Copyright (c) 1999-2007, NeoStats",
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
	BOT_FLAG_ROOT|BOT_FLAG_DEAF, 
	ts_commands, 
	NULL,
};

/** database file format for confuse */
static cfg_opt_t tsdb_info[] = {
	CFG_STR ("about", NULL, CFGF_NODEFAULT),
	CFG_STR ("credits", NULL, CFGF_NODEFAULT),
	CFG_STR ("version", NULL, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t tsdb_cmd[] = {
	CFG_STR ("helpstring", NULL, CFGF_NODEFAULT | CFGF_LIST),
	CFG_STR ("output", NULL, CFGF_NODEFAULT | CFGF_LIST),
	CFG_BOOL ("action", 0, CFGF_NONE),
	CFG_INT ("params", 0, CFGF_NONE),
	CFG_STR ("paramlist", NULL, CFGF_NONE | CFGF_LIST),
	CFG_INT ("triggertype", 1, CFGF_NONE),
	CFG_INT ("sendtosource", 0, CFGF_NONE),
	CFG_END()
};

static cfg_opt_t tsdb[] = {
	CFG_SEC ("description", tsdb_info, CFGF_NONE),
	CFG_SEC ("command", tsdb_cmd, CFGF_MULTI | CFGF_TITLE),
	CFG_END()
};

int ispunch(char x) {
	if ((x >= 33 && x <= 47) || (x >= 58 && x <= 64) || (x >= 91 && x <= 96) || (x >= 123 && x <= 126)) {
		return 1;
	} else {
		return -1;
	}
}



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

static int tsprintf( const CmdParams *cmdparams, int targettype, char *buf, const size_t size, const char *fmt)
{
	static char nullstring[] = "(null)";
	size_t len = 0;
    	char *str;
    	char c;
    	int ac;
    	int add = 0;
    	if ((targettype == 2) || (targettype == 3))
    		add = 1;
	
	while( ( c = *fmt++ ) != 0 && ( len < size ) )
	{
		/* Is it a format string character? */
	    if( c == '%' ) 
		{
			switch( *fmt ) 
			{
				/* handle %B (botname) */
				case 'B': 
					str = cmdparams->bot->u->name;
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
					str = cmdparams->source->name;
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
				/* handle %P (params) */
				case 'P': 
					*fmt++;
					if (isdigit(*fmt)) {
						/* if its a digit, he wants the thats param value */
						ac = atoi(fmt++);
						if (ac > cmdparams->ac) {
							str = NULL;
						} else {
							str = cmdparams->av[ac+add];
						}
					} else if (isspace(*fmt) || isalpha(*fmt) || ispunch(*fmt) || (*fmt == '\0')) {
						/* he wants all the param */
						str = joinbuf(cmdparams->av, cmdparams->ac, 1+add);
					} else {
						str = NULL;
					}
					/* If NULL string point to our null output string */
					if( str == NULL ) {
						str = nullstring;
					}
					/* copy string to output observing limit */
					while( *str != '\0' && len < size ) {
						buf[len++] = *str++;
					}
					/* next char... */
					/* fmt++; */
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
    	return len;
}

/** @brief Confuse Error Reporter
 */
void ts_db_error(cfg_t *cfg, const char *fmt, va_list ap) {
	char buf[BUFSIZE];
	ircvsnprintf(buf, BUFSIZE, fmt, ap);
	CommandReport(ts_bot, "TextServ DB Error: %s", buf);
	nlog(LOG_WARNING, "TextServ DB Error: %s", buf);
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
	char filename[MAXPATH];
	int commandreadcount = 0;
	int i, j, k;
	cfg_t *cfg;
	cfg_t *cmd;
	int ret;

	strlcpy( filename, "data/TSDB/", MAXPATH );
	strlcat( filename, db->tsbot.dbname, MAXPATH );
	cfg = cfg_init(tsdb, CFGF_NOCASE);
	if (!cfg) 
		return;
	cfg_set_error_function(cfg, ts_db_error);
	if ( ( ret = cfg_parse(cfg, filename) ) != 0 ) {
		/* error */
		CommandReport(ts_bot, "Parsing TextServ DB %s Failed with error: %s", db->tsbot.dbname, ret == CFG_FILE_ERROR ? "File Not Found" : ret == CFG_PARSE_ERROR ? "TextServ DB Corrupt" : "Unknown Error");
		nlog(LOG_WARNING, "Parsing TextServ DB %s Failed with error: %s", db->tsbot.dbname, ret == CFG_FILE_ERROR ? "File Not Found" : ret == CFG_PARSE_ERROR ? "TextServ DB Corrupt" : "Unknown Error");			
		cfg_free(cfg);
		return;
	}
	/* count the number of commands */
	commandreadcount = cfg_size(cfg, "command");
	/* first, do the description section */
	db->botinfo.bot_cmd_list = ns_calloc( ( commandreadcount + 4) * sizeof( bot_cmd ) );
	db->abouttext = cfg_getstr(cfg, "description|about");
	db->creditstext = cfg_getstr(cfg, "description|credits");
	db->versiontext = cfg_getstr(cfg, "description|version");
	db->outstr = ns_calloc(commandreadcount * sizeof(output));
	for (i = 0; i < commandreadcount; i++) {
		cmd = cfg_getnsec(cfg, "command", i);
		os_memcpy( &db->botinfo.bot_cmd_list[i], &ts_commandtemplate, sizeof( bot_cmd ) );
		/* get min no of commands */
		db->botinfo.bot_cmd_list[i].minparams = cfg_size(cmd, "paramlist");
		/* get message type (!chan trigger or PM to bot (default is chan trigger)) */
		if (cfg_getint(cmd, "triggertype") == 0) 
			db->botinfo.bot_cmd_list[i].flags = 0;
		db->botinfo.bot_cmd_list[i].cmd = ns_malloc(strlen(cfg_title(cmd)+1));
		strlcpy((char *)db->botinfo.bot_cmd_list[i].cmd, cfg_title(cmd), strlen(cfg_title(cmd))+1);

		db->botinfo.bot_cmd_list[i].helptext = ns_calloc( cfg_size(cmd, "helpstring") * sizeof(char *));
		/* first item is generic help string */
		db->botinfo.bot_cmd_list[i].helptext[0] = ns_malloc(strlen(cfg_getnstr(cmd, "helpstring", 0)));
		strlcpy((char *)db->botinfo.bot_cmd_list[i].helptext[0], cfg_getnstr(cmd, "helpstring", 0), strlen(cfg_getnstr(cmd, "helpstring", 0))+1);
		for (k = 0; k < cfg_size(cmd, "paramlist"); k++) {
			db->botinfo.bot_cmd_list[i].helptext[k+1] = ns_malloc(strlen(cfg_getnstr(cmd, "paramlist", k)+1));
			strlcpy((char *)db->botinfo.bot_cmd_list[i].helptext[k+1], cfg_getnstr(cmd, "paramlist", k), strlen(cfg_getnstr(cmd, "paramlist", k))+1);
		}			
		for (j = 1; j < cfg_size(cmd, "helpstring"); j++) {		
			db->botinfo.bot_cmd_list[i].helptext[j+k] = ns_malloc(strlen(cfg_getnstr(cmd, "helpstring", j)+1));
			strlcpy((char *)db->botinfo.bot_cmd_list[i].helptext[j+k], cfg_getnstr(cmd, "helpstring", j), strlen(cfg_getnstr(cmd, "helpstring", j))+1);
		}
		/* make sure the last helpstring is null */
		db->botinfo.bot_cmd_list[i].helptext[j++] = NULL;

		db->outstr[i] = ns_calloc(sizeof(output));
		db->outstr[i]->outputstring = ns_calloc(cfg_size(cmd, "output") * sizeof(char *));
		if (cfg_getint(cmd, "sendtosource") == 1)
			db->outstr[i]->target = 1;
			
		for (j = 0; j < cfg_size(cmd, "output"); j++) {		
			db->outstr[i]->outputstring[j] = ns_malloc(strlen(cfg_getnstr(cmd, "output", j))+1);
			strlcpy(db->outstr[i]->outputstring[j], cfg_getnstr(cmd, "output", j), strlen(cfg_getnstr(cmd, "output", j))+1);
		}
		db->outstr[i]->action = cfg_getbool(cmd, "action");
		/* make sure the last output string is null */
		db->outstr[i]->outputstring[j++] = NULL;
		db->botinfo.bot_cmd_list[i].moddata = db->outstr[i];
	}
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateabout, sizeof( bot_cmd ) );
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplatecredits, sizeof( bot_cmd ) );
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateversion, sizeof( bot_cmd ) );
#if 0
/* broke at the moment. */
	if( db->tsbot.ispublic == 1 )
	{
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateaddchanpublic, sizeof( bot_cmd ) );
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplatedelchanpublic, sizeof( bot_cmd ) );
	} else {
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplateaddchanprivate, sizeof( bot_cmd ) );
		os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplatedelchanprivate, sizeof( bot_cmd ) );
	}
#endif
	os_memcpy( &db->botinfo.bot_cmd_list[i++], &ts_commandtemplate, sizeof( bot_cmd ) );
#if 0
	output *blah;
	for (i = 0; i < commandreadcount; i++) {
		printf("cmd: %s\n", db->botinfo.bot_cmd_list[i].cmd);
		printf("help: %s\n", db->botinfo.bot_cmd_list[i].helptext[0]);
		blah = db->botinfo.bot_cmd_list[i].moddata;
		printf("output %s\n", blah->outputstring[0]);
	}
#endif
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
	db->botinfo.flags = 0;
	if( db->tsbot.ispublic > 1 )
	{
		db->botinfo.flags = BOT_FLAG_PERSIST;
	}
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
		irc_join( db->botptr, db->tsbot.channel, me.servicescmode );
	hash_scan_begin( &hs, db->chanhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		channame = ( ( char * )hnode_get( hn ) );
		irc_join( db->botptr, channame, me.servicescmode );
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
	int i, j;

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
		j = 0;
		while (db->botinfo.bot_cmd_list[i].helptext[j] != NULL) {
			ns_free( db->botinfo.bot_cmd_list[i].helptext[j] );
			j++;
		}
		j = 0;
		while (db->outstr[i]->outputstring[j] != NULL) {
			ns_free(db->outstr[i]->outputstring[j]);
			j++;
		};
		ns_free( db->botinfo.bot_cmd_list[i].cmd);
	}
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
			db->tsbot.ispublic = 1;
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
		irc_prefmsg( ts_bot, cmdparams->source, "%s (%s@%s), %s, %s, %s", db->tsbot.botname, db->tsbot.botuser, db->tsbot.bothost, db->tsbot.ispublic ? "Public" : "Private", db->tsbot.dbname, db->tsbot.channel );
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
	char buf[BUFSIZE];
	output *fmt;
	dbbot *db;
	Client *target = NULL;
	Channel *targetc = NULL;
	lnode_t *chans;
	int i = 0;
	int validt = 0;	
	/* targettype 0 is channel, targettype 1 is requestor, targettype 2 is nickname, targettype 3 is channel specified */
	int targettype = 0;

	SET_SEGV_LOCATION();
	db = (dbbot *) GetBotModValue( cmdparams->bot );
	fmt = cmdparams->cmd_ptr->moddata;
	
	/* logic here:
	 *   (1) if command executed in a channel, we dont require a destination for the message 
	 *       (2) the command can specify to be sent to the requestor
	 *       (2a) or the source channel 
	 *   (3) if the command is executed via PM, we require a destination for the message
	 *       (4) the command can specify to be sent to the requester
	 *       (5) or a channel
	 *       (6) or a channel member
         */	 

         /* this is (1) above */
         if (cmdparams->chanmsg == 1) {
         	/* (2) if target is 1, then its sent to the requestor */
		if (fmt->target == 1) {
			targettype = 1;         
			target = cmdparams->source;
		/* (2a) send to the source channel */
		} else {
			targettype = 0;
			targetc = cmdparams->channel;
		}
	/* (3) now it gets complicated */
	 } else {
	 	/* (4) send to the requestor */
	 	if (fmt->target == 1) {
	 		targettype = 1;
			target = cmdparams->source;
		/* (5) send to a channel */
	 	} else if (cmdparams->av[0][0] == '#') {
	 		targettype = 3;
			targetc = FindChannel(cmdparams->av[0]);
			if (!targetc) {
				irc_prefmsg(cmdparams->bot, cmdparams->source, "%s is not a valid Channel", cmdparams->av[0]);
				return NS_FAILURE;
			} else {
				/* check if I am on this channel */
				chans = list_first(cmdparams->bot->u->user->chans);
				while (chans) {
					if (IsChannelMember(FindChannel(lnode_get(chans)), cmdparams->bot->u)) {
						validt = 1;
					}
					chans = list_next(cmdparams->bot->u->user->chans, chans);
				}
				if (validt == 0) {
					irc_prefmsg(cmdparams->bot, cmdparams->source, "I am not on %s channel. Message Not Sent", targetc->name);
					return NS_FAILURE;
				}
			}
		/* (6) send to channel member */
	 	} else {
	 		targettype = 2;   
			target = FindValidUser( cmdparams->bot, cmdparams->source, cmdparams->av[0] );
			if( !target ) 
			{
				/* FindValidUser already send a error to the user */
				return NS_FAILURE;
			} else {
				chans = list_first(cmdparams->bot->u->user->chans);
				while (chans) {
					if (IsChannelMember(FindChannel(lnode_get(chans)), target)) {
						validt = 1;
					}
					chans = list_next(cmdparams->bot->u->user->chans, chans);
				}		
				if (validt == 0) {
					irc_prefmsg(cmdparams->bot, cmdparams->source, "%s is not on any channels I am on. Command Not Sent", target->name);
					return NS_FAILURE;
				} 
			}
		}
	}
	while (fmt->outputstring[i] != NULL) {
		tsprintf(cmdparams, targettype, buf, BUFSIZE, fmt->outputstring[i]);
		/* its a trigger message to the channel */
		if ((targettype == 0) || (targettype == 3)) {
			if (fmt->action == 1)
				irc_ctcp_action_req_channel( cmdparams->bot, targetc, buf );
			else
				irc_chanprivmsg( cmdparams->bot, targetc->name, buf );
		/* its a message to a user */
		} else {
			if (fmt->action == 1) 
				irc_ctcp_action_req(cmdparams->bot, target, buf);
			else
				irc_privmsg(cmdparams->bot, target, buf);
		}
		i++;
	}
	if ((targettype == 0) || (targettype == 3)) 
		irc_prefmsg( cmdparams->bot, cmdparams->source, "%s has been sent to %s", cmdparams->cmd, targetc->name );
	else 
		irc_prefmsg( cmdparams->bot, cmdparams->source, "%s has been sent to %s", cmdparams->cmd, target->name );
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
	if ((db->tsbot.ispublic == 0) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
		return NS_FAILURE;
	if ((db->tsbot.ispublic == 1) && (!IsChanOp(FindChannel(cmdparams->av[0]), cmdparams->source)) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
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
	irc_join( db->botptr, channame, me.servicescmode );
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
	if ((db->tsbot.ispublic == 0) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
		return NS_FAILURE;
	if ((db->tsbot.ispublic == 1) && (!IsChanOp(FindChannel(cmdparams->av[0]), cmdparams->source)) && (cmdparams->source->user->ulevel < NS_ULEVEL_ADMIN) )
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
