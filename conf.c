/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: conf.c,v 1.5 2000/04/22 04:45:07 fishwaldo Exp $
*/

#include "stats.h"
#include "dotconf.h"
#include "dl.h"

static void cb_Server(char *, int);
static void cb_Module(char *, int);
static void *load_mods[NUM_MODULES];
static int done_mods;

static config_option options[] =
{ 
{ "SERVER_NAME", ARG_STR, cb_Server, 0},
{ "SERVER_PORT", ARG_STR, cb_Server, 1},
{ "CONNECT_TO", ARG_STR, cb_Server, 2},
{ "CONNECT_PASS", ARG_STR, cb_Server, 3},
{ "SERVER_INFOLINE", ARG_STR, cb_Server, 4},
{ "STATSERV_NETNAME", ARG_STR, cb_Server, 5},
{ "RECONNECT_TIME", ARG_STR, cb_Server, 6},
{ "NEOSTAT_HOST", ARG_STR, cb_Server, 7},
{ "NEOSTAT_USER", ARG_STR, cb_Server, 8},
{ "WANT_PRIVMSG", ARG_STR, cb_Server, 9},
{ "SERVICES_CHAN", ARG_STR, cb_Server, 10},
{ "MODULE_PATH", ARG_STR, cb_Server, 11},
{ "LOAD_MODULE", ARG_STR, cb_Module, 0},
{ "ONLY_OPERS", ARG_STR, cb_Server, 12}
};




void strip(char *line)
{
	char *c;
	if ((c = strchr(line, '\n')))	*c = '\0';
	if ((c = strchr(line, '\r')))	*c = '\0';
}

void ConfLoad() {
/* Read in the Config File */
printf("Reading the Config File. Please wait.....\n");
if (!config_read("stats.cfg", options) == 0 ) {
	printf("***************************************************\n");
	printf("*                  Error!                         *\n");
	printf("*                                                 *\n");
	printf("* Config File not found, or Unable to Open        *\n");
	printf("* Please check its Location, and try again        *\n");
	printf("*                                                 *\n");
	printf("*             NeoStats NOT Started                *\n");
	printf("***************************************************\n");
	exit(0);
}
printf("Sucessfully Loaded Config File, Now Booting NeoStats\n");
done_mods = 0;
}
void cb_Module(char *arg, int configtype) {
	int i;
		segv_loc("cb_Module");
		for (i = 1; (i < NUM_MODULES) && (load_mods[i] != 0); i++) { 
			if (!strcasecmp(load_mods[i], arg)) {
				return;
			}
		}
		load_mods[i] = sstrdup(arg);
		log("Added Module %d :%s", i, load_mods[i]);
}

int init_modules() {
	int i;
	int rval;
	
	segv_loc("init_modules");
	for (i = 1; (i < NUM_MODULES) && (load_mods[i] !=0); i++) {
#ifdef DEBUG
		log("Loading Module %s", load_mods[i]);
#endif
		rval = load_module(load_mods[i], NULL);
		if (!rval) {
			log("Successfully Loaded Module %s", load_mods[i]);
		} else {
			log("Could Not Load Module %s, Please check above error Messages", load_mods[i]);
			return -1;
		}
	}
return 1;
}
void cb_Server(char *arg, int configtype) {

		if (configtype == 0) {
			/* Server name */
			memcpy(me.name, arg, sizeof(me.name));
		} else if (configtype == 1) {
			/* Server Port */
			me.port = atoi(arg);
		} else if (configtype == 2) {
			/* Connect To */
			memcpy(me.uplink, arg, sizeof(me.uplink));
		} else if (configtype == 3) {
			/* Connect Pass */
			memcpy(me.pass, arg, sizeof(me.pass));
                } else if (configtype == 4) {
			/* Server InfoLine */
                        memcpy(me.infoline, arg, sizeof(me.infoline));
                } else if (configtype == 5) {
			/* NetName */
                        memcpy(me.netname, arg, sizeof(me.netname));
		} else if (configtype == 6) {
			/* Reconnect time */
			me.r_time = atoi(arg);
		} else if (configtype == 7) {
			/* NeoStat Host */
			memcpy(Servbot.host,arg,sizeof(Servbot.host));
		} else if (configtype == 8) {
			/* NeoStat User */
			memcpy(Servbot.user,arg,sizeof(Servbot.user));
		} else if (configtype == 9) {
			me.want_privmsg = 1;
		} else if (configtype == 10) {
			memcpy(me.chan,arg, sizeof(me.chan));
		} else if (configtype == 11) {
			memcpy(me.modpath, arg, sizeof(me.chan));
			add_ld_path(me.modpath);
		} else if (configtype == 12) {
			me.onlyopers = 1;
		}

}

/*		old Config stuff, that will have to be moved to Modules
		} else if (!strcmp(s, "LAG_NOTICE")) {
			s = strtok(NULL, "\n");
			me.lag_time = atoi(s);
		} else if (!strcmp(s, "ICQ_HOST")) {
			s = strtok(NULL, "\n");
			strip(s);
			memcpy(IcqServ.host, s, sizeof(IcqServ.host));
		} else if (!strcmp(s, "ICQ_PASSWORD")) {
			s = strtok(NULL, "\n");
			strip(s);
			memcpy(IcqServ.passwd, s, sizeof(IcqServ.passwd));
		} else if (!strcmp(s, "ICQ_USER")) {
			s = strtok(NULL, "\n");
			strip(s);
			memcpy(IcqServ.user, s, sizeof(IcqServ.user));
		} else if (!strcmp(s, "ICQ_SERVER")) {
			s = strtok(NULL, "\n");
			strip(s);
			memcpy(IcqServ.server, s, sizeof(IcqServ.server));
		} else if (!strcmp(s, "ICQ_UIN")) {
			s = strtok(NULL, "\n");
			strip(s);
			IcqServ.uin = atoi(s);
		} else if (!strcmp(s, "ICQ_PORT")) {
			s = strtok(NULL, "\n");
			strip(s);
			IcqServ.port = atoi(s);
		} else if (!strncmp(s, "ENABLE_SPAM", 11)) {
			me.enable_spam = 1;
		} else if (!strncmp(s, "ONLYOPERS", 9)) {
			me.onlyopers = 1;
		} else if (!strcmp(s, "STATSERV_NICK")) {
			s = strtok(NULL, "\n");
			strip(s);
			memcpy(s_StatServ, s, MAXNICK);
		} else if (!strcmp(s, "SERVICES_NAME")) {
			s = strtok(NULL, "\n");
			strip(s);
			memcpy(me.services_name, s, MAXHOST);
		} else if (!strcmp(s, "SEND_EXTREME_LAG_NOTICES_TO")) {
			me.send_extreme_lag_notices = 1;
		} else if (!strcmp(s, "PUBLISH_LAG")) {
			me.noticelag = 1;

*/

void rehash()
{
	/* nothing, yet */
}
