/* NeoStats - IRC Statistical Services k
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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

#include <setjmp.h>
#include <stdio.h>
#include "stats.h"
#include "ircd.h"
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif
#include "signal.h"
#include "dl.h"
#include "conf.h"
#include "keeper.h"
#include "log.h"
#include "sock.h"
#include "users.h"
#include "server.h"
#include "chans.h"
#include "dns.h"
#include "transfer.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif


/* this is the name of the services bot */
char s_Services[MAXNICK] = "NeoStats";
/*! Date when we were compiled */
const char version_date[] = __DATE__;
/*! Time we were compiled */
const char version_time[] = __TIME__;

static void start (void);
static void setup_signals (void);
static int get_options (int argc, char **argv);

/*! have we forked */
int forked = 0;
static int attempts = 0;
jmp_buf sigvbuf;

/** @brief Main Entry point into program
 *
 * Sets up defaults, and parses the config file.
 * Also initializes different parts of NeoStats
 * 
 * @return Exits the program!
 *
 * @todo Close STDIN etc correctly
 */
int
main (int argc, char *argv[])
{
	FILE *fp;
	/* get our commandline options */
	if(get_options (argc, argv)!=NS_SUCCESS)
		return EXIT_FAILURE;

	/* Change to the working Directory */
	if (chdir (NEO_PREFIX) < 0) {
		printf ("NeoStats Could not change to %s\n", NEO_PREFIX);
		printf ("Did you 'make install' after compiling?\n");
		printf ("Error Was: %s\n", strerror (errno));
		return EXIT_FAILURE;
	}

	/* before we do anything, make sure logging is setup */
	if(init_logs () != NS_SUCCESS)
		return EXIT_FAILURE;

	/* our crash trace variables */
	SET_SEGV_LOCATION();
	CLEAR_SEGV_INMODULE();
	/* for modules, let them know we are not ready */
	me.onchan = 0;
	/* keep quiet if we are told to :) */
	if (!config.quiet) {
		printf ("NeoStats %d.%d.%d%s Loading...\n", MAJOR, MINOR, REV, ircd_version);
		printf ("-----------------------------------------------\n");
		printf ("Copyright: NeoStats Group. 2000-2003\n");
		printf ("Justin Hammond (fish@neostats.net)\n");
		printf ("Adam Rutter (shmad@neostats.net)\n");
		printf ("Mark (m@neostats.net)\n");
		printf ("-----------------------------------------------\n\n");
	}
	/* set some defaults before we parse the config file */
	me.t_start = time(NULL);
	me.want_privmsg = 0;
	me.die = 0;
	me.local[0] = '\0';
	me.debug_mode = 0;
	me.noticelag = 0;
	me.r_time = 10;
	me.numeric = 1;
	me.lastmsg = me.now;
	me.SendM = me.SendBytes = me.RcveM = me.RcveBytes = 0;
	me.synced = 0;
	me.onchan = 0;
	me.maxsocks = getmaxsock ();
#ifdef ULTIMATE3
	me.client = 0;
#endif
#ifdef SQLSRV
	me.sqlport = 8888;
#endif
	/* if we are doing recv.log, remove the previous version */
	if (config.recvlog)
		remove (RECV_LOG);

		
	/* initilze our Module subsystem */
	if(InitModuleHash () != NS_SUCCESS)
		return EXIT_FAILURE;

	/* prepare to catch errors */
	setup_signals ();

	/* init the sql subsystem if used */
#ifdef SQLSRV
	rta_init(sqlsrvlog);
#endif

	/* load the config files */
	if(ConfLoad () != NS_SUCCESS)
		return EXIT_FAILURE;

	/* init NeoStats bot */
	if(init_services () != NS_SUCCESS)
		return EXIT_FAILURE;

#ifdef EXTAUTH
	/* load extauth if we need to */
	load_module ("extauth", NULL);
	InitExtAuth();
#endif

	if (me.die) {
		printf ("\n-----> ERROR: Read the README file then edit %s! <-----\n\n",CONFIG_NAME);
		nlog (LOG_CRITICAL, LOG_CORE, "Read the README file and edit your %s",CONFIG_NAME);
		/* we are exiting the parent, not the program, so just return */
		return EXIT_FAILURE;
	}

	/* initialize the rest of the subsystems */
	if(init_dns () != NS_SUCCESS)
		return EXIT_FAILURE;
	if(init_server_hash () != NS_SUCCESS)
		return EXIT_FAILURE;
	if(init_user_hash () != NS_SUCCESS)
		return EXIT_FAILURE;
	if(init_chan_hash () != NS_SUCCESS)
		return EXIT_FAILURE;
	/* initilize out transfer subsystem */
	if (init_curl () != NS_SUCCESS)
		return EXIT_FAILURE;
	init_ircd ();


#ifndef DEBUG
	/* if we are compiled with debug, or forground switch was specified, DONT FORK */
	if (!config.foreground) {
		/* fix the double log message problem by closing logs prior to fork() */ 
		close_logs(); 
		forked = fork ();
		/* Error check fork() */ 
		if (forked<0) { 
			perror("fork"); 
			return EXIT_FAILURE; /* fork error */ 
		} 
#endif
		/* we are the parent */ 
		if (forked>0) { 
			/* write out our PID */
			fp = fopen (PID_FILENAME, "w");
			fprintf (fp, "%i", forked);
			fclose (fp);
			if (!config.quiet) {
				printf ("\n");
				printf ("NeoStats %d.%d.%d%s Successfully Launched into Background\n", MAJOR, MINOR, REV, ircd_version);
				printf ("PID: %i - Wrote to %s\n", forked, PID_FILENAME);
			}
			return EXIT_SUCCESS; /* parent exits */ 
		}
#ifndef DEBUG
		/* child (daemon) continues */ 
		/* reopen logs for child */ 
		if(init_logs () != NS_SUCCESS)
			return EXIT_FAILURE;
		/* detach from parent process */
		if (setpgid (0, 0) < 0) {
			nlog (LOG_WARNING, LOG_CORE, "setpgid() failed");
		}
	}
#endif
	nlog (LOG_NOTICE, LOG_CORE, "NeoStats started (NeoStats %d.%d.%d%s).", MAJOR, MINOR, REV, ircd_version);

	/* don't init_modules till after we fork. This fixes the load->fork-exit->call _fini problems when we fork */
	ConfLoadModules ();

	/* we are ready to start now Duh! */
	start ();

	/* We should never reach here but the compiler does not realise and may
	   complain about not all paths control returning values without the return 
	   Since it should never happen, treat as an error condition! */
	return EXIT_FAILURE;
}

