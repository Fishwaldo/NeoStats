/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

#ifndef _dl_h_
#define _dl_h_

/*
 * dl.h
 * dynamic runtime library loading routines
 */

#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <sys/poll.h>
#include "stats.h"
#include "hash.h"

/* 
 * NeoStats core API version.
 * A module should check this when loaded to ensure compatibility
 */
#define API_VER 2

/* 
 *   Ensure RTLD flags correctly defined
 */
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY	/* openbsd deficiency */
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif

/* socket interface type */
#define SOCK_POLL 1
#define SOCK_STANDARD 2

/** @brief Message function types
 * 
 */
typedef int (*message_function) (char *origin, char **av, int ac);
typedef int (*timer_function) (void);

/** @brief Socket function types
 * 
 */
typedef int (*socket_function) (int sock_no, char *sockname);
typedef int (*before_poll_function) (void *data, struct pollfd *);
typedef void (*after_poll_function) (void *data, struct pollfd *, unsigned int);

/** @brief Module socket list structure
 * 
 */
typedef struct ModSock {
	/** Socket number */
	int sock_no;
	/** Socket name */
	char sockname[MAX_MOD_NAME];
	/** socket interface (poll or standard) type */
	int socktype;
	/** if socktype = SOCK_POLL, before poll function */
	/** Socket before poll function */
	before_poll_function beforepoll;
	/** Socket after poll function */
	after_poll_function afterpoll;
	/** data */
	void *data;
	/* if socktype = SOCK_STANDARD, function calls */
	/** Socket read function */
	socket_function readfnc;
	/** Socket write function */
	socket_function writefnc;
	/** Socket error function */
	socket_function errfnc;
	/** Module name */
	char modname[MAX_MOD_NAME];
	/** rmsgs */
	long rmsgs;
	/** rbytes */
	long rbytes;
}ModSock;

/** @brief Module Timer structure
 * 
 */
typedef struct ModTimer {
	/** Module name */
	char modname[MAX_MOD_NAME];
	/** Timer name */
	char timername[MAX_MOD_NAME];
	/** Timer interval */
	int interval;
	/** Time last run */
	time_t lastrun;
	/** Timer function */
	timer_function function;
}ModTimer;

/** @brief Module User structure
 * 
 */

typedef struct ModUser {
	/** Nick */
	char nick[MAXNICK];
	/** Module name */
	char modname[MAX_MOD_NAME];
	/* bot flags */
	unsigned int flags;
	/* hash for command list */
	hash_t *botcmds;
	/* hash for settings */
	bot_setting *bot_settings;
	/** bot message function */
	message_function function;
	/** channel message function */
	message_function chanfunc;
}ModUser;

/** @brief Channel bot structure
 * 
 */
typedef struct ModChanBot {
	/** channel name */
	char chan[CHANLEN];
	/** bot list */
	list_t *bots;
}ModChanBot;

/** @brief Module functions structure
 * 
 */
typedef struct Functions {
	char *cmd_name;
	message_function function;
	int srvmsg;
}Functions;

/** @brief Module Event functions structure
 * 
 */
typedef int (*event_function) (char **av, int ac);

typedef struct EventFnList {
	char *cmd_name;
	event_function function;
}EventFnList;

/** @brief Module Info structure (old style)
 * 
 */
typedef struct Module_Info {
	char *module_name;
	char *module_description;
	char *module_version;
}Module_Info;

/** @brief Module Info structure (new style)
 * 
 */
typedef struct ModuleInfo {
	char *module_name;
	char *module_description;
	char *module_version;
	char *module_build_date;
	char *module_build_time;
}ModuleInfo;

/** @brief Module structure
 * 
 */
typedef struct Module {
	ModuleInfo *info;
	Functions *function_list;
	EventFnList *event_list;
	void *dl_handle;
}Module;

/* @brief Module Socket List hash
 * 
 */
extern hash_t *sockh;

/* 
 * Prototypes
 */
int InitModuleHash (void);
void ModuleEvent (char * event, char **av, int ac);
/* Temp define for secureserv */
#define Module_Event ModuleEvent	
void ModuleFunction (int cmdptr, char *cmd, char* origin, char **av, int ac);
int load_module (char *path, User * u);
int unload_module (char *module_name, User * u);
int list_modules (User * u, char **av, int ac);
int list_bots (User * u, char **av, int ac);
ModUser* add_mod_user (char *nick, char *mod_name);
int del_mod_user (char *nick);
int add_mod_timer (char *func_name, char *timer_name, char *mod_name, int interval);
int del_mod_timer (char *timer_name);
int change_mod_timer_interval (char *timer_name, int interval);
ModTimer *findtimer(char *timer_name);
int list_timers (User * u, char **av, int ac);
void run_mod_timers (void);
int add_socket (char *readfunc, char *writefunc, char *errfunc, char *sock_name, int socknum, char *mod_name);
int add_sockpoll (char *beforepoll, char *afterpoll, char *sock_name, char *mod_name, void *data);
int del_socket (char *sockname);
int list_sockets (User * u, char **av, int ac);
ModSock *findsock (char *sock_name);
ModUser *findbot (char * bot_name);
int get_dl_handle (char *mod_name);
void add_bot_to_chan (char *bot, char *chan);
void del_bot_from_chan (char *bot, char *chan);
void bot_chan_message (char *origin, char **av, int ac);
int list_bot_chans (User * u, char **av, int ac);
int get_mod_num (char *mod_name);
void unload_modules(void);
int bot_nick_change (char * oldnick, char *newnick);
void verify_hashes(void);

int add_bot_cmd_list(ModUser *bot_ptr, bot_cmd *bot_cmd_list);
int del_bot_cmd_list(ModUser *bot_ptr, bot_cmd *bot_cmd_list);
int run_bot_cmd (ModUser *bot_ptr, User *u, char **av, int ac);
ModUser * init_mod_bot (char * nick, char * user, char * host, char * rname, const char *modes, unsigned int flags, bot_cmd *bot_cmd_list, bot_setting *bot_setting_list, char * modname);
int del_mod_bot (ModUser *bot_ptr, char * reason);

/* 
 * Module Interface 
 */
int __ModInit(int modnum, int apiver);
void __ModFini(void);
int __BotMessage(char *origin, char **av, int ac);
/* temp define while rename propogates */
#define __Bot_Message __BotMessage
int __ChanMessage(char *origin, char **argv, int argc);

/* temporary define to support for module API backwards compatibility 
 * old system used function addresses to call into a module to get data addresses
 * new system just grabs data addresses directly to simplify module coding
 */
#define OLD_MODULE_EXPORT_SUPPORT

#ifdef OLD_MODULE_EXPORT_SUPPORT
Module_Info *__module_get_info(void);
Functions *__module_get_functions(void);
EventFnList *__module_get_events(void);
#endif /* OLD_MODULE_EXPORT_SUPPORT */

extern ModuleInfo		__module_info;   
extern Functions		__module_functions[];
extern EventFnList		__module_events[];  

#endif /* !_dl_h_ */
