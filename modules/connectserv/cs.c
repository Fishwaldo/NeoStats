/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "cs.h"

/* Uncomment this line to enable colours in ConnectServ channel messages */
/* #define ENABLE_COLOUR_SUPPORT */

/** local structures */
/** Configuration structure  */
struct cs_cfg { 
	int sign_watch;
	int kill_watch;
	int mode_watch;
	int nick_watch;
	int away_watch;
	int serv_watch;
	int exclusions;
	int logging;
} cs_cfg;

/** Mode table definition */
typedef struct ModeDef {
	unsigned int mask;
	unsigned int serverflag;
}ModeDef;

/** output message and format strings */
#ifdef ENABLE_COLOUR_SUPPORT
static char msg_nickchange[] = "\2\0037NICK\2 user: \2%s\2 (%s@%s) changed their nick to \2%s\2\003"; 
static char msg_away[] = "\2AWAY\2 %s (%s@%s) is %s away %s";
static char msg_signon[] = "\2\0034SIGNON\2 user: \2%s\2 (%s@%s %s) at \2%s\2\003";
static char msg_signoff[] = "\2\0033SIGNOFF\2 user: %s (%s@%s %s) at %s %s\003";
static char msg_localkill[] = "\2\00312LOCAL KILL\2 user: \2%s\2 (%s@%s) killed by \2%s\2 for \2%s\2\003";
static char msg_globalkill[] = "\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) killed by \2%s\2 for \2%s\2\003";
static char msg_serverkill[] = "\2\00312SERVER KILL\2 user: \2%s\2 (%s@%s) killed by \2%s\2 for \2%s\2\003";
static char msg_mode[] = "\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c)\003";
static char msg_mode_serv[] = "\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c) on \2%s\2\003";
static char msg_bot[] = "\2\00313%s\2 is \2%s\2 a \2Bot\2 (%c%c)\003";
static char msg_server[] = "\2SERVER\2 %s joined the network at %s";
static char msg_squit[] = "\2SERVER\2 %s left the network at %s for %s";
#else
static char msg_nickchange[] = "\2NICK\2 %s (%s@%s) changed their nick to %s";
static char msg_away[] = "\2AWAY\2 %s (%s@%s) is %s away %s";
static char msg_signon[] = "\2SIGNON\2 %s (%s@%s %s) signed on at %s";
static char msg_signoff[] = "\2SIGNOFF\2 %s (%s@%s %s) signed off at %s %s";
static char msg_localkill[] = "\2LOCAL KILL\2 %s (%s@%s) killed by %s for \2%s\2";
static char msg_globalkill[] = "\2GLOBAL KILL\2 %s (%s@%s) killed by %s for \2%s\2";
static char msg_serverkill[] = "\2SERVER KILL\2 %s (%s@%s) killed by %s for \2%s\2";  
static char msg_mode[] = "\2MODE\2 %s is %s a %s (%c%c)";
static char msg_mode_serv[] = "\2MODE\2 %s is %s a %s (%c%c) on %s";
static char msg_bot[] = "\2BOT\2 %s is %s a Bot (%c%c)";
static char msg_server[] = "\2SERVER\2 %s joined the network at %s";
static char msg_squit[] = "\2SERVER\2 %s left the network at %s for %s";
#endif

/** Bot event function prototypes */
static int cs_event_signon( CmdParams *cmdparams );
static int cs_event_umode( CmdParams *cmdparams );
static int cs_event_smode( CmdParams *cmdparams );
static int cs_event_quit( CmdParams *cmdparams );
static int cs_event_localkill( CmdParams *cmdparams );
static int cs_event_globalkill( CmdParams *cmdparams );
static int cs_event_serverkill( CmdParams *cmdparams );
static int cs_event_nick( CmdParams *cmdparams );
static int cs_event_away( CmdParams *cmdparams );
static int cs_event_server( CmdParams *cmdparams );
static int cs_event_squit( CmdParams *cmdparams );

