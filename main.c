/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
** See the file called 'LICENSE' for more details.
*/

#include <setjmp.h>
#include "stats.h"
#include "signal.h"
#include "dl.h"

char s_Debug[MAXNICK] = "Stats_Debug";
char s_Services[MAXNICK] = "NeoStats";
const char version[] = "NeoStats-2.5_Alpha2";
const char version_date[] = __DATE__;
const char version_time[] = __TIME__;

void RemoveLock();

static void start();
static void setup_signals();
int forked = 0;

int main()
{
	FILE *fp;
	segv_location = sstrdup("main");
	strcpy(segvinmodule, "");
	me.onchan = 0;
	printf("\n\n");
	printf("%s Loading...\n", version);
	printf("-----------------------------------------------\n");
	printf("Copyright: NeoStats Group. 2000-2002\n");
	printf("Justin Hammond (Fish@dynam.ac)\n");
	printf("Adam Rutter (shmad@neostats.net)\n");
	printf("^Enigma^ (enigma@neostats.net)\n");
	printf("-----------------------------------------------\n\n");
	me.t_start = time(NULL);
	me.want_privmsg = 0;
	me.enable_spam = 0;
	me.coder_debug=0;
	me.noticelag=0;
	me.usesmo=0;
	me.r_time=10;
	me.SendM = me.SendBytes = me.RcveM = me.RcveBytes = 0;
	RemoveLock();

	__init_mod_list();
	setup_signals();
	ConfLoad();
/* Shmad */
/* This section ALWAYS craps out so we ignore it-- for now */
	if (init_modules()) {
/*		printf("WARNING: Some Modules Failed to Load"); */
	}


#ifndef DEBUG
	forked=fork();
#endif
	if (forked) {
		fp = fopen("stats.pid", "w");
		fprintf(fp, "%i", forked);
		fclose(fp);
		printf("\n");
		printf("%s Successfully Launched into Background\n", version);
		printf("PID: %i - Wrote to stats.pid\n",forked);

		return 0;
	}

	log("Statistics Started (%s).", version);
	start();

	return 1;
}

RETSIGTYPE serv_die() {
	User *u;
	u = finduser(s_Services);
	log("Sigterm Recieved, Shuting Down Server!!!!");
	ns_shutdown(u,"SigTerm Recieved");
	ssquit_cmd(me.name);

}


RETSIGTYPE conf_rehash() {
/*	struct sigaction act; */
	notice(s_Services, "Recieved SIGHUP, Attempting to Rehash");
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

RETSIGTYPE serv_segv() {
	char name[30];

	if (strlen(segvinmodule) > 1) {
		log("Uh Oh, Segmentation Fault in Modules Code %s", segvinmodule);
		log("Unloading Module and restoring stacks");
		globops(me.name, "Oh Damn, Module %s Segv'd, Unloading Module", segvinmodule);
		notice(s_Services, "Oh Damn, Module %s Segv'd, Unloading Module", segvinmodule);
		strcpy(name, segvinmodule);
		strcpy(segvinmodule, "");
		unload_module(name, NULL);
		notice(s_Services, "Restoring Stack to before Crash");
		longjmp(sigvbuf, -1);
		notice(s_Services, "Done");
	} else {	
		/* Thanks to Stskeeps and Unreal for this stuff :) */
		log("Uh Oh, Segmentation Fault.. Server Terminating");
		log("Details: Buffer: %s", recbuf);
		log("Approx Location: %s", segv_location);
		/* Broadcast it out! */
		globops(me.name,"Ohhh Crap, Server Terminating, Segmentation Fault. Buffer: %s, Approx Location %s", recbuf, segv_location);
		notice(s_Services, "Damn IT, Server Terminating, Segmentation Fault. Buffer: %s, Approx Location %s", recbuf, segv_location);
		globops(me.name,"Dumped Core to netstats.debug, Please Read the Readme file to find out what to do with it!");
		
		sleep(2);
		kill(forked, 3);
		kill(forked, 9);
		exit(-1);
	        ssquit_cmd(me.name);
	}
}


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


/* Routine For Removing Chmod Lock On Modules */
void RemoveLock()
{
	FILE *lckfile;
	char buf[512];
	int fmode;

	segv_location = sstrdup("main_RemoveLock");

	lckfile = fopen("data/Lock.db", "r");

	if (lckfile) {
	   while (fgets(buf, sizeof(buf), lckfile)) {
		   buf[strlen(buf)] = '\0';
	   fmode = 0755;
	   chmod(buf, fmode);
	   }
	fclose(lckfile);
	remove("data/Lock.db");
	}
}


void start()
{
	static int attempts = 0;
	
	segv_location = sstrdup("start");
	TimerReset();
	init_server_hash();
	init_user_hash();
	init_ircd();
/* 
	init_tld();
*/
	if (attempts < 10) {
		attempts++;
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
		sleep(me.r_time);
		start();
	} else {
		log("Could Not Connect to Server %s after 10 Attempts",me.uplink);
		log("Exiting NeoStats");
		printf("\n%s Terminating..... Could not connect to Server\n", version);
		printf("%s Terminated\n", version);
	}	
}

void login()
	{
	segv_location = sstrdup("login");
	slogin_cmd(me.name, 1, me.infoline, me.pass);
	sprotocol_cmd("TOKEN");
}


void *smalloc(long size)
{
	void *buf;
	
	segv_location = sstrdup("smalloc");
	if (!size) {
		log("smalloc(): illegal attempt to allocate 0 bytes!");
		size = 1;
	}
	buf = malloc(size);
	if (!buf) {
		log("smalloc(): out of memory.");
		exit(0);
	}
/*	free(segv_location); */
	return buf;
}

char *sstrdup(const char *s)
{
	char *t = strdup(s);
	if (!t) {
		log("sstrdup(): out of memory.");
		exit(0);
	}
	return t;
}

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

char *strlower(const char *s)
{
/*	char *t = s;
	while ((*t++ = tolower(*t)))
		;  */ 

	char *t = s; 
	while (*t)
	*t++ = tolower (*t);
return s;
}

