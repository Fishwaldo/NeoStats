/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
** $Id: conf.c,v 1.23 2003/04/21 10:30:37 fishwaldo Exp $
*/

#include "stats.h"
#include "dotconf.h"
#include "conf.h"
#include "dl.h"
#include "log.h"

static void cb_Server(char *, int);
static void cb_Module(char *, int);
/** @brief The list of modules to load
 */
static void *load_mods[NUM_MODULES];
/** @brief Hrm?
 */
static int done_mods;

/** @brief Core Configuration Items
 * 
 * Contains Configuration Items for the Core NeoStats service
 */
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
{ "LOAD_MODULE", ARG_STR, cb_Module, 0},
{ "ONLY_OPERS", ARG_STR, cb_Server, 11},
{ "NO_LOAD", ARG_STR, cb_Server, 12},
{ "BINDTO", ARG_STR, cb_Server, 13}
};


/** @brief Initilize the configuration parser
 *
 * Currently does nothing
 *
 * @return Nothing
 */

void init_conf() {
}

/** @brief strip newlines carriage returns
 *
 * removes newlines and carriage returns from a string
 *
 * @param line the line to strip (warning, Modfied!)
 * @retval line the stripped line
 */

void strip(char *line)
{
	char *c;
	if ((c = strchr(line, '\n')))	*c = '\0';
	if ((c = strchr(line, '\r')))	*c = '\0';
}


/** @brief Load the Config file
 *
 * Parses the Configuration File and optionally loads the external authentication libary
 *
 * @returns Nothing
 */


void ConfLoad() {
	/* Read in the Config File */
	printf("Reading the Config File. Please wait.....\n");
	if (!config_read("neostats.cfg", options) == 0 ) {
		printf("***************************************************\n");
		printf("*                  Error!                         *\n");
		printf("*                                                 *\n");
		printf("* Config File not found, or Unable to Open        *\n");
		printf("* Please check its Location, and try again        *\n");
		printf("*                                                 *\n");
		printf("*             NeoStats NOT Started                *\n");
		printf("***************************************************\n");
		/* no need to call do_exit, we havn't even started! */
		exit(0);
	}
	printf("Sucessfully Loaded Config File, Now Booting NeoStats\n");
	#ifdef EXTAUTH
		load_module("extauth", NULL);
	#endif

	/* if all bots should join the chan */
	if (GetConf((void *)&me.allbots, CFGBOOL, "AllBotsJoinChan") <= 0) {
		me.allbots = 0;
	}
	done_mods = 0;
}


/** @brief prepare Modules defined in the config file
 *
 * When the config file encounters directives to Load Modules, it calls this function which prepares to load the modules (but doesn't actually load them)
 *
 * @param arg the module name in this case
 * @param configtype a index of what config item is currently being processed. Ignored
 * @returns Nothing
 */

void cb_Module(char *arg, int configtype) {
	int i;
		strcpy(segv_location, "cb_Module");
		if (!config.modnoload) {
			for (i = 1; (i < NUM_MODULES) && (load_mods[i] != 0); i++) { 
				if (!strcasecmp(load_mods[i], arg)) {
					return;
				}
			}
			load_mods[i] = sstrdup(arg);
			nlog(LOG_NORMAL, LOG_CORE, "Added Module %d :%s", i, load_mods[i]);
		}
}

/** @brief Load the modules 
 *
 * Actually load the modules that were found in the config file
 *
 * @returns 1 on success, -1 when a module failed to load
 * @bugs if a single module fails to load, it stops trying to load any other modules
 */

int init_modules() {
	int i;
	int rval;
	
	strcpy(segv_location,"init_modules");
	
	for (i = 1; (i < NUM_MODULES) && (load_mods[i] !=0); i++) {
		nlog(LOG_DEBUG1, LOG_CORE, "Loading Module %s", load_mods[i]);
		rval = load_module(load_mods[i], NULL);
		if (!rval) {
			nlog(LOG_NORMAL, LOG_CORE, "Successfully Loaded Module %s", load_mods[i]);
		} else {
			nlog(LOG_WARNING, LOG_CORE, "Could Not Load Module %s, Please check above error Messages", load_mods[i]);
		}
	}
	return 1;
}


/** @brief Process config file items
 *
 * Processes the config file and sets up the variables. No Error Checking is performed :(
 *
 * @param arg the variable value as a string
 * @param configtype the index of the variable being called now
 * @returns Nothing
 */
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
			me.onlyopers = 1;
		} else if (configtype == 12) {
			me.die = 1;
		} else if (configtype == 13) {
			memcpy(me.local, arg, sizeof(me.local));
		}

}


/** @brief Rehash Function
 *
 * Called when we recieve a rehash signal. Does nothing atm
 *
 * @returns Nothing
 */

void rehash()
{
	/* nothing, yet */
}