/** Set callbacks */
static int cs_set_exclusions_cb( CmdParams *cmdparams, SET_REASON reason );
static int cs_set_sign_watch_cb( CmdParams *cmdparams, SET_REASON reason );
static int cs_set_kill_watch_cb( CmdParams *cmdparams, SET_REASON reason );
static int cs_set_mode_watch_cb( CmdParams *cmdparams, SET_REASON reason );
static int cs_set_nick_watch_cb( CmdParams *cmdparams, SET_REASON reason );
static int cs_set_away_watch_cb( CmdParams *cmdparams, SET_REASON reason );
static int cs_set_serv_watch_cb( CmdParams *cmdparams, SET_REASON reason );

/** Bot pointer */
static Bot *cs_bot;

/** Copyright info */
const char *cs_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"ConnectServ",
	"Connection monitoring service",
	cs_copyright,
	cs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** Bot setting table */
static bot_setting cs_settings[]=
{
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "SignWatch",	NULL,	cs_help_set_signwatch, cs_set_sign_watch_cb,( void* )1 },
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "KillWatch",	NULL,	cs_help_set_killwatch, cs_set_kill_watch_cb,( void* )1 },
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ModeWatch",	NULL,	cs_help_set_modewatch, cs_set_mode_watch_cb,( void* )1 },
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "NickWatch",	NULL,	cs_help_set_nickwatch, cs_set_nick_watch_cb,( void* )1 },
	{"AWAYWATCH",	&cs_cfg.away_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "AwayWatch",	NULL,	cs_help_set_awaywatch, cs_set_away_watch_cb,( void* )1 },
	{"SERVWATCH",	&cs_cfg.serv_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "ServWatch",	NULL,	cs_help_set_servwatch, cs_set_serv_watch_cb,( void* )1 },
	{"EXCLUSIONS",	&cs_cfg.exclusions,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "Exclusions",	NULL,	cs_help_set_exclusions, cs_set_exclusions_cb,( void* )1 },
	{"LOGGING",		&cs_cfg.logging,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, "logging",		NULL,	cs_help_set_logging, NULL,( void* )1 },
	{NULL,			NULL,				0,					0, 0, 	0,				 NULL,			NULL,	NULL	},
};

/** BotInfo */
static BotInfo cs_botinfo = 
{
	"ConnectServ", 
	"ConnectServ1", 
	"CS", 
	BOT_COMMON_HOST, 
	"Connection monitoring service", 	
	BOT_FLAG_SERVICEBOT|BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
	NULL, 
	cs_settings,
};

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_SIGNON,		cs_event_signon,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_UMODE,		cs_event_umode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SMODE,		cs_event_smode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_QUIT,		cs_event_quit,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_LOCALKILL,	cs_event_localkill,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_GLOBALKILL,	cs_event_globalkill,EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SERVERKILL,	cs_event_serverkill,EVENT_FLAG_EXCLUDE_ME},
	{EVENT_NICK,		cs_event_nick,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_AWAY,		cs_event_away,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SERVER,		cs_event_server,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SQUIT,		cs_event_squit,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_NULL,		NULL}
};

/** umodes handled by this module */
ModeDef OperUmodes[]=
{
	{UMODE_NETADMIN,	0},
	{UMODE_TECHADMIN,	0},
	{UMODE_ADMIN,		1},
	{UMODE_COADMIN,		1},
	{UMODE_SADMIN,		0},
	{UMODE_OPER,		1},
	{UMODE_LOCOP,		1},
	{UMODE_SERVICES,	0},
	{0, 0},
};

