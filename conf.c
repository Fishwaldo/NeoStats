/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
** $Id$
*/

#include "stats.h"
#include "dotconf.h"
#include "conf.h"
#include "dl.h"
#include "log.h"
#include "services.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

static void cb_Server (char *, int);
static void cb_Module (char *, int);
#ifdef SQLSRV
static void cb_SqlConf (char *, int);
#endif

/** @brief The list of modules to load
 */
static void *load_mods[NUM_MODULES];

/** @brief Core Configuration Items
 * 
 * Contains Configuration Items for the Core NeoStats service
 */
static config_option options[] = {
	{"SERVER_NAME", ARG_STR, cb_Server, 0},
	{"SERVER_PORT", ARG_STR, cb_Server, 1},
	{"CONNECT_TO", ARG_STR, cb_Server, 2},
	{"CONNECT_PASS", ARG_STR, cb_Server, 3},
	{"SERVER_INFOLINE", ARG_STR, cb_Server, 4},
	{"STATSERV_NETNAME", ARG_STR, cb_Server, 5},
	{"RECONNECT_TIME", ARG_STR, cb_Server, 6},
	{"NEOSTAT_HOST", ARG_STR, cb_Server, 7},
	{"NEOSTAT_USER", ARG_STR, cb_Server, 8},
	{"WANT_PRIVMSG", ARG_STR, cb_Server, 9},
	{"SERVICES_CHAN", ARG_STR, cb_Server, 10},
	{"LOAD_MODULE", ARG_STR, cb_Module, 0},
	{"ONLY_OPERS", ARG_STR, cb_Server, 11},
	{"NO_LOAD", ARG_STR, cb_Server, 12},
	{"BINDTO", ARG_STR, cb_Server, 13},
	{"LOGFILENAMEFORMAT", ARG_STR, cb_Server, 14},
	{"SERVER_NUMERIC", ARG_STR, cb_Server, 15},
#ifdef SQLSRV
	{"SQLSRV_AUTH", ARG_STR, cb_SqlConf, 0},
	{"SQLSRV_PORT", ARG_STR, cb_SqlConf, 1},
#endif
};

/** @brief initialize the configuration parser
 *
 * Currently does nothing
 *
 * @return Nothing
 */

void
init_conf ()
{
}

/** @brief Load the Config file
 *
 * Parses the Configuration File and optionally loads the external authentication libary
 *
 * @returns Nothing
 */

int
ConfLoad ()
{
	/* Read in the Config File */
	printf ("Reading the Config File. Please wait.....\n");
	if (!config_read (CONFIG_NAME, options) == 0) {
		printf ("***************************************************\n");
		printf ("*                  Error!                         *\n");
		printf ("*                                                 *\n");
		printf ("* Config File not found, or Unable to Open        *\n");
		printf ("* Please check its Location, and try again        *\n");
		printf ("*                                                 *\n");
		printf ("*             NeoStats NOT Started                *\n");
		printf ("***************************************************\n");
		return NS_FAILURE;
	}
	printf ("Sucessfully Loaded Config File, Now Booting NeoStats\n");

	/* if all bots should join the chan */
	if (GetConf ((void *) &me.allbots, CFGINT, "AllBotsJoinChan") <= 0) {
		me.allbots = 0;
	}
	if (GetConf ((void *) &me.pingtime, CFGINT, "PingServerTime") <= 0) {
		me.pingtime = 120;
	}
	return NS_SUCCESS;
}


/** @brief prepare Modules defined in the config file
 *
 * When the config file encounters directives to Load Modules, it calls this function which prepares to load the modules (but doesn't actually load them)
 *
 * @param arg the module name in this case
 * @param configtype an index of what config item is currently being processed. Ignored
 * @returns Nothing
 */
void
cb_Module (char *arg, int configtype)
{
	int i;

	SET_SEGV_LOCATION();
	if (!config.modnoload) {
		for (i = 1; (i < NUM_MODULES) && (load_mods[i] != 0); i++) {
			if (!strcasecmp (load_mods[i], arg)) {
				return;
			}
		}
		load_mods[i] = sstrdup (arg);
		nlog (LOG_DEBUG1, LOG_CORE, "Added Module %d :%s", i, (char *)load_mods[i]);
	}
}


#ifdef SQLSRV
/** @brief prepare SqlAuthentication defined in the config file
 *
 * load the Sql UserName/Password and Host if we are using SQL Server option
 *
 * @param arg the module name in this case
 * @param configtype an index of what config item is currently being processed. Ignored
 * @returns Nothing
 */

