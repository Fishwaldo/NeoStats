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
#include "lang.h"

#define PID_FILENAME	"neostats.pid"

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
	nsconfig.debug = 1;
#endif
	nsconfig.r_time = 10;
	me.numeric = 1;
}

/** @brief init core sub systems
 *
 * @return 
 */
static int InitCore(void)
{
	char dbpath[MAXPATH];
	InitMe();
	/* if we are doing recv.log, remove the previous version */
	if (nsconfig.recvlog)
		remove (RECV_LOG);
	/* prepare to catch errors */
	InitSignals ();
	/* load the config files */
	if (ConfLoad () != NS_SUCCESS)
		return NS_FAILURE;
	/* initialize Lang Subsystem */
	ircsnprintf(dbpath, MAXPATH, "%s/data/lang.db", NEO_PREFIX);
	LANGinit(1, dbpath, NULL);
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
	if (InitIrcd () != NS_SUCCESS)
		return NS_FAILURE;
	rtaserv_init ();
	InitServices();
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
#ifndef WIN32
int
main (int argc, char *argv[])
#else
int
neostats ()
#endif
{
	FILE *fp;

	/* initialise version */
	strlcpy(me.version, VERSION, VERSIONSIZE);
	/* out default lang is always -1 */
	me.lang = -1;
#ifndef WIN32
	/* get our commandline options */
	if(get_options (argc, argv)!=NS_SUCCESS)
		return EXIT_FAILURE;
#endif
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
	if (!nsconfig.quiet) {
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

#ifndef WIN32
#ifndef DEBUG
	/* if we are compiled with debug, or forground switch was specified, DONT FORK */
	if (!nsconfig.foreground) {
		/* fix the double log message problem by closing logs prior to fork() */ 
		CloseLogs (); 
		forked = fork ();
		/* Error check fork() */ 
		if (forked < 0) { 
			perror("fork"); 
			return EXIT_FAILURE; /* fork error */ 
		} 
#endif
#endif
		/* we are the parent */ 
		if (forked > 0) { 
			/* write out our PID */
			fp = fopen (PID_FILENAME, "w");
			fprintf (fp, "%i", forked);
			fclose (fp);
			if (!nsconfig.quiet) {
				printf ("\n");
				printf (_("NeoStats %s Successfully Launched into Background\n"), me.version);
				printf (_("PID: %i - Wrote to %s\n"), forked, PID_FILENAME);
			}
			return EXIT_SUCCESS; /* parent exits */ 
		}
#ifndef WIN32
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
#endif
	nlog (LOG_NOTICE, "NeoStats \"%s\" started.", VERSION);

	/* Load modules after we fork. This fixes the load->fork-exit->call 
	   _fini problems when we fork */
	ConfLoadModules ();
	/* Connect to server */
	Connect ();
	/* We should never reach here but the compiler does not realise and may
	   complain about not all paths control returning values without the return 
	   Since it should never happen, treat as an error condition! */
#ifndef WIN32
	return EXIT_FAILURE;
#else
	return 0;
#endif
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
#ifdef WIN32
	nsconfig.debug = 1;
	nsconfig.loglevel = LOG_INFO;
	nsconfig.debuglevel = DEBUG10;
#else
	int c;
	int level;

	/* Clear config */
	memset(&nsconfig, 0 , sizeof(config));
	/* Debug mode overrides */
#ifdef DEBUG
	nsconfig.loglevel = LOG_INFO;
	nsconfig.debuglevel = DEBUG10;
	nsconfig.foreground = 1;
#endif
	/* default debugmodule to all */
	strlcpy(nsconfig.debugmodule, "all", MAX_MOD_NAME);

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
			nsconfig.recvlog = 1;
			break;
		case 'd':
			level = atoi (optarg);
			if ((level >= DEBUGMAX) || (level < 1)) {
				printf ("Invalid debug level %d\n", level);
				return NS_FAILURE;
			}
			printf ("debug.log enabled at level %d. Watch your disk space\n", level);
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
			printf ("Unknown command line switch %c\n", optopt);
		}
	}
#endif
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
			irc_quit (ns_botptr, quitmsg);
			irc_squit (me.name, quitmsg);
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
		rtaserv_fini ();
		if (exitcode == NS_EXIT_RECONNECT) {
			if(nsconfig.r_time>0) {
				nlog (LOG_NOTICE, "Reconnecting to the server in %d seconds (Attempt %i)", nsconfig.r_time, attempts);
				sleep (nsconfig.r_time);
			}
			else {
				nlog (LOG_NOTICE, "Reconnect time is zero, shutting down");
			}
		}
	}

	kp_flush();
	kp_exit();
	FiniLogs ();
	LANGfini();
	if ((exitcode == NS_EXIT_RECONNECT && nsconfig.r_time > 0) || exitcode == NS_EXIT_RELOAD) {
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
