/*
 * dl.c
 * dynamic runtime library loading routines
 */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dl.h"
#include "stats.h"

Module *module_list;
LD_Path *ld_path_list;
Mod_User *module_bot_lists;
Mod_Timer *module_timer_lists;

void __init_mod_list() {
	Mod_User *Mod_Usr_list;
	
	segv_location = "__init_mod_list";
	Mod_Usr_list = (Mod_User *)malloc(sizeof(Mod_User));
	bzero(Mod_Usr_list, sizeof(Mod_User));
	Mod_Usr_list->prev = NULL;
	Mod_Usr_list->next = NULL;
	module_list = (Module *)malloc(sizeof(Module));
	bzero(module_list, sizeof(Module));
	module_list->prev = NULL;
	module_list->next = NULL;
	ld_path_list = (LD_Path *)malloc(sizeof(LD_Path));
	bzero(ld_path_list, sizeof(LD_Path));
	ld_path_list->prev = NULL;
	ld_path_list->next = NULL;
	module_bot_lists = (Mod_User *)malloc(sizeof(Mod_User));
	bzero(module_bot_lists, sizeof(Mod_User));
	module_bot_lists->next = NULL;
	module_bot_lists->prev = NULL;
	module_timer_lists = (Mod_Timer *)malloc(sizeof(Mod_Timer));
	bzero(module_timer_lists, sizeof(Mod_Timer));
	module_timer_lists->next = NULL;
	module_timer_lists->prev = NULL;	
};

int add_ld_path(char *path) {
	LD_Path *path_ent, *list;
	
	segv_location = "add_ld_path";
	path_ent = (LD_Path *)malloc(sizeof(LD_Path));
	
	bzero(path_ent, sizeof(LD_Path));

	strncpy(path_ent->dl_path, path, 100);

	list = ld_path_list;

	while (list->next != NULL) list = list->next;

	list->next = path_ent;
	path_ent->prev = list;
	return 0;
}

int add_mod_timer(char *func_name, char *timer_name, char *mod_name, int interval) {
	Mod_Timer *Mod_timer_list;
	Module *list_ptr;

	segv_location = "add_mod_timer";
	Mod_timer_list = (Mod_Timer *)malloc(sizeof(Mod_Timer));
	
	bzero(Mod_timer_list, sizeof(Mod_Timer));

	while (module_timer_lists->next != NULL) module_timer_lists = module_timer_lists->next;


	module_timer_lists->next = Mod_timer_list;
	Mod_timer_list->prev = module_timer_lists;
	Mod_timer_list->interval = interval;
	Mod_timer_list->lastrun = time(NULL);
	Mod_timer_list->modname = sstrdup(mod_name);
	Mod_timer_list->timername = sstrdup(timer_name);
		
	list_ptr = module_list->next;
	while (list_ptr != NULL) {
		if (!strcasecmp(list_ptr->info->module_name, mod_name)) {
			Mod_timer_list->function = dlsym(list_ptr->dl_handle, func_name);
			log("Registered Module %s with timer for Function %s", list_ptr->info->module_name, func_name);
			return 1;
		}
		list_ptr = list_ptr->next;
	}
	return 0;
}

int del_mod_timer(char *timer_name) {
	Mod_Timer *list;

	segv_location = "del_mod_timer";
	list = module_timer_lists->next;
	while (list != NULL) {
		if (!strcasecmp(list->timername, timer_name)) {
			log("Unregistered Timer function %s from Module %s", timer_name, list->modname);
			if (list->next != NULL) {
				(list->prev)->next = list->next;
				(list->next)->prev = list->prev;
			} else {
				(list->prev)->next = NULL;
			}
			free(list);
			log("Done Timer Unload");
			return 0;
		}
		list = list->next;
	}
	return -1;
}

void list_module_timer(User *u) {
	Mod_Timer *mod_ptr = NULL;
	segv_location = "list_module_timer";
	mod_ptr = module_timer_lists->next;
	privmsg(u->nick,s_Services,"Module timer List:");
	while(mod_ptr != NULL) {
		privmsg(u->nick,s_Services,"%s:--------------------------------",mod_ptr->modname);
		privmsg(u->nick,s_Services,"Module Timer Name: %s",mod_ptr->timername);
		privmsg(u->nick,s_Services,"Module Interval: %d",mod_ptr->interval);
		privmsg(u->nick,s_Services,"Time till next Run: %d", mod_ptr->interval - (time(NULL) - mod_ptr->lastrun));
		mod_ptr = mod_ptr->next;
	}
	privmsg(u->nick,s_Services,"End of Module timer List");
	free(mod_ptr);
}		


int bot_nick_change(char *oldnick, char *newnick)
{
	User *u;
	Mod_User *mod_ptr = NULL;
	segv_location = "bot_nick_change";

	/* First, try to find out if the newnick is unique! */
	log("oldnick %s newnick %s",oldnick,newnick);
	u = finduser(oldnick);
	if (!u) {
		log("A non-registered bot(%s) attempted to change its nick to %s",oldnick, newnick);
		return -1;
		}
	u = finduser(newnick);
	if (!u) { 
		mod_ptr = module_bot_lists;
		while(mod_ptr != NULL ) {
			if (!strcasecmp(mod_ptr->nick, oldnick)) {
				/* Ok, found the nick, change it */
				log("Bot %s Changed its nick to %s", oldnick, newnick);
				mod_ptr->nick = sstrdup(newnick);
				Change_User(finduser(oldnick), newnick);
				sts(":%s NICK %s", oldnick, newnick);
				return 1;
			}
			mod_ptr = mod_ptr->next;
		}
		log("Couldn't find Bot Nick %s in Bot list", oldnick);
		return -1;
	}
	return -1; 
}	


