/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Dynamic Runtime Library Loading Routines
*
** NeoStats Identification:
** ID:      dl.c, 
** Version: 2.1
** Date:    13/1/2002
*/

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dl.h"
#include "stats.h"
#include "config.h"


Module *module_list;
LD_Path *ld_path_list;

void __init_mod_list() {
	int i;
	Mod_Timer *t, *Tprev;
	Mod_User *u, *Uprev;
	Sock_List *s, *Sprev;
	
	segv_location = sstrdup("__init_mod_list");
	module_list = (Module *)malloc(sizeof(Module));
	bzero(module_list, sizeof(Module));
	module_list->prev = NULL;
	module_list->next = NULL;
	ld_path_list = (LD_Path *)malloc(sizeof(LD_Path));
	bzero(ld_path_list, sizeof(LD_Path));
	ld_path_list->prev = NULL;
	ld_path_list->next = NULL;
	for (i = 0; i < B_TABLE_SIZE; i++) {
		u = module_bot_lists[i];
		while (u) {
			Uprev = u->next;
			free(u);
			u = Uprev;
		}
		module_bot_lists[i] = NULL;
	}
	bzero((char *)module_bot_lists, sizeof(module_bot_lists));	
	for (i = 0; i < T_TABLE_SIZE; i++) {
		t = module_timer_lists[i];
		while (t) {
			Tprev = t->next;
			free(t);
			t = Tprev;
		}
		module_timer_lists[i] = NULL;
	}
	bzero((char *)module_timer_lists, sizeof(module_timer_lists));
	for (i = 0; i < MAX_SOCKS; i++) {
		s = Socket_lists[i];
		while (s) {
			Sprev = s->next;
			free(s);
			s = Sprev;
		}
		Socket_lists[i] = NULL;
	}
	bzero((char *)Socket_lists, sizeof(Socket_lists));
/* Shmad */
		free(u);
		free(t);
		free(s);
};

int add_ld_path(char *path) {
	LD_Path *path_ent, *list;
	
	segv_location = sstrdup("add_ld_path");
	path_ent = (LD_Path *)malloc(sizeof(LD_Path));
	
	bzero(path_ent, sizeof(LD_Path));

	strncpy(path_ent->dl_path, path, 100);

	list = ld_path_list;

	while (list->next != NULL) list = list->next;

	list->next = path_ent;
	path_ent->prev = list;
	return 0;
}
static void add_timer_to_hash_table(char *timer_name, Mod_Timer *t) {
	segv_location = sstrdup("add_timer_to_hash_table");
	t->hash = HASH(timer_name, T_TABLE_SIZE);
	t->next = module_timer_lists[t->hash];
	module_timer_lists[t->hash] = (void *)t;
}

