/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000 - 2001 ^Enigma^
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

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dl.h"
#include "hash.h"
#include "stats.h"
#include "config.h"
#include "log.h"




void
__init_mod_list ()
{

	SET_SEGV_LOCATION();
	mh = hash_create (NUM_MODULES, 0, 0);
	bh = hash_create (B_TABLE_SIZE, 0, 0);
	th = hash_create (T_TABLE_SIZE, 0, 0);
	bch = hash_create (C_TABLE_SIZE, 0, 0);
	sockh = hash_create (me.maxsocks, 0, 0);
}

static Mod_Timer *
new_timer (char *timer_name)
{
	Mod_Timer *t;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "New Timer: %s", timer_name);
	t = malloc (sizeof (Mod_Timer));
	if (!timer_name)
		timer_name = "";
	strncpy (t->timername, timer_name, MAXHOST);
	tn = hnode_create (t);
	if (hash_isfull (th)) {
		nlog (LOG_WARNING, LOG_CORE, "new_timer(): Couldn't add new Timer, Hash is Full!");
		return NULL;
	} else {
		hash_insert (th, tn, timer_name);
	}
	return t;
}

Mod_Timer *
findtimer (char *timer_name)
{
	hnode_t *tn;

	tn = hash_lookup (th, timer_name);
	if (tn)
		return (Mod_Timer *) hnode_get (tn);
	return NULL;
}

int
add_mod_timer (char *func_name, char *timer_name, char *mod_name, int interval)
{
	Mod_Timer *Mod_timer_list;

	SET_SEGV_LOCATION();

	if (dlsym ((int *) get_dl_handle (mod_name), func_name) == NULL) {
		nlog (LOG_WARNING, LOG_CORE, "Oh Oh, The Timer Function doesn't exist");
		return -1;
	}
	Mod_timer_list = new_timer (timer_name);
	if (Mod_timer_list) {
		Mod_timer_list->interval = interval;
		Mod_timer_list->lastrun = time (NULL);
		strncpy (Mod_timer_list->modname, mod_name, MAXHOST);
		Mod_timer_list->function = dlsym ((int *) get_dl_handle (mod_name), func_name);
		nlog (LOG_DEBUG2, LOG_CORE, "Registered Module %s with timer for Function %s", mod_name, func_name);
		return 1;
	} else {
		return -1;
	}
}

int
del_mod_timer (char *timer_name)
{
	Mod_Timer *list;
	hnode_t *tn;
	SET_SEGV_LOCATION();
	tn = hash_lookup (th, timer_name);
	if (tn) {
		list = hnode_get (tn);
		nlog (LOG_DEBUG2, LOG_CORE, "Unregistered Timer function %s from Module %s", timer_name, list->modname);
		hash_delete (th, tn);
		hnode_destroy (tn);
		free (list);
		return 1;
	}
	return -1;
}

void
list_module_timer (User * u)
{
	Mod_Timer *mod_ptr = NULL;
	hscan_t ts;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Module timer List:");
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		mod_ptr = hnode_get (tn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_ptr->modname);
		prefmsg (u->nick, s_Services, "Module Timer Name: %s", mod_ptr->timername);
		prefmsg (u->nick, s_Services, "Module Interval: %d", mod_ptr->interval);
		prefmsg (u->nick, s_Services, "Time till next Run: %d", mod_ptr->interval - (time (NULL) - mod_ptr->lastrun));
	}
	prefmsg (u->nick, s_Services, "End of Module timer List");
}

static Sock_List *
new_sock (char *sock_name)
{
	Sock_List *s;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "New Socket: %s", sock_name);
	s = smalloc (sizeof (Sock_List));
	if (!sock_name)
		sock_name = "";
	strncpy (s->sockname, sock_name, MAXHOST);
	sn = hnode_create (s);
	if (hash_isfull (sockh)) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeek, SocketHash is full, can not add a new socket");
		return NULL;
	} else {
		hash_insert (sockh, sn, s->sockname);
	}
	return s;
}

