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

extern char *sftime(time_t);

struct sock_list_struct {
	long hash;
	int sock_no;
	char *sockname;
	int (*function)();
	char *modname;
	long rmsgs;
	long rbytes;
};
typedef struct sock_list_struct Sock_List;
hash_t *sockh;

struct mod_timer_list {
	long hash;
	char *modname;
	char *timername;
	int interval;
	time_t lastrun;
	int (*function)();
};
typedef struct mod_timer_list Mod_Timer;
hash_t *th;

struct mod_user_list {
	long hash;
	char *nick;
	char *modname;
	int (*function)(char *origin, char **av, int ac);
	int (*chanfunc)(char *origin, char *chan, char **av, int ac);
	hash_t *chanlist;
};

typedef struct mod_user_list Mod_User;
hash_t *bh;

struct _chan_bot_list {
	char *chan;
	list_t *bots;
};

typedef struct _chan_bot_list Chan_Bot;
hash_t *bch;


struct functions {
	char *cmd_name;
	int (*function)(char *origin, char **av, int ac);
	int srvmsg;
};

struct evtfunctions {
	char *cmd_name;
	int (*function)(char **av, int ac);
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
	Module_Info *info;
	Functions *function_list;
	EventFnList *other_funcs;
	void *dl_handle;

};


typedef struct module Module;

hash_t *mh;


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
extern int get_dl_handle(char *mod_name);
extern void add_bot_to_chan(char *, char *);
extern void del_bot_from_chan(char *, char *);
extern void bot_chan_message(char *origin, char *chan, char **av, int ac);
extern void botchandump(User *u);
#endif /* !_dl_h_ */
