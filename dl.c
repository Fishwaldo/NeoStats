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
#include "hash.h"
#include "stats.h"
#include "config.h"


void init_dl() {
	if (usr_mds);
}
void __init_mod_list() {
	strcpy(segv_location, "__init_mod_list");

	mh = hash_create(NUM_MODULES, 0, 0);
	bh = hash_create(B_TABLE_SIZE, 0, 0);
	th = hash_create(T_TABLE_SIZE, 0, 0);
	sockh = hash_create(MAX_SOCKS, 0, 0);
}

static Mod_Timer *new_timer(char *timer_name)
{
	Mod_Timer *t;
	hnode_t *tn;
	strcpy(segv_location, "Mod_Timer");
#ifdef DEBUG
	log("New Timer: %s", timer_name);
#endif
	t = malloc(sizeof(Mod_Timer));
	if (!timer_name)
		timer_name="";
	t->timername = timer_name;	
	tn = hnode_create(t);
	if (hash_isfull(th)) {
		log("new_timer(): Couldn't add new Timer, Hash is Full!");
		return NULL;
	} else {
		hash_insert(th, tn, timer_name);
	}
	return t;
}

Mod_Timer *findtimer(char *timer_name) {
	hnode_t *tn;
	
	strcpy(segv_location, "findtimer");
	tn = hash_lookup(th, timer_name);
	if (tn) return (Mod_Timer *)hnode_get(tn);
	return NULL;
}
int add_mod_timer(char *func_name, char *timer_name, char *mod_name, int interval) {
	Mod_Timer *Mod_timer_list;

	strcpy(segv_location, "add_mod_timer");


	if (dlsym((int *)get_dl_handle(mod_name), func_name) == NULL) {
		log("Oh Oh, The Timer Function doesn't exist");
		return -1;
	}
	Mod_timer_list = new_timer(timer_name);
	Mod_timer_list->interval = interval;
	Mod_timer_list->lastrun = time(NULL);
	Mod_timer_list->modname = sstrdup(mod_name);
	Mod_timer_list->function = dlsym((int *)get_dl_handle(mod_name), func_name);
	log("Registered Module %s with timer for Function %s", mod_name, func_name);
	return 1;
}

int del_mod_timer(char *timer_name) {
	Mod_Timer *list;
	hnode_t *tn;
	strcpy(segv_location, "del_mod_timer");
	
	tn = hash_lookup(th, timer_name);
	if (tn) {
		list = hnode_get(tn);
#ifdef DEBUG
		log("Unregistered Timer function %s from Module %s", timer_name, list->modname);
#endif
		hash_delete(th, tn);
		hnode_destroy(tn);	
		free(list);
		return 1;
	}		
	return -1;
}

void list_module_timer(User *u) {
	Mod_Timer *mod_ptr = NULL;
	hscan_t ts;
	hnode_t *tn;

	strcpy(segv_location, "list_module_timer");
	privmsg(u->nick,s_Services,"Module timer List:");
	hash_scan_begin(&ts, th);
	while ((tn = hash_scan_next(&ts)) != NULL) {
		mod_ptr= hnode_get(tn);
		privmsg(u->nick,s_Services,"%s:--------------------------------",mod_ptr->modname);
		privmsg(u->nick,s_Services,"Module Timer Name: %s",mod_ptr->timername);
		privmsg(u->nick,s_Services,"Module Interval: %d",mod_ptr->interval);
		privmsg(u->nick,s_Services,"Time till next Run: %d", mod_ptr->interval - (time(NULL) - mod_ptr->lastrun));
	}
	privmsg(u->nick,s_Services,"End of Module timer List");
}		

static Sock_List *new_sock(char *sock_name)
{
	Sock_List *s;
	hnode_t *sn;

	strcpy(segv_location, "Sock_List");
#ifdef DEBUG	
	log("New Socket: %s", sock_name);
#endif	
	s = malloc(sizeof(Sock_List));
	if (!sock_name)
		sock_name="";
	s->sockname = sock_name;	
	sn = hnode_create(s);
	if (hash_isfull(sockh)) {
		log("Eeek, SocketHash is full, can not add a new socket");
		return NULL;
	} else {
		hash_insert(sockh, sn, sock_name);
	}
	return s;
}