Sock_List *
findsock (char *sock_name)
{
	hnode_t *sn;
	sn = hash_lookup (sockh, sock_name);
	if (sn)
		return hnode_get (sn);
	return NULL;
}

int
add_socket (char *readfunc, char *writefunc, char *errfunc, char *sock_name, int socknum, char *mod_name)
{
	Sock_List *Sockets_mod_list;

	SET_SEGV_LOCATION();
	if (readfunc) {
		if (dlsym ((int *) get_dl_handle (mod_name), readfunc) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "oh oh, the Read socket function doesn't exist = %s (%s)", readfunc, mod_name);
			return -1;
		}
	}
	if (writefunc) {
		if (dlsym ((int *) get_dl_handle (mod_name), writefunc) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "oh oh, the Write socket function doesn't exist = %s (%s)", writefunc, mod_name);
			return -1;
		}
	}
	if (errfunc) {
		if (dlsym ((int *) get_dl_handle (mod_name), errfunc) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "oh oh, the Error socket function doesn't exist = %s (%s)", errfunc, mod_name);
			return -1;
		}
	}
	Sockets_mod_list = new_sock (sock_name);
	Sockets_mod_list->sock_no = socknum;
	strncpy (Sockets_mod_list->modname, mod_name, MAXHOST);
	Sockets_mod_list->readfnc = dlsym ((int *) get_dl_handle (mod_name), readfunc);
	Sockets_mod_list->writefnc = dlsym ((int *) get_dl_handle (mod_name), writefunc);
	Sockets_mod_list->errfnc = dlsym ((int *) get_dl_handle (mod_name), errfunc);
	nlog (LOG_DEBUG2, LOG_CORE, "Registered Module %s with Socket functions %s", mod_name, Sockets_mod_list->sockname);
	return 1;
}

int
del_socket (char *sock_name)
{
	Sock_List *list;
	hnode_t *sn;
	SET_SEGV_LOCATION();
	sn = hash_lookup (sockh, sock_name);
	if (sn) {
		list = hnode_get (sn);
		nlog (LOG_DEBUG2, LOG_CORE, "Unregistered Socket function %s from Module %s", sock_name, list->modname);
		hash_scan_delete (sockh, sn);
		hnode_destroy (sn);
//              free(list->sockname);
		free (list);
		return 1;
	}
	return -1;
}

void
list_sockets (User * u)
{
	Sock_List *mod_ptr = NULL;
	hscan_t ss;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Sockets List: (%d)", hash_count (sockh));
	hash_scan_begin (&ss, sockh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		mod_ptr = hnode_get (sn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_ptr->modname);
		prefmsg (u->nick, s_Services, "Socket Name: %s", mod_ptr->sockname);
		prefmsg (u->nick, s_Services, "Socket Number: %d", mod_ptr->sock_no);
	}
	prefmsg (u->nick, s_Services, "End of Socket List");
}

extern void
add_bot_to_chan (char *bot, char *chan)
{
	hnode_t *cbn;
	Chan_Bot *bc;
	lnode_t *bmn;
	char *botname;
	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		bc = malloc (sizeof (Chan_Bot));
		strncpy (bc->chan, chan, CHANLEN);
		bc->bots = list_create (B_TABLE_SIZE);
		cbn = hnode_create (bc);
		if (hash_isfull (bch)) {
			nlog (LOG_CRITICAL, LOG_CORE, "eek, bot channel hash is full");
			return;
		}
		hash_insert (bch, cbn, bc->chan);
	} else {
		bc = hnode_get (cbn);
	}
	if (list_isfull (bc->bots)) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeek, Bot Channel List is full for Chan %s", chan);
		return;
	}
	botname = sstrdup (bot);
	bmn = lnode_create (botname);
	list_append (bc->bots, bmn);
	return;
}