void
cb_SqlConf (char *arg, int configtype)
{
	char *uname, *pass, *host;
	SET_SEGV_LOCATION();
	if (configtype == 0) {
		if ((uname = strtok(arg, "!")) == NULL) {
			nlog(LOG_WARNING, LOG_CORE, "Invalid SQLSRV_AUTH syntax in config file (Username)");
			return;
		}
		if ((pass = strtok(NULL, "@")) == NULL) {
			nlog(LOG_WARNING, LOG_CORE, "Invalid SQLSRV_AUTH syntax in config file (Pass)");
			return;
		}
		if ((host = strtok(NULL, "")) == NULL) {
			nlog(LOG_WARNING, LOG_CORE, "Invalid SQLSRV_AUTH syntax in config file (Host)");
			return;
		}
		nlog(LOG_DEBUG1, LOG_CORE, "SqlSrv Uname %s Pass %s Host %s", uname, pass, host);
		rta_change_auth(uname, pass);
		strncpy(me.sqlhost, host, MAXHOST);
	} else if (configtype == 1) {
		me.sqlport = atoi(arg);
		if (me.sqlport == 0) {
			nlog(LOG_WARNING, LOG_CORE, "Invalid Port Specified for SQLSRV_PORT, Using Default");
			me.sqlport = 8888;
		}
	}
	
}
#endif
/** @brief Load the modules 
 *
 * Actually load the modules that were found in the config file
 *
 * @returns 1 on success, -1 when a module failed to load
 * @bugs if a single module fails to load, it stops trying to load any other modules
 */

int
ConfLoadModules ()
{
	int i;
	int rval;

	SET_SEGV_LOCATION();
	if(load_mods[1] == 0) {
		nlog (LOG_NORMAL, LOG_CORE, "No modules configured for loading"); 
		return NS_SUCCESS;
	}
	nlog (LOG_NORMAL, LOG_CORE, "Loading configured modules"); 
	for (i = 1; (i < NUM_MODULES) && (load_mods[i] != 0); i++) {
		nlog (LOG_DEBUG1, LOG_CORE, "ConfLoadModules: Loading Module %s", (char *)load_mods[i]);
		rval = load_module (load_mods[i], NULL);
		if (rval == NS_SUCCESS) {
			nlog (LOG_NORMAL, LOG_CORE, "Successfully Loaded Module %s", (char *)load_mods[i]);
		} else {
			nlog (LOG_WARNING, LOG_CORE, "Could Not Load Module %s, Please check above error Messages", (char *)load_mods[i]);
		}
	}
	nlog (LOG_NORMAL, LOG_CORE, "Completed loading configured modules"); 
	return NS_SUCCESS;
}


/** @brief Process config file items
 *
 * Processes the config file and sets up the variables. No Error Checking is performed :(
 *
 * @param arg the variable value as a string
 * @param configtype the index of the variable being called now
 * @returns Nothing
 */

void
cb_Server (char *arg, int configtype)
{
	if (configtype == 0) {
		/* Server name */
		strlcpy (me.name, arg, sizeof (me.name));
	} else if (configtype == 1) {
		/* Server Port */
		me.port = atoi (arg);
	} else if (configtype == 2) {
		/* Connect To */
		strlcpy (me.uplink, arg, sizeof (me.uplink));
	} else if (configtype == 3) {
		/* Connect Pass */
		strlcpy (me.pass, arg, sizeof (me.pass));
	} else if (configtype == 4) {
		/* Server InfoLine */
		strlcpy (me.infoline, arg, sizeof (me.infoline));
	} else if (configtype == 5) {
		/* NetName */
		strlcpy (me.netname, arg, sizeof (me.netname));
	} else if (configtype == 6) {
		/* Reconnect time */
		me.r_time = atoi (arg);
	} else if (configtype == 7) {
		/* NeoStat Host */
		strlcpy (me.host, arg, MAXHOST);
	} else if (configtype == 8) {
		/* NeoStat User */
		strlcpy (me.user, arg, MAXUSER);
	} else if (configtype == 9) {
		me.want_privmsg = 1;
	} else if (configtype == 10) {
		strlcpy (me.chan, arg, sizeof (me.chan));
	} else if (configtype == 11) {
		me.onlyopers = 1;
	} else if (configtype == 12) {
		me.die = 1;
	} else if (configtype == 13) {
		strlcpy (me.local, arg, sizeof (me.local));
	} else if (configtype == 14) {
		strlcpy(LogFileNameFormat,arg,MAX_LOGFILENAME);
	} else if (configtype == 15) {
		me.numeric = atoi (arg);
		/* limit value - really need to print error and quit */
		if(me.numeric<=0)
			me.numeric=1;
		if(me.numeric>254)
			me.numeric=254;
	}

}

/** @brief Rehash Function
 *
 * Called when we recieve a rehash signal. Does nothing atm
 *
 * @returns Nothing
 */

void
rehash ()
{
	/* nothing, yet */
}
