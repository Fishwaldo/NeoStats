/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include <signal.h>
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif
#include "services.h"
#include "log.h"
#include "modules.h"
#include "sock.h"

/*! File handle for segfault report */
static FILE *segfault;
static char msg_sigterm[] = "SIGTERM received, shutting down server.";

/** @brief SIGTERM/SIGINT handler
 *
 * Called by the signal handler if we get a SIGTERM
 * Shutdown NeoStats and exit
 * 
 * @return Exits the program!
 *
 */

RETSIGTYPE sigterm_handler( int signum )
{
#ifdef VALGRIND
	exit( NS_SUCCESS );
#else /* VALGRIND */
	nlog( LOG_CRITICAL, msg_sigterm );
	irc_globops( NULL, msg_sigterm );
	do_exit( NS_EXIT_NORMAL, msg_sigterm );
#endif /* VALGRIND */
}

/** @brief SIGHUP handler
 *
 * Called by the signal handler if we get a SIGHUP
 * and rehashes the config file.
 * 
 * @return Nothing
 *
 * @todo Implement a Rehash function. What can we actually rehash?
 */
RETSIGTYPE sighup_handler( int signum )
{
	irc_globops( NULL, _( "SIGHUP received, attempted to rehash" ) );
	/* at the moment, the rehash just checks for a the SQL port is opened, if enabled */
#ifdef SQLSRV
	check_sql_sock();
#endif
}

/** @brief SEGV handler
 *
 * This function is called when we get a SEGV and will send some 
 * debug into to logs and to IRC to help us track down where the 
 * problem occured. 
 * If the platform we are using supports backtrace, print out the 
 * backtrace.
 * If the segv happened inside a module, try to unload the module
 * and continue.
 * 
 * @return Nothing
 *
 */
static void do_backtrace( void )
{
#ifdef HAVE_BACKTRACE
	static void *array[50];
	size_t size;
	char **strings;
	int i;

	fprintf( segfault, "Backtrace:\n" );
	size = backtrace( array, 10 );
	strings = backtrace_symbols( array, size );
	for( i = 1; i < size; i++ ) {
		fprintf( segfault, "BackTrace(%d): %s\n", i - 1, strings[i] );
	}
	free( strings );
#else
	fprintf( segfault, "Backtrace not available on this platform\n" );
#endif
}

void report_segfault( const char* modulename )
{
	static char segfault_fmttime[TIMEBUFSIZE];

	segfault = fopen( "segfault.log", "a" );
	if( modulename ) {
		irc_globops( NULL, _( "Segmentation fault in %s. Refer to segfault.log for details." ), GET_CUR_MODNAME() );
		nlog( LOG_CRITICAL, "Segmentation fault in %s. Refer to segfault.log for details.", GET_CUR_MODNAME() );
	} else {
		irc_globops( NULL, _( "Segmentation fault. Server terminating. Refer to segfault.log." ) );
		nlog( LOG_CRITICAL, "Segmentation fault. Server terminating. Refer to segfault.log." );
	}

	me.now = time(NULL);
	ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", me.now);
	strftime (segfault_fmttime, TIMEBUFSIZE, "%d/%m/%Y[%H:%M:%S]", localtime (&me.now));
	fprintf( segfault, "------------------------SEGFAULT REPORT-------------------------\n" );
	fprintf( segfault, "Please view the README for how to submit a bug report\n" );
	fprintf( segfault, "and include this segfault report in your submission.\n" );
	fprintf (segfault, "(%s)\n", segfault_fmttime);
	fprintf( segfault, "Version:  %s\n", me.version );
	if( modulename ) {
		fprintf( segfault, "Module:   %s\n", GET_CUR_MODNAME() );
	}
	fprintf( segfault, "Location: %s\n", segv_location );
	fprintf( segfault, "recbuf:   %s\n", recbuf );
	do_backtrace();
	fprintf( segfault, "-------------------------END OF REPORT--------------------------\n" );
	fflush( segfault );
	fclose( segfault );		
}

RETSIGTYPE sigsegv_handler( int signum )
{
	static char name[MAX_MOD_NAME];
	/** segv happened inside a module, so unload and try to restore the stack 
	 *  to location before we jumped into the module and continue
	 */
	if( RunLevel > 0 ) {
		report_segfault( GET_CUR_MODNAME() );
		strlcpy( name, GET_CUR_MODNAME(), MAX_MOD_NAME );
		RunLevel = 0;
		unload_module( name, NULL );
		irc_globops( ns_botptr, "Restoring Stack to before Crash" );
		/* flush the logs */
		CloseLogs(); 
		longjmp( sigvbuf, -1 );
		irc_globops( ns_botptr, "Done" );
		return;
	}
	/* segv happened in our core */
	report_segfault( NULL );
	CloseLogs();
	do_exit( NS_EXIT_SEGFAULT, NULL );
}

/** @brief Set up signal handlers
 *
 * Set up signal handlers for SIGHUP, SIGTERM and SIGSEGV
 * Ignore others such as SIGPIPE
 * 
 * @return Nothing
 *
 */
void
InitSignals( void )
{
#ifndef WIN32
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;

	/* SIGPIPE/SIGALRM */
	( void ) sigemptyset( &act.sa_mask );
	( void ) sigaddset( &act.sa_mask, SIGPIPE );
	( void ) sigaddset( &act.sa_mask, SIGALRM );
	( void ) sigaction( SIGPIPE, &act, NULL );
	( void ) sigaction( SIGALRM, &act, NULL );

	/* SIGHUP */
	act.sa_handler = sighup_handler;
	( void ) sigemptyset( &act.sa_mask );
	( void ) sigaddset( &act.sa_mask, SIGHUP );
	( void ) sigaction( SIGHUP, &act, NULL );

	/* SIGTERM/SIGINT */
	act.sa_handler = sigterm_handler;
	( void ) sigaddset( &act.sa_mask, SIGTERM );
	( void ) sigaction( SIGTERM, &act, NULL );
	( void ) sigaddset( &act.sa_mask, SIGINT );
	( void ) sigaction( SIGINT, &act, NULL );

    /* SIGSEGV */
	act.sa_handler = sigsegv_handler;
	( void ) sigaddset( &act.sa_mask, SIGSEGV );
	( void ) sigaction( SIGSEGV, &act, NULL );

	( void ) signal( SIGHUP, sighup_handler );
#endif
	( void ) signal( SIGTERM, sigterm_handler );
	( void ) signal( SIGSEGV, sigsegv_handler );
	( void ) signal( SIGINT, sigterm_handler );
}
