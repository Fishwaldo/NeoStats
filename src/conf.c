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

/* #define USE_CONFUSE */

#include "neostats.h"
#ifdef USE_CONFUSE
#include "confuse.h"
#else
#include "dotconf.h"
#endif
#include "conf.h"
#include "log.h"
#include "services.h"
#include "modules.h"

#define CONFIG_NAME		"neostats.conf"

static void cb_Server( char *arg, int configtype );
static void cb_Module( char *arg, int configtype );

/** @brief The list of modules to load
 */
static void *load_mods[NUM_MODULES];

#ifdef USE_CONFUSE
static void set_config_values(cfg_t *cfg);
int cb_verify_chan(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_numeric(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_bind(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_file(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_log(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_mask(cfg_t *cfg, cfg_opt_t *opt);
int cb_noload(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_host(cfg_t *cfg, cfg_opt_t *opt);
int cb_verify_settime(cfg_t *cfg, cfg_opt_t *opt);

typedef struct validate_args {
    char name[BUFSIZE];
    cfg_validate_callback_t cb;
} validate_args;

/** @brief Core Configuration Items
 * 
 * Contains Configuration Items for the Core NeoStats service
 */

cfg_opt_t server_details[] = {
    CFG_STR("Name", "neostats.nonetwork.org", CFGF_NONE),
    CFG_STR("Info", "NeoStats 3.0 Services", CFGF_NONE),
    CFG_STR("ServiceChannel", "#services", CFGF_NONE),
    CFG_INT("UnrealNumeric", 123, CFGF_NONE),
    CFG_STR("BindTo", 0, CFGF_NONE),
    CFG_STR("Protocol", "unreal32", CFGF_NONE)
};

cfg_opt_t options[] = {
    CFG_INT("ReconnectTime", 10, CFGF_NONE),
    CFG_BOOL("UsePrivmsg", cfg_false, CFGF_NONE),
    CFG_BOOL("OperOnly", cfg_false, CFGF_NONE),
    CFG_INT("UnrealSettime", 1, CFGF_NONE),
    CFG_STR("DatabaseType", "gdbm", CFGF_NONE),
    CFG_STR("LogFileNameFormat", "-%m-%d", CFGF_NONE),
    CFG_STR("RootNick", "NeoStats", CFGF_NONE),
    CFG_STR("ServicesHost", "services.neostats.net", CFGF_NONE),
    CFG_BOOL("NOLOAD", cfg_true, CFGF_NONE)
};

cfg_opt_t servers[] = {
    CFG_STR("IpAddress", 0, CFGF_NONE),
    CFG_INT("Port", 6667, CFGF_NONE),
    CFG_STR("Password", 0, CFGF_NONE)
};

cfg_opt_t serviceroots[] = {
    CFG_STR("Mask", 0, CFGF_NONE)
};

cfg_opt_t modules[] = {
    CFG_STR("Name", 0, CFGF_MULTI)
};

cfg_opt_t fileconfig[] = {
    CFG_SEC("ServerConfig", server_details, CFGF_NONE),
    CFG_SEC("Options", options, CFGF_NONE),
#if 0
    CFG_SEC("Servers", servers, CFGF_MULTI | CFGF_TITLE),
    CFG_SEC("ServiceRoots", serviceroots, CFGF_MULTI | CFGF_TITLE),
#endif
    CFG_SEC("Servers", servers, CFGF_NONE),
    CFG_SEC("ServiceRoots", serviceroots, CFGF_NONE),
    CFG_SEC("Modules", modules, CFGF_NONE)
};

validate_args arg_validate[] = {
    {"ServerConfig|ServiceChannel", &cb_verify_chan },
    {"ServerConfig|UnrealNumeric", &cb_verify_numeric },
    {"ServerConfig|BindTo", &cb_verify_bind },
    {"ServerConfig|Protocol", &cb_verify_file },
    {"Options|UnrealSettime", &cb_verify_settime },
    {"Options|DataBaseType", &cb_verify_file },
    {"Options|LogFileNameFormat", &cb_verify_log },
    {"Options|RootNick", &cb_verify_mask },
    {"Options|NOLOAD", &cb_noload },
    {"Servers|IpAddress", &cb_verify_host },
    {"ServiceRoots|Mask", &cb_verify_mask },
    {"Modules|Name", &cb_verify_file}
};

/** @brief Load configuration file
 *
 * Parses the configuration file
 *
 * @returns nothing
 */

int ConfLoad( void )
{
    cfg_t *cfg;
    int i, ret;
	FILE *fp;

	/* Read in the Config File */
	printf( "Reading the Config File. Please wait.....\n" );
    cfg = cfg_init(fileconfig, CFGF_NOCASE);
    for ( i = 0; i < arraylen(arg_validate); i++) {
printf("%d %s\n", i, arg_validate[i].name);
        cfg_set_validate_func(cfg, arg_validate[i].name, arg_validate[i].cb);
    } 
    fp = fopen("neostats.conf.out", "w");
    cfg_print(cfg, fp);
    fclose(fp);
	if((ret =  cfg_parse(cfg, CONFIG_NAME)) != 0 ) {
		printf( "***************************************************\n" );
		printf( "*                  Error!                         *\n" );
		printf( "*                                                 *\n" );
    	switch (ret) {
	        case CFG_FILE_ERROR:
	                printf( "* Config file not found                           *\n" );
                    break;
            case CFG_PARSE_ERROR:
                    printf( "* Config Parse Error                              *\n" );
                    break;
            default:
                    printf( "* Uknown Error                                    *\n" );
                    break;
        }  
		printf( "*                                                 *\n" );
		printf( "*             NeoStats NOT Started                *\n" );
		printf( "***************************************************\n" );
        cfg_free(cfg);
		return NS_FAILURE;
	}
	if( nsconfig.die ) {
		printf( "\n-----> ERROR: Read the README file then edit %s <-----\n\n",CONFIG_NAME );
		nlog( LOG_CRITICAL, "Read the README file then edit %s",CONFIG_NAME );
        cfg_free(cfg);
		return NS_FAILURE;
	}
	if( nsconfig.error ) {
		printf( "\n-----> CONFIG ERROR: Check log file for more information then edit %s <-----\n\n",CONFIG_NAME );
		nlog( LOG_CRITICAL, "CONFIG ERROR: Check log file for more information then edit %s",CONFIG_NAME );
		cfg_free(cfg);
		return NS_FAILURE;
	}
	printf( "Sucessfully loaded config file, booting NeoStats\n" );
	set_config_values(cfg);
	cfg_free(cfg);
	return NS_SUCCESS;
}

void set_config_values(cfg_t *cfg) {
		/* Server name has a default*/
printf("hello\n");
		strlcpy( me.name, cfg_getstr(cfg, "ServerConfig|Name"), sizeof( me.name ) );
printf("ehh\n");
		/* Server Port has a default*/
		me.port = cfg_getint(cfg, "Servers|Port");
		/* Connect To */
        if (cfg_size(cfg, "Servers|IpAddress") > 0) {
    		strlcpy( me.uplink, cfg_getstr(cfg,"Servers|IpAddress"), sizeof( me.uplink ) );
        } else {
            printf("no server\n");
            exit(-1);
        }
		if (cfg_size(cfg, "Servers|Password")> 0) {
    		/* Connect Pass */
	    	strlcpy( nsconfig.pass, cfg_getstr(cfg,"Servers|Password"), sizeof( nsconfig.pass ) );
        } else {
            exit(-1);
        }
		/* Server InfoLine has a default*/
		strlcpy( me.infoline, cfg_getstr(cfg,"ServerConfig|Info"), sizeof( me.infoline ) );
		/* Service host has a default*/
		strlcpy( me.servicehost, cfg_getstr(cfg,"Options|ServicesHost"), sizeof( me.servicehost ) );
		/* Reconnect time has a default*/
		nsconfig.r_time = cfg_getint(cfg, "Options|ReconnectTime");
        /* want privmsg  has a default */
		nsconfig.want_privmsg =  cfg_getbool(cfg, "Options|UsePrivMsg");
        /* service chan has a default */
		strlcpy( me.serviceschan, cfg_getstr(cfg, "ServerConfig|ServiceChannel"), sizeof( me.serviceschan ) );
		/* only opers has a default */
		nsconfig.onlyopers = cfg_getbool(cfg, "Options|OperOnly");
        /* vhost has no default, nor is it required */
        if (cfg_size(cfg, "ServerConfig|BindTo") > 0) {
    		strlcpy( me.local, cfg_getstr(cfg, "ServerConfig|BindTo"), sizeof( me.local ) );
        }
        /* LogFile Format has a default */
		strlcpy(LogFileNameFormat,cfg_getstr(cfg, "Options|LogFileNameFormat"),MAX_LOGFILENAME );
		/* numeric has a default */
		me.numeric = cfg_getint(cfg, "ServerConfig|UnrealNumeric");
		/* has a default */
		nsconfig.setservertimes = cfg_getint(cfg, "Options|UnrealSettime") * 60 * 60;
		if (cfg_size(cfg, "ServiceRoots|Mask") > 0) {
    		char *nick;
	    	char *user;
            char *host;
            char *arg;
            /* already validate */
            arg = cfg_getstr(cfg, "ServiceRoots|Mask");
			nick = strtok(arg, "!" );
			user = strtok(NULL, "@" );
			host = strtok(NULL, "" );
			strlcpy(nsconfig.rootuser.nick, nick, MAXNICK );
			strlcpy(nsconfig.rootuser.user, user, MAXUSER );
			strlcpy(nsconfig.rootuser.host, host, MAXHOST );
			nsconfig.rootuser.level = NS_ULEVEL_ROOT;
		}
		/* protocol is required */
		strlcpy(me.protocol,cfg_getstr(cfg, "ServerConfig|Protocol"),MAXHOST );
        /* dbm has a default */
		strlcpy(me.dbm,cfg_getstr(cfg, "Options|DataBaseType"),MAXHOST );
        /* has a default */
		strlcpy(me.rootnick,cfg_getstr(cfg, "Options|RootNick"),MAXNICK );
}



/** @brief Load the modules 
 *
 * Actually load the modules that were found in the config file
 *
 * @returns 1 on success, -1 when a module failed to load
 * @bugs if a single module fails to load, it stops trying to load any other modules
 */

int ConfLoadModules( void )
{
#if 0
	int i;

	SET_SEGV_LOCATION( );
	if(load_mods[0] == 0 ) {
		nlog( LOG_NORMAL, "No modules configured for loading" ); 
	} else {
		nlog( LOG_NORMAL, "Loading configured modules" ); 
		for( i = 0;( i < NUM_MODULES ) &&( load_mods[i] != 0 ); i++ ) {
			dlog(DEBUG1, "ConfLoadModules: Loading Module %s",( char * )load_mods[i] );
			if( load_module( load_mods[i], NULL ) ) {
				nlog( LOG_NORMAL, "Loaded module %s",( char * )load_mods[i] );
			} else {
				nlog( LOG_WARNING, "Failed to load module %s. Please check above error messages",( char * )load_mods[i] );
			}
			ns_free(load_mods[i] );
		}
		nlog( LOG_NORMAL, "Completed loading configured modules" ); 
	}
#endif
	return NS_SUCCESS;
}



int cb_verify_chan(cfg_t *cfg, cfg_opt_t *opt) {

      if (ValidateChannel(opt->values[0]->string) == NS_FAILURE) {
          cfg_error(cfg, "Invalid Channel Name %s for Option %s", opt->values[0]->string, opt->name);
          return CFG_PARSE_ERROR;
      }
      return CFG_SUCCESS;
}
int cb_verify_numeric(cfg_t *cfg, cfg_opt_t *opt) {
        long int num = opt->values[0]->number;
		/* limit value - really need to print error and quit */
		if((num<=0) || (num>254)) {
            cfg_error(cfg, "Numeric Value %d is out of Range for %s", num, opt->name);
            return CFG_PARSE_ERROR;
        }
      return CFG_SUCCESS;
}
int cb_verify_bind(cfg_t *cfg, cfg_opt_t *opt) {
      return CFG_SUCCESS;
}
int cb_verify_file(cfg_t *cfg, cfg_opt_t *opt) {
      return CFG_SUCCESS;
}
int cb_verify_log(cfg_t *cfg, cfg_opt_t *opt) {
      return CFG_SUCCESS;
}
int cb_verify_mask(cfg_t *cfg, cfg_opt_t *opt) {
      char *value = opt->values[0]->string;
      if( strstr(value, "!" )&& !strstr(value, "@" ) ) {
	      cfg_error(cfg, "Invalid HostMask %s for %s", value, opt->name);
	      return CFG_PARSE_ERROR;
      }
      return CFG_SUCCESS;
}
int cb_noload(cfg_t *cfg, cfg_opt_t *opt) {
      if (opt->values[0]->boolean == cfg_true) {
          cfg_error(cfg, "Error. You didn't edit NeoStats.conf");
          return CFG_PARSE_ERROR;
      }
      return CFG_SUCCESS;
}
int cb_verify_host(cfg_t *cfg, cfg_opt_t *opt) {
      return CFG_SUCCESS;
}
int cb_verify_settime(cfg_t *cfg, cfg_opt_t *opt) {
      long int time = opt->values[0]->number;
        
      if (time <= 0) {
          cfg_error(cfg, "SetTime Value of %d is out of range for %s", time, opt->name);
          return CFG_PARSE_ERROR;
      }
      return CFG_SUCCESS;
}


#else /* USE_CONFUSE */

static config_option options[] = {
	{"SERVER_NAME", ARG_STR, cb_Server, 0},
	{"SERVER_PORT", ARG_STR, cb_Server, 1},
	{"CONNECT_TO", ARG_STR, cb_Server, 2},
	{"CONNECT_PASS", ARG_STR, cb_Server, 3},
	{"SERVER_INFOLINE", ARG_STR, cb_Server, 4},
	{"SERVICE_HOST", ARG_STR, cb_Server, 5},
	{"RECONNECT_TIME", ARG_STR, cb_Server, 6},
	{"WANT_PRIVMSG", ARG_STR, cb_Server, 9},
	{"SERVICES_CHAN", ARG_STR, cb_Server, 10},
	{"LOAD_MODULE", ARG_STR, cb_Module, 0},
	{"ONLY_OPERS", ARG_STR, cb_Server, 11},
	{"NO_LOAD", ARG_STR, cb_Server, 12},
	{"BIND_TO", ARG_STR, cb_Server, 13},
	{"LOG_FILENAME_FORMAT", ARG_STR, cb_Server, 14},
	{"SERVER_NUMERIC", ARG_STR, cb_Server, 15},
	{"SET_SERVER_TIMES", ARG_STR, cb_Server, 16},
	{"SERVICE_ROOT", ARG_STR, cb_Server, 17},
	{"PROTOCOL", ARG_STR, cb_Server, 18},
	{"DBM", ARG_STR, cb_Server, 19},
	{"ROOT_NICK", ARG_STR, cb_Server, 20},
};

/** @brief Load configuration file
 *
 * Parses the configuration file
 *
 * @returns nothing
 */

int ConfLoad( void )
{
	/* Read in the Config File */
	printf( "Reading the Config File %s. Please wait.....\n", CONFIG_NAME);
	if( config_read( CONFIG_NAME, options ) != 0 ) {
		printf( "***************************************************\n" );
		printf( "*                  Error!                         *\n" );
		printf( "*                                                 *\n" );
		printf( "* Config file not found, or unable to open. Check *\n" );
		printf( "* its location and permissions and try again.     *\n" );
		printf( "*                                                 *\n" );
		printf( "*             NeoStats NOT Started                *\n" );
		printf( "***************************************************\n" );
		return NS_FAILURE;
	}
	if( nsconfig.die ) {
		printf( "\n-----> ERROR: Read the README file then edit %s <-----\n\n",CONFIG_NAME );
		nlog( LOG_CRITICAL, "Read the README file then edit %s",CONFIG_NAME );
		return NS_FAILURE;
	}
	if( nsconfig.error ) {
		printf( "\n-----> CONFIG ERROR: Check log file for more information then edit %s <-----\n\n",CONFIG_NAME );
		nlog( LOG_CRITICAL, "CONFIG ERROR: Check log file for more information then edit %s",CONFIG_NAME );
		return NS_FAILURE;
	}
	printf( "Sucessfully loaded config file, booting NeoStats\n" );
	return NS_SUCCESS;
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
cb_Module( char *arg, int configtype )
{
	int i;

	SET_SEGV_LOCATION( );
	if( !nsconfig.modnoload ) {
		for( i = 0;( i < NUM_MODULES ) &&( load_mods[i] != 0 ); i++ ) {
			if( !ircstrcasecmp( load_mods[i], arg ) ) {
				return;
			}
		}
		load_mods[i] = sstrdup( arg );
		dlog(DEBUG1, "Added Module %d :%s", i,( char * )load_mods[i] );
	}
}

/** @brief Load the modules 
 *
 * Actually load the modules that were found in the config file
 *
 * @returns 1 on success, -1 when a module failed to load
 * @bugs if a single module fails to load, it stops trying to load any other modules
 */

int ConfLoadModules( void )
{
	int i;

	SET_SEGV_LOCATION( );
	if(load_mods[0] == 0 ) {
		nlog( LOG_NORMAL, "No modules configured for loading" ); 
	} else {
		nlog( LOG_NORMAL, "Loading configured modules" ); 
		for( i = 0;( i < NUM_MODULES ) &&( load_mods[i] != 0 ); i++ ) {
			dlog(DEBUG1, "ConfLoadModules: Loading Module %s",( char * )load_mods[i] );
			if( load_module( load_mods[i], NULL ) ) {
				nlog( LOG_NORMAL, "Loaded module %s",( char * )load_mods[i] );
			} else {
				nlog( LOG_WARNING, "Failed to load module %s. Please check above error messages",( char * )load_mods[i] );
			}
			ns_free(load_mods[i] );
		}
		nlog( LOG_NORMAL, "Completed loading configured modules" ); 
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
cb_Server( char *arg, int configtype )
{
	if( configtype == 0 ) {
		/* Server name */
		strlcpy( me.name, arg, sizeof( me.name ) );
	} else if( configtype == 1 ) {
		/* Server Port */
		me.port = atoi( arg );
	} else if( configtype == 2 ) {
		/* Connect To */
		strlcpy( me.uplink, arg, sizeof( me.uplink ) );
	} else if( configtype == 3 ) {
		/* Connect Pass */
		strlcpy( nsconfig.pass, arg, sizeof( nsconfig.pass ) );
	} else if( configtype == 4 ) {
		/* Server InfoLine */
		strlcpy( me.infoline, arg, sizeof( me.infoline ) );
	} else if( configtype == 5 ) {
		/* Service host */
		strlcpy( me.servicehost, arg, sizeof( me.servicehost ) );
	} else if( configtype == 6 ) {
		/* Reconnect time */
		nsconfig.r_time = atoi( arg );
	} else if( configtype == 9 ) {
		nsconfig.want_privmsg = 1;
	} else if( configtype == 10 ) {
		strlcpy( me.serviceschan, arg, sizeof( me.serviceschan ) );
	} else if( configtype == 11 ) {
		nsconfig.onlyopers = 1;
	} else if( configtype == 12 ) {
		nsconfig.die = 1;
	} else if( configtype == 13 ) {
		strlcpy( me.local, arg, sizeof( me.local ) );
	} else if( configtype == 14 ) {
		strlcpy(LogFileNameFormat,arg,MAX_LOGFILENAME );
	} else if( configtype == 15 ) {
		me.numeric = atoi( arg );
		/* limit value - really need to print error and quit */
		if(me.numeric<=0 )
			me.numeric=1;
		if(me.numeric>254 )
			me.numeric=254;
	} else if( configtype == 16 ) {
		nsconfig.setservertimes = atoi( arg );
		/* Convert hours input to seconds */
		nsconfig.setservertimes = nsconfig.setservertimes * 60 * 60;
		/* limit value - really need to print error and quit */
		if(nsconfig.setservertimes <= 0 ) {
			nsconfig.setservertimes =( 24 * 60 * 60 );
		}
	} else if( configtype == 17 ) {
		char *nick;
		char *user;
		char *host;

		if( strstr(arg, "!" )&& !strstr(arg, "@" ) ) {
			nlog(LOG_WARNING, 
				"Invalid SERVICEROOT. Must be of the form nick!ident@host, was %s",
				arg );
			nsconfig.error = 1;
		} else {
			nick = strtok(arg, "!" );
			user = strtok(NULL, "@" );
			host = strtok(NULL, "" );
			strlcpy(nsconfig.rootuser.nick, nick, MAXNICK );
			strlcpy(nsconfig.rootuser.user, user, MAXUSER );
			strlcpy(nsconfig.rootuser.host, host, MAXHOST );
			nsconfig.rootuser.level = NS_ULEVEL_ROOT;
		}
	} else if( configtype == 18 ) {
		strlcpy(me.protocol,arg,MAXHOST );
	} else if( configtype == 19 ) {
		strlcpy(me.dbm,arg,MAXHOST );
	} else if( configtype == 20 ) {
		strlcpy(me.rootnick,arg,MAXNICK );
	}
}

#endif /* USE_CONFUSE */
