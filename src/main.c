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
#include "ircd.h"
#include "conf.h"
#include "keeper.h"
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
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

char segv_location[SEGV_LOCATION_BUFSIZE];

/*! Date when we were compiled */
const char version_date[] = __DATE__;
/*! Time we were compiled */
const char version_time[] = __TIME__;

static int get_options (int argc, char **argv);

/*! have we forked */
static int forked = 0;
static int attempts = 0;
jmp_buf sigvbuf;

/** @brief init me structure
 *
 * @return 
 */
void InitMe(void)
{
	/* set some defaults before we parse the config file */
	memset(&me, 0, sizeof(me));
	me.t_start = time(NULL);
	me.now = time(NULL);
	ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
#ifdef DEBUG
	config.debug = 1;
#endif
	config.r_time = 10;
	me.numeric = 1;
#ifdef SQLSRV
	me.sqlport = 8888;
#endif
}

/** @brief init core sub systems
 *
 * @return 
 */
static int InitCore(void)
{
	InitMe();
	/* if we are doing recv.log, remove the previous version */
	if (config.recvlog)
		remove (RECV_LOG);
	/* prepare to catch errors */
	InitSignals ();
	/* load the config files */
	if (ConfLoad () != NS_SUCCESS)
		return NS_FAILURE;
	/* init the sql subsystem if used */
#ifdef SQLSRV
	rta_init(sqlsrvlog);
#endif	
	/* initialize Module subsystem */
	if (InitModules () != NS_SUCCESS)
		return NS_FAILURE;
	if (InitAuth() != NS_SUCCESS)
		return NS_FAILURE;
	if (InitTimers() != NS_SUCCESS)
		return NS_FAILURE;
	if (InitBots() != NS_SUCCESS)
		return NS_FAILURE;
	if (InitSocks() != NS_SUCCESS)
		return NS_FAILURE;
	if (InitDns () != NS_SUCCESS)
		return NS_FAILURE;
	if (InitExcludes() != NS_SUCCESS)
		return NS_FAILURE;
	if (InitServers () != NS_SUCCESS)
		return NS_FAILURE;
	if (InitUsers () != NS_SUCCESS)
		return NS_FAILURE;
	if (InitChannels () != NS_SUCCESS)
		return NS_FAILURE;
	if (InitBans () != NS_SUCCESS)
		return NS_FAILURE;	
	if (InitCurl () != NS_SUCCESS)
		return NS_FAILURE;
	InitIrcd ();
	dlog(DEBUG1, "Core init successful");
	return NS_SUCCESS;
}

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

	/* initialise version */
	strlcpy(me.version, NEOSTATS_VERSION, VERSIONSIZE);
	/* get our commandline options */
	if(get_options (argc, argv)!=NS_SUCCESS)
		return EXIT_FAILURE;
#if 0
	/* Change to the working Directory */
	if (chdir (NEO_PREFIX) < 0) {
		printf ("NeoStats Could not change to %s\n", NEO_PREFIX);
		printf ("Did you 'make install' after compiling?\n");
		printf ("Error Was: %s\n", strerror (errno));
		return EXIT_FAILURE;
	}
#endif
	/* Init run level to NeoStats core */
	RunModule[0]=&ns_module;
	/* before we do anything, make sure logging is setup */
	if(InitLogs () != NS_SUCCESS)
		return EXIT_FAILURE;
	/* our crash trace variables */
	SET_SEGV_LOCATION();
	/* keep quiet if we are told to :) */
	if (!config.quiet) {
		printf ("NeoStats %s Loading...\n", me.version);
		printf ("-----------------------------------------------\n");
		printf ("Copyright: NeoStats Group. 2000-2004\n");
		printf ("Justin Hammond (fish@neostats.net)\n");
		printf ("Adam Rutter (shmad@neostats.net)\n");
		printf ("Mark Hetherington (m@neostats.net)\n");
		printf ("-----------------------------------------------\n\n");
	}
	/* Init NeoStats sub systems */
	if (InitCore () != NS_SUCCESS)
		return EXIT_FAILURE;

#ifndef DEBUG
	/* if we are compiled with debug, or forground switch was specified, DONT FORK */
	if (!config.foreground) {
		/* fix the double log message problem by closing logs prior to fork() */ 
		CloseLogs (); 
		forked = fork ();
		/* Error check fork() */ 
		if (forked < 0) { 
			perror("fork"); 
			return EXIT_FAILURE; /* fork error */ 
		} 
#endif
		/* we are the parent */ 
		if (forked > 0) { 
			/* write out our PID */
			fp = fopen (PID_FILENAME, "w");
			fprintf (fp, "%i", forked);
			fclose (fp);
			if (!config.quiet) {
				printf ("\n");
				printf ("NeoStats %s Successfully Launched into Background\n", me.version);
				printf ("PID: %i - Wrote to %s\n", forked, PID_FILENAME);
			}
			return EXIT_SUCCESS; /* parent exits */ 
		}
#ifndef DEBUG
		/* child (daemon) continues */ 
		/* reopen logs for child */ 
		if(InitLogs () != NS_SUCCESS)
			return EXIT_FAILURE;
		/* detach from parent process */
		if (setpgid (0, 0) < 0) {
			nlog (LOG_WARNING, "setpgid() failed");
		}
	}
