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
#include <string.h>
#include <time.h>
#include "hash.h"
#include "stats.h"

/* 
 * NeoStats core API version.
 * A module should check this when loaded to ensure compatibility
 */
#define API_VER 1

/* 
 *   Ensure RTLD flags correctly defined
 */
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY	/* openbsd deficiency */
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif

/* 
 * Need to locate lib include for this
 */
extern char *sftime (time_t);

/* 
 * Module socket list structure
 */
struct _Sock_List {
	long hash;
	int sock_no;
	char sockname[MAXHOST];
	int (*readfnc) (int sock_no, char *sockname);
	int (*writefnc) (int sock_no, char *sockname);
	int (*errfnc) (int sock_no, char *sockname);
	char modname[MAXHOST];
	long rmsgs;
	long rbytes;
};

typedef struct _Sock_List Sock_List;

/* 
 * Module Socket List hash
 */
hash_t *sockh;

/* 
 * Module Timer structure
 */
struct _Mod_Timer {
	long hash;
	char modname[MAXHOST];
	char timername[MAXHOST];
	int interval;
	time_t lastrun;
	int (*function) ();
};
typedef struct _Mod_Timer Mod_Timer;
hash_t *th;

/* 
 *
 */
struct _Mod_User {
	long hash;
	char nick[MAXNICK];
	char modname[MAXHOST];
	int (*function) (char *origin, char **av, int ac);
	int (*chanfunc) (char *origin, char *chan, char **av, int ac);
	hash_t *chanlist;
};

typedef struct _Mod_User Mod_User;
hash_t *bh;

/* 
 *
 */
struct _Chan_Bot {
	char chan[CHANLEN];
	list_t *bots;
};

typedef struct _Chan_Bot Chan_Bot;
hash_t *bch;

/* 
 *
 */
struct _Functions {
	char *cmd_name;
	int (*function) (char *origin, char **av, int ac);
	int srvmsg;
};

typedef struct _Functions Functions;

/* 
 *
 */
struct _EventFnList {
	char *cmd_name;
	int (*function) (char **av, int ac);
};

typedef struct _EventFnList EventFnList;

/* 
 *
 */
struct _Module_Info {
	char *module_name;
	char *module_description;
	char *module_version;
};

typedef struct _Module_Info Module_Info;

/* 
 * New module info structure
 */
struct _ModuleInfo {
	char *module_name;
	char *module_description;
	char *module_version;
	char *module_build_date;
	char *module_build_time;
};

typedef struct _ModuleInfo ModuleInfo;

/* 
 *
 */
struct _Module {
	Module_Info *info;
	Functions *function_list;
	EventFnList *other_funcs;
	void *dl_handle;
};

typedef struct _Module Module;

hash_t *mh;

/* 
 *
 */
struct mod_num {
	Module *mod;
	int used;
};
struct mod_num ModNum[NUM_MODULES];

/* 
 * Prototypes
 */
void __init_mod_list (void);
int load_module (char *path, User * u);
int unload_module (char *module_name, User * u);
int add_ld_path (char *path);
void list_module (User * u);
void list_module_bots (User * u);
int add_mod_user (char *nick, char *mod_name);
int del_mod_user (char *nick);
int add_mod_timer (char *func_name, char *timer_name, char *mod_name, int interval);
int del_mod_timer (char *timer_name);
void list_module_timer (User * u);
int add_socket (char *readfunc, char *writefunc, char *errfunc, char *sock_name, int socknum, char *mod_name);
int del_socket (char *sockname);
void list_sockets (User * u);
Sock_List *findsock (char *sock_name);
Mod_User *findbot (char * bot_name);
int get_dl_handle (char *mod_name);
void add_bot_to_chan (char *bot, char *chan);
void del_bot_from_chan (char *bot, char *chan);
void bot_chan_message (char *origin, char *chan, char **av, int ac);
void botchandump (User * u);
int get_mod_num (char *mod_name);

/* 
 * Module Interface 
 */
int __ModInit(int modnum, int apiver);
void __ModFini(void);
int __Bot_Message(char *origin, char **av, int ac);
int __Chan_Message(char *origin, char *chan, char **argv, int argc);
Module_Info *__module_get_info(void);
Functions *__module_get_functions(void);
EventFnList *__module_get_events(void);

/* WIP: New module interface */
/*Module_Info __module_info;   */
/*Functions __module_functions[];*/
/*EventFnList __module_events[];  */

#endif /* !_dl_h_ */