extern void
del_bot_from_chan (char *bot, char *chan)
{
	hnode_t *cbn;
	Chan_Bot *bc;
	lnode_t *bmn;
	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		nlog (LOG_WARNING, LOG_CORE, "Hu? Can't Find Channel %s for botchanhash", chan);
		return;
	}
	bc = hnode_get (cbn);
	bmn = list_find (bc->bots, bot, comparef);
	if (!bmn) {
		nlog (LOG_WARNING, LOG_CORE, "Hu? Can't find bot %s in %s in botchanhash", bot, chan);
		return;
	}
	list_delete (bc->bots, bmn);
	free (lnode_get (bmn));
	if (list_isempty (bc->bots)) {
		/* delete the hash and list because its all over */
		hash_delete (bch, cbn);
		list_destroy (bc->bots);
		lnode_destroy (bmn);
		free (bc->chan);
		hnode_destroy (cbn);
	}
}

extern void
bot_chan_message (char *origin, char *chan, char **av, int ac)
{
	hnode_t *cbn;
	Chan_Bot *bc;
	lnode_t *bmn;
	Mod_User *u;
	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		/* this isn't bad, just means our bot parted the channel? */
		nlog (LOG_DEBUG1, LOG_CORE, "eeeh, Can't find channel %s for BotChanMessage", chan);
		return;
	}
	bc = hnode_get (cbn);
	bmn = list_first (bc->bots);
	while (bmn) {
		u = findbot (lnode_get (bmn));
		if (u->chanfunc) {
			nlog (LOG_DEBUG2, LOG_CORE, "Running Module for Chanmessage %s", chan);
			u->chanfunc (origin, chan, av, ac);
		}
		bmn = list_next (bc->bots, bmn);
	}



}

extern void
botchandump (User * u)
{
	hscan_t hs;
	hnode_t *hn;
	lnode_t *ln;
	Chan_Bot *bc;
	prefmsg (u->nick, s_Services, "BotChanDump:");
	hash_scan_begin (&hs, bch);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		bc = hnode_get (hn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", bc->chan);
		ln = list_first (bc->bots);
		while (ln) {
			prefmsg (u->nick, s_Services, "Bot Name: %s", lnode_get (ln));
			ln = list_next (bc->bots, ln);
		}
	}
}

static Mod_User *
new_bot (char *bot_name)
{
	Mod_User *u;
	hnode_t *bn;
	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "New Bot: %s", bot_name);
	u = malloc (sizeof (Mod_User));
	if (!bot_name)
		bot_name = "";
	strncpy (u->nick, bot_name, MAXNICK);
	u->chanlist = hash_create (C_TABLE_SIZE, 0, 0);
	bn = hnode_create (u);
	if (hash_isfull (bh)) {
		chanalert (s_Services, "Warning ModuleBotlist is full");
		return NULL;
	} else {
		hash_insert (bh, bn, bot_name);
	}
	return u;
}

int
add_mod_user (char *nick, char *mod_name)
{
	Mod_User *Mod_Usr_list;
	Module *list_ptr;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	Mod_Usr_list = new_bot (nick);
	/* add a brand new user */
	strncpy (Mod_Usr_list->nick, nick, MAXNICK);
	strncpy (Mod_Usr_list->modname, mod_name, MAXHOST);

	mn = hash_lookup (mh, mod_name);
	if (mn) {
		list_ptr = hnode_get (mn);
		Mod_Usr_list->function = dlsym (list_ptr->dl_handle, "__Bot_Message");
		Mod_Usr_list->chanfunc = dlsym (list_ptr->dl_handle, "__Chan_Message");
		return 1;
	}
	nlog (LOG_WARNING, LOG_CORE, "add_mod_user(): Couldn't Add ModuleBot to List");
	return 0;
}


Mod_User *
findbot (char *bot_name)
{
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		return (Mod_User *) hnode_get (bn);
	}
	return NULL;
}

int
del_mod_user (char *bot_name)
{
	Mod_User *list;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		hash_delete (bh, bn);
		list = hnode_get (bn);
		hnode_destroy (bn);
		free (list);
		return 1;
	}
	return -1;



}


