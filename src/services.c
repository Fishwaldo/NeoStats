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
#include "main.h"
#include "modules.h"
#include "nsevents.h"
#include "bots.h"
#include "timer.h"
#include "commands.h"
#include "sock.h"
#include "helpstrings.h"
#include "users.h"
#include "servers.h"
#include "channels.h"
#include "ircprotocol.h"
#include "exclude.h"
#include "services.h"
#include "bans.h"
#include "auth.h"

/* Command function prototypes */
static int ns_cmd_shutdown( CmdParams *cmdparams );
static int ns_cmd_reload( CmdParams *cmdparams );
static int ns_cmd_jupe( CmdParams *cmdparams );
#ifdef USE_RAW
static int ns_cmd_raw( CmdParams *cmdparams );
#endif
static int ns_cmd_status( CmdParams *cmdparams );

static int services_event_ctcpversion( CmdParams *cmdparams );

config nsconfig;
tme me;

static char quitmsg[BUFSIZE];

static const char *ns_about[] = {
	"\2NeoStats\2 statistical services",
	NULL
};

/** Copyright info */
static const char *ns_copyright[] = {
	"Copyright (c) 1999-2005, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo ns_module_info = {
	"NeoStats",
	"NeoStats Statistical services", 	
	ns_copyright,
	ns_about,
	NEOSTATS_VERSION,
	NEOSTATS_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

/** Fake Module pointer for run level code */
Module ns_module = {
	MOD_TYPE_STANDARD,
	&ns_module_info
};

/** Bot comand table */
static bot_cmd ns_commands[] =
{
	{"LEVEL",		ns_cmd_level,		0, 	0,					ns_help_level},
	{"STATUS",		ns_cmd_status,		0, 	0,					ns_help_status},
	{"SHUTDOWN",	ns_cmd_shutdown,	1, 	NS_ULEVEL_ADMIN, 	ns_help_shutdown},
#ifndef WIN32
	{"RELOAD",		ns_cmd_reload,		1, 	NS_ULEVEL_ADMIN, 	ns_help_reload},
#endif /* !WIN32 */
	{"MODLIST",		ns_cmd_modlist,		0, 	NS_ULEVEL_ADMIN,  	ns_help_modlist},
	{"LOAD",		ns_cmd_load,		1, 	NS_ULEVEL_ADMIN, 	ns_help_load},
	{"UNLOAD",		ns_cmd_unload,		1,	NS_ULEVEL_ADMIN, 	ns_help_unload},
	{"JUPE",		ns_cmd_jupe,		1, 	NS_ULEVEL_ADMIN, 	ns_help_jupe},
	{"EXCLUDE",		ns_cmd_exclude,		1,	NS_ULEVEL_ADMIN,	ns_help_exclude},
#ifdef USE_RAW																
	{"RAW",			ns_cmd_raw,			0, 	NS_ULEVEL_ADMIN, 	ns_help_raw},
#endif																	
	{"BOTLIST",		ns_cmd_botlist,		0, 	NS_ULEVEL_ROOT,  	ns_help_botlist},
	{"SOCKLIST",	ns_cmd_socklist,	0, 	NS_ULEVEL_ROOT,  	ns_help_socklist},
	{"TIMERLIST",	ns_cmd_timerlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_timerlist},
	{"USERLIST",	ns_cmd_userlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_userlist},
	{"CHANNELLIST",	ns_cmd_channellist,	0, 	NS_ULEVEL_ROOT,  	ns_help_channellist},
	{"SERVERLIST",	ns_cmd_serverlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_serverlist},
	{"BANLIST",		ns_cmd_banlist,		0, 	NS_ULEVEL_ROOT,  	ns_help_banlist},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting ns_settings[] =
{
	{"MSGSAMPLETIME",	&nsconfig.msgsampletime,	SET_TYPE_INT,		1,	100,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_msgsampletime, NULL,( void * )10 },
	{"MSGTHRESHOLD",	&nsconfig.msgthreshold,		SET_TYPE_INT,		1,	100,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_msgthreshold, NULL,( void * )5 },
	{"SPLITTIME",		&nsconfig.splittime,		SET_TYPE_INT,		0,	1000,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_splittime, NULL,( void * )300 },
	{"JOINSERVICESCHAN",&nsconfig.joinserviceschan, SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_joinserviceschan, NULL,( void* )1 },
	{"PINGTIME",		&nsconfig.pingtime,			SET_TYPE_INT,		0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_pingtime, NULL,( void* )120 },
	{"VERSIONSCAN",		&me.versionscan,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_versionscan, NULL,( void* )1 },
	{"SERVICECMODE",	me.servicescmode,			SET_TYPE_STRING,	0, MODESIZE, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_servicecmode, NULL, NULL },
	{"SERVICEUMODE",	me.servicesumode,			SET_TYPE_STRING,	0, MODESIZE, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_serviceumode, NULL, NULL },
	{"CMDCHAR",			nsconfig.cmdchar,			SET_TYPE_STRING,	0, 2, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_cmdchar, NULL,( void* )"!" },
	{"CMDREPORT",		&nsconfig.cmdreport,		SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_cmdreport, NULL,( void* )1 },
	{"LOGLEVEL",		&nsconfig.loglevel,			SET_TYPE_INT,		1, 6, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_loglevel, NULL,( void* )5 },
	{"RECVQ",			&nsconfig.recvq,			SET_TYPE_INT,		1024,10240000,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_recvq, NULL, ( void*)2048}, 
	{"DEBUGCHAN",		nsconfig.debugchan,			SET_TYPE_STRING,	0, MAXCHANLEN, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_debugchan, NULL,( void* )"#debug" },
	NS_SETTING_END()
};

/** Bot debug setting table */
static bot_setting ns_debugsettings[] =
{
#ifndef DEBUG
	{"DEBUG",			&nsconfig.debug,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_debug, NULL,( void* )0 },
#endif
	{"DEBUGMODULE",		nsconfig.debugmodule,		SET_TYPE_STRING,	0, MAX_MOD_NAME,NS_ULEVEL_ADMIN, NULL,	ns_help_set_debugmodule, NULL,( void* )"all" },
	{"DEBUGLEVEL",		&nsconfig.debuglevel,		SET_TYPE_INT,		1, 10, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_debuglevel, NULL,( void* )0 },
	{"DEBUGTOCHAN",		&nsconfig.debugtochan,		SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_debugtochan, NULL,( void* )0 },
	NS_SETTING_END()
};

/** Bot pointer */
Bot* ns_botptr = NULL;

/** Core bot info */
BotInfo ns_botinfo =
{
	"NeoStats",
	"NeoStats1",
	"Neo",
	BOT_COMMON_HOST,
	"/msg NeoStats \2HELP\2",
	/* 0x80000000 is a "hidden" flag to identify the core bot */
	0x80000000|BOT_FLAG_ONLY_OPERS|BOT_FLAG_SERVICEBOT|BOT_FLAG_DEAF,
	ns_commands, 
	ns_settings,
};

/** Core event table */
ModuleEvent neostats_events[] =
{
	{EVENT_CTCPVERSIONRPL,	services_event_ctcpversion,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NULL,		NULL}
};

/** @brief services_event_ctcpversion
 *
 *  NeoStats CTCP VERSION reply event handler
 *
 *  @param cmdparams structure with command information
 *
 *  @return none
 */

static int services_event_ctcpversion( CmdParams *cmdparams )
{
	dlog(DEBUG1, "Got Version reply event in services.c from %s: %s", cmdparams->source->name, cmdparams->param);
	strlcpy( cmdparams->source->version, cmdparams->param, MAXHOST );
	SendAllModuleEvent( EVENT_CTCPVERSIONRPLBC, cmdparams );
	return NS_SUCCESS;
}

/** @brief InitServices
 *
 *  init NeoStats core
 *
 *  @param none
 *
 *  @return none
 */

void InitServices( void )
{
	ModuleConfig( ns_settings );
}

/** @brief FiniServices
 *
 *  fini NeoStats core
 *
 *  @param none
 *
 *  @return none
 */

void FiniServices( void )
{
	FreeEventList( &ns_module );
}

/** @brief init_services_bot
 *
 *  init NeoStats bot
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int init_services_bot( void )
{
	SET_SEGV_LOCATION();
	strlcpy( ns_botinfo.nick, me.rootnick, MAXNICK );
	ircsnprintf( ns_botinfo.altnick, MAXNICK, "%s1", me.rootnick );
	ircsnprintf( ns_botinfo.realname, MAXREALNAME, "/msg %s \2HELP\2", ns_botinfo.nick );
	if( nsconfig.onlyopers ) 
		ns_botinfo.flags |= BOT_FLAG_ONLY_OPERS;
	SetModuleInSynch( &ns_module );
	ns_botptr = AddBot( &ns_botinfo );
	add_services_set_list( ns_debugsettings );
	AddEventList( neostats_events );
	SetModuleSynched( &ns_module );
	me.synched = 1;
	SynchAllModules();
	RequestServerUptimes();	
	return NS_SUCCESS;
}

/** @brief ns_cmd_shutdown
 *
 *  SHUTDOWN command handler
 *  Shutdown NeoStats
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_shutdown( CmdParams *cmdparams )
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf( cmdparams->av, cmdparams->ac, 0 );
	irc_chanalert( ns_botptr, _( "%s requested SHUTDOWN for %s" ), cmdparams->source->name, message );
	ircsnprintf( quitmsg, BUFSIZE, _( "%s [%s] (%s) requested SHUTDOWN for %s." ), 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, message );
	ns_free( message );
	irc_globops( ns_botptr, "%s", quitmsg );
	nlog( LOG_NOTICE, "%s", quitmsg );
	do_exit( NS_EXIT_NORMAL, quitmsg );
   	return NS_SUCCESS;
}

/** @brief ns_cmd_reload
 *
 *  RELOAD command handler
 *  Reload NeoStats
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_reload( CmdParams *cmdparams )
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf( cmdparams->av, cmdparams->ac, 0 );
	irc_chanalert( ns_botptr, _( "%s requested RELOAD for %s" ), cmdparams->source->name, message );
	ircsnprintf( quitmsg, BUFSIZE, _( "%s [%s] (%s) requested RELOAD for %s." ), 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, message );
	ns_free( message );
	irc_globops( ns_botptr, "%s", quitmsg );
	nlog( LOG_NOTICE, "%s", quitmsg );
	do_exit( NS_EXIT_RELOAD, quitmsg );
   	return NS_SUCCESS;
}

/** @brief ns_cmd_jupe
 *
 *  JUPE command handler
 *  Jupiter a server
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_jupe( CmdParams *cmdparams )
{
	static char infoline[255];

	SET_SEGV_LOCATION();
	ircsnprintf( infoline, 255, "[jupitered by %s]", cmdparams->source->name );
	irc_server( cmdparams->av[0], 1, infoline );
	nlog( LOG_NOTICE, "%s!%s@%s jupitered %s", cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, cmdparams->av[0] );
	irc_chanalert( ns_botptr, _( "%s jupitered %s" ), cmdparams->source->name, cmdparams->av[0] );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "%s has been jupitered", cmdparams->source ), cmdparams->av[0] );
   	return NS_SUCCESS;
}

/** @brief ns_cmd_status
 *
 *  STATUS command handler
 *  Display NeoStats status
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_status( CmdParams *cmdparams )
{
	time_t uptime = me.now - me.ts_boot;

	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "%s status:", cmdparams->source ), ns_botptr->name );
	if( uptime > TS_ONE_DAY )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%ld\2 day%s, \2%02ld:%02ld\2", cmdparams->source ), ns_botptr->name, uptime / TS_ONE_DAY,( uptime / TS_ONE_DAY == 1 ) ? "" : "s",( uptime / TS_ONE_HOUR ) % 24,( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE );
	else if( uptime > TS_ONE_HOUR )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%ld hour%s, %ld minute%s\2", cmdparams->source ), ns_botptr->name, uptime / TS_ONE_HOUR, uptime / TS_ONE_HOUR == 1 ? "" : "s",( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE,( uptime / 60 ) % TS_ONE_MINUTE == 1 ? "" : "s" );
	else if( uptime > TS_ONE_MINUTE )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%ld minute%s, %ld second%s\2", cmdparams->source ), ns_botptr->name, uptime / TS_ONE_MINUTE, uptime / TS_ONE_MINUTE == 1 ? "" : "s", uptime % TS_ONE_MINUTE, uptime % TS_ONE_MINUTE == 1 ? "" : "s" );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%d second%s\2", cmdparams->source ), ns_botptr->name,( int )uptime, uptime == 1 ? "" : "s" );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Sent %ld messages, %ld bytes", cmdparams->source ), me.SendM, me.SendBytes );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Received %ld messages, %ld bytes", cmdparams->source ), me.RcveM, me.RcveBytes );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Reconnect time: %d", cmdparams->source ), nsconfig.r_time );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Requests: %d",cmdparams->source ), me.requests );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Max sockets: %d( in use: %d )", cmdparams->source ), me.maxsocks, me.cursocks );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Current servers: %d", cmdparams->source ), NSGetServerCount() );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Current channels: %d", cmdparams->source ), NSGetChannelCount() );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Current users: %d( Away: %d )", cmdparams->source ), NSGetUserCount(), NSGetAwayCount() );
	if( nsconfig.debug )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Debugging mode enabled", cmdparams->source ) );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Debugging mode disabled", cmdparams->source ) );
	return NS_SUCCESS;
}

#ifdef USE_RAW
/** @brief ns_cmd_raw
 *
 *  RAW command handler
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_raw( CmdParams *cmdparams )
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf( cmdparams->av, cmdparams->ac, 1 );
	irc_chanalert( ns_botptr, _( "\2RAW COMMAND\2 \2%s\2 issued a raw command! (%s)" ), cmdparams->source->name, message );
	nlog( LOG_NORMAL, "RAW COMMAND %s issued a raw command! (%s)", cmdparams->source->name, message );
	send_cmd( "%s", message );
	ns_free( message );
   	return NS_SUCCESS;
}
#endif
