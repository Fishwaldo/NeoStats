/* NeoStats - IRC Statistical Services 
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
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif
#include "signal.h"
#include "dl.h"
#include "conf.h"
#include "keeper.h"
#include "log.h"

/* this is the name of the services bot */
char s_Services[MAXNICK] = "NeoStats";

/*! depending on what IRCD is selected, we change the version string */
#ifdef UNREAL
const char version[] = "(U)";
#elif ULTIMATE3
const char version[] = "(UL3)";
#elif ULTIMATE
const char version[] = "(UL)";
#elif HYBRID7
const char version[] = "(H)";
#elif NEOIRCD
const char version[] = "(N)";
#elif MYSTIC
const char version[] = "(M)";
#elif QUANTUM
const char version[] = "(Q)";
#elif IRCU
const char version[] = "(IRCU)";
#elif BAHAMUT
const char version[] = "(B)";
#endif




/*! Date when we were compiled */
const char version_date[] = __DATE__;
/*! Time we were compiled */
const char version_time[] = __TIME__;

static void start ();
static void setup_signals ();
static int get_options (int argc, char **argv);


/*! have we forked */
int forked = 0;


/** @brief Main Entry point into program
 *
 * Sets up defaults, and parses the config file.
 * Also initilizes different parts of NeoStats
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
	if(get_options (argc, argv)<0)
		exit(-1);

	/* Change to the working Directory */
	if (chdir (NEO_PREFIX) < 0) {
		printf ("NeoStats Could not change to %s\n", NEO_PREFIX);
		printf ("Did you 'make install' after compiling?\n");
		printf ("Error Was: %s\n", strerror (errno));
		exit (-1);
	}

	/* before we do anything, make sure logging is setup */
	if(init_logs ()<0)
		exit(-1);

	/* our crash trace variables */
	SET_SEGV_LOCATION();
	CLEAR_SEGV_INMODULE();
	/* for modules, let them know we are not ready */
	me.onchan = 0;
	/* keep quiet if we are told to :) */
	if (!config.quiet) {
		printf ("NeoStats %d.%d.%d%s Loading...\n", MAJOR, MINOR, REV, version);
		printf ("-----------------------------------------------\n");
		printf ("Copyright: NeoStats Group. 2000-2003\n");
		printf ("Justin Hammond (fish@neostats.net)\n");
		printf ("Adam Rutter (shmad@neostats.net)\n");
		printf ("-----------------------------------------------\n\n");
	}
	/* set some defaults before we parse the config file */
	me.t_start = time (NULL);
	me.want_privmsg = 0;
	me.die = 0;
	me.local[0] = '\0';
	me.coder_debug = 0;
	me.noticelag = 0;
	me.usesmo = 0;
	me.r_time = 10;
	me.lastmsg = time (NULL);
	me.SendM = me.SendBytes = me.RcveM = me.RcveBytes = 0;
	me.synced = 0;
	me.onchan = 0;
	me.maxsocks = getmaxsock ();
#ifdef ULTIMATE3
	me.client = 0;
#endif
/* M - Not used */
/*	strcpy (me.modpath, "dl"); */

	/* if we are doing recv.log, remove the previous version */
	if (config.recvlog)
		remove ("logs/recv.log");

	/* initilze our Module subsystem */
	__init_mod_list ();

	/* prepare to catch errors */
	setup_signals ();

	/* load the config files */
	if(ConfLoad ()<0)
		exit(-1);
	if (me.die) {
		printf ("\n-----> ERROR: Read the README file then edit %s! <-----\n\n",CONFIG_NAME);
		nlog (LOG_CRITICAL, LOG_CORE, "Read the README file and edit your %s",CONFIG_NAME);
		sleep (1);
		close (servsock);
		/* we are exiting the parent, not the program, don't call do_exit() */
		exit (0);
	}

	/* initilize the rest of the subsystems */
	TimerReset ();
	init_dns ();
	init_server_hash ();
	init_user_hash ();
	init_chan_hash ();
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
			exit(1); /* fork error */ 
		} 
#endif
		/* we are the parent */ 
		if (forked>0) { 
			/* write out our PID */
			fp = fopen ("neostats.pid", "w");
			fprintf (fp, "%i", forked);
			fclose (fp);
			if (!config.quiet) {
				printf ("\n");
				printf ("NeoStats %d.%d.%d%s Successfully Launched into Background\n", MAJOR, MINOR, REV, version);
				printf ("PID: %i - Wrote to neostats.pid\n", forked);
			}
			exit(0); /* parent exits */ 
		}