/** @brief Process Commandline Options
 *
 * Processes commandline options
 *
 * returns 0 on success, -1 on error
*/
static int
get_options (int argc, char **argv)
{
	int c;
	int dbg;

	/* set some defaults first */
#ifdef DEBUG
	config.debug = 10;
	config.foreground = 1;
#else
	config.debug = 5;
	config.foreground = 0;
#endif

	while ((c = getopt (argc, argv, "hvrd:nqf")) != -1) {
		switch (c) {
		case 'h':
			printf ("NeoStats: Usage: \"neostats [options]\"\n");
			printf ("     -h (Show this screen)\n");
			printf ("	  -v (Show version number)\n");
			printf ("	  -r (Enable recv.log)\n");
			printf ("	  -d 1-10 (Enable debugging output 1= lowest, 10 = highest)\n");
			printf ("	  -n (Do not load any modules on startup)\n");
			printf ("	  -q (Quiet start - for cron scripts)\n");
			printf ("     -f (Do not fork into background\n");
			return NS_FAILURE;
		case 'v':
			printf ("NeoStats Version %d.%d.%d%s\n", MAJOR, MINOR, REV, ircd_version);
			printf ("Compiled: %s at %s\n", version_date, version_time);
			printf ("Flag after version number indicates what IRCd NeoStats is compiled for:\n");
			printf ("(U)  - Unreal IRCd\n");
			printf ("(UL3)- Ultimate 3.x.x IRCd\n");
			printf ("(UL) - Ultimate 2.x.x IRCd (Depreciated)\n");
			printf ("(H)  - Hybrid 7.x IRCd\n");
			printf ("(N)  - NeoIRCd IRCd\n");
			printf ("(M)  - Mystic IRCd\n");
			printf ("(Q)  - Quantum IRCd\n");
			printf ("(B)  - Bahamut IRCd\n");
			printf ("(IRCu) - IRCu (P10) IRCd\n");
			printf ("\nNeoStats: http://www.neostats.net\n");
			return NS_FAILURE;
		case 'r':
			printf ("recv.log enabled. Watch your DiskSpace\n");
			config.recvlog = 1;
			break;
		case 'n':
			config.modnoload = 1;
			break;
		case 'q':
			config.quiet = 1;
			break;
		case 'd':
			dbg = atoi (optarg);
			if ((dbg > 10) || (dbg < 1)) {
				printf ("Invalid Debug Level %d\n", dbg);
				return NS_FAILURE;
			}
			config.debug = dbg;
			break;
		case 'f':
			config.foreground = 1;
			break;
		default:
			printf ("Unknown command line switch %c\n", optopt);
		}
	}
	return NS_SUCCESS;
}



