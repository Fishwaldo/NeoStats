/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
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

#include "stats.h"
#include "signal.h"


char s_Debug[MAXNICK] = "Stats_Debug";
char s_Services[MAXNICK] = "NeoStats";
const char version[] = "NeoStats-2.0-Alpha4";
const char version_date[] = __DATE__;
const char version_time[] = __TIME__;

static void start();
static void setup_signals();

int main()
{
	int forked = 0;
	FILE *fp;
	segv_location = "main";
	me.onchan = 0;
	printf("%s Loading...\n", version);
	printf("-----------------------------------------------\n");
	printf("CopyRight: CodeBase 2000\n");
	printf("Justin Hammond (Fish@dynam.ac)\n");
	printf("Adam Rutter (shmad@kamserve.com)\n\n");
	me.t_start = time(NULL);
	me.want_privmsg = 0;
	me.enable_spam = 0;
	me.coder_debug=0;
	me.noticelag=0;
	me.usesmo=0;
	me.r_time=10;
	IcqServ.loginok=0;
	IcqServ.loglevel=2;
        IcqServ.onchan=0;
	__init_mod_list();
	setup_signals();
	ConfLoad();
	if (init_modules()) {
		printf("WARNING: Some Modules Failed to Load");
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
		printf("PID: %i (Written to stats.pid)\n",forked);

		return 0;
	}

	log("Statistics Started (%s).", version);
	start();

	return 1;
}

void serv_die() {
	User *u;
	u = finduser(s_Services);
	log("Sigterm Recieved, Shuting Down Server!!!!");
	ns_shutdown(u,"SigTerm Recieved");
	sts("SQUIT %s",me.name);

}


void conf_rehash() {
	struct sigaction act;
	notice(s_Services, "Recieved SIGHUP, Attempting to Rehash");
	globops(me.name, "Received SIGHUP, Attempted to Rehash");
	act.sa_handler = conf_rehash;
	act.sa_flags=0;
	(void)sigemptyset(&act.sa_mask);
	(void)sigaddset(&act.sa_mask, SIGHUP);
	(void)sigaction(SIGHUP, &act, NULL);



	/* gotta do the rehash code dun I? */
}

void serv_segv() {

	/* Thanks to Stskeeps and Unreal for this stuff :) */
	log("Uh Oh, Segmentation Fault.. Server Terminating");
	log("Details: Buffer: %s", recbuf);
	log("Approx Location: %s", segv_location);
	/* Broadcast it out! */
	globops(me.name,"Ohhh Crap, Server Terminating, Segmentation Fault. Buffer: %s, Approx Location %s", recbuf, segv_location);
	notice(s_Services, "Damn IT, Server Terminating, Segmentation Fault. Buffer: %s, Approx Location %s", recbuf, segv_location);
	sts("SQUIT %s",me.name);
	
	/* Should put some clean up code in here, but for the moment, just die... */
	printf("Ouch, Segmentation Fault, Server Terminating Check Log file for details\n");
	sleep(2);
	exit(0);	

	
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


void start()
{
	static int attempts = 0;
	
	segv_location = "start";
	TimerReset();
	init_server_hash();
	init_user_hash();
	init_chan_hash();
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
	segv_location = "login";
	sts("PASS %s", me.pass);
	sts("SERVER %s 1 :%s", me.name,me.infoline);
	sts("PROTOCTL :TOKEN");
	init_ServBot();

}

void init_ServBot()
{
	segv_location = "init_ServBot";
	sts("NICK %s 1 %d %s %s %s 0 :/msg %s \2HELP\2", s_Services, time(NULL),
		Servbot.user, Servbot.host, me.name, s_Services);
	AddUser(s_Services, Servbot.user, Servbot.host, me.name);
	sts(":%s MODE %s +Sqd", s_Services, s_Services);
	sts(":%s JOIN %s",s_Services ,me.chan);
	sts(":%s MODE %s +o %s",me.name,me.chan,s_Services);
	sts(":%s MODE %s +a %s",s_Services,me.chan,s_Services);
}

void *smalloc(long size)
{
	void *buf;
	
	segv_location = "smalloc";
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

char *strlower(char *s)
{
	char *t = s;
	while ((*t++ = tolower(*t)))
		;
	return s;
}