#ifndef DEBUG
		/* child (daemon) continues */ 
		/* reopen logs for child */ 
		if(init_logs ()<0)
			exit(-1);
		/* detach from parent process */
		if (setpgid (0, 0) < 0) {
			nlog (LOG_WARNING, LOG_CORE, "setpgid() failed");
		}
	}
#endif
	nlog (LOG_NOTICE, LOG_CORE, "Statistics Started (NeoStats %d.%d.%d%s).", MAJOR, MINOR, REV, version);

	/* don't init_modules till after we fork. This fixes the load->fork-exit->call _fini problems when we fork */
	init_modules ();

	/* we are ready to start now Duh! */
	start ();

	return 1;
}

/** @brief Process COmmandline Options
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
			return (-1);
		case 'v':
			printf ("NeoStats Version %d.%d.%d%s\n", MAJOR, MINOR, REV, version);
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
			return (-1);
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
				return (-1);
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
	return(0);
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
RETSIGTYPE
serv_die ()
{
#ifdef VALGRIND
	exit(1);
#else /* VALGRIND */
	User *u;
	u = finduser (s_Services);
	nlog (LOG_CRITICAL, LOG_CORE, "Sigterm Received, Shuting Down Server!!!!");
	ns_shutdown (u, "SigTerm Received");
	ssquit_cmd (me.name);  
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
/*	struct sigaction act; */
	chanalert (s_Services, "Received SIGHUP, Attempting to Rehash");
	globops (me.name, "Received SIGHUP, Attempted to Rehash");
/*	act.sa_handler = conf_rehash;
	act.sa_flags=0;
	(void)sigemptyset(&act.sa_mask);
	(void)sigaddset(&act.sa_mask, SIGHUP);
	(void)sigaction(SIGHUP, &act, NULL);
*/
	globops (me.name, "REHASH not completed in this version!");


	/* gotta do the rehash code dun I? */
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

RETSIGTYPE
serv_segv ()
{
	char name[30];
#ifdef HAVE_BACKTRACE
	void *array[50];
	size_t size;
	char **strings;
	size_t i;
/* thanks to gnulibc libary for letting me find this usefull function */
	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);