/** @brief Sigterm Signal handler
 *
 * Called by the signal handler if we get a SIGTERM
 * This shutsdown NeoStats and exits
 * 
 * @return Exits the program!
 *
 * @todo Do a nice shutdown, no thtis crap :)
 */
char msg_sigterm[]="SIGTERM received, shutting down server.";

RETSIGTYPE
serv_die ()
{
#ifdef VALGRIND
	exit(NS_SUCCESS);
#else /* VALGRIND */
	User *u;

	u = finduser (s_Services);
	nlog (LOG_CRITICAL, LOG_CORE, msg_sigterm);
	globops (s_Services, msg_sigterm);
	do_exit (NS_EXIT_NORMAL, msg_sigterm);
#endif /* VALGRIND */
}

/** @brief Sighup Signal handler
 *
 * Called by the signal handler if we get a SIGHUP
 * and rehashes the config file.
 * 
 * @return Nothing
 *
 * @todo Implement a Rehash function. What can we actually rehash?
 */
RETSIGTYPE
conf_rehash ()
{
	chanalert (s_Services, "SIGHUP received, attempting to rehash");
	globops (me.name, "SIGHUP received, attempted to rehash");
	/* at the moment, the reshash just checks for a the SQL port is opened, if enabled */
#ifdef SQLSRV
	check_sql_sock();
#endif
}


/** @brief Sigsegv  Signal handler
 *
 * This function is called when we get a SEGV
 * and will send some debug into to the logs and to IRC
 * to help us track down where the problem occured.
 * if the platform we are using supports backtrace
 * also print out the backtrace.
 * if the segv happened inside a module, try to unload the module
 * and continue on our merry way :)
 * 
 * @return Nothing
 *
 */
#ifndef HAVE_BACKTRACE
static char backtrace_unavailable[]="Backtrace not available on this platform";
#endif
static 
void do_backtrace(void)
{
#ifdef HAVE_BACKTRACE
	void *array[50];
	size_t size;
	char **strings;
	int i;

	nlog (LOG_CRITICAL, LOG_CORE, "Backtrace:");
	chanalert (s_Services, "Backtrace: %s", segv_location);
	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);
	for (i = 1; i < size; i++) {
		chanalert (s_Services, "Backtrace(%d): %s", i, strings[i]);
		nlog (LOG_CRITICAL, LOG_CORE, "BackTrace(%d): %s", i - 1, strings[i]);
	}
	free (strings);
#else
	chanalert (s_Services, backtrace_unavailable);
	nlog (LOG_CRITICAL, LOG_CORE, backtrace_unavailable);
#endif
}

