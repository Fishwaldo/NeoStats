/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

#include "neostats.h"
#include "dotconf.h"
#include "conf.h"
#include "log.h"
#include "services.h"
#include "modules.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

static void cb_Server (char *arg, int configtype);
static void cb_Module (char *arg, int configtype);
static void cb_AuthModule (char *arg, int configtype);
#ifdef SQLSRV
static void cb_SqlConf (char *arg, int configtype);
#endif

/** @brief The list of modules to load
 */
static void *load_mods[NUM_MODULES];
void *load_auth_mods[NUM_MODULES];

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
	{"WANT_PRIVMSG", ARG_STR, cb_Server, 9},
	{"SERVICES_CHAN", ARG_STR, cb_Server, 10},
	{"LOAD_MODULE", ARG_STR, cb_Module, 0},
	{"LOAD_AUTH_MODULE", ARG_STR, cb_AuthModule, 17},
	{"ONLY_OPERS", ARG_STR, cb_Server, 11},
	{"NO_LOAD", ARG_STR, cb_Server, 12},
	{"BINDTO", ARG_STR, cb_Server, 13},
	{"LOGFILENAMEFORMAT", ARG_STR, cb_Server, 14},
	{"SERVER_NUMERIC", ARG_STR, cb_Server, 15},
	{"SETSERVERTIMES", ARG_STR, cb_Server, 16},
	{"SERVICEROOT", ARG_STR, cb_Server, 18},
	{"PROTOCOL", ARG_STR, cb_Server, 19},
#ifdef SQLSRV
	{"SQLSRV_AUTH", ARG_STR, cb_SqlConf, 0},
	{"SQLSRV_PORT", ARG_STR, cb_SqlConf, 1},
#endif
};

/** @brief Load configuration file
 *
 * Parses the configuration file
 *
 * @returns nothing
 */