/** smodes handled by this module */
ModeDef OperSmodes[]=
{
	{SMODE_NETADMIN,	0},
	{SMODE_CONETADMIN,	0},
	{SMODE_TECHADMIN,	0},
	{SMODE_COTECHADMIN, 0},
	{SMODE_ADMIN,		1},
	{SMODE_COADMIN,		1},
	{SMODE_GUESTADMIN,	1},
	{0, 0},
};

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( Module *mod_ptr )
{
	/* Load stored configuration */
	ModuleConfig( cs_settings );
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
	/* Create module bot. */
	cs_bot = AddBot( &cs_botinfo );
	/* If failed to create bot, module will terminate */
	if( !cs_bot ) {
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
 *  @return none
 */

void ModFini( void )
{
}

/** @brief cs_report
 *
 *  Handle reporting of event and logging if enabled
 *
 *  @param none
 *
 *  @return none
 */

void cs_report( const char *fmt, ... )
{
	static char buf[BUFSIZE];
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_chanalert( cs_bot, buf );
	if( cs_cfg.logging ) {
		nlog( LOG_NORMAL, buf );
	}
}

/** @brief cs_event_signon
 *
 *  Echo signon event
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_signon( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	/* Print Connection Notice */
	cs_report( msg_signon, cmdparams->source->name, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->info, cmdparams->source->uplink->name );
	return NS_SUCCESS;
}

/** @brief cs_event_quit
 *
 *  Echo quit event
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_quit( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	/* Print Disconnection Notice */
	if( cs_cfg.sign_watch ) {
		cs_report( msg_signoff, cmdparams->source->name, 
			cmdparams->source->user->username, cmdparams->source->user->hostname, 
			cmdparams->source->info, cmdparams->source->uplink->name, 
			cmdparams->param );
	}
	return NS_SUCCESS;
}