RETSIGTYPE
serv_segv ()
{
	char name[MAX_MOD_NAME];
	/** if the segv happened while we were inside a module, unload and try to restore 
	 *  the stack to where we were before we jumped into the module
	 *  and continue on
	 */
	if (segvinmodule[0] != 0) {
		globops (me.name, "Segmentation Fault in %s. Refer to log file for details.", segvinmodule);
		chanalert (s_Services, "Segmentation Fault in %s. Refer to log file for details.", segvinmodule);
		nlog (LOG_CRITICAL, LOG_CORE, "------------------------SEGFAULT REPORT-------------------------");
		nlog (LOG_CRITICAL, LOG_CORE, "Please view the README for how to submit a bug report");
		nlog (LOG_CRITICAL, LOG_CORE, "and include this segfault report in your submission.");
		nlog (LOG_CRITICAL, LOG_CORE, "Module:   %s", segvinmodule);
		nlog (LOG_CRITICAL, LOG_CORE, "Location: %s", segv_location);
		nlog (LOG_CRITICAL, LOG_CORE, "recbuf:   %s", recbuf);
		nlog (LOG_CRITICAL, LOG_CORE, "Unloading Module and restoring stacks. Backtrace:");
		chanalert (s_Services, "Location *could* be %s.", segv_location);
		do_backtrace();
		nlog (LOG_CRITICAL, LOG_CORE, "-------------------------END OF REPORT--------------------------");
		strlcpy (name, segvinmodule, MAX_MOD_NAME);
		CLEAR_SEGV_INMODULE();
		unload_module (name, NULL);
		chanalert (s_Services, "Restoring Stack to before Crash");
		/* flush the logs out */
		close_logs(); 
		longjmp (sigvbuf, -1);
		chanalert (s_Services, "Done");
		return;
	}
	/** The segv happened in our core, damn it */
	/* Thanks to Stskeeps and Unreal for this stuff :) */
	/* Broadcast it out! */
	globops (me.name, "Segmentation Fault. Server Terminating. Refer to log file for details.");
	chanalert (s_Services, "Segmentation Fault. Server Terminating. Refer to log file for details.");
	globops (me.name, "Buffer: %s, Approx Location %s", recbuf, segv_location);
	chanalert (s_Services, "NeoStats (%d%d%d%s) Buffer: %s, Approx Location: %s Backtrace:", MAJOR, MINOR, REV, ircd_version, recbuf, segv_location);
	nlog (LOG_CRITICAL, LOG_CORE, "------------------------SEGFAULT REPORT-------------------------");
	nlog (LOG_CRITICAL, LOG_CORE, "Please view the README for how to submit a bug report");
	nlog (LOG_CRITICAL, LOG_CORE, "and include this segfault report in your submission.");
	nlog (LOG_CRITICAL, LOG_CORE, "Location: %s", segv_location);
	nlog (LOG_CRITICAL, LOG_CORE, "recbuf:   %s", recbuf);
	do_backtrace();
	nlog (LOG_CRITICAL, LOG_CORE, "-------------------------END OF REPORT--------------------------");
	close_logs();
	/* clean up */
	do_exit (NS_EXIT_SEGFAULT, NULL);
}

/** @brief Sets up the signal handlers
 *
 * Sets up the signal handlers for SIGHUP (rehash)
 * SIGTERM (die) and SIGSEGV (segv fault)
 * and ignore the others (Such as SIGPIPE)
 * 
 * @return Nothing
 *
 */
static void
setup_signals (void)
{
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;

	/* SIGPIPE/SIGALRM */
	(void) sigemptyset (&act.sa_mask);
	(void) sigaddset (&act.sa_mask, SIGPIPE);
	(void) sigaddset (&act.sa_mask, SIGALRM);
	(void) sigaction (SIGPIPE, &act, NULL);
	(void) sigaction (SIGALRM, &act, NULL);

	/* SIGHUP */
	act.sa_handler = conf_rehash;
	(void) sigemptyset (&act.sa_mask);
	(void) sigaddset (&act.sa_mask, SIGHUP);
	(void) sigaction (SIGHUP, &act, NULL);

	/* SIGTERM/SIGINT */
	act.sa_handler = serv_die;
	(void) sigaddset (&act.sa_mask, SIGTERM);
	(void) sigaction (SIGTERM, &act, NULL);
	(void) sigaddset (&act.sa_mask, SIGINT);
	(void) sigaction (SIGINT, &act, NULL);

    /* SIGSEGV as well -sts */
	act.sa_handler = serv_segv;
	(void) sigaddset (&act.sa_mask, SIGSEGV);
	(void) sigaction (SIGSEGV, &act, NULL);

	(void) signal (SIGHUP, conf_rehash);
	(void) signal (SIGTERM, serv_die);
	(void) signal (SIGSEGV, serv_segv);
	(void) signal (SIGINT, serv_die);
}