int
ConfLoad ()
{
	/* Read in the Config File */
	printf ("Reading the Config File. Please wait.....\n");
	if (config_read (CONFIG_NAME, options) != 0) {
		printf ("***************************************************\n");
		printf ("*                  Error!                         *\n");
		printf ("*                                                 *\n");
		printf ("* Config file not found, or unable to open. Check *\n");
		printf ("* its location and permissions and try again.     *\n");
		printf ("*                                                 *\n");
		printf ("*             NeoStats NOT Started                *\n");
		printf ("***************************************************\n");
		return NS_FAILURE;
	}
	if (config.die) {
		printf ("\n-----> ERROR: Read the README file then edit %s <-----\n\n",CONFIG_NAME);
		nlog (LOG_CRITICAL, "Read the README file then edit %s",CONFIG_NAME);
		return NS_FAILURE;
	}
	if (config.error) {
		printf ("\n-----> CONFIG ERROR: Check log file for more information then edit %s <-----\n\n",CONFIG_NAME);
		nlog (LOG_CRITICAL, "CONFIG ERROR: Check log file for more information then edit %s",CONFIG_NAME);
		return NS_FAILURE;
	}
	printf ("Sucessfully loaded config file, booting NeoStats\n");
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
		for (i = 0; (i < NUM_MODULES) && (load_mods[i] != 0); i++) {
			if (!ircstrcasecmp (load_mods[i], arg)) {
				return;
			}
		}
		load_mods[i] = sstrdup (arg);
		dlog(DEBUG1, "Added Module %d :%s", i, (char *)load_mods[i]);
	}
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
cb_AuthModule (char *arg, int configtype)
{
	int i;

	SET_SEGV_LOCATION();
	for (i = 0; (i < NUM_MODULES) && (load_auth_mods[i] != 0); i++) {
		if (!ircstrcasecmp (load_auth_mods[i], arg)) {
			return;
		}
	}
	load_auth_mods[i] = sstrdup (arg);
	dlog(DEBUG1, "Added Auth Module %d :%s", i, (char *)load_auth_mods[i]);
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
			nlog(LOG_WARNING, "Invalid SQLSRV_AUTH syntax in config file (Username)");
			return;
		}
		if ((pass = strtok(NULL, "@")) == NULL) {
			nlog(LOG_WARNING, "Invalid SQLSRV_AUTH syntax in config file (Pass)");
			return;
		}
		if ((host = strtok(NULL, "")) == NULL) {
			nlog(LOG_WARNING, "Invalid SQLSRV_AUTH syntax in config file (Host)");
			return;
		}
		dlog(DEBUG1, "SqlSrv Uname %s Pass %s Host %s", uname, pass, host);
		rta_change_auth(uname, pass);
		strncpy(me.sqlhost, host, MAXHOST);
	} else if (configtype == 1) {
		me.sqlport = atoi(arg);
		if (me.sqlport == 0) {
			nlog(LOG_WARNING, "Invalid Port Specified for SQLSRV_PORT, Using Default");
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

	SET_SEGV_LOCATION();
	if(load_mods[0] == 0) {
		nlog (LOG_NORMAL, "No modules configured for loading"); 
	} else {
		nlog (LOG_NORMAL, "Loading configured modules"); 
		for (i = 0; (i < NUM_MODULES) && (load_mods[i] != 0); i++) {
			dlog(DEBUG1, "ConfLoadModules: Loading Module %s", (char *)load_mods[i]);
			if (load_module (load_mods[i], NULL)) {
				nlog (LOG_NORMAL, "Successfully Loaded Module %s", (char *)load_mods[i]);
			} else {
				nlog (LOG_WARNING, "Could Not Load Module %s, Please check above error Messages", (char *)load_mods[i]);
			}
			sfree(load_mods[i]);
		}
		nlog (LOG_NORMAL, "Completed loading configured modules"); 
	}
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
		config.port = atoi (arg);
	} else if (configtype == 2) {
		/* Connect To */
		strlcpy (me.uplink, arg, sizeof (me.uplink));
	} else if (configtype == 3) {
		/* Connect Pass */
		strlcpy (config.pass, arg, sizeof (config.pass));
	} else if (configtype == 4) {
		/* Server InfoLine */
		strlcpy (me.infoline, arg, sizeof (me.infoline));
	} else if (configtype == 5) {
		/* NetName */
		strlcpy (me.netname, arg, sizeof (me.netname));
	} else if (configtype == 6) {
		/* Reconnect time */
		config.r_time = atoi (arg);
	} else if (configtype == 9) {
		config.want_privmsg = 1;
	} else if (configtype == 10) {
		strlcpy (me.serviceschan, arg, sizeof (me.serviceschan));
	} else if (configtype == 11) {
		config.onlyopers = 1;
	} else if (configtype == 12) {
		config.die = 1;
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
	} else if (configtype == 16) {
		config.setservertimes = atoi (arg);
		/* Convert hours input to seconds */
		config.setservertimes = config.setservertimes * 60 * 60;
		/* limit value - really need to print error and quit */
		if(config.setservertimes <= 0) {
			config.setservertimes = (24 * 60 * 60);
		}
	} else if (configtype == 18) {
		char *nick;
		char *user;
		char *host;

		if (strstr(arg, "!")&& !strstr(arg, "@")) {
			nlog(LOG_WARNING, 
				"Invalid SERVICEROOT. Must be of the form nick!user@host, was %s",
				arg);
			config.error = 1;
		} else {
			nick = strtok(arg, "!");
			user = strtok(NULL, "@");
			host = strtok(NULL, "");
			strlcpy(config.rootuser.nick, nick, MAXNICK);
			strlcpy(config.rootuser.user, user, MAXUSER);
			strlcpy(config.rootuser.host, host, MAXHOST);
			config.rootuser.level = NS_ULEVEL_ROOT;
		}
	} else if (configtype == 19) {
		strlcpy(me.protocol,arg,MAXHOST);
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