/** @brief cs_event_localkill
 *
 *  Echo local kill event
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_localkill( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( cs_cfg.kill_watch ) {
		cs_report( msg_localkill, cmdparams->target->name, 
			cmdparams->target->user->username, cmdparams->target->user->hostname,
			cmdparams->source->name, cmdparams->param );
	}
	return NS_SUCCESS;
}

/** @brief cs_event_globalkill
 *
 *  Echo global kill event
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_globalkill( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( cs_cfg.kill_watch ) {
		cs_report( msg_globalkill, cmdparams->target->name, 
			cmdparams->target->user->username, cmdparams->target->user->hostname,
			cmdparams->source->name, cmdparams->param );
	}
	return NS_SUCCESS;
}

/** @brief cs_event_serverkill
 *
 *  Echo server kill event
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_serverkill( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( cs_cfg.kill_watch ) {
		cs_report( msg_serverkill, cmdparams->target->name, 
			cmdparams->target->user->username, cmdparams->target->user->hostname,
			cmdparams->source->name, cmdparams->param );
	}
	return NS_SUCCESS;
}

/** @brief cs_report_mode
 *
 *  report mode change
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_report_mode( const char *modedesc, int serverflag, Client *u, int mask, int add, char mode )
{
	if( serverflag ) {
		cs_report( msg_mode_serv, u->name, 
			add ? "now" : "no longer", 
			modedesc,
			add ? '+' : '-',
			mode, u->uplink->name );
	} else {
		cs_report( msg_mode, u->name, 
			add ? "now" : "no longer", 
			modedesc,
			add ? '+' : '-',
			mode );
	}
	return NS_SUCCESS;
}

/** @brief cs_event_umode
 *
 *  report umode change
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_umode( CmdParams *cmdparams )
{
	int mask;
	int add = 1;
	char *modes;
	ModeDef* def;

	SET_SEGV_LOCATION();
	modes = cmdparams->param;
	while( *modes ) {
		switch( *modes ) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			mask = UmodeCharToMask( *modes );
			if( mask == UMODE_BOT ) {
				cs_report( msg_bot, cmdparams->source->name, 
					add ? "now" : "no longer", add ? '+' : '-', *modes );			
			} else {
				def = OperUmodes;
				while( def->mask ) {
					if( def->mask == mask ) 
					{
						cs_report_mode( GetUmodeDesc( def->mask ), def->serverflag, cmdparams->source, mask, add, *modes );
						break;
					}
					def ++;
				}
			}
			break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief cs_event_smode
 *
 *  report smode change
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_smode( CmdParams *cmdparams )
{
	int mask;
	int add = 1;
	char *modes;
	ModeDef* def;

	SET_SEGV_LOCATION();
	modes = cmdparams->param;
	while( *modes ) {
		switch( *modes ) {
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			default:
				mask = SmodeCharToMask( *modes );
				def = OperSmodes;
				while( def->mask ) {
					if( def->mask == mask ) 
					{
						cs_report_mode( GetSmodeDesc( def->mask ), def->serverflag, cmdparams->source, mask, add, *modes );
						break;
					}
					def ++;
				}

				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief cs_event_nick
 *
 *  report nick change
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_nick( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( msg_nickchange, cmdparams->param, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->name );
	return NS_SUCCESS;
}

/** @brief cs_event_away
 *
 *  report away event
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_away( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( msg_away, cmdparams->source->name, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		IsAway( cmdparams->source ) ? "now" : "no longer", cmdparams->source->user->awaymsg );
	return NS_SUCCESS;
}

/** @brief cs_event_server
 *
 *  report server connect
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_server( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( msg_server, cmdparams->source->name, cmdparams->source->uplink->name );
	return NS_SUCCESS;
}

/** @brief cs_event_squit
 *
 *  report server quit
 *
 *  @cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_squit( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( msg_squit, cmdparams->source->name, cmdparams->source->uplink->name, 
		cmdparams->param ? cmdparams->param : "reason unknown" );
	return NS_SUCCESS;
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

static int cs_set_exclusions_cb( CmdParams *cmdparams, SET_REASON reason )
{
	SetAllEventFlags( EVENT_FLAG_USE_EXCLUDE, cs_cfg.exclusions );
	return NS_SUCCESS;
}

/** @brief cs_set_sign_watch_cb
 *
 *  Set callback for sign watch
 *  Enable or disable events associated with sign on/off
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_sign_watch_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( cs_cfg.sign_watch ) {
		EnableEvent( EVENT_SIGNON );
		EnableEvent( EVENT_QUIT );
	} else {
		DisableEvent( EVENT_SIGNON );
		DisableEvent( EVENT_QUIT );
	}
	return NS_SUCCESS;
}

/** @brief cs_set_kill_watch_cb
 *
 *  Set callback for kill watch
 *  Enable or disable events associated with kills
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_kill_watch_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( cs_cfg.kill_watch ) {
		EnableEvent( EVENT_GLOBALKILL );
		EnableEvent( EVENT_SERVERKILL );
	} else {
		DisableEvent( EVENT_GLOBALKILL );
		DisableEvent( EVENT_SERVERKILL );
	}
	return NS_SUCCESS;
}

/** @brief cs_set_mode_watch_cb
 *
 *  Set callback for mode watch
 *  Enable or disable events associated with modes
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_mode_watch_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( cs_cfg.mode_watch ) {
		EnableEvent( EVENT_UMODE );
		EnableEvent( EVENT_SMODE );
	} else {
		DisableEvent( EVENT_UMODE );
		DisableEvent( EVENT_SMODE );
	}
	return NS_SUCCESS;
}

/** @brief cs_set_nick_watch_cb
 *
 *  Set callback for nick watch
 *  Enable or disable events associated with nick changes
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_nick_watch_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( cs_cfg.nick_watch ) {
		EnableEvent( EVENT_NICK );
	} else {
		DisableEvent( EVENT_NICK );
	}
	return NS_SUCCESS;
}

/** @brief cs_set_away_watch_cb
 *
 *  Set callback for away watch
 *  Enable or disable events associated with away events
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_away_watch_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( cs_cfg.away_watch ) {
		EnableEvent( EVENT_AWAY );
	} else {
		DisableEvent( EVENT_AWAY );
	}
	return NS_SUCCESS;
}

/** @brief cs_set_serv_watch_cb
 *
 *  Set callback for server watch
 *  Enable or disable events associated with server connects/quits
 *
 *  @cmdparams pointer to commands param struct
 *  @cmdparams reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_serv_watch_cb( CmdParams *cmdparams, SET_REASON reason )
{
	if( cs_cfg.serv_watch ) {
		EnableEvent( EVENT_SERVER );
		EnableEvent( EVENT_SQUIT );
	} else {
		DisableEvent( EVENT_SERVER );
		DisableEvent( EVENT_SQUIT );
	}
	return NS_SUCCESS;
}