static void del_timer_from_hash_table(char *timer_name, Mod_Timer *t) {
	Mod_Timer *tmp, *prev = NULL;

	segv_location = sstrdup("del_timer_from_hash_table");
	for (tmp = module_timer_lists[t->hash]; tmp; tmp = tmp->next) {
		if (tmp == t) {
			if (prev)
				prev->next = tmp->next;
			else
				module_timer_lists[t->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

static Mod_Timer *new_timer(char *timer_name)
{
	Mod_Timer *t;

	segv_location = sstrdup("Mod_Timer");
#ifdef DEBUG
	log("New Timer: %s", timer_name);
#endif
	t = smalloc(sizeof(Mod_Timer));
	if (!timer_name)
		timer_name="";
	t->timername = timer_name;	
	add_timer_to_hash_table(timer_name, t);
	return t;
}

Mod_Timer *findtimer(char *timer_name) {
	Mod_Timer *t;
	
	segv_location = sstrdup("findtimer");
	t = module_timer_lists[HASH(timer_name, T_TABLE_SIZE)];
	while (t && strcasecmp(t->timername, timer_name) != 0)
		t = t->next;
	return t;
}
int add_mod_timer(char *func_name, char *timer_name, char *mod_name, int interval) {
	Mod_Timer *Mod_timer_list;
	Module *list_ptr;

	segv_location = sstrdup("add_mod_timer");


	Mod_timer_list = new_timer(timer_name);
	Mod_timer_list->interval = interval;
	Mod_timer_list->lastrun = time(NULL);
	Mod_timer_list->modname = sstrdup(mod_name);

	list_ptr = module_list->next;
	while (list_ptr != NULL) {
		if (!strcasecmp(list_ptr->info->module_name, mod_name)) {
			/* Check to see if the function exists */
			if (dlsym(list_ptr->dl_handle, func_name) == NULL) {
				log("Oh Oh, the Timer function doesn't exist");
				del_mod_timer(Mod_timer_list->timername);
				return -1;
			}
			Mod_timer_list->function = dlsym(list_ptr->dl_handle, func_name);
#ifdef DEBUG
			log("Registered Module %s with timer for Function %s", list_ptr->info->module_name, func_name);
#endif
			return 1;
		}
		list_ptr = list_ptr->next;
	}
	return 0;
}

int del_mod_timer(char *timer_name) {
	Mod_Timer *list;

	segv_location = sstrdup("del_mod_timer");
	
	list = findtimer(timer_name);
		
	if (list) {
#ifdef DEBUG
		log("Unregistered Timer function %s from Module %s", timer_name, list->modname);
#endif
		del_timer_from_hash_table(timer_name, list);
		free(list);
		return 1;
	}		
	return -1;
}

void list_module_timer(User *u) {
	Mod_Timer *mod_ptr = NULL;
	register int j;
	segv_location = sstrdup("list_module_timer");
	privmsg(u->nick,s_Services,"Module timer List:");
	for (j = 0; j < T_TABLE_SIZE; j++) {
		for (mod_ptr = module_timer_lists[j]; mod_ptr; mod_ptr = mod_ptr->next) { 
			privmsg(u->nick,s_Services,"%s:--------------------------------",mod_ptr->modname);
			privmsg(u->nick,s_Services,"Module Timer Name: %s",mod_ptr->timername);
			privmsg(u->nick,s_Services,"Module Interval: %d",mod_ptr->interval);
			privmsg(u->nick,s_Services,"Time till next Run: %d", mod_ptr->interval - (time(NULL) - mod_ptr->lastrun));
		}
	}
	privmsg(u->nick,s_Services,"End of Module timer List");
	free(mod_ptr);
}		

static void add_sock_to_hash_table(char *sockname, Sock_List *s) {
	segv_location = sstrdup("add_sock_to_hash_table");
	s->hash = HASH(sockname, MAX_SOCKS);
	s->next = Socket_lists[s->hash];
	Socket_lists[s->hash] = (void *)s;
}

static void del_sock_from_hash_table(char *sockname, Sock_List *s) {
	Sock_List *tmp = NULL, *prev = NULL;
	
	segv_location = sstrdup("del_sock_from_hash_table");
	for (tmp = Socket_lists[s->hash]; tmp; tmp = tmp->next) {
		if (tmp == s) {
			if (prev)
				prev->next = tmp->next;
			else
				Socket_lists[s->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

static Sock_List *new_sock(char *sock_name)
{
	Sock_List *s;

	segv_location = sstrdup("Sock_List");
#ifdef DEBUG	
	log("New Socket: %s", sock_name);
#endif	
	s = smalloc(sizeof(Sock_List));
	if (!sock_name)
		sock_name="";
	s->sockname = sock_name;	
	add_sock_to_hash_table(sock_name, s);
	return s;
}

Sock_List *findsock(char *sock_name) {
	Sock_List *s;

	segv_location = sstrdup("findsock");
	s = Socket_lists[HASH(sock_name, MAX_SOCKS)];
	while (s && strcmp(s->sockname, sock_name) != 0)
		s = s->next;
	return s;
}
int add_socket(char *func_name, char *sock_name, int socknum, char *mod_name) {
	Sock_List *Sockets_mod_list;
	Module *list_ptr;

	segv_location = sstrdup("add_Socket");


	Sockets_mod_list = new_sock(sock_name);
	Sockets_mod_list->sock_no = socknum;
	Sockets_mod_list->modname = sstrdup(mod_name);

	list_ptr = module_list->next;
	while (list_ptr != NULL) {
		if (!strcasecmp(list_ptr->info->module_name, mod_name)) {
			/* Check to see if the function exists */
			if (dlsym(list_ptr->dl_handle, func_name) == NULL) {
				log("Oh Oh, the Socket function doesn't exist");
				del_socket(Sockets_mod_list->sockname);
				return -1;
			}
			Sockets_mod_list->function = dlsym(list_ptr->dl_handle, func_name);
#ifdef DEBUG
			log("Registered Module %s with Socket for Function %s", list_ptr->info->module_name, func_name);
#endif
			return 1;
		}
		list_ptr = list_ptr->next;
	}
	return 0;
}

int del_socket(char *sock_name) {
	Sock_List *list;

	segv_location = sstrdup("del_mod_timer");
#ifdef DEBUG
	log("Del_Sock");
#endif
	
	list = findsock(sock_name);
		
	if (list) {
#ifdef DEBUG
		log("Unregistered Socket function %s from Module %s", sock_name, list->modname);
#endif
		del_sock_from_hash_table(sock_name, list);
		free(list);
		return 1;
	}		
	return -1;
}

void list_sockets(User *u) {
	Sock_List *mod_ptr = NULL;
	register int j;
	segv_location = sstrdup("list_sockets");
	privmsg(u->nick,s_Services,"Sockets List:");
	for (j = 0; j < MAX_SOCKS; j++) {
		for (mod_ptr = Socket_lists[j]; mod_ptr; mod_ptr = mod_ptr->next) { 
			privmsg(u->nick,s_Services,"%s:--------------------------------",mod_ptr->modname);
			privmsg(u->nick,s_Services,"Socket Name: %s",mod_ptr->sockname);
			privmsg(u->nick,s_Services,"Socket Number: %d",mod_ptr->sock_no);
		}
	}
	privmsg(u->nick,s_Services,"End of Socket List");
	free(mod_ptr);
}		


static void add_bot_to_hash_table(char *bot_name, Mod_User *u) {
	segv_location = sstrdup("add_bot_to_hash_table");
	u->hash = HASH(bot_name, B_TABLE_SIZE);
	u->next = module_bot_lists[u->hash];
	module_bot_lists[u->hash] = (void *)u;
}

static void del_bot_from_hash_table(char *bot_name, Mod_User *u) {
	Mod_User *tmp, *prev = NULL;
	
	segv_location = sstrdup("del_bot_from_hash_table");
	for (tmp = module_bot_lists[u->hash]; tmp; tmp = tmp->next) {
		if (tmp == u) {
			if (prev)
				prev->next = tmp->next;
			else
				module_bot_lists[u->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		
		prev = tmp;
	}
}

static Mod_User *new_bot(char *bot_name)
{
	Mod_User *u;
	segv_location = sstrdup("Mod_User");
#ifdef DEBUG	
	log("New Bot: %s", bot_name);
#endif
	u = smalloc(sizeof(Mod_User));
	if (!bot_name)
		bot_name="";
	u->nick = sstrdup(bot_name);	
	add_bot_to_hash_table(bot_name, u);
	return u;
}

Mod_User *findbot(char *bot_name) {
	Mod_User *u;
	int i;
	
	segv_location = sstrdup("findbot");
	for (i = 0; i < B_TABLE_SIZE; i++) {
		for (u = module_bot_lists[i]; u; u = u->next) {
			if (!strcasecmp(u->nick, bot_name))
				return u;
		}
	}
	return NULL;
}

int del_mod_user(char *bot_name) {
	Mod_User *list;
	segv_location = sstrdup("del_mod_user");
	
	list = findbot(bot_name);
		
	if (list) {
		del_bot_from_hash_table(bot_name, list);
		free(list);
		return 1;
	}		
	return -1;



}


int bot_nick_change(char *oldnick, char *newnick)
{
	User *u;
	Mod_User *mod_tmp, *mod_ptr = NULL;
	segv_location = sstrdup("bot_nick_change");

	/* First, try to find out if the newnick is unique! */
#ifdef DEBUG
	log("oldnick %s newnick %s",oldnick,newnick);
#endif	
	u = finduser(oldnick);
	if (!u) {
		log("A non-registered bot(%s) attempted to change its nick to %s",oldnick, newnick);
		return -1;
		}
	u = finduser(newnick);
	if (!u) { 
		mod_ptr = findbot(oldnick);
		 if (!u) {
#ifdef DEBUG
			log("Bot %s Changed its nick to %s", oldnick, newnick);
#endif
			mod_tmp = new_bot(newnick);
			/* add a brand new user */
			mod_tmp->nick = sstrdup(newnick);
			mod_tmp->modname = sstrdup(mod_ptr->modname);
			mod_tmp->function = mod_ptr->function;
			/* Now Delete the Old bot nick */
			del_mod_user(oldnick);
			Change_User(finduser(oldnick), newnick);
			sts(":%s NICK %s", oldnick, newnick);
			return 1;
			}
		}
	log("Couldn't find Bot Nick %s in Bot list", oldnick);
	return -1;
}


int add_mod_user(char *nick, char *mod_name) {
	Mod_User *Mod_Usr_list;
	Module *list_ptr;
	
	segv_location = sstrdup("add_mod_user");


	Mod_Usr_list = new_bot(nick);
	/* add a brand new user */
	Mod_Usr_list->nick = sstrdup(nick);
	Mod_Usr_list->modname = sstrdup(mod_name);
	list_ptr = module_list->next;
	while (list_ptr != NULL) {
		if (!strcasecmp(list_ptr->info->module_name, mod_name)) {
			Mod_Usr_list->function = dlsym(list_ptr->dl_handle, "__Bot_Message");
			return 1;
		}
		list_ptr = list_ptr->next;
	}
	return 0;
}


void list_module_bots(User *u) {
	Mod_User *mod_ptr = NULL;
	register int j;
	segv_location = sstrdup("list_module_bots");

	privmsg(u->nick,s_Services,"Module Bot List:");

	for (j = 0; j < B_TABLE_SIZE; j++) {
		for (mod_ptr = module_bot_lists[j]; mod_ptr; mod_ptr = mod_ptr->next) { 
			privmsg(u->nick,s_Services,"Module: %s",mod_ptr->modname);
			privmsg(u->nick,s_Services,"Module Bots: %s",mod_ptr->nick);
		}
	}
	privmsg(u->nick,s_Services,"End of Module Bot List");
}		




int load_module(char *path1, User *u) {
	char buf[512];
	char *dl_error = NULL;
	void *dl_handle = NULL;
	int do_msg;
	char *path = NULL;

	int fmode;
	char *lock= NULL;
	FILE *lockmod;
	FILE *lmfile;
	FILE *lockdbfile;

	Module_Info * (*mod_get_info)() = NULL;
	Functions * (*mod_get_funcs)() = NULL;
	EventFnList * (*mod_get_events)() = NULL;
	Module_Info *mod_info_ptr = NULL;	
	Functions *mod_funcs_ptr = NULL;
	EventFnList *event_fn_ptr = NULL;
	Module *mod_ptr = NULL, *list_ptr = NULL;

	segv_location = sstrdup("load_module");

	if (u == NULL) {
		do_msg = 0;
	} else {
		do_msg = 1;
	}
	path = sstrdup(path1);
	path = strcat(path,".so");

	dl_handle = dlopen(path, RTLD_NOW || RTLD_GLOBAL); 
	if (!dl_handle) {
		LD_Path *list;
		list = ld_path_list->next;
		while (list != NULL) {
			char p[255];
			snprintf(p, 255, "%s/%s", list->dl_path, path);
			dl_handle = dlopen(p, RTLD_NOW || RTLD_GLOBAL); 
			if (!dl_handle) {
				list = list->next;
			} else {
				list = NULL;
			}
		}
		if (!dl_handle) {
			if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
			if (do_msg) privmsg(u->nick, s_Services, "%s",dlerror());
			log("Couldn't Load Module: %s", dlerror());
			return -1;
		}
	}

	mod_get_info = dlsym(dl_handle, "__module_get_info");
	if ((dl_error = dlerror()) != NULL) {
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dl_error);
		log("Couldn't Load Module: %s", dlerror());
		dlclose(dl_handle);
		return -1;
	}

	mod_get_funcs = dlsym(dl_handle, "__module_get_functions");
	if ((dl_error = dlerror()) != NULL) {
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dl_error);
		log("Couldn't Load Module: %s", dlerror());
		dlclose(dl_handle);
		return -1;
	}

	mod_get_events = dlsym(dl_handle, "__module_get_events");
	/* no error check here - this one isn't essential to the functioning of the module */

	mod_info_ptr = (*mod_get_info)();
	mod_funcs_ptr = (*mod_get_funcs)();
	if (mod_get_events) event_fn_ptr = (*mod_get_events)();

	if (mod_info_ptr == NULL || mod_funcs_ptr == NULL) {
		dlclose(dl_handle);
		log("%s: Could not load dynamic library %s!\n", __PRETTY_FUNCTION__, path);
		log("Couldn't Load Module: %s", dlerror());
		return -1;
	}
	
	/* Check that the Module hasn't already been loaded */
	
	list_ptr = module_list->next;
	while (list_ptr != NULL) {
		if (!strcasecmp(list_ptr->info->module_name, mod_info_ptr->module_name )) {
			dlclose(dl_handle);
			if (do_msg) privmsg(u->nick,s_Services,"Module %s already Loaded, Can't Load 2 Copies",mod_info_ptr->module_name);
			return -1;
		}
		list_ptr = list_ptr->next;
	}
	
	mod_ptr = (Module *)malloc(sizeof(Module));
	if (mod_ptr == NULL) {
		fprintf(stderr, "%s: Out of memory!\n", __PRETTY_FUNCTION__);
		dlclose(dl_handle);
		exit(1);
	}

	list_ptr = module_list;
	while (list_ptr->next != NULL) list_ptr = list_ptr->next;

	bzero(mod_ptr, sizeof(Module));

	log("internal module name: %s", mod_info_ptr->module_name);
	log("module description: %s", mod_info_ptr->module_description);

	mod_ptr->prev = list_ptr;
	list_ptr->next = mod_ptr;
	mod_ptr->info = mod_info_ptr;
	mod_ptr->function_list = mod_funcs_ptr;
	mod_ptr->dl_handle = dl_handle;
	mod_ptr->other_funcs = event_fn_ptr;

	/* Let this module know we are online if we are! */
	if (me.onchan == 1) {
		while (event_fn_ptr->cmd_name != NULL ) {
			if (!strcasecmp(event_fn_ptr->cmd_name, "ONLINE")) {
				event_fn_ptr->function(me.s);
				break;
			}
			event_fn_ptr++;
		}
	}

/* Lock .so Module File With 555 Permissions */
	/* Shmad Perscribes 444 Perms */
	lockmod = fopen("Neo-Lock.tmp","w");
	fprintf(lockmod, "%s/%s", me.modpath, path);
	fclose(lockmod);

	lmfile = fopen("Neo-Lock.tmp", "r");
	while (fgets(buf, sizeof(buf), lmfile)) {
		buf[strlen(buf)] = '\0';
		lock = buf;
	}
	fclose(lmfile);
	fmode = 0555;

	remove("Neo-Lock.tmp");
	chmod(lock, fmode);
/* End .so Module 555 Permission Setting */

/* Create List Of Loaded Modules */
lockdbfile = fopen("data/Lock.db", "a");
fprintf(lockdbfile, "%s\n", lock);
fclose(lockdbfile);
/* End List Creation Process */

	if (do_msg) privmsg(u->nick,s_Services,"Module %s Loaded, Description: %s",mod_info_ptr->module_name,mod_info_ptr->module_description);
	return 0;


};
extern int get_dl_handle(char *mod_name) {
	Module *list_ptr;

	segv_location = sstrdup("get_dl_handle");


	list_ptr = module_list->next;
	while (list_ptr != NULL) {
		if (!strcasecmp(list_ptr->info->module_name, mod_name)) {
			return (int)list_ptr->dl_handle;
		}
		list_ptr = list_ptr->next;
	}
	return 0;
}




void list_module(User *u) {
	Module *mod_ptr = NULL;
	segv_location = sstrdup("list_module");
	mod_ptr = module_list->next;
	privmsg(u->nick,s_Services,"Module List:");
	while(mod_ptr != NULL) {
		privmsg(u->nick,s_Services,"Module: %s (%s)",mod_ptr->info->module_name, mod_ptr->info->module_version);
		privmsg(u->nick,s_Services,"Module Description: %s",mod_ptr->info->module_description);
		mod_ptr = mod_ptr->next;
	}
	privmsg(u->nick,s_Services,"End of Module List");
	free(mod_ptr);
}		

int unload_module(char *module_name, User *u) {
	char buf[512];
	char fname[512];
	Module *list;
	Mod_User *mod_ptr = NULL;
	Mod_Timer *mod_tmr = NULL;

	char *lock= NULL;
	char *moduname= NULL;
	FILE *lockmod;
	FILE *lmfile;

	FILE *modnme;
	FILE *getmname;

	int fmode;
	register int j;

	segv_location = sstrdup("unload_module");
	/* Check to see if this Module has any timers registered....  */
	notice(s_Services, "Unloading Module %s", module_name);
	for (j = 0; j < T_TABLE_SIZE; j++ ) {
		for (mod_tmr = module_timer_lists[j]; mod_tmr; mod_tmr = mod_tmr->next) {
#ifdef DEBUG
			log("Timers %s", mod_tmr->timername);
#endif
			if (!strcasecmp(mod_tmr->modname, module_name)) {
				del_mod_timer(mod_tmr->timername);
				break;
			}
		}
	}


	/* now, see if this Module has any bots with it */
	for (j = 0; j < B_TABLE_SIZE; j++) {
		for (mod_ptr = module_bot_lists[j]; mod_ptr; mod_ptr = mod_ptr->next) { 
			if (!strcasecmp(mod_ptr->modname, module_name)) {
				notice(s_Services,"Module %s had bot %s Registered. Deleting..",module_name,mod_ptr->nick);
				del_bot(mod_ptr->nick, "Module Unloaded");

					segv_location = sstrdup("unload_unlock");
					/* Unlock .so Module File to 755 Permissions */
					/* Shmad Perscribes 666 */

					modnme = fopen("Mod-Name.tmp","w");
					fprintf(modnme, "%s", mod_ptr->modname);
					fclose(modnme);

					getmname = fopen("Mod-Name.tmp", "r");
					while (fgets(fname, sizeof(fname), getmname)) {
						fname[strlen(fname)] = '\0';
						moduname = fname;
					}
					fclose(getmname);
					remove("Mod-Name.tmp");

					/* moduname = module_name; */
					strlower(moduname);

					lockmod = fopen("Neo-Lock.tmp","w");
					fprintf(lockmod, "%s/%s.so", me.modpath, moduname);
					fclose(lockmod); 

					lmfile = fopen("Neo-Lock.tmp", "r");
					while (fgets(buf, sizeof(buf), lmfile)) {
						buf[strlen(buf)] = '\0';
						lock = buf;
					}
					fclose(lmfile);
					remove("Neo-Lock.tmp");

				   /* Set octal perms to 0755 so modules can be read, and wrote to. */
					fmode = 0755;
					chmod(lock, fmode);
				   /* End .so Module 755 Permission Setting */

				break;
			}	
		}
	}
	list = module_list->next;
	while (list != NULL) {
		if (!strcasecmp(list->info->module_name, module_name)) {
			dlclose(list->dl_handle);
			if (list->next != NULL) {
				(list->prev)->next = list->next;
				(list->next)->prev = list->prev;
			} else {
				(list->prev)->next = NULL;
			}
			/* Shmad */
			free(mod_ptr);
			free(mod_tmr);
			free(list);
			/* Shmad */

			return 0;
		}
		list = list->next;
	}
	if (u) {
		privmsg(u->nick,s_Services,"Couldn't Find Module  %s Loaded, Try /msg %s modlist",module_name,s_Services);
		notice(s_Services,"%s tried to Unload %s but its not loaded",u->nick,module_name);
	}
	return -1;
}