int
bot_nick_change (char *oldnick, char *newnick)
{
	User *u;
	Mod_User *mod_tmp, *mod_ptr;

	SET_SEGV_LOCATION();
	/* First, try to find out if the newnick is unique! */
	u = finduser (oldnick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "A non-registered bot(%s) attempted to change its nick to %s", oldnick, newnick);
		return -1;
	}
	u = finduser (newnick);
	if (!u) {
		if ((mod_ptr = findbot (oldnick)) != NULL) {
			nlog (LOG_DEBUG3, LOG_CORE, "Bot %s Changed its nick to %s", oldnick, newnick);
			mod_tmp = new_bot (newnick);

			/* add a brand new user */

			strncpy (mod_tmp->nick, newnick, MAXNICK);
			strncpy (mod_tmp->modname, mod_ptr->modname, MAXHOST);
			mod_tmp->function = mod_ptr->function;

			/* Now Delete the Old bot nick */

			del_mod_user (oldnick);
			snick_cmd (oldnick, newnick);
			return 1;
		}
	}
	nlog (LOG_NOTICE, LOG_CORE, "Couldn't find Bot Nick %s in Bot list", oldnick);
	return -1;
}




void
list_module_bots (User * u)
{
	Mod_User *mod_ptr;
	hnode_t *bn;
	hscan_t bs;
	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Module Bot List:");
	hash_scan_begin (&bs, bh);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		mod_ptr = hnode_get (bn);
		prefmsg (u->nick, s_Services, "Module: %s", mod_ptr->modname);
		prefmsg (u->nick, s_Services, "Module Bots: %s", mod_ptr->nick);
	}
	prefmsg (u->nick, s_Services, "End of Module Bot List");
}