#endif
	/** if the segv happened while we were inside a module, unload and try to restore 
	 *  the stack to where we were before we jumped into the module
	 *  and continue on
	 */
	if (strlen (segvinmodule) > 1) {
		globops (me.name, "Oh Damn, Module %s Segv'd, Unloading Module", segvinmodule);
		chanalert (s_Services, "Oh Damn, Module %s Segv'd, Unloading Module", segvinmodule);
		nlog (LOG_CRITICAL, LOG_CORE, "Uh Oh, Segmentation Fault in Modules Code %s", segvinmodule);
		nlog (LOG_CRITICAL, LOG_CORE, "Location could be %s", segv_location);
		nlog (LOG_CRITICAL, LOG_CORE, "Unloading Module and restoring stacks. Doing Backtrace:");
		chanalert (s_Services, "Location *could* be %s. Doing Backtrace:", segv_location);
#ifdef HAVE_BACKTRACE
		for (i = 1; i < size; i++) {
			chanalert (s_Services, "Backtrace(%d): %s", i, strings[i]);
			nlog (LOG_CRITICAL, LOG_CORE, "BackTrace(%d): %s", i - 1, strings[i]);
		}
#else
		chanalert (s_Services, "Backtrace not available on this platform");
		nlog (LOG_CRITICAL, LOG_CORE, "Backtrace not available on this platform");
#endif
		strncpy (name, segvinmodule, 30);
		CLEAR_SEGV_INMODULE();
		unload_module (name, NULL);
		chanalert (s_Services, "Restoring Stack to before Crash");
		longjmp (sigvbuf, -1);
		chanalert (s_Services, "Done");
#ifdef HAVE_BACKTRACE
		free (strings);
#endif
	} else {
		/** The segv happened in our core, damn it */
		/* Thanks to Stskeeps and Unreal for this stuff :) */
		nlog (LOG_CRITICAL, LOG_CORE, "Uh Oh, Segmentation Fault.. Server Terminating");
		nlog (LOG_CRITICAL, LOG_CORE, "Details: Buffer: %s", recbuf);
		nlog (LOG_CRITICAL, LOG_CORE, "Approx Location: %s Backtrace:", segv_location);
		/* Broadcast it out! */
		globops (me.name, "Ohhh Crap, Server Terminating, Segmentation Fault. Buffer: %s, Approx Location %s", recbuf, segv_location);
		chanalert (s_Services, "Damn IT, Server Terminating (%d%d%d%s), Segmentation Fault. Buffer: %s, Approx Location: %s Backtrace:", MAJOR, MINOR, REV, version, recbuf, segv_location);
#ifdef HAVE_BACKTRACE
		for (i = 1; i < size; i++) {
			chanalert (s_Services, "Backtrace(%d): %s", i, strings[i]);
			nlog (LOG_CRITICAL, LOG_CORE, "BackTrace(%d): %s", i - 1, strings[i]);
		}
#else
		chanalert (s_Services, "Backtrace not available on this platform");
		nlog (LOG_CRITICAL, LOG_CORE, "Backtrace not available on this platform");
#endif
		sleep (2);
		kill (forked, 3);
		kill (forked, 9);
		/* clean up */
		do_exit (1);
	}
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
setup_signals ()
{
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	(void) sigemptyset (&act.sa_mask);
	(void) sigaddset (&act.sa_mask, SIGPIPE);
	(void) sigaddset (&act.sa_mask, SIGALRM);
	(void) sigaction (SIGPIPE, &act, NULL);
	(void) sigaction (SIGALRM, &act, NULL);

	act.sa_handler = conf_rehash;
	(void) sigemptyset (&act.sa_mask);
	(void) sigaddset (&act.sa_mask, SIGHUP);
	(void) sigaction (SIGHUP, &act, NULL);

	act.sa_handler = serv_die;
	(void) sigaddset (&act.sa_mask, SIGTERM);
	(void) sigaction (SIGTERM, &act, NULL);
	(void) sigaddset (&act.sa_mask, SIGINT);
	(void) sigaction (SIGINT, &act, NULL);

/* handling of SIGSEGV as well -sts */
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
start ()
{
	static int attempts = 0;
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	nlog (LOG_NOTICE, LOG_CORE, "Connecting to %s:%d", me.uplink, me.port);
	if (servsock > 0)
		close (servsock);

	servsock = ConnectTo (me.uplink, me.port);

	if (servsock <= 0) {
		nlog (LOG_WARNING, LOG_CORE, "Unable to connect to %s", me.uplink);
	} else {
		attempts = 0;
		login ();
		read_loop ();
	}
	if(me.r_time>0)
		nlog (LOG_NOTICE, LOG_CORE, "Reconnecting to the server in %d seconds (Attempt %i)", me.r_time, attempts);
	else
		nlog (LOG_NOTICE, LOG_CORE, "Reconnect time is zero, shutting down");
	close (servsock);

	unload_modules(NULL);

	if(me.r_time>0) {
		sleep (me.r_time);
		do_exit (2);
	}
	else
		do_exit (0);
}

/** @brief Login to IRC
 *
 * Calls the IRC specific function slogin_cmd to login as a server to IRC
 * 
 * @return Nothing
 *
 */

void
login ()
{
	SET_SEGV_LOCATION();
	slogin_cmd (me.name, 1, me.infoline, me.pass);
	sprotocol_cmd ("TOKEN CLIENT");
}

/** @brief Our Own implementation of Malloc.
 *
 * Allocates memory for internal variables. Usefull for Memory Debugging
 * if enough memory can't be malloced, exit the program 
 *
 * @param size The amount of memory to Malloc
 *
 * @returns size bytes of memory or NULL on malloc error
 *
 * @todo move this to a util type file, as it being in main is crazy
 */

void *
smalloc (long size)
{
	void *buf;

	if (!size) {
		nlog (LOG_WARNING, LOG_CORE, "smalloc(): illegal attempt to allocate 0 bytes!");
		size = 1;
	}
	buf = malloc (size);
	if (!buf) {
		nlog (LOG_CRITICAL, LOG_CORE, "smalloc(): out of memory.");
		do_exit (1);
	}
	return buf;
}

/** @brief Duplicate a string
 *
 * make a copy of a string, with memory allocated for the new string
 *
 * @param s a pointer to the string to duplicate
 *
 * @returns a pointer to the new string
 *
 * @deprecated  Try not to use this function, it will go away one day
 *
 */

char *
sstrdup (const char *s)
{
	char *t = strdup (s);
	if (!t) {
		nlog (LOG_CRITICAL, LOG_CORE, "sstrdup(): out of memory.");
		do_exit (1);
	}
	return t;
}

/** @brief Create a HASH from a string
 *
 * Makes a hash of a string for a table
 *
 * @param name The string to use as the base for the hash
 *
 * @param size_of_table The size of the hash table
 *
 * @returns unsigned long of the hash
 *
 * @deprecated  Try not to use this function, it will go away one day
 *
 */

unsigned long
HASH (const unsigned char *name, int size_of_table)
{
	unsigned long h = 0, g;

	while (*name) {
		h = (h << 4) + *name++;
		if ((g = h & 0xF0000000))
			h ^= g >> 24;
		h &= ~g;
	}
	return h % size_of_table;
}

/** @brief convert a string to lowercase
 *
 * makes a string lowercase
 *
 * @param s the string to convert to lowercase. WARNING: the result overwrites this variable
 *
 * @returns pointer to the lowercase version of s
 *
 */

char *
strlower (char *s)
{
	char *t;
	t = malloc (strlen (s));
	strncpy (t, s, strlen (s));
	while (*t) {
		*t++ = tolower (*t);
	}
	return t;
}

/** @brief Adds a string to a array of strings
 *
 * used for the event functions, adds a string to a array of string pointers to pass to modules
 *
 * @param List the array you wish to append S to 
 * @param S the string you wish to append
 * @param C the current size of the array
 * 
 * @returns Nothing
 *
 */

void
AddStringToList (char ***List, char S[], int *C)
{
	if (*C == 0) {
		*List = calloc (sizeof (char *) * 8, 1);
	}
	++*C;
	(*List)[*C - 1] = S;
}

/** @brief Frees a list created with AddStringToList
 *
 * Frees the memory used for a string array used with AddStringToList
 *
 * @param List the array you wish to delete
 * @param C the current size of the array as returned by AddStringToList
 * 
 * @returns Nothing
 *
 */
void
FreeList (char **List, int C)
{
	int i;
	for (i = 0; i == C; i++)
		free (List[i]);
	C = 0;
}

/** @brief before exiting call this function. It flushes log files and tidy's up.
 *
 *  Cleans up before exiting 
 *  @parm segv 1 = we are exiting because of a segv fault, 0, we are not.
 *  if 1, we don't prompt to save data
 */
void
do_exit (int segv)
{
	/* Initialise exit code to OK */
	int exit_code=1;

	switch (segv) {
	case 0:
		nlog (LOG_CRITICAL, LOG_CORE, "Normal shut down SubSystems");
		break;
	case 2:
		nlog (LOG_CRITICAL, LOG_CORE, "Restarting NeoStats SubSystems");
		break;
	case 1:
		nlog (LOG_CRITICAL, LOG_CORE, "Shutting Down SubSystems without saving data due to core");
		/* exit code to error */
		exit_code=-1;
		break;
	}
	kp_flush();
	close_logs ();
	if (segv == 2) {
		execve ("./neostats", NULL, NULL);
		/* exit code to error */
		exit_code=-2;
	}
	remove ("neostats.pid");
	exit (exit_code);
}

/* this came from eggdrop sources */
/* Remove the color control codes that mIRC,pIRCh etc use to make
 * their client seem so fecking cool! (Sorry, Khaled, you are a nice
 * guy, but when you added this feature you forced people to either
 * use your *SHAREWARE* client or face screenfulls of crap!)
 */
void strip_mirc_codes(char *text)
{
  char *dd = text;

  while (*text) {
    switch (*text) {
    case 1:
    	text++;			/* ctcp stuff */
    	continue;
      break;
    case 2:			/* Bold text */
	text++;
	continue;
      break;
    case 3:			/* mIRC colors? */
	if (isdigit(text[1])) {	/* Is the first char a number? */
	  text += 2;		/* Skip over the ^C and the first digit */
	  if (isdigit(*text))
	    text++;		/* Is this a double digit number? */
	  if (*text == ',') {	/* Do we have a background color next? */
	    if (isdigit(text[1]))
	      text += 2;	/* Skip over the first background digit */
	    if (isdigit(*text))
	      text++;		/* Is it a double digit? */
	  }
	continue;
      }
      break;
    case 7:
	text++;
	continue;
      break;
    case 0x16:			/* Reverse video */
	text++;
	continue;
      break;
    case 0x1f:			/* Underlined text */
	text++;
	continue;
      break;
    case 033:
	text++;
	if (*text == '[') {
	  text++;
	  while ((*text == ';') || isdigit(*text))
	    text++;
	  if (*text)
	    text++;		/* also kill the following char */
	}
	continue;
      break;
    }
    *dd++ = *text++;		/* Move on to the next char */
  }
  *dd = 0;
}

