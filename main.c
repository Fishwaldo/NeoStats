/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: main.c,v 1.76 2002/12/26 14:15:07 fishwaldo Exp $
*/

#include <setjmp.h>
#include <stdio.h>
#include "stats.h"
#include "signal.h"
#include "dl.h"
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif


/*! this is the name of the services bot */
char s_Services[MAXNICK] = "NeoStats";

/*! depending on what IRCD is selected, we change the version string */
#ifdef UNREAL
const char version[] = "NeoStats-2.5-RC3(U)";
#elif ULTIMATE3
const char version[] = "NeoStats-2.5-RC3(UL3)";
#elif ULTIMATE
const char version[] = "NeoStats-2.5-RC3(UL)";
#elif HYBRID7
const char version[] = "NeoStats-2.5-RC3(H)";
#elif NEOIRCD
const char version[] = "NeoStats-2.5-RC3(N)";
#endif




/*! Date when we were compiled */
const char version_date[] = __DATE__;
/*! Time we were compiled */
const char version_time[] = __TIME__;

static void start();
static void setup_signals();



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
int main()
{
	FILE *fp;

	strcpy(segv_location, "main");
	strcpy(segvinmodule, "");
	me.onchan = 0;
	if (usr_mds)
	printf("\n\n");
	printf("%s Loading...\n", version);
	printf("-----------------------------------------------\n");
	printf("Copyright: NeoStats Group. 2000-2002\n");
	printf("Justin Hammond (fish@neostats.net)\n");
	printf("Adam Rutter (shmad@neostats.net)\n");
	printf("^Enigma^ (enigma@neostats.net)\n");
	printf("-----------------------------------------------\n\n");
	me.t_start = time(NULL);
	me.want_privmsg = 0;
	me.enable_spam = 0;
	me.die = 0;
	me.coder_debug=0;
	me.noticelag=0;
	me.usesmo=0;
	me.r_time=10;
	me.lastmsg = time(NULL);
	me.SendM = me.SendBytes = me.RcveM = me.RcveBytes = 0;
	me.synced = 0;
	me.onchan = 0;
	me.maxsocks = getmaxsock();
	strcpy(me.modpath,"dl");
#ifdef RECVLOG
	remove("logs/recv.log");
#endif
	__init_mod_list();
	setup_signals();
	ConfLoad();
        if (me.die) {
		printf("\n-----> ERROR: Read the README file then edit neostats.cfg! <-----\n\n");
                log("Read the README file and edit your neostats.cfg");
                sleep(1);
                close(servsock);
                remove("neostats.pid");
                exit(0);
        }
	TimerReset();
	init_dns();
	init_server_hash();
	init_user_hash();
	init_chan_hash();

/** This section ALWAYS craps out so we ignore it-- for now */
	if (init_modules()) {
/*		printf("WARNING: Some Modules Failed to Load"); */
	}


#ifndef DEBUG
	forked=fork();
#endif
	if (forked) {
		fp = fopen("neostats.pid", "w");
		fprintf(fp, "%i", forked);
		fclose(fp);
		printf("\n");
		printf("%s Successfully Launched into Background\n", version);
		printf("PID: %i - Wrote to neostats.pid\n",forked);

		return 0;
	}
#ifndef DEBUG
	if (setpgid(0, 0) < 0) {
		log("setpgid() failed");
	}
		
#endif
	log("Statistics Started (%s).", version);
	start();

	return 1;
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
RETSIGTYPE serv_die() {
	User *u;
	u = finduser(s_Services);
	log("Sigterm Recieved, Shuting Down Server!!!!");
	ns_shutdown(u,"SigTerm Recieved");
	ssquit_cmd(me.name);

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
RETSIGTYPE conf_rehash() {
/*	struct sigaction act; */
	chanalert(s_Services, "Recieved SIGHUP, Attempting to Rehash");
	globops(me.name, "Received SIGHUP, Attempted to Rehash");
/*	act.sa_handler = conf_rehash;
	act.sa_flags=0;
	(void)sigemptyset(&act.sa_mask);
	(void)sigaddset(&act.sa_mask, SIGHUP);
	(void)sigaction(SIGHUP, &act, NULL);
*/
	globops(me.name, "REHASH not completed in this version!");


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

RETSIGTYPE serv_segv() {
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
	if (strlen(segvinmodule) > 1) {
		globops(me.name, "Oh Damn, Module %s Segv'd, Unloading Module", segvinmodule);
		chanalert(s_Services, "Oh Damn, Module %s Segv'd, Unloading Module", segvinmodule);
		log("Uh Oh, Segmentation Fault in Modules Code %s", segvinmodule);
		log("Location could be %s", segv_location);
		log("Unloading Module and restoring stacks. Doing Backtrace:");
		chanalert(s_Services, "Location *could* be %s. Doing Backtrace:", segv_location);
#ifdef HAVE_BACKTRACE
		for (i = 1; i < size; i++) {
			chanalert(s_Services, "Backtrace(%d): %s", i, strings[i]);
			log("BackTrace(%d): %s", i-1, strings[i]);
		}
#else 
		chanalert(s_Services, "Backtrace not available on this platform");
		log("Backtrace not available on this platform");
#endif
		strcpy(name, segvinmodule);
		strcpy(segvinmodule, "");
		unload_module(name, NULL);
		chanalert(s_Services, "Restoring Stack to before Crash");
		longjmp(sigvbuf, -1);
		chanalert(s_Services, "Done");
#ifdef HAVE_BACKTRACE
		free(strings);
#endif
	} else {	
		/** The segv happened in our core, damn it */
		/* Thanks to Stskeeps and Unreal for this stuff :) */
		log("Uh Oh, Segmentation Fault.. Server Terminating");
		log("Details: Buffer: %s", recbuf);
		log("Approx Location: %s Backtrace:", segv_location);
		/* Broadcast it out! */
		globops(me.name,"Ohhh Crap, Server Terminating, Segmentation Fault. Buffer: %s, Approx Location %s", recbuf, segv_location);
		chanalert(s_Services, "Damn IT, Server Terminating (%s), Segmentation Fault. Buffer: %s, Approx Location: %s Backtrace:", version, recbuf, segv_location);
#ifdef HAVE_BACKTRACE
		for (i = 1; i < size; i++) {
			chanalert(s_Services, "Backtrace(%d): %s", i, strings[i]);
			log("BackTrace(%d): %s", i-1, strings[i]);
		}
#else 
		chanalert(s_Services, "Backtrace not available on this platform");
		log("Backtrace not available on this platform");
#endif
		sleep(2);
		kill(forked, 3);
		kill(forked, 9);
		exit(-1);
		ssquit_cmd(me.name);
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
static	void	setup_signals()
{
	struct	sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	(void)sigemptyset(&act.sa_mask);
	(void)sigaddset(&act.sa_mask, SIGPIPE);
	(void)sigaddset(&act.sa_mask, SIGALRM);
	(void)sigaction(SIGPIPE, &act, NULL);
	(void)sigaction(SIGALRM, &act, NULL);

	act.sa_handler = conf_rehash;
	(void)sigemptyset(&act.sa_mask);
	(void)sigaddset(&act.sa_mask, SIGHUP);
	(void)sigaction(SIGHUP, &act, NULL);

	act.sa_handler = serv_die;
	(void)sigaddset(&act.sa_mask, SIGTERM);
	(void)sigaction(SIGTERM, &act, NULL);
/* handling of SIGSEGV as well -sts */
	act.sa_handler = serv_segv;
	(void)sigaddset(&act.sa_mask, SIGSEGV);
	(void)sigaction(SIGSEGV, &act, NULL);


	(void)signal(SIGHUP, conf_rehash);
	(void)signal(SIGTERM, serv_die); 
	(void)signal(SIGSEGV, serv_segv);
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
void start()
{
	static int attempts = 0;
	Module *mod_ptr = NULL;
	hscan_t ms;
	hnode_t *mn;
	
	strcpy(segv_location, "start");

	
	log("Connecting to %s:%d", me.uplink, me.port);
	if (servsock > 0)
		close(servsock);

	servsock = ConnectTo(me.uplink, me.port);
	
	if (servsock <= 0) {
		log("Unable to connect to %s", me.uplink);
	} else {
		attempts=0;
		login();
		read_loop();
	}
	log("Reconnecting to the server in %d seconds (Attempt %i)", me.r_time, attempts);
	close(servsock);

	/* Unload the Modules */
	hash_scan_begin(&ms, mh);
	while ((mn = hash_scan_next(&ms)) != NULL) {
		mod_ptr = hnode_get(mn);
		unload_module(mod_ptr->info->module_name, finduser(s_Services));
	}
	sleep(5);
	execve("./neostats", NULL, NULL);
}
/** @brief Login to IRC
 *
 * Calls the IRC specific function slogin_cmd to login as a server to IRC
 * 
 * @return Nothing
 *
 */

void login()
	{
	strcpy(segv_location, "login");
	slogin_cmd(me.name, 1, me.infoline, me.pass);
	sprotocol_cmd("TOKEN");
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

void *smalloc(long size)
{
	void *buf;
	
	strcpy(segv_location, "smalloc");
	if (!size) {
		log("smalloc(): illegal attempt to allocate 0 bytes!");
		size = 1;
	}
	buf = malloc(size);
	if (!buf) {
		log("smalloc(): out of memory.");
		exit(0);
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

char *sstrdup(const char *s)
{
	char *t = strdup(s);
	if (!t) {
		log("sstrdup(): out of memory.");
		exit(0);
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

unsigned long HASH(const unsigned char *name, int size_of_table)
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

char *strlower(char *s)
{
	char *t = s; 
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

void AddStringToList(char ***List,char S[],int *C)
{
	if (*C == 0) {
		*List = calloc(sizeof(char *) * 8, 1);
	}
	++*C;
	(*List)[*C-1] = S;
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
void FreeList(char **List,int C)
{
int i;
for (i = 0; i == C; i++) 
	free(List[i]);
C = 0;
}

