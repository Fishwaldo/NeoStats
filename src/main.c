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
#include "ircd.h"
#include "conf.h"
#include "log.h"
#include "sock.h"
#include "users.h"
#include "servers.h"
#include "channels.h"
#include "dns.h"
#include "dotconf.h"
#include "transfer.h"
#include "exclude.h"
#include "bans.h"
#include "services.h"
#include "modules.h"
#include "auth.h"
#include "bots.h"
#include "timer.h"
#include "signals.h"
#include "lang.h"
#include "nsdba.h"
#include "rtaserv.h"
#include "dcc.h"

#define PID_FILENAME	"neostats.pid"

#ifdef WIN32
void *( *old_malloc )( size_t );
void( *old_free )( void * );
#endif /* WIN32 */

char segv_location[SEGV_LOCATION_BUFSIZE];

/*! Date we were compiled */
const char version_date[] = __DATE__;
/*! Time we were compiled */
const char version_time[] = __TIME__;

/*! have we forked */
static int forked = 0;
static int attempts = 0;
jmp_buf sigvbuf;

/** @brief get_options
 *
 *  Processes command line options
 *  NeoStats core use only.
 *
 *  @param argc count of command line parameters
 *  @param argv array of command line parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

#ifndef WIN32
static int get_options( int argc, char **argv )
{
	int c;
	int level;

	while( ( c = getopt( argc, argv, "hvrd:nqf" ) ) != -1 ) {
		switch( c ) {
		case 'h':
			printf( "NeoStats: Usage: \"neostats [options]\"\n" );
			printf( "     -h (Show this screen)\n" );
			printf( "	  -v (Show version number)\n" );
			printf( "	  -d 1-10 (Debug log output level 1 = lowest, 10 = highest)\n" );
			printf( "	  -n (Do not load any modules on startup)\n" );
			printf( "	  -q (Quiet start - for cron scripts)\n" );
			printf( "     -f (Do not fork into background)\n" );
			return NS_FAILURE;
		case 'v':
			printf( "NeoStats: http://www.neostats.net\n" );
			printf( "Version:  %s\n", me.version );
			printf( "Compiled: %s at %s\n", ns_module_info.build_date, ns_module_info.build_time );
			return NS_FAILURE;
		case 'd':
			level = atoi( optarg );
			if( ( level >= DEBUGMAX ) ||( level < 1 ) ) {
				printf( "Invalid debug level %d\n", level );
				return NS_FAILURE;
			}
			printf( "debug.log enabled at level %d. Watch your disk space\n", level );
			nsconfig.debuglevel = level;
			break;
		case 'n':
			nsconfig.modnoload = 1;
			break;
		case 'q':
			nsconfig.quiet = 1;
			break;
		case 'f':
			nsconfig.foreground = 1;
			break;
		default:
			printf( "Unknown command line switch %c\n", optopt );
		}
	}
	return NS_SUCCESS;
}
#endif /* !WIN32 */

/** @brief InitMe
 *
 *  init me structure
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitMe( void )
{
	/* set some defaults before we parse the config file */
	memset( &me, 0, sizeof( me ) );
	/* initialise version */
	strlcpy( me.version, NEOSTATS_VERSION, VERSIONSIZE );
	/* our default lang is always -1 */
	me.lang = -1;
	me.numeric = 1;
	me.now = me.ts_boot = time( NULL );
	ircsnprintf( me.strnow, STR_TIME_T_SIZE, "%lu", ( long )me.now );
	/* Clear config */
	memset( &nsconfig, 0 , sizeof( config ) );
	/* Default reconnect time */
	nsconfig.r_time = 10;
	/* Debug mode overrides */
#ifdef DEBUG
	nsconfig.debug = 1;
	nsconfig.loglevel = LOG_INFO;
	nsconfig.debuglevel = DEBUG10;
	nsconfig.foreground = 1;
#endif /* DEBUG */
	/* default debugmodule to all */
	strlcpy( nsconfig.debugmodule, "all", MAX_MOD_NAME );
#ifdef WIN32
	nsconfig.loglevel = LOG_NORMAL;
#endif /* WIN32 */
	return NS_SUCCESS;
}