int add_mod_user(char *nick, char *mod_name) {
	Mod_User *Mod_Usr_list;
	Module *list_ptr;
	
	segv_location = "add_mod_user";
	Mod_Usr_list = (Mod_User *)malloc(sizeof(Mod_User));
	bzero(Mod_Usr_list, sizeof(Mod_User));

	if (module_bot_lists->nick == NULL) {
		/* add a brand new user */
		Mod_Usr_list->nick = sstrdup(nick);
		Mod_Usr_list->modname = sstrdup(mod_name);
		module_bot_lists = Mod_Usr_list;		
	} else {
		while (module_bot_lists->next != NULL) {
			module_bot_lists = module_bot_lists->next;
		}
		module_bot_lists->next = Mod_Usr_list;	
		Mod_Usr_list->prev = module_bot_lists;
		Mod_Usr_list->nick = sstrdup(nick);
		Mod_Usr_list->modname = sstrdup(mod_name);
	}		
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

int del_mod_user(char *nick) {
	Mod_User *list;

	segv_location = "del_mod_user";
	list = module_bot_lists->next;
	while (list != NULL) {
		if (!strcasecmp(list->nick, nick)) {
			if (list->next != NULL) {
				(list->prev)->next = list->next;
				(list->next)->prev = list->prev;
			} else {
				(list->prev)->next = NULL;
			}
			free(list);
			return 0;
		}
		list = list->next;
	}
	return -1;
}

void list_module_bots(User *u) {
	Mod_User *mod_ptr = NULL;
	segv_location = "list_module_bots";
	mod_ptr = module_bot_lists;
	privmsg(u->nick,s_Services,"Module Bot List:");
	while(mod_ptr != NULL) {
		privmsg(u->nick,s_Services,"Module: %s",mod_ptr->modname);
		privmsg(u->nick,s_Services,"Module Bots: %s",mod_ptr->nick);
		mod_ptr = mod_ptr->next;
	}
	privmsg(u->nick,s_Services,"End of Module Bot List");
}		




int load_module(char *path1, User *u) {
	char *dl_error = NULL;
	void *dl_handle = NULL;
	int do_msg;
	char *path = NULL;
	Module_Info * (*mod_get_info)() = NULL;
	Functions * (*mod_get_funcs)() = NULL;
	EventFnList * (*mod_get_events)() = NULL;
	Module_Info *mod_info_ptr = NULL;	
	Functions *mod_funcs_ptr = NULL;
	EventFnList *event_fn_ptr = NULL;
	Module *mod_ptr = NULL, *list_ptr = NULL;

	segv_location = "load_module";
	if (u == NULL) {
		do_msg = 0;
	} else {
		do_msg = 1;
	}
	path = sstrdup(path1);
	path = strcat(path,".so");
	dl_handle = dlopen(path, RTLD_NOW);
	if (!dl_handle) {
		LD_Path *list;
		list = ld_path_list->next;
		while (list != NULL) {
			char p[255];
			snprintf(p, 255, "%s/%s", list->dl_path, path);
			dl_handle = dlopen(p, RTLD_NOW);
			if (!dl_handle) {
				list = list->next;
			} else {
				list = NULL;
			}
		}
		if (!dl_handle) {
			if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
			if (do_msg) privmsg(u->nick, s_Services, "%s",dlerror());
			return -1;
		}
	}

	mod_get_info = dlsym(dl_handle, "__module_get_info");
	if ((dl_error = dlerror()) != NULL) {
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dl_error);
		dlclose(dl_handle);
		return -1;
	}

	mod_get_funcs = dlsym(dl_handle, "__module_get_functions");
	if ((dl_error = dlerror()) != NULL) {
		if (do_msg) privmsg(u->nick, s_Services, "Error, Couldn't Load Module");
		if (do_msg) privmsg(u->nick, s_Services, "%s",dl_error);
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

	log("%s: internal module name: %s\n", __PRETTY_FUNCTION__, mod_info_ptr->module_name);
	log("%s: module description: %s\n", __PRETTY_FUNCTION__, mod_info_ptr->module_description);

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
				event_fn_ptr->function();
				break;
			}
			event_fn_ptr++;
		}
	}
	if (do_msg) privmsg(u->nick,s_Services,"Module %s Loaded, Description: %s",mod_info_ptr->module_name,mod_info_ptr->module_description);
	
	return 0;


};
void list_module(User *u) {
	Module *mod_ptr = NULL;
	segv_location = "list_module";
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
	Module *list;
	Mod_User *mod_ptr = NULL;
	Mod_Timer *mod_tmr = NULL;

	segv_location = "unload_module";
	/* Check to see if this Module has any timers registered....  */
	mod_tmr = module_timer_lists->next;
	while(mod_tmr != NULL) {
		if (!strcasecmp(mod_tmr->modname, module_name)) {
			del_mod_timer(mod_tmr->timername);
			break;
		}
		mod_tmr=mod_tmr->next;
	}			


	/* now, see if this Module has any bots with it */
	mod_ptr = module_bot_lists;
	while(mod_ptr != NULL) {
		if (!strcasecmp(mod_ptr->modname, module_name)) {
			notice(s_Services,"Module %s had bot %s Registered. Deleting..",module_name,mod_ptr->nick);
			del_bot(mod_ptr->nick, "Module Unloaded");
			break;
		}	
		mod_ptr=mod_ptr->next;		
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
			free(list); 

			return 0;
		}
		list = list->next;
	}
	privmsg(u->nick,s_Services,"Couldn't Find Module  %s Loaded, Try /msg %s modlist",module_name,s_Services);
	notice(s_Services,"%s tried to Unload %s but its not loaded",u->nick,module_name);
	return -1;
}

