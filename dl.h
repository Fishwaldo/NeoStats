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
#include "stats.h"

extern char *sftime(time_t);

struct sock_list_struct {
	struct sock_list_struct *prev;
	struct sock_list_struct *next;
	long hash;
	int sock_no;
	char *sockname;
	int (*function)();
	char *modname;
	long rmsgs;
	long rbytes;
};
typedef struct sock_list_struct Sock_List;
Sock_List *Socket_lists[MAX_SOCKS];

struct mod_timer_list {
	struct mod_timer_list *prev;
	struct mod_timer_list *next;
	long hash;
	char *modname;
	char *timername;
	int interval;
	time_t lastrun;
	int (*function)();
};
typedef struct mod_timer_list Mod_Timer;
Mod_Timer *module_timer_lists[T_TABLE_SIZE];


struct mod_user_list {
	struct mod_user_list *prev;
	struct mod_user_list *next;
	long hash;
	char *nick;
	char *modname;
	int (*function)(char *origin, char *av);
};

typedef struct mod_user_list Mod_User;
Mod_User *module_bot_lists[B_TABLE_SIZE];

struct functions {
	char *cmd_name;
	int (*function)(char *origin, char *av);
	int srvmsg;
};

struct evtfunctions {
	char *cmd_name;
	int (*function)(void *data);
};


typedef struct functions Functions;
typedef struct evtfunctions EventFnList;

struct mod_info {
	char *module_name;
	char *module_description;
	char *module_version;
};

typedef struct mod_info Module_Info;

struct module {
	struct module *prev;
	struct module *next;
	Module_Info *info;
	Functions *function_list;
	EventFnList *other_funcs;
	void *dl_handle;

};


typedef struct module Module;

extern Module *module_list;

struct path {
	struct path *prev;
	struct path *next;
	char dl_path[100];
};

typedef struct path LD_Path;

extern void __init_mod_list();
extern int load_module(char *path,User *u);
extern int unload_module(char *module_name,User *u);
extern int add_ld_path(char *path);
extern void list_module(User *);
extern void list_module_bots(User *);
extern int add_mod_user(char *nick, char *mod_name);
extern int del_mod_user(char *nick);
extern int add_mod_timer(char *func_name, char *timer_name, char *mod_name, int interval);
extern int del_mod_timer(char *timer_name);
extern void list_module_timer(User *);
extern int add_socket(char *func_name, char *sock_name, int socknum, char *mod_name);
extern int del_socket(char *sockname);
extern void list_sockets(User *);
extern Mod_User *findbot(char *);

#endif /* !_dl_h_ */