#endif
	nlog (LOG_NOTICE, "NeoStats \"%s\" started.", NEOSTATS_VERSION);

	/* Load modules after we fork. This fixes the load->fork-exit->call 
	   _fini problems when we fork */
	ConfLoadModules ();
	/* Connect to server */
	Connect ();
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
	int level;

	/* Clear config */
	memset(&config, 0 , sizeof(config));
	/* Debug mode overrides */
#ifdef DEBUG
	config.loglevel = LOG_INFO;
	config.debuglevel = DEBUG10;
	config.foreground = 1;
#endif

	while ((c = getopt (argc, argv, "hvrd:nqf")) != -1) {
		switch (c) {
		case 'h':
			printf ("NeoStats: Usage: \"neostats [options]\"\n");
			printf ("     -h (Show this screen)\n");
			printf ("	  -v (Show version number)\n");
			printf ("	  -r (Enable recv.log)\n");
			printf ("	  -d 1-10 (Debug log output level 1= lowest, 10 = highest)\n");
			printf ("	  -n (Do not load any modules on startup)\n");
			printf ("	  -q (Quiet start - for cron scripts)\n");
			printf ("     -f (Do not fork into background\n");
			return NS_FAILURE;
		case 'v':
			printf ("NeoStats: http://www.neostats.net\n");
			printf ("Version:  %s\n", me.version);
			printf ("Compiled: %s at %s\n", ns_module_info.build_date, ns_module_info.build_time);
			return NS_FAILURE;
		case 'r':
			printf ("recv.log enabled. Watch your disk space\n");
			config.recvlog = 1;
			break;
		case 'd':
			printf ("debug.log enabled. Watch your disk space\n");
			level = atoi (optarg);
			if ((level >= DEBUGMAX) || (level < 1)) {
				printf ("Invalid debug level %d\n", level);
				return NS_FAILURE;
			}
			config.debuglevel = level;
			break;
		case 'n':
			config.modnoload = 1;
			break;
		case 'q':
			config.quiet = 1;
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
		nlog (LOG_CRITICAL, "Normal shut down subsystems");
		break;
	case NS_EXIT_RELOAD:
		nlog (LOG_CRITICAL, "Reloading NeoStats");
		break;
	case NS_EXIT_RECONNECT:
		nlog (LOG_CRITICAL, "Restarting NeoStats subsystems");
		break;
	case NS_EXIT_ERROR:
		nlog (LOG_CRITICAL, "Exiting due to error");
		return_code=EXIT_FAILURE;	/* exit code to error */
		break;		
	case NS_EXIT_SEGFAULT:
		nlog (LOG_CRITICAL, "Shutting down subsystems without saving data due to core");
		return_code=EXIT_FAILURE;	/* exit code to error */
		break;
	}

	if (exitcode != NS_EXIT_SEGFAULT) {
		unload_modules();
		if(quitmsg)
		{
			squit_cmd (ns_botptr->nick, quitmsg);
			ssquit_cmd (me.name, quitmsg);
		}
		sleep(1);
		/* now free up the users and servers memory */
		FiniUsers();
		FiniChannels();
		FiniServers();
		FiniBans();
		FiniDns();
		FiniModules();
		FiniSocks();
		FiniBots();
		FiniTimers();
		if (exitcode == NS_EXIT_RECONNECT) {
			if(config.r_time>0) {
				nlog (LOG_NOTICE, "Reconnecting to the server in %d seconds (Attempt %i)", config.r_time, attempts);
				sleep (config.r_time);
			}
			else {
				nlog (LOG_NOTICE, "Reconnect time is zero, shutting down");
			}
		}
	}

	kp_flush();
	kp_exit();
	FiniLogs ();

	if ((exitcode == NS_EXIT_RECONNECT && config.r_time > 0) || exitcode == NS_EXIT_RELOAD) {
		execve ("./neostats", NULL, NULL);
		return_code=EXIT_FAILURE;	/* exit code to error */
	}
	remove (PID_FILENAME);
	exit (return_code);
}

void fatal_error(char* file, int line, char* func, char* error_text)
{
	nlog (LOG_CRITICAL, "Fatal Error: %s %d %s %s", file, line, func, error_text);
	do_exit (NS_EXIT_ERROR, "Fatal Error - check log file");
}