/** @brief InitCore
 *
 *  init core sub systems
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitCore( void )
{
	char dbpath[MAXPATH];
	/* prepare to catch errors */
	InitSignals();
	/* load the config files */
	if( ConfLoad() != NS_SUCCESS )
		return NS_FAILURE;
	/* initialize Lang Subsystem */
	ircsnprintf( dbpath, MAXPATH, "%s/data/lang.db", NEO_PREFIX );
	LANGinit( 1, dbpath, NULL );
	/* initialize Module subsystem */
	if( InitDBA() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitModules() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitAuth() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitTimers() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitBots() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitSocks() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitDns() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitExcludes() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitServers() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitUsers() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitChannels() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitBans() != NS_SUCCESS )
		return NS_FAILURE;	
	if( InitCurl() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitIrcd() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitDCC() != NS_SUCCESS )
		return NS_FAILURE;
	InitServices();
	dlog( DEBUG1, "Core init successful" );
	return NS_SUCCESS;
}

/** @brief FiniCore
 *
 *  cleanup core sub systems
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniCore( void )
{
	FiniDCC();
	FiniCurl();
	FiniUsers();
	FiniChannels();
	FiniExcludes();
	FiniServers();
	FiniBans();
	FiniDns();
	FiniModules();
	FiniSocks();
	FiniBots();
	FiniTimers();
}

/** @brief InitCore
 *
 *  print copyright notice
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

static void print_copyright( void )
{
	printf( "NeoStats %s Loading...\n", me.version );
	printf( "-----------------------------------------------\n" );
	printf( "Copyright: NeoStats Group. 2000-2004\n" );
	printf( "Justin Hammond( fish@neostats.net )\n" );
	printf( "Adam Rutter( shmad@neostats.net )\n" );
	printf( "Mark Hetherington( m@neostats.net )\n" );
	printf( "-----------------------------------------------\n\n" );
}

/** @brief main
 *
 *  Program entry point
 *  NeoStats core use only.
 *  Under Win32 this is used purely as a startup function and not 
 *  the main entry point
 *
 *  @param argc count of command line parameters
 *  @param argv array of command line parameters
 *
 *  @return EXIT_SUCCESS if succeeds, EXIT_FAILURE if not 
 *
 *  @todo Close STDIN etc correctly
 */

#ifdef WIN32
int neostats( void )
#else /* WIN32 */
int main( int argc, char *argv[] )
#endif /* WIN32 */
{
#ifndef WIN32
	FILE *fp;
#endif /* WIN32 */

	if( InitMe() != NS_SUCCESS )
		return EXIT_FAILURE;
#ifndef WIN32
	/* get our commandline options */
	if( get_options( argc, argv ) != NS_SUCCESS )
		return EXIT_FAILURE;
#endif /* !WIN32 */
#if 0
	/* Change to the working Directory */
	if( chdir( NEO_PREFIX ) < 0 ) {
		printf( "NeoStats Could not change to %s\n", NEO_PREFIX );
		printf( "Did you 'make install' after compiling?\n" );
		printf( "Error Was: %s\n", strerror( errno ) );
		return EXIT_FAILURE;
	}
#endif
	/* Init run level to NeoStats core */
	RunModule[0] = &ns_module;
	/* before we do anything, make sure logging is setup */
	if( InitLogs() != NS_SUCCESS )
		return EXIT_FAILURE;
	/* our crash trace variables */
	SET_SEGV_LOCATION();
	/* keep quiet if we are told to : ) */
	if( !nsconfig.quiet ) 
		print_copyright();
	/* Init NeoStats sub systems */
	if( InitCore() != NS_SUCCESS )
		return EXIT_FAILURE;

#ifndef WIN32
#ifndef DEBUG
	/* if we are compiled with debug, or forground switch was specified, DONT FORK */
	if( !nsconfig.foreground ) {
		/* fix the double log message problem by closing logs prior to fork() */ 
		CloseLogs(); 
		forked = fork();
		/* Error check fork() */ 
		if( forked < 0 ) { 
			perror( "fork" ); 
			return EXIT_FAILURE; /* fork error */ 
		} 
#endif  /* !DEBUG */
		/* we are the parent */ 
		if( forked > 0 ) { 
			/* write out our PID */
			fp = fopen( PID_FILENAME, "w" );
			fprintf( fp, "%i", forked );
			fclose( fp );
			if( !nsconfig.quiet ) {
				printf( "\n" );
				printf( _( "NeoStats %s Successfully Launched into Background\n" ), me.version );
				printf( _( "PID: %i - Wrote to %s\n" ), forked, PID_FILENAME );
			}
			return EXIT_SUCCESS; /* parent exits */ 
		}
#ifndef DEBUG
		/* child (daemon) continues */ 
		/* reopen logs for child */ 
		if( InitLogs() != NS_SUCCESS )
			return EXIT_FAILURE;
		/* detach from parent process */
		if( setpgid( 0, 0 ) < 0 ) {
			nlog( LOG_WARNING, "setpgid() failed" );
		}
	}
#endif  /* !DEBUG */
#endif /* !WIN32 */
	nlog( LOG_NOTICE, "NeoStats \"%s\" started.", VERSION );
#ifdef WIN32
	/* override pcre lib malloc calls with our own version */
	old_malloc = pcre_malloc;
	old_free = pcre_free;
	pcre_malloc = os_malloc;
	pcre_free = os_free;
#endif /* WIN32 */
	/* Load modules after we fork. This fixes the load->fork-exit->call 
	   _fini problems when we fork */
	ConfLoadModules();
	/* Connect to server */
	Connect();
#ifdef WIN32
	return 0;
#else /* WIN32 */
	/* We should never reach here but the compiler does not realise and may
	   complain about not all paths control returning values without the return 
	   Since it should never happen, treat as an error condition! */
	return EXIT_FAILURE;
#endif /* WIN32 */
}