Sock_List *findsock(char *sock_name) {
	hnode_t *sn;
	strcpy(segv_location, "findsock");
	sn = hash_lookup(sockh, sock_name);
	if (sn) return hnode_get(sn);
	return NULL;
}

int add_socket(char *func_name, char *sock_name, int socknum, char *mod_name) {
	Sock_List *Sockets_mod_list;

	strcpy(segv_location, "add_Socket");

	if (dlsym((int *)get_dl_handle(mod_name), func_name) == NULL) {
		log("oh oh, the socket function doesn't exist");
		return -1;
	}
	Sockets_mod_list = new_sock(sock_name);
	Sockets_mod_list->sock_no = socknum;
	Sockets_mod_list->modname = sstrdup(mod_name);
	Sockets_mod_list->function = dlsym((int *)get_dl_handle(mod_name), func_name);
#ifdef DEBUG
			log("Registered Module %s with Socket for Function %s", mod_name, func_name);
#endif
	return 1;
}

int del_socket(char *sock_name) {
	Sock_List *list;
	hnode_t *sn;
	strcpy(segv_location, "del_mod_timer");
	
	sn = hash_lookup(sockh, sock_name);
	if (sn) {
		list = hnode_get(sn);
#ifdef DEBUG
		log("Unregistered Socket function %s from Module %s", sock_name, list->modname);
#endif
		hash_delete(sockh, sn);
		hnode_destroy(sn);
		free(list);
		return 1;
	}		
	return -1;
}

void list_sockets(User *u) {
	Sock_List *mod_ptr = NULL;
	hscan_t ss;
	hnode_t *sn;

	strcpy(segv_location, "list_sockets");
	privmsg(u->nick,s_Services,"Sockets List: (%d)", hash_count(sockh));
	hash_scan_begin(&ss, sockh);
	while ((sn = hash_scan_next(&ss)) != NULL) {
		mod_ptr = hnode_get(sn);
		privmsg(u->nick,s_Services,"%s:--------------------------------",mod_ptr->modname);
		privmsg(u->nick,s_Services,"Socket Name: %s",mod_ptr->sockname);
		privmsg(u->nick,s_Services,"Socket Number: %d",mod_ptr->sock_no);
	}
	privmsg(u->nick,s_Services,"End of Socket List");
}		


static Mod_User *new_bot(char *bot_name)
{
	Mod_User *u;
	hnode_t *bn;
	strcpy(segv_location, "Mod_User");
#ifdef DEBUG	
	log("New Bot: %s", bot_name);
#endif
	u = malloc(sizeof(Mod_User));
	if (!bot_name)
		bot_name="";
	u->nick = sstrdup(bot_name);	
	bn = hnode_create(u);
	if (hash_isfull(bh)) {
		notice(s_Services, "Warning ModuleBotlist is full");
		return NULL;
	} else {
		hash_insert(bh, bn, bot_name);
	}
	return u;
}

int add_mod_user(char *nick, char *mod_name) {
	Mod_User *Mod_Usr_list;
	Module *list_ptr;
	hnode_t *mn;	

	strcpy(segv_location, "add_mod_user");


	Mod_Usr_list = new_bot(nick);
	/* add a brand new user */
	Mod_Usr_list->nick = sstrdup(nick);
	Mod_Usr_list->modname = sstrdup(mod_name);
	
	mn = hash_lookup(mh, mod_name);
	if (mn) {
		list_ptr = hnode_get(mn);
		Mod_Usr_list->function = dlsym(list_ptr->dl_handle, "__Bot_Message");
		return 1;
	}
	log("add_mod_user(): Tried to add a bot, bot Module does not support bots!");
	return 0;
}


Mod_User *findbot(char *bot_name) {
	hnode_t *bn;
	
	strcpy(segv_location, "findbot");
	bn = hash_lookup(bh, bot_name);
	if (bn) {
		return (Mod_User *)hnode_get(bn);
	} 
	return NULL;
}

int del_mod_user(char *bot_name) {
	Mod_User *list;
	hnode_t *bn;
	
	strcpy(segv_location, "del_mod_user");
		
	bn = hash_lookup(bh, bot_name);
	if (bn) {
		hash_delete(bh, bn);
		list = hnode_get(bn);
		hnode_destroy(bn);
		free(list);
		return 1;
	}		
	return -1;



}