/** @brief Connects to IRC and starts the main loop
 *
 * Connects to the IRC server and attempts to login
 * If it connects and logs in, then starts the main program loop
 * if control is returned to this function, restart
 * 
 * @return Nothing
 *
 * @todo make the restart code nicer so it doesn't go mad when we can't connect
 */
static void
start (void)
{
	SET_SEGV_LOCATION();
	nlog (LOG_NOTICE, LOG_CORE, "Connecting to %s:%d", me.uplink, me.port);

	servsock = ConnectTo (me.uplink, me.port);

	if (servsock <= 0) {
		nlog (LOG_WARNING, LOG_CORE, "Unable to connect to %s", me.uplink);
	} else {
		/* Call the IRC specific function slogin_cmd to login as a server to IRC */
		slogin_cmd (me.name, me.numeric, me.infoline, me.pass);
		sprotocol_cmd ("TOKEN CLIENT");
		read_loop ();
	}

	do_exit (NS_EXIT_RECONNECT, NULL);
}

/** @brief before exiting call this function. It flushes log files and tidy's up.
 *
 *  Cleans up before exiting 
 *  @parm segv 1 = we are exiting because of a segv fault, 0, we are not.
 *  if 1, we don't prompt to save data
 */
void
do_exit (NS_EXIT_TYPE exitcode, char* quitmsg)
{
	/* Initialise exit code to OK */
	int return_code=EXIT_SUCCESS;

	switch (exitcode) {
	case NS_EXIT_NORMAL:
		nlog (LOG_CRITICAL, LOG_CORE, "Normal shut down subsystems");
		break;
	case NS_EXIT_RELOAD:
		nlog (LOG_CRITICAL, LOG_CORE, "Reloading NeoStats");
		break;
	case NS_EXIT_RECONNECT:
		nlog (LOG_CRITICAL, LOG_CORE, "Restarting NeoStats subsystems");
		break;
	case NS_EXIT_ERROR:
		nlog (LOG_CRITICAL, LOG_CORE, "Exiting due to error");
		return_code=EXIT_FAILURE;	/* exit code to error */
		break;		
	case NS_EXIT_SEGFAULT:
		nlog (LOG_CRITICAL, LOG_CORE, "Shutting down subsystems without saving data due to core");
		return_code=EXIT_FAILURE;	/* exit code to error */
		break;
	}

	if (exitcode != NS_EXIT_SEGFAULT) {
		unload_modules();
		if(quitmsg)
		{
			squit_cmd (s_Services, quitmsg);
			ssquit_cmd (me.name);
		}
		sleep(1);
		if (servsock > 0)
			close (servsock);
		if (exitcode == NS_EXIT_RECONNECT) {
			if(me.r_time>0) {
				nlog (LOG_NOTICE, LOG_CORE, "Reconnecting to the server in %d seconds (Attempt %i)", me.r_time, attempts);
				sleep (me.r_time);
			}
			else {
				nlog (LOG_NOTICE, LOG_CORE, "Reconnect time is zero, shutting down");
			}
		}
	}

	kp_flush();
	close_logs ();
	if ((exitcode == NS_EXIT_RECONNECT && me.r_time > 0) || exitcode == NS_EXIT_RELOAD) {
		execve ("./neostats", NULL, NULL);
		return_code=EXIT_FAILURE;	/* exit code to error */
	}
	remove (PID_FILENAME);
	exit (return_code);
}

void fatal_error(char* file, int line, char* func, char* error_text)
{
	nlog (LOG_CRITICAL, LOG_CORE, "Fatal Error: %s %d %s %s", file, line, func, error_text);
	do_exit (NS_EXIT_ERROR, "Fatal Error - check log file");
}