/** @brief do_exit
 *
 *  Exit routine. Cleans up systems and flushes data files
 *  then exits cleanly. During a segfaul data is not saved.
 *  NeoStats core use only.
 *
 *  @param exitcode reason for exit
 *  @param quitmsg optional quit message to send over IRC
 *
 *  @return none
 */

void do_exit( NS_EXIT_TYPE exitcode, char *quitmsg )
{
	/* Initialise exit code to OK */
	int return_code=EXIT_SUCCESS;

	switch( exitcode ) {
		case NS_EXIT_NORMAL:
			nlog( LOG_CRITICAL, "Normal shut down subsystems" );
			break;
		case NS_EXIT_RELOAD:
			nlog( LOG_CRITICAL, "Reloading NeoStats" );
			break;
		case NS_EXIT_RECONNECT:
			nlog( LOG_CRITICAL, "Restarting NeoStats subsystems" );
			break;
		case NS_EXIT_ERROR:
			nlog( LOG_CRITICAL, "Exiting due to error" );
			return_code=EXIT_FAILURE;	/* exit code to error */
			break;		
		case NS_EXIT_SEGFAULT:
			nlog( LOG_CRITICAL, "Shutting down subsystems without saving data due to core" );
			return_code=EXIT_FAILURE;	/* exit code to error */
			break;
	}

	if( exitcode != NS_EXIT_SEGFAULT ) {
		rtaserv_fini();
		unload_modules();
		DBACloseDatabase();
		if( quitmsg )
		{
			irc_quit( ns_botptr, quitmsg );
			irc_squit( me.name, quitmsg );
		}
		sleep( 1 );
		/* cleanup up core subsystems */
		FiniCore();
		if( exitcode == NS_EXIT_RECONNECT ) {
			if( nsconfig.r_time > 0 ) {
				nlog( LOG_NOTICE, "Reconnecting to the server in %d seconds (Attempt %i)", nsconfig.r_time, attempts );
				sleep( nsconfig.r_time );
			}
			else {
				nlog( LOG_NOTICE, "Reconnect time is zero, shutting down" );
			}
		}
	}
	FiniDBA();
	FiniLogs();
	LANGfini();
#ifdef WIN32
	/* restore pcre lib malloc pointers */
	pcre_malloc = old_malloc;
	pcre_free = old_free;
#endif /* WIN32 */
	if( ( exitcode == NS_EXIT_RECONNECT && nsconfig.r_time > 0 ) || exitcode == NS_EXIT_RELOAD ) {
		execve( "./neostats", NULL, NULL );
		return_code = EXIT_FAILURE;	/* exit code to error */
	}
	remove( PID_FILENAME );
	exit( return_code );
}

/** @brief fatal_error
 *
 *  fatal error messsage handler
 *  NeoStats core use only.
 *
 *  @param file name of calling file
 *  @param line line of calling code
 *  @param func name of calling function
 *  @param error_text text of error
 *
 *  @return none
 */

void fatal_error( char *file, int line, char *func, char *error_text )
{
	nlog( LOG_CRITICAL, "Fatal Error: %s %d %s %s", file, line, func, error_text );
	do_exit( NS_EXIT_ERROR, "Fatal Error - check log file" );
}