int bot_nick_change(char *oldnick, char *newnick)
{
	User *u;
	Mod_User *mod_tmp, *mod_ptr;

	strcpy(segv_location, "bot_nick_change");

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
		if ((mod_ptr = findbot(oldnick)) != NULL) {
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
			snick_cmd(oldnick, newnick);
			return 1;
		}
		}
	log("Couldn't find Bot Nick %s in Bot list", oldnick);
	return -1;
}




void list_module_bots(User *u) {
	Mod_User *mod_ptr;
	hnode_t *bn;
	hscan_t bs;
	strcpy(segv_location, "list_module_bots");

	privmsg(u->nick,s_Services,"Module Bot List:");
	hash_scan_begin(&bs, bh);
	while ((bn = hash_scan_next(&bs)) != NULL) {
		mod_ptr = hnode_get(bn);
		privmsg(u->nick,s_Services,"Module: %s",mod_ptr->modname);
		privmsg(u->nick,s_Services,"Module Bots: %s",mod_ptr->nick);
	}
	privmsg(u->nick,s_Services,"End of Module Bot List");
}		




int load_module(char *path1, User *u) {

	
#ifndef HAVE_LIBDL
	const char *dl_error;
#else 
	char *dl_error;
#endif
	void *dl_handle;
	int do_msg;
	char *path = NULL;
	char p[255];
#ifndef DEBUG
	char buf[512];
	int fmode;
	char *lock= NULL;
	FILE *lockmod;
	FILE *lmfile;
#endif
	Module_Info * (*mod_get_info)() = NULL;
	Functions * (*mod_get_funcs)() = NULL;
	EventFnList * (*mod_get_events)() = NULL;
	Module_Info *mod_info_ptr = NULL;	
	Functions *mod_funcs_ptr = NULL;
	EventFnList *event_fn_ptr = NULL;
	Module *mod_ptr = NULL;
	hnode_t *mn;

	strcpy(segv_location, "load_module");

	if (u == NULL) {
		do_msg = 0;
	} else {
		do_msg = 1;
	}
	path = sstrdup(path1);
	path = strcat(path,".so");

	dl_handle = dlopen(path, RTLD_NOW || RTLD_GLOBAL); 
	if (!dl_handle) {
		snprintf(p, 255, "%s/%s", me.modpath, path);
		dl_handle = dlopen(p, RTLD_NOW || RTLD_GLOBAL); 
	}
	if (!dl_handle) {
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dlerror());
		log("Couldn't Load Module: %s", dlerror());
		return -1;
	}

	mod_get_info = dlsym(dl_handle, "__module_get_info");
#ifndef HAVE_LIBDL
        if (mod_get_info == NULL) {
	        dl_error = dlerror();
#else
        if ((dl_error = dlerror()) != NULL) {
#endif
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dl_error);
		log("Couldn't Load Module: %s", dl_error);
		dlclose(dl_handle);
		return -1;
	}

	mod_get_funcs = dlsym(dl_handle, "__module_get_functions");
#ifndef HAVE_LIBDL
        if (mod_get_info == NULL) {
                dl_error = dlerror();
#else
        if ((dl_error = dlerror()) != NULL) {
#endif
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dl_error);
		log("Couldn't Load Module: %s", dl_error);
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
	
	if (hash_lookup(mh, mod_info_ptr->module_name)) {
			dlclose(dl_handle);
			if (do_msg) privmsg(u->nick,s_Services,"Module %s already Loaded, Can't Load 2 Copies",mod_info_ptr->module_name);
			free(mod_ptr);
			return -1;
	}
	
	mod_ptr = (Module *)malloc(sizeof(Module));

	mn = hnode_create(mod_ptr);
	if (hash_isfull(mh)) {
		if (do_msg) notice(s_Services, "Module List is Full. Can't Load any more modules");	
		if (do_msg) privmsg(u->nick, s_Services, "Module List is Full, Can't Load any more Modules");
		dlclose(dl_handle);
		free(mod_ptr);
		return -1;
	} else {
		hash_insert(mh, mn, mod_info_ptr->module_name);
	}
	log("internal module name: %s", mod_info_ptr->module_name);
	log("module description: %s", mod_info_ptr->module_description);

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
#ifndef DEBUG
	/* Lock .so Module File With 555 Permissions */
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
#endif
	if (do_msg) privmsg(u->nick,s_Services,"Module %s Loaded, Description: %s",mod_info_ptr->module_name,mod_info_ptr->module_description);
	return 0;


}
extern int get_dl_handle(char *mod_name) {
	Module *list_ptr;
	hnode_t *mn;
	strcpy(segv_location, "get_dl_handle");


	mn = hash_lookup(mh, mod_name);
	if (mn) {
		list_ptr=hnode_get(mn);
		return (int)list_ptr->dl_handle;
	}
	return 0;
}