int
load_module (char *path1, User * u)
{


#ifndef HAVE_LIBDL
	const char *dl_error;
#else
	char *dl_error;
#endif
	void *dl_handle;
	int do_msg;
	char path[255];
	char p[255];
	char **av;
	int ac = 0;
	int i = 0;
	Module_Info *(*mod_get_info) () = NULL;
	Functions *(*mod_get_funcs) () = NULL;
	EventFnList *(*mod_get_events) () = NULL;
	Module_Info *mod_info_ptr = NULL;
	Functions *mod_funcs_ptr = NULL;
	EventFnList *event_fn_ptr = NULL;
	Module *mod_ptr = NULL;
	hnode_t *mn;
	int (*doinit) (int modnum, int apiver);


	SET_SEGV_LOCATION();
	if (u == NULL) {
		do_msg = 0;
	} else {
		do_msg = 1;
	}
	snprintf (p, 255, "dl/%s.so", path1);
	dl_handle = dlopen (p, RTLD_NOW || RTLD_GLOBAL);
	CLEAR_SEGV_INMODULE();
	if (!dl_handle) {
		dl_error = dlerror ();
		if (do_msg) {
			prefmsg (u->nick, s_Services, "Error, Couldn't Load Module");
			prefmsg (u->nick, s_Services, "%s", dl_error);
		}
		nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s", dl_error);
		nlog (LOG_WARNING, LOG_CORE, "Module was %s", p);
		return -1;
	}

	mod_get_info = dlsym (dl_handle, "__module_get_info");
#ifndef HAVE_LIBDL
	if (mod_get_info == NULL) {
		dl_error = dlerror ();
#else
	if ((dl_error = dlerror ()) != NULL) {
#endif
		if (do_msg) {
			prefmsg (u->nick, s_Services, "Error, Couldn't Load Module");
			prefmsg (u->nick, s_Services, "%s", dl_error);
		}
		nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s", dl_error);
		dlclose (dl_handle);
		return -1;
	}

	mod_get_funcs = dlsym (dl_handle, "__module_get_functions");
#ifndef HAVE_LIBDL
	if (mod_get_funcs == NULL) {
		dl_error = dlerror ();
#else
	if ((dl_error = dlerror ()) != NULL) {
#endif
		if (do_msg) {
			prefmsg (u->nick, s_Services, "Error, Couldn't Load Module");
			prefmsg (u->nick, s_Services, "%s", dl_error);
		}
		nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s", dl_error);
		dlclose (dl_handle);
		return -1;
	}

	mod_get_events = dlsym (dl_handle, "__module_get_events");
	/* no error check here - this one isn't essential to the functioning of the module */

	mod_info_ptr = (*mod_get_info) ();
	mod_funcs_ptr = (*mod_get_funcs) ();
	if (mod_get_events)
		event_fn_ptr = (*mod_get_events) ();

	if (mod_info_ptr == NULL || mod_funcs_ptr == NULL) {
		dlclose (dl_handle);
		nlog (LOG_WARNING, LOG_CORE, "%s: Could not load dynamic library %s!\n", __PRETTY_FUNCTION__, path);
		nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s", p);
		return -1;
	}

	/* Check that the Module hasn't already been loaded */

	if (hash_lookup (mh, mod_info_ptr->module_name)) {
		dlclose (dl_handle);
		if (do_msg)
			prefmsg (u->nick, s_Services, "Module %s already Loaded, Can't Load 2 Copies", mod_info_ptr->module_name);
		free (mod_ptr);
		return -1;
	}

	mod_ptr = (Module *) smalloc (sizeof (Module));

	mn = hnode_create (mod_ptr);
	if (hash_isfull (mh)) {
		if (do_msg) {
			chanalert (s_Services, "Module List is Full. Can't Load any more modules");
			prefmsg (u->nick, s_Services, "Module List is Full, Can't Load any more Modules");
		}
		dlclose (dl_handle);
		free (mod_ptr);
		return -1;
	} else {
		hash_insert (mh, mn, mod_info_ptr->module_name);
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Module Internal name: %s", mod_info_ptr->module_name);
	nlog (LOG_DEBUG1, LOG_CORE, "Module description: %s", mod_info_ptr->module_description);

	mod_ptr->info = mod_info_ptr;
	mod_ptr->function_list = mod_funcs_ptr;
	mod_ptr->dl_handle = dl_handle;
	mod_ptr->other_funcs = event_fn_ptr;

	/* assign a module number to this module */
	i = 0;
	while (ModNum[i].used != 0)
		i++;
	/* no need to check for overflow of NUM_MODULES, as its done above */
	ModNum[i].used = 1;
	ModNum[i].mod = mod_ptr;
	nlog (LOG_DEBUG1, LOG_CORE, "Assigned %d to Module %s for ModuleNum", i, ModNum[i].mod->info->module_name);


	doinit = dlsym ((int *) dl_handle, "__ModInit");
	if (doinit) {
		SET_SEGV_LOCATION();
		SET_SEGV_INMODULE(mod_ptr->info->module_name);
		if ((*doinit) (i, API_VER) < 1) {
			unload_module(ModNum[i].mod->info->module_name, NULL);
			return -1;
		}
		SET_SEGV_LOCATION();
		CLEAR_SEGV_INMODULE();

	}




	/* Let this module know we are online if we are! */
	if (me.onchan == 1) {
		while (event_fn_ptr->cmd_name != NULL) {
			if (!strcasecmp (event_fn_ptr->cmd_name, "ONLINE")) {
				AddStringToList (&av, me.s->name, &ac);
				SET_SEGV_LOCATION();
				SET_SEGV_INMODULE(mod_ptr->info->module_name);
				event_fn_ptr->function (av, ac);
				SET_SEGV_LOCATION();
				CLEAR_SEGV_INMODULE();
				free (av);
				break;
			}
			event_fn_ptr++;
		}
	}
	if (do_msg) {
		prefmsg (u->nick, s_Services, "Module %s Loaded, Description: %s", mod_info_ptr->module_name, mod_info_ptr->module_description);
		globops (me.name, "%s Module Loaded", mod_info_ptr->module_name);
	}
	return 0;


}
extern int
get_dl_handle (char *mod_name)
{
	Module *list_ptr;
	hnode_t *mn;
	mn = hash_lookup (mh, mod_name);
	if (mn) {
		list_ptr = hnode_get (mn);
		return (int) list_ptr->dl_handle;
	}
	return 0;
}
extern int
get_mod_num (char *mod_name)
{
	int i;
	for (i = 0; i <= NUM_MODULES; i++) {
		if (ModNum[i].used > 0) {
			if (!strcasecmp (ModNum[i].mod->info->module_name, mod_name)) {
				return i;
			}
		}
	}
	/* if we get here, it wasn't found */
	nlog (LOG_DEBUG1, LOG_CORE, "Can't find %s in module number list", mod_name);
	return -1;
};


void
list_module (User * u)
{
	Module *mod_ptr = NULL;
	hnode_t *mn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, mh);
	while ((mn = hash_scan_next (&hs)) != NULL) {
		mod_ptr = hnode_get (mn);
		prefmsg (u->nick, s_Services, "Module: %s (%s)", mod_ptr->info->module_name, mod_ptr->info->module_version);
		prefmsg (u->nick, s_Services, "Module Description: %s", mod_ptr->info->module_description);
		prefmsg (u->nick, s_Services, "Module Number: %d", get_mod_num (mod_ptr->info->module_name));
	}
	prefmsg (u->nick, s_Services, "End of Module List");
}

int
unload_module (char *module_name, User * u)
{
	Module *list;
	Mod_User *mod_ptr = NULL;
	Mod_Timer *mod_tmr = NULL;
	Sock_List *mod_sock = NULL;
	hnode_t *modnode;
	hscan_t hscan;
	int i;
	void (*dofini) ();


	SET_SEGV_LOCATION();
	

	/* Check to see if this Module is loaded....  */
	modnode = hash_lookup (mh, module_name);
	if (modnode) {
		chanalert (s_Services, "Unloading Module %s", module_name);
	} else {
		if (u) {
			prefmsg (u->nick, s_Services, "Couldn't Find Module  %s Loaded, Try /msg %s modlist", module_name, s_Services);
			chanalert (s_Services, "%s tried to Unload %s but its not loaded", u->nick, module_name);
			return -1;
		}
		return -1;
	}

	/* Check to see if this Module has any timers registered....  */
	hash_scan_begin (&hscan, th);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_tmr = hnode_get (modnode);
		if (!strcasecmp (mod_tmr->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "Module %s has timer %s Registered. Deleting..", module_name, mod_tmr->timername);
			del_mod_timer (mod_tmr->timername);
		}
	}
	/* check if the module had a socket registered... */
	hash_scan_begin (&hscan, sockh);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_sock = hnode_get (modnode);
		if (!strcasecmp (mod_sock->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "Module %s had Socket %s Registered. Deleting..", module_name, mod_sock->sockname);
			del_socket (mod_sock->sockname);
		}
	}

	/* now, see if this Module has any bots with it */
	hash_scan_begin (&hscan, bh);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_ptr = hnode_get (modnode);
		if (!strcasecmp (mod_ptr->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "Module %s had bot %s Registered. Deleting..", module_name, mod_ptr->nick);
			del_bot (mod_ptr->nick, "Module Unloaded");
		}
	}

	/* Remove module....  */
	modnode = hash_lookup (mh, module_name);
	if (modnode) {
		nlog (LOG_DEBUG1, LOG_CORE, "Deleting Module %s from ModHash", module_name);
		globops (me.name, "%s Module Unloaded", module_name);

		i = get_mod_num (module_name);

		list = hnode_get (modnode);
		SET_SEGV_INMODULE(module_name);
		dofini = dlsym ((int *) list->dl_handle, "__ModFini");
		if (dofini) {
			(*dofini) ();
		}
		CLEAR_SEGV_INMODULE();


		hash_delete (mh, modnode);
		hnode_destroy (modnode);
		/* set segv in module */
		SET_SEGV_INMODULE(module_name);
		dlclose (list->dl_handle);
		CLEAR_SEGV_INMODULE();

		if (i >= 0) {
			nlog (LOG_DEBUG1, LOG_CORE, "Freeing %d from Module Numbers", i);
			/* free the module number */
			ModNum[i].mod = NULL;
			ModNum[i].used = 0;
		}
		free (list);
		return 1;
	}
	return -1;
}
