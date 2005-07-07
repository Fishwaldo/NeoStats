/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "confuse.h"
#include "conf.h"
#include "log.h"
#include "services.h"
#include "modules.h"
#include "dl.h"
#include "perlmod.h"
#define CONFIG_NAME		"neostats.conf"


/** @brief The list of modules to load
 */
static void *load_mods[NUM_MODULES];

static void cb_Module (char *arg);
static void set_config_values (cfg_t * cfg);
int cb_verify_chan (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_numeric (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_bind (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_file (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_log (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_mask (cfg_t * cfg, cfg_opt_t * opt);
int cb_noload (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_host (cfg_t * cfg, cfg_opt_t * opt);
int cb_verify_settime (cfg_t * cfg, cfg_opt_t * opt);

typedef struct validate_args {
	char name[BUFSIZE];
	cfg_validate_callback_t cb;
} validate_args;

/** @brief Core Configuration Items
 * 
 * Contains Configuration Items for the Core NeoStats service
 */

cfg_opt_t server_details[] = {
	CFG_STR ("Name", "neostats.nonetwork.org", CFGF_NONE),
	CFG_STR ("Info", "NeoStats 3.0 Services", CFGF_NONE),
	CFG_STR ("ServiceChannel", "#services", CFGF_NONE),
	CFG_INT ("ServerNumeric", 123, CFGF_NONE),
	CFG_STR ("BindTo", 0, CFGF_NONE),
	CFG_STR ("Protocol", "unreal32", CFGF_NONE)
};

cfg_opt_t options[] = {
	CFG_INT ("ReconnectTime", 10, CFGF_NONE),
	CFG_BOOL ("UsePrivmsg", cfg_false, CFGF_NONE),
	CFG_BOOL ("OperOnly", cfg_false, CFGF_NONE),
	CFG_INT ("ServerSettime", 1, CFGF_NONE),
	CFG_STR ("DatabaseType", "gdbm", CFGF_NONE),
	CFG_STR ("LogFileNameFormat", "-%m-%d", CFGF_NONE),
	CFG_STR ("RootNick", "NeoStats", CFGF_NONE),
	CFG_STR ("ServicesHost", "services.neostats.net", CFGF_NONE),
	CFG_BOOL ("NOLOAD", cfg_true, CFGF_NONE)
};

cfg_opt_t servers[] = {
	CFG_STR ("IpAddress", 0, CFGF_NONE),
	CFG_INT ("Port", 6667, CFGF_NONE),
	CFG_STR ("Password", 0, CFGF_NONE)
};

cfg_opt_t serviceroots[] = {
	CFG_STR ("Mask", 0, CFGF_NONE)
};

cfg_opt_t modules[] = {
	CFG_STR_LIST ("ModuleName", 0, CFGF_NONE)
};

cfg_opt_t fileconfig[] = {
	CFG_SEC ("ServerConfig", server_details, CFGF_NONE),
	CFG_SEC ("Options", options, CFGF_NONE),
/* XXX do we want to specify backup linking servers? */
#if 0
	CFG_SEC ("Servers", servers, CFGF_MULTI | CFGF_TITLE),
/* Multiple Service Roots? Only usefull if extauth is loaded */
	CFG_SEC ("ServiceRoots", serviceroots, CFGF_MULTI | CFGF_TITLE),
#endif
	CFG_SEC ("Servers", servers, CFGF_NONE),
	CFG_SEC ("ServiceRoots", serviceroots, CFGF_NONE),
	CFG_SEC ("Modules", modules, CFGF_NONE)
};

validate_args arg_validate[] = {
	{"ServerConfig|Name", &cb_verify_host},
	{"ServerConfig|ServiceChannel", &cb_verify_chan},
	{"ServerConfig|ServerNumeric", &cb_verify_numeric},
	{"ServerConfig|BindTo", &cb_verify_bind},
	{"ServerConfig|Protocol", &cb_verify_file},
	{"Options|ServerSettime", &cb_verify_settime},
	{"Options|DataBaseType", &cb_verify_file},
	{"Options|LogFileNameFormat", &cb_verify_log},
	{"Options|RootNick", &cb_verify_mask},
	{"Options|NOLOAD", &cb_noload},
	{"Servers|IpAddress", &cb_verify_host},
	{"ServiceRoots|Mask", &cb_verify_mask},
	{"Modules|ModuleName", &cb_verify_file}
};

/** @brief Load configuration file
 *
 * Parses the configuration file
 *
 * @returns nothing
 */

int
ConfLoad (void)
{
	cfg_t *cfg;
	int i, ret;
#if 0
	FILE *fp;
#endif
	/* Read in the Config File */
	printf ("Reading the Config File. Please wait.....\n");
	cfg = cfg_init (fileconfig, CFGF_NOCASE);
	for (i = 0; i < arraylen (arg_validate); i++) {
		cfg_set_validate_func (cfg, arg_validate[i].name, arg_validate[i].cb);
	}
#if 0
	fp = fopen ("neostats.conf.out", "wt");
	cfg_print (cfg, fp);
	fclose (fp);
#endif
	if ((ret = cfg_parse (cfg, CONFIG_NAME)) != 0) {
		printf ("***************************************************\n");
		printf ("*                  Error!                         *\n");
		printf ("*                                                 *\n");
		switch (ret) {
		case CFG_FILE_ERROR:
			printf ("*           Config file not found                 *\n");
			break;
		case CFG_PARSE_ERROR:
			printf ("*            Config Parse Error                   *\n");
			break;
		default:
			printf ("*               Uknown Error                      *\n");
			break;
		}
		printf ("*                                                 *\n");
		printf ("*             NeoStats NOT Started                *\n");
		printf ("***************************************************\n");
		cfg_free (cfg);
		return NS_FAILURE;
	}
	if (nsconfig.die) {
		printf ("\n-----> ERROR: Read the README file then edit %s <-----\n\n", CONFIG_NAME);
		nlog (LOG_CRITICAL, "Read the README file then edit %s", CONFIG_NAME);
		cfg_free (cfg);
		return NS_FAILURE;
	}
	if (nsconfig.error) {
		printf ("\n-----> CONFIG ERROR: Check log file for more information then edit %s <-----\n\n", CONFIG_NAME);
		nlog (LOG_CRITICAL, "CONFIG ERROR: Check log file for more information then edit %s", CONFIG_NAME);
		cfg_free (cfg);
		return NS_FAILURE;
	}
	printf ("-----------------------------------------------\n");
	set_config_values (cfg);
	cfg_free (cfg);
	printf ("Sucessfully loaded config file, booting NeoStats\n");
	printf ("If NeoStats does not connect, please check logs/neostats-<date>.log for further information\n");
	return NS_SUCCESS;
}

void
set_config_values (cfg_t * cfg)
{
	int i;
	/* Server name has a default */
	strlcpy (me.name, cfg_getstr (cfg, "ServerConfig|Name"), sizeof (me.name));
	
	/* Server Port has a default */
	me.port = cfg_getint (cfg, "Servers|Port");
	/* Connect To */
	if (cfg_size (cfg, "Servers|IpAddress") > 0) {
		strlcpy (me.uplink, cfg_getstr (cfg, "Servers|IpAddress"), sizeof (me.uplink));
	} else {
		printf ("ERROR: No Server was configured for Linking. Please fix this\n");
		exit (-1);
	}
	if (cfg_size (cfg, "Servers|Password") > 0) {
		/* Connect Pass */
		strlcpy (nsconfig.pass, cfg_getstr (cfg, "Servers|Password"), sizeof (nsconfig.pass));
	} else {
		printf ("ERROR: No Password was specified for Linking. Please fix this\n");
		exit (-1);
	}
	printf("NeoStats ServerName: %s\n", me.name);
	printf("Connecting To:       %s:%d\n", me.uplink, me.port);
	/* Server InfoLine has a default */
	strlcpy (me.infoline, cfg_getstr (cfg, "ServerConfig|Info"), sizeof (me.infoline));
	/* Service host has a default */
	strlcpy (me.servicehost, cfg_getstr (cfg, "Options|ServicesHost"), sizeof (me.servicehost));
	/* Reconnect time has a default */
	nsconfig.r_time = cfg_getint (cfg, "Options|ReconnectTime");
	/* want privmsg  has a default */
	nsconfig.want_privmsg = cfg_getbool (cfg, "Options|UsePrivMsg");
	/* service chan has a default */
	strlcpy (me.serviceschan, cfg_getstr (cfg, "ServerConfig|ServiceChannel"), sizeof (me.serviceschan));
	/* only opers has a default */
	nsconfig.onlyopers = cfg_getbool (cfg, "Options|OperOnly");
	/* vhost has no default, nor is it required */
	if (cfg_size (cfg, "ServerConfig|BindTo") > 1) {
		strlcpy (me.local, cfg_getstr (cfg, "ServerConfig|BindTo"), sizeof (me.local));
		printf("Source IP:          %s\n", me.local);
	}
	/* LogFile Format has a default */
	strlcpy (LogFileNameFormat, cfg_getstr (cfg, "Options|LogFileNameFormat"), MAX_LOGFILENAME);
	/* numeric has a default */
	me.numeric = cfg_getint (cfg, "ServerConfig|ServerNumeric");
	/* has a default */
	nsconfig.setservertimes = cfg_getint (cfg, "Options|ServerSettime") * 60 * 60;
	/* serviceroots has no default */
	if (cfg_size (cfg, "ServiceRoots|Mask") > 0) {
		char *nick;
		char *user;
		char *host;
		char *arg;
		/* already validate */
		arg = cfg_getstr (cfg, "ServiceRoots|Mask");
		nick = strtok (arg, "!");
		user = strtok (NULL, "@");
		host = strtok (NULL, "");
		strlcpy (nsconfig.rootuser.nick, nick, MAXNICK);
		strlcpy (nsconfig.rootuser.user, user, MAXUSER);
		strlcpy (nsconfig.rootuser.host, host, MAXHOST);
/*		nsconfig.rootuser.level = NS_ULEVEL_ROOT; */
		printf("ServiceRoot:         %s!%s@%s\n", nsconfig.rootuser.nick, nsconfig.rootuser.user, nsconfig.rootuser.host);
	} else {
		printf("\nWARNING: No ServiceRoot Entry Defined\n\n");
	}
	/* protocol is required, but defaults to unreal32 */
	strlcpy (me.protocol, cfg_getstr (cfg, "ServerConfig|Protocol"), MAXHOST);
	/* dbm has a default */
	strlcpy (me.dbm, cfg_getstr (cfg, "Options|DataBaseType"), MAXHOST);
	/* has a default */
	strlcpy (me.rootnick, cfg_getstr (cfg, "Options|RootNick"), MAXNICK);
	/* now load the modules */
	printf("Modules Loaded:\n");
	for (i = 0; i < cfg_size(cfg, "Modules|ModuleName"); i++) {
		cb_Module(cfg_getnstr(cfg, "Modules|ModuleName", i));
		printf("                     %s\n", cfg_getnstr(cfg, "Modules|ModuleName", i));
	}	
	printf ("-----------------------------------------------\n");
}



/** @brief Load the modules 
 *
 * Actually load the modules that were found in the config file
 *
 * @returns 1 on success, -1 when a module failed to load
 * @bugs if a single module fails to load, it stops trying to load any other modules
 */

int
ConfLoadModules (void)
{
	int i;

	SET_SEGV_LOCATION ();
	if (load_mods[0] == 0) {
		nlog (LOG_NORMAL, "No modules configured for loading");
	} else {
		nlog (LOG_NORMAL, "Loading configured modules");
		for (i = 0; (i < NUM_MODULES) && (load_mods[i] != 0); i++) {
			dlog (DEBUG1, "ConfLoadModules: Loading Module %s", (char *) load_mods[i]);
			if (ns_load_module (load_mods[i], NULL)) {
				nlog (LOG_NORMAL, "Loaded module %s", (char *) load_mods[i]);
			} else {
				nlog (LOG_WARNING, "Failed to load module %s. Please check above error messages", (char *) load_mods[i]);
			}
			ns_free (load_mods[i]);
		}
		nlog (LOG_NORMAL, "Completed loading configured modules");
	}
	return NS_SUCCESS;
}



int
cb_verify_chan (cfg_t * cfg, cfg_opt_t * opt)
{

	if (ValidateChannel (opt->values[0]->string) == NS_FAILURE) {
		cfg_error (cfg, "Invalid Channel Name %s for Option %s", opt->values[0]->string, opt->name);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

int
cb_verify_numeric (cfg_t * cfg, cfg_opt_t * opt)
{
	long int num = opt->values[0]->number;
	/* limit value - really need to print error and quit */
	if ((num <= 0) || (num > 254)) {
		cfg_error (cfg, "Numeric Value %d is out of Range for %s", num, opt->name);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

int
cb_verify_bind (cfg_t * cfg, cfg_opt_t * opt)
{
	OS_SOCKET s;
	struct hostent *hp;
	/* test if we can bind */
	s = os_sock_socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		cfg_error(cfg, "Error Testing Bind Setting.");
		return CFG_PARSE_ERROR;
	}
	os_memset(&me.lsa, 0, sizeof(me.lsa));
	if ((hp = gethostbyname(opt->values[0]->string)) == NULL) {
		cfg_error(cfg, "Unable to Bind to Address %s for Option %s: %s", opt->values[0]->string, opt->name, strerror(errno));
		return CFG_PARSE_ERROR;
	} else {
		os_memcpy((char *)&me.lsa.sin_addr, hp->h_addr, hp->h_length);
		me.lsa.sin_family = hp->h_addrtype;
		if (os_sock_bind(s, (struct sockaddr *) &me.lsa, sizeof(me.lsa)) == SOCKET_ERROR) {
			cfg_error(cfg, "Unable to Bind to Address %s for Option %s: %s", opt->values[0]->string, opt->name, os_sock_getlasterrorstring());
			return CFG_PARSE_ERROR;
		}
		/* if we get here, the socket is ok*/
		os_close(s);
/* 		ns_free(hp); */
		me.dobind = 1;
	}
	
	return CFG_SUCCESS;
}

int
cb_verify_file (cfg_t * cfg, cfg_opt_t * opt)
{
	char *file = opt->values[0]->string;
	char buf2[MAXPATH];
	struct stat buf;
	ircsnprintf(buf2, MAXPATH, "%s/%s%s", MOD_PATH, file, MOD_STDEXT);
	if (stat(buf2, &buf) == -1) {
#ifdef USE_PERL
		ircsnprintf(buf2, MAXPATH, "%s/%s%s", MOD_PATH, file, MOD_PERLEXT);
		if (stat(buf2, &buf) == -1) {
#endif
			cfg_error(cfg, "File %s Specified in Option %s is Invalid: %s", buf2, opt->name, strerror(errno));
			return CFG_PARSE_ERROR;
#ifdef USE_PERL
		}
#endif
	}
	if (!S_ISREG(buf.st_mode)) {
		cfg_error(cfg, "File %s Specified in Option %s is Invalid: Not a Regular File", buf2, opt->name);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

int
cb_verify_log (cfg_t * cfg, cfg_opt_t * opt)
{
	return CFG_SUCCESS;
}

int
cb_verify_mask (cfg_t * cfg, cfg_opt_t * opt)
{
	char *value = opt->values[0]->string;
	if (strstr (value, "!") && !strstr (value, "@")) {
		cfg_error (cfg, "Invalid HostMask %s for %s", value, opt->name);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

int
cb_noload (cfg_t * cfg, cfg_opt_t * opt)
{
	if (opt->values[0]->boolean == cfg_true) {
		cfg_error (cfg, "Error. You didn't edit NeoStats.conf");
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

int
cb_verify_host (cfg_t * cfg, cfg_opt_t * opt)
{
	/* this should actually be a validate ip as well */
	if (ValidateHost (opt->values[0]->string) == NS_FAILURE) {
		cfg_error (cfg, "Invalid HostName %s for Option %s", opt->values[0]->string, opt->name);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

int
cb_verify_settime (cfg_t * cfg, cfg_opt_t * opt)
{
	long int time = opt->values[0]->number;

	if (time <= 0) {
		cfg_error (cfg, "SetTime Value of %d is out of range for %s", time, opt->name);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief prepare Modules defined in the config file
 *
 * When the config file encounters directives to Load Modules, it calls this function which prepares to load the modules( but doesn't actually load them )
 *
 * @param arg the module name in this case
 * @param configtype an index of what config item is currently being processed. Ignored
 * @returns Nothing
 */
void
cb_Module (char *arg)
{
	int i;

	SET_SEGV_LOCATION ();
	if (!nsconfig.modnoload) {
		for (i = 0; (i < NUM_MODULES) && (load_mods[i] != 0); i++) {
			if (!ircstrcasecmp (load_mods[i], arg)) {
				return;
			}
		}
		load_mods[i] = sstrdup (arg);
	}
}