void list_module(User *u) {
	Module *mod_ptr = NULL;
	hnode_t *mn;
	hscan_t hs;
	
	strcpy(segv_location, "list_module");
	hash_scan_begin(&hs, mh);
	while ((mn = hash_scan_next(&hs)) != NULL) {
		mod_ptr=hnode_get(mn);
		privmsg(u->nick,s_Services,"Module: %s (%s)",mod_ptr->info->module_name, mod_ptr->info->module_version);
		privmsg(u->nick,s_Services,"Module Description: %s",mod_ptr->info->module_description);
	}
	privmsg(u->nick,s_Services,"End of Module List");
}		

int unload_module(char *module_name, User *u) {
	char buf[512];
	char fname[512];
	Module *list;
	Mod_User *mod_ptr = NULL;
	Mod_Timer *mod_tmr = NULL;
	Sock_List *mod_sock = NULL;
	hnode_t *modnode;
	hscan_t hscan;

	char *lock= NULL;
	char *moduname= NULL;
	FILE *lockmod;
	FILE *lmfile;

	FILE *modnme;
	FILE *getmname;

	int fmode;

	strcpy(segv_location, "unload_module");
	/* Check to see if this Module has any timers registered....  */
	notice(s_Services, "Unloading Module %s", module_name);

	hash_scan_begin(&hscan, th);
	while ((modnode = hash_scan_next(&hscan)) != NULL) {
		mod_tmr = hnode_get(modnode);
		if (!strcasecmp(mod_tmr->modname, module_name)) {
			notice(s_Services, "Module %s has timer %s Registered. Deleting..", module_name, mod_tmr->timername);
			del_mod_timer(mod_tmr->timername);
		}
	}
	/* check if the module had a socket registered... */
	hash_scan_begin(&hscan, sockh);
	while ((modnode = hash_scan_next(&hscan)) != NULL) {
		mod_sock = hnode_get(modnode);
		if (!strcasecmp(mod_sock->modname, module_name)) {
			notice (s_Services, "Module %s had Socket %s Registered. Deleting..", module_name, mod_sock->sockname);
			del_socket(mod_sock->sockname);
		}
	}

	/* now, see if this Module has any bots with it */
	hash_scan_begin(&hscan, bh);
	while ((modnode = hash_scan_next(&hscan)) != NULL) {
		mod_ptr = hnode_get(modnode);
		if (!strcasecmp(mod_ptr->modname, module_name)) {
			notice(s_Services,"Module %s had bot %s Registered. Deleting..",module_name,mod_ptr->nick);
			del_bot(mod_ptr->nick, "Module Unloaded");
		}
	}
	strcpy(segv_location, "unload_unlock");
	/* Unlock .so Module File to 755 Permissions */
	modnme = fopen("Mod-Name.tmp","w");
	fprintf(modnme, "%s", module_name);
	fclose(modnme);

	getmname = fopen("Mod-Name.tmp", "r");
	while (fgets(fname, sizeof(fname), getmname)) {
		fname[strlen(fname)] = '\0';
		moduname = fname;
	}
	fclose(getmname);
	remove("Mod-Name.tmp");

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

	fmode = 0755;
	chmod(lock, fmode);
	/* End .so Module 755 Permission Setting */

	modnode = hash_lookup(mh, module_name);
	if (modnode) {
#ifdef DEBUG
		log("Deleting Module %s from ModHash", module_name);
#endif
		list = hnode_get(modnode);
		hash_delete(mh, modnode);
		hnode_destroy(modnode);
		dlclose(list->dl_handle);
		free(list);
		return 1;
	} else {
		if (u) {
			privmsg(u->nick,s_Services,"Couldn't Find Module  %s Loaded, Try /msg %s modlist",module_name,s_Services);
			notice(s_Services,"%s tried to Unload %s but its not loaded",u->nick,module_name);
		}
	}
	return -1;
}

