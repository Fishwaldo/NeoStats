/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

/** @file dl.c 
 *  @brief module functions
 */ 

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "stats.h"
#include "dl.h"
#include "hash.h"
#include "config.h"
#include "log.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

#define MAX_CMD_LINE_LENGTH		350

/** @brief Module list
 * 
 */
Module *ModList[NUM_MODULES];

char segv_location[SEGV_LOCATION_BUFSIZE];
char segv_inmodule[SEGV_INMODULE_BUFSIZE];

int del_all_bot_cmds(ModUser* bot_ptr);

/* @brief Module hash list */
static hash_t *mh;
/* @brief Module Socket List hash */
hash_t *sockh;
/* @brief Module Timer hash list */
static hash_t *th;
/* @brief Module Bot hash list */
static hash_t *bh;
/* @brief Module Chan Bot hash list */
static hash_t *bch;

#ifdef SQLSRV

char sqlbuf[BUFSIZE];


void *display_module_name (void *tbl, char *col, char *sql, void *row) {
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->module_name, MAX_MOD_NAME);
	return sqlbuf;	
}

void *display_module_desc (void *tbl, char *col, char *sql, void *row) {
	Module *mod_ptr = row;
	
	strlcpy(sqlbuf, mod_ptr->info->module_description, BUFSIZE);
	return sqlbuf;
}

void *display_module_version (void *tbl, char *col, char *sql, void *row) {
	Module *mod_ptr = row;
	
	if (mod_ptr->isnewstyle == 1) {
		strlcpy(sqlbuf, mod_ptr->info->module_version, BUFSIZE);
		return sqlbuf;
	} else {
		bzero(sqlbuf, BUFSIZE);
		return sqlbuf;
	}
}

void *display_module_builddate (void *tbl, char *col, char *sql, void *row) {
	Module *mod_ptr = row;
	
	if (mod_ptr->isnewstyle == 1) {
		ircsnprintf(sqlbuf, BUFSIZE, "%s - %s", mod_ptr->info->module_build_date, mod_ptr->info->module_build_time);
		return sqlbuf;
	} else {
		bzero(sqlbuf, BUFSIZE);
		return sqlbuf;
	}
}

void *display_core_info (void *tbl, char *col, char *sql, void *row) {
	ircsnprintf(sqlbuf, BUFSIZE, "%d.%d.%d - %s", MAJOR, MINOR, REV, ircd_version);
	return sqlbuf;	
}

COLDEF neo_modulecols[] = {
	{
		"modules",
		"name",
		RTA_STR,
		MAX_MOD_NAME,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_name,
		NULL,
		"The name of the Module"
	},
	{
		"modules",
		"description",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_desc, 
		NULL,
		"The Module Description"
	},
	{
		"modules",
		"version",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_version,
		NULL,
		"The module version"
	},
	{
		"modules",
		"builddate",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_module_builddate,
		NULL,
		"The module build date"
	},
	{
		"modules",
		"coreinfo",
		RTA_STR,
		BUFSIZE,
		offsetof(struct Module, info),
		RTA_READONLY,
		display_core_info,
		NULL,
		"The NeoStats core Version"
	},
};

TBLDEF neo_modules = {
	"modules",
	NULL, 	/* for now */
	sizeof(struct Module),
	0,
	TBL_HASH,
	neo_modulecols,
	sizeof(neo_modulecols) / sizeof(COLDEF),
	"",
	"The list of Modules loaded by NeoStats"
};
#endif /* SQLSRV */





/** @brief Initialise module list hashes
 *
 * For core use only, initialises module list hashes
 *
 * @param none
 * 
 * @return none
*/
int 
InitModuleHash ()
{
	SET_SEGV_LOCATION();
	mh = hash_create (NUM_MODULES, 0, 0);
	if(!mh)
		return NS_FAILURE;
	bh = hash_create (B_TABLE_SIZE, 0, 0);
	if(!bh)
		return NS_FAILURE;
	th = hash_create (T_TABLE_SIZE, 0, 0);
	if(!th)
		return NS_FAILURE;
	bch = hash_create (C_TABLE_SIZE, 0, 0);
	if(!bch)
		return NS_FAILURE;
	sockh = hash_create (me.maxsocks, 0, 0);
	if(!sockh)
		return NS_FAILURE;

#ifdef SQLSRV
        /* add the module hash to the sql library */
	neo_modules.address = mh;
	rta_add_table(&neo_modules);
#endif
	                        

	return NS_SUCCESS;
}

#ifdef DEBUG
void verify_hashes(void)
{
	if (hash_verify (sockh) == 0) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the socket hash");
	}
	if (hash_verify (mh) == 0) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the Module hash");
	}
	if (hash_verify (bh) == 0) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the Bot hash");
	}
	if (hash_verify (th) == 0) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Corruption of the Timer hash");
	}
}
#endif /* DEBUG */

/** @brief create new timer
 *
 * For core use only, creates a timer
 *
 * @param timer_name the name of new timer
 * 
 * @return pointer to new timer on success, NULL on error
*/
static ModTimer *
new_timer (char *timer_name)
{
	ModTimer *mod_tmr;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "new_timer: %s", timer_name);
	mod_tmr = malloc (sizeof (ModTimer));
	strlcpy (mod_tmr->timername, timer_name, MAX_MOD_NAME);
	tn = hnode_create (mod_tmr);
	if (hash_isfull (th)) {
		nlog (LOG_WARNING, LOG_CORE, "new_timer: couldn't add new timer, hash is full!");
		return NULL;
	}
	hash_insert (th, tn, timer_name);
	return mod_tmr;
}

/** @brief find timer
 *
 * For core use only, finds a timer in the current list of timers
 *
 * @param timer_name the name of timer to find
 * 
 * @return pointer to timer if found, NULL if not found
*/
ModTimer *
findtimer (char *timer_name)
{
	hnode_t *tn;

	tn = hash_lookup (th, timer_name);
	if (tn)
		return (ModTimer *) hnode_get (tn);
	return NULL;
}

/** @brief add a timer to the timer list
 *
 * For module use. Adds a timer with the given function to the timer list
 *
 * @param func_name the name of function to register with this timer
 * @param timer_name the name of timer to register
 * @param mod_name the name of module registering the timer
 * @param interval the interval at which the timer triggers in seconds
 * 
 * @return NS_SUCCESS if added, NS_FAILURE if not 
*/
int
add_mod_timer (char *func_name, char *timer_name, char *mod_name, int interval)
{
	ModTimer *mod_tmr;

	SET_SEGV_LOCATION();
	if (dlsym ((int *) get_dl_handle (mod_name), func_name) == NULL) {
		nlog (LOG_WARNING, LOG_CORE, "%s: Timer %s Function %s doesn't exist", mod_name, timer_name, func_name);
		return NS_FAILURE;
	}
	mod_tmr = new_timer (timer_name);
	if (mod_tmr) {
		mod_tmr->interval = interval;
		mod_tmr->lastrun = me.now;
		strlcpy (mod_tmr->modname, mod_name, MAX_MOD_NAME);
		mod_tmr->function = dlsym ((int *) get_dl_handle (mod_name), func_name);
		nlog (LOG_DEBUG2, LOG_CORE, "add_mod_timer: Registered Module %s with timer for Function %s", mod_name, func_name);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete a timer from the timer list
 *
 * For module use. Deletes a timer with the given name from the timer list
 *
 * @param timer_name the name of timer to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_mod_timer (char *timer_name)
{
	ModTimer *mod_tmr;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (th, timer_name);
	if (tn) {
		mod_tmr = hnode_get (tn);
		nlog (LOG_DEBUG2, LOG_CORE, "del_mod_timer: Unregistered Timer function %s from Module %s", timer_name, mod_tmr->modname);
		hash_delete (th, tn);
		hnode_destroy (tn);
		free (mod_tmr);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete all timers from the timer list for given module
 *
 * For core use. 
 *
 * @param 
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_mod_timers (char *module_name)
{
	ModTimer *mod_tmr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, th);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_tmr = hnode_get (modnode);
		if (!strcasecmp (mod_tmr->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "del_mod_timers: Module %s has timer %s Registered. Deleting..", module_name, mod_tmr->timername);
			del_mod_timer (mod_tmr->timername);
		}
	}
	return NS_SUCCESS;
}

/** @brief delete a timer from the timer list
 *
 * For module use. Deletes a timer with the given name from the timer list
 *
 * @param timer_name the name of timer to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
change_mod_timer_interval (char *timer_name, int interval)
{
	ModTimer *mod_tmr;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	tn = hash_lookup (th, timer_name);
	if (tn) {
		mod_tmr = hnode_get (tn);
		mod_tmr->interval = interval;
		nlog (LOG_DEBUG2, LOG_CORE, "change_mod_timer_interval: Changing timer interval for %s from Module %s", timer_name, mod_tmr->modname);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief list timers in use
 *
 * NeoStats command to list the current timers from IRC
 *
 * @param u pointer to user structure of the user issuing the request
 * 
 * @return none
*/
int
list_timers (User * u, char **av, int ac)
{
	ModTimer *mod_tmr = NULL;
	hscan_t ts;
	hnode_t *tn;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Module timer List:");
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		mod_tmr = hnode_get (tn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_tmr->modname);
		prefmsg (u->nick, s_Services, "Module Timer Name: %s", mod_tmr->timername);
		prefmsg (u->nick, s_Services, "Module Interval: %d", mod_tmr->interval);
		prefmsg (u->nick, s_Services, "Time till next Run: %ld", mod_tmr->interval - (me.now - mod_tmr->lastrun));
	}
	prefmsg (u->nick, s_Services, "End of Module timer List");
	return 0;
}

/** @brief run pending module timer functions
 *
 * NeoStats command to run pending timer functions
 *
 * @param none
 * 
 * @return none
*/
void
run_mod_timers (void)
{
	ModTimer *mod_tmr = NULL;
	hscan_t ts;
	hnode_t *tn;

/* First, lets see if any modules have a function that is due to run..... */
	hash_scan_begin (&ts, th);
	while ((tn = hash_scan_next (&ts)) != NULL) {
		SET_SEGV_LOCATION();
		mod_tmr = hnode_get (tn);
		if (me.now - mod_tmr->lastrun > mod_tmr->interval) {
			strlcpy (segv_location, mod_tmr->modname, SEGV_LOCATION_BUFSIZE);
			SET_SEGV_INMODULE(mod_tmr->modname);
			if (setjmp (sigvbuf) == 0) {
				if (mod_tmr->function () < 0) {
					nlog(LOG_DEBUG2, LOG_CORE, "run_mod_timers: Deleting Timer %s for Module %s as requested", mod_tmr->timername, mod_tmr->modname);
					hash_scan_delete(th, tn);
					hnode_destroy(tn);
					free(mod_tmr);
				} else {
					mod_tmr->lastrun = (int) me.now;
				}
			} else {
				nlog (LOG_CRITICAL, LOG_CORE, "run_mod_timers: setjmp() failed, can't call module %s\n", mod_tmr->modname);
			}
			CLEAR_SEGV_INMODULE();
		}
	}
}

/** @brief create a new socket
 *
 * For core use only, creates a new socket for a module
 *
 * @param sock_name the name of the socket to create
 * 
 * @return pointer to created socket on success, NULL on error
*/
static ModSock *
new_sock (char *sock_name)
{
	ModSock *mod_sock;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "new_sock: %s", sock_name);
	mod_sock = smalloc (sizeof (ModSock));
	strlcpy (mod_sock->sockname, sock_name, MAX_MOD_NAME);
	sn = hnode_create (mod_sock);
	if (hash_isfull (sockh)) {
		nlog (LOG_CRITICAL, LOG_CORE, "new_sock: socket hash is full, can't add a new socket");
		return NULL;
	}
	hash_insert (sockh, sn, mod_sock->sockname);
	return mod_sock;
}

/** \fn @brief find socket
 *
 * For core use only, finds a socket in the current list of socket
 *
 * @param sock_name the name of socket to find
 * 
 * @return pointer to socket if found, NULL if not found
 */
ModSock *
findsock (char *sock_name)
{
	hnode_t *sn;

	sn = hash_lookup (sockh, sock_name);
	if (sn)
		return hnode_get (sn);
	return NULL;
}

/** @brief add a socket to the socket list
 *
 * For core use. Adds a socket with the given functions to the socket list
 *
 * @param readfunc the name of read function to register with this socket
 * @param writefunc the name of write function to register with this socket
 * @param errfunc the name of error function to register with this socket
 * @param sock_name the name of socket to register
 * @param socknum the socket number to register with this socket
 * @param mod_name the name of module registering the socket
 * 
 * @return pointer to socket if found, NULL if not found
*/
int
add_socket (char *readfunc, char *writefunc, char *errfunc, char *sock_name, int socknum, char *mod_name)
{
	ModSock *mod_sock;

	SET_SEGV_LOCATION();
	if (readfunc) {
		if (dlsym ((int *) get_dl_handle (mod_name), readfunc) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "add_socket: read socket function doesn't exist = %s (%s)", readfunc, mod_name);
			return NS_FAILURE;
		}
	}
	if (writefunc) {
		if (dlsym ((int *) get_dl_handle (mod_name), writefunc) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "add_socket: write socket function doesn't exist = %s (%s)", writefunc, mod_name);
			return NS_FAILURE;
		}
	}
	if (errfunc) {
		if (dlsym ((int *) get_dl_handle (mod_name), errfunc) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "add_socket: error socket function doesn't exist = %s (%s)", errfunc, mod_name);
			return NS_FAILURE;
		}
	}
	mod_sock = new_sock (sock_name);
	mod_sock->sock_no = socknum;
	strlcpy (mod_sock->modname, mod_name, MAX_MOD_NAME);
	mod_sock->readfnc = dlsym ((int *) get_dl_handle (mod_name), readfunc);
	mod_sock->writefnc = dlsym ((int *) get_dl_handle (mod_name), writefunc);
	mod_sock->errfnc = dlsym ((int *) get_dl_handle (mod_name), errfunc);
	mod_sock->socktype = SOCK_STANDARD;
	
	nlog (LOG_DEBUG2, LOG_CORE, "add_socket: Registered Module %s with Standard Socket functions %s", mod_name, mod_sock->sockname);
	return NS_SUCCESS;
}

/** @brief add a poll interface to the socket list
 *
 * For core use. Adds a socket with the given functions to the socket list
 *
 * @param beforepoll the name of function to call before we select
 * @param afterpoll the name of the function to call after we select
 * @param sock_name the name of socket to register
 * @param mod_name the name of module registering the socket
 * 
 * @return pointer to socket if found, NULL if not found
*/
int
add_sockpoll (char *beforepoll, char *afterpoll, char *sock_name, char *mod_name, void *data)
{
	ModSock *mod_sock;

	SET_SEGV_LOCATION();
	if (beforepoll) {
		if (dlsym ((int *) get_dl_handle (mod_name), beforepoll) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "add_sockpoll: read socket function doesn't exist = %s (%s)", beforepoll, mod_name);
			return NS_FAILURE;
		}
	}
	if (afterpoll) {
		if (dlsym ((int *) get_dl_handle (mod_name), afterpoll) == NULL) {
			nlog (LOG_WARNING, LOG_CORE, "add_sockpoll: write socket function doesn't exist = %s (%s)", afterpoll, mod_name);
			return NS_FAILURE;
		}
	}
	mod_sock = new_sock (sock_name);
	strlcpy (mod_sock->modname, mod_name, MAX_MOD_NAME);
	mod_sock->socktype = SOCK_POLL;
	mod_sock->beforepoll = dlsym ((int *) get_dl_handle (mod_name), beforepoll);
	mod_sock->afterpoll = dlsym ((int *) get_dl_handle (mod_name), afterpoll);
	mod_sock->data = data;
	nlog (LOG_DEBUG2, LOG_CORE, "add_sockpoll: Registered Module %s with Poll Socket functions %s", mod_name, mod_sock->sockname);
	return NS_SUCCESS;
}

/** @brief delete a socket from the socket list
 *
 * For module use. Deletes a socket with the given name from the socket list
 *
 * @param socket_name the name of socket to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_socket (char *sock_name)
{
	ModSock *mod_sock;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	sn = hash_lookup (sockh, sock_name);
	if (sn) {
		mod_sock = hnode_get (sn);
		nlog (LOG_DEBUG2, LOG_CORE, "del_socket: Unregistered Socket function %s from Module %s", sock_name, mod_sock->modname);
		hash_scan_delete (sockh, sn);
		hnode_destroy (sn);
		free (mod_sock);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete a socket from the socket list
 *
 * For module use. Deletes a socket with the given name from the socket list
 *
 * @param socket_name the name of socket to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int
del_sockets (char *module_name)
{
	ModSock *mod_sock;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, sockh);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_sock = hnode_get (modnode);
		if (!strcasecmp (mod_sock->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "del_sockets: Module %s had Socket %s Registered. Deleting..", module_name, mod_sock->sockname);
			del_socket (mod_sock->sockname);
		}
	}
	return NS_SUCCESS;
}

/** @brief list sockets in use
 *
 * NeoStats command to list the current sockets from IRC
 *
 * @param u pointer to user structure of the user issuing the request
 * 
 * @return none
*/
int
list_sockets (User * u, char **av, int ac)
{
	ModSock *mod_sock = NULL;
	hscan_t ss;
	hnode_t *sn;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Sockets List: (%d)", (int)hash_count (sockh));
	hash_scan_begin (&ss, sockh);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		mod_sock = hnode_get (sn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_sock->modname);
		prefmsg (u->nick, s_Services, "Socket Name: %s", mod_sock->sockname);
		if (mod_sock->socktype == SOCK_STANDARD) {
			prefmsg (u->nick, s_Services, "Socket Number: %d", mod_sock->sock_no);
		} else {
			prefmsg (u->nick, s_Services, "Poll Interface");
		}
	}
	prefmsg (u->nick, s_Services, "End of Socket List");
	return 0;
}

/** @brief Add a bot to a channel
 *
 * @param bot string containing bot name
 * @param chan string containing channel name
 * 
 * @return none
 */
void
add_bot_to_chan (char *bot, char *chan)
{
	hnode_t *cbn;
	ModChanBot *mod_chan_bot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		mod_chan_bot = malloc (sizeof (ModChanBot));
		strlcpy (mod_chan_bot->chan, chan, CHANLEN);
		mod_chan_bot->bots = list_create (B_TABLE_SIZE);
		cbn = hnode_create (mod_chan_bot);
		if (hash_isfull (bch)) {
			nlog (LOG_CRITICAL, LOG_CORE, "eek, bot channel hash is full");
			return;
		}
		hash_insert (bch, cbn, mod_chan_bot->chan);
	} else {
		mod_chan_bot = hnode_get (cbn);
	}
	if (list_isfull (mod_chan_bot->bots)) {
		nlog (LOG_CRITICAL, LOG_CORE, "add_bot_to_chan: bot channel list is full adding channel %s", chan);
		return;
	}
	botname = sstrdup (bot);
	bmn = lnode_create (botname);
	list_append (mod_chan_bot->bots, bmn);
	return;
}

/** @brief delete a bot from a channel
 *
 * @param bot string containing bot name
 * @param chan string containing channel name
 * 
 * @return none
 */
void
del_bot_from_chan (char *bot, char *chan)
{
	hnode_t *cbn;
	ModChanBot *mod_chan_bot;
	lnode_t *bmn;
	char *botname;

	cbn = hash_lookup (bch, chan);
	if (!cbn) {
		nlog (LOG_WARNING, LOG_CORE, "del_bot_from_chan: can't find channel %s for botchanhash", chan);
		return;
	}
	mod_chan_bot = hnode_get (cbn);
	bmn = list_find (mod_chan_bot->bots, bot, comparef);
	if (!bmn) {
		nlog (LOG_WARNING, LOG_CORE, "del_bot_from_chan: can't find bot %s in %s in botchanhash", bot, chan);
		return;
	}
	list_delete (mod_chan_bot->bots, bmn);
	botname = lnode_get(bmn);
	free (botname);
	lnode_destroy (bmn);
	if (list_isempty (mod_chan_bot->bots)) {
		/* delete the hash and list because its all over */
		hash_delete (bch, cbn);
		list_destroy (mod_chan_bot->bots);
		free (mod_chan_bot->chan);
		hnode_destroy (cbn);
	}
}

/** @brief send a message to a channel bot
 *
 * @param origin 
 * @param av (note chan string in av[0])
 * @param ac
 * 
 * @return none
 */
void
bot_chan_message (char *origin, char **av, int ac)
{
	hnode_t *cbn;
	ModChanBot *mod_chan_bot;
	lnode_t *bmn;
	ModUser *mod_usr;

	cbn = hash_lookup (bch, av[0]);
	if (!cbn) {
		/* this isn't bad, just means our bot parted the channel? */
		nlog (LOG_DEBUG1, LOG_CORE, "bot_chan_message: can't find channel %s", av[0]);
		return;
	}
	mod_chan_bot = hnode_get (cbn);
	bmn = list_first (mod_chan_bot->bots);
	while (bmn) {
		mod_usr = findbot (lnode_get (bmn));
		if (mod_usr->chanfunc) {
			nlog (LOG_DEBUG2, LOG_CORE, "bot_chan_message: running module for chanmessage %s", av[0]);
			SET_SEGV_INMODULE(mod_usr->modname);
			mod_usr->chanfunc (origin, av, ac);
			CLEAR_SEGV_INMODULE();
		}
		bmn = list_next (mod_chan_bot->bots, bmn);
	}
}

/** @brief send a message to a channel bot
 *
 * @param origin 
 * @param av (note chan string in av[0])
 * @param ac
 * 
 * @return none
 */
void
bot_message (char *origin, char **av, int ac)
{
	User *u;
	ModUser *mod_usr;

	/* Check command length */
	if (strnlen (av[1], MAX_CMD_LINE_LENGTH) >= MAX_CMD_LINE_LENGTH) {
		prefmsg (origin, s_Services, "command line too long!");
		notice (s_Services, "%s tried to send a very LARGE command, we told them to shove it!", origin);
		return;
	}

	u = finduser (origin);

	if (flood (u)) {
		return;
	}

	mod_usr = findbot (av[0]);
	/* Check to see if any of the Modules have this nick Registered */
	if (!mod_usr) {
		nlog (LOG_DEBUG1, LOG_CORE, "bot_message: %s not found: %s", mod_usr->nick);
		return;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "bot_message: %s", mod_usr->nick);

	SET_SEGV_LOCATION();
	SET_SEGV_INMODULE(mod_usr->modname);
	if (setjmp (sigvbuf) == 0) {
		if(mod_usr->function) {
			mod_usr->function (origin, av, ac);
		}
		else {
			/* Trap CTCP commands and silently drop them to avoid unknown command errors 
			* Why bother? Well we might be able to use some of them in the future
			* so this is mainly a test and we may want to pass some of this onto
			* SecureServ for a quick trojan check so log attempts to give an indication 
			* of usage.
			*/
			if (av[1][0] == '\1') {
				char* buf;
				buf = joinbuf(av, ac, 1);
				nlog (LOG_NORMAL, LOG_MOD, "%s requested CTCP %s", origin, buf);
				free(buf);
				return;
			}
			if (!u) {
				nlog (LOG_WARNING, LOG_CORE, "Unable to finduser %s (%s)", origin, mod_usr->nick);
			} else {
				run_bot_cmd(mod_usr, u, av, ac);
			}
		}
	}
	CLEAR_SEGV_INMODULE();
	return;
}

/** @brief dump list of module bots and channels
 *
 * @param u
 * 
 * @return none
 */
int
list_bot_chans (User * u, char **av, int ac)
{
	hscan_t hs;
	hnode_t *hn;
	lnode_t *ln;
	ModChanBot *mod_chan_bot;

	prefmsg (u->nick, s_Services, "BotChanDump:");
	hash_scan_begin (&hs, bch);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		mod_chan_bot = hnode_get (hn);
		prefmsg (u->nick, s_Services, "%s:--------------------------------", mod_chan_bot->chan);
		ln = list_first (mod_chan_bot->bots);
		while (ln) {
			prefmsg (u->nick, s_Services, "Bot Name: %s", (char *)lnode_get (ln));
			ln = list_next (mod_chan_bot->bots, ln);
		}
	}
	return 0;
}

/** @brief create a new bot
 *
 * @param bot_name string containing bot name
 * 
 * @return none
 */
static ModUser *
new_bot (char *bot_name)
{
	ModUser *mod_usr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "new_bot: %s", bot_name);
	mod_usr = malloc (sizeof (ModUser));
	strlcpy (mod_usr->nick, bot_name, MAXNICK);
	bn = hnode_create (mod_usr);
	if (hash_isfull (bh)) {
		chanalert (s_Services, "Warning ModuleBotlist is full");
		return NULL;
	}
	hash_insert (bh, bn, mod_usr->nick);
	return mod_usr;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
ModUser *
add_mod_user (char *nick, char *mod_name)
{
	ModUser *mod_usr;
	Module *mod_ptr;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	mn = hash_lookup (mh, mod_name);
	if (mn) {
		mod_ptr = hnode_get (mn);
		if(mod_ptr) {
			/* add a brand new user */
			mod_usr = new_bot (nick);
			if(mod_usr) {
				strlcpy (mod_usr->modname, mod_name, MAX_MOD_NAME);
				mod_usr->function = dlsym (mod_ptr->dl_handle, "__BotMessage");
				mod_usr->chanfunc = dlsym (mod_ptr->dl_handle, "__ChanMessage");
				mod_usr->botcmds = hash_create(-1, 0, 0);
				return mod_usr;
			}
		}
	}
	nlog (LOG_WARNING, LOG_CORE, "add_mod_user: Couldn't Add ModuleBot to List");
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
ModUser *
add_neostats_mod_user (char *nick)
{
	ModUser *mod_usr;

	SET_SEGV_LOCATION();
	/* add a brand new user */
	mod_usr = new_bot (nick);
	if(mod_usr) {
		strlcpy (mod_usr->modname, "NeoStats", MAX_MOD_NAME);
		mod_usr->function = NULL;
		mod_usr->chanfunc = NULL;
		mod_usr->botcmds = hash_create(-1, 0, 0);
		return mod_usr;
	}
	nlog (LOG_WARNING, LOG_CORE, "add_neostats_mod_user: Couldn't Add ModuleBot to List");
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
ModUser *
findbot (char *bot_name)
{
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		return (ModUser *) hnode_get (bn);
	}
	return NULL;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
del_mod_user (char *bot_name)
{
	ModUser *mod_usr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup (bh, bot_name);
	if (bn) {
		hash_delete (bh, bn);
		mod_usr = hnode_get (bn);
		del_all_bot_cmds(mod_usr);
		hnode_destroy (bn);
		free (mod_usr);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
bot_nick_change (char *oldnick, char *newnick)
{
	User *u;
	ModUser *mod_usr_new, *mod_usr;

	SET_SEGV_LOCATION();
	/* First, try to find out if the newnick is unique! */
	u = finduser (oldnick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "A non-registered bot(%s) attempted to change its nick to %s", oldnick, newnick);
		return NS_FAILURE;
	}
	u = finduser (newnick);
	if (!u) {
		if ((mod_usr = findbot (oldnick)) != NULL) {
			nlog (LOG_DEBUG3, LOG_CORE, "Bot %s Changed its nick to %s", oldnick, newnick);
			mod_usr_new = new_bot (newnick);

			/* add a brand new user */ 
			strlcpy (mod_usr_new->nick, newnick, MAXNICK);
			strlcpy (mod_usr_new->modname, mod_usr->modname, MAX_MOD_NAME);
			mod_usr_new->function = mod_usr->function;

			/* Now Delete the Old bot nick */   
			del_mod_user (oldnick);
			snick_cmd (oldnick, newnick);
			return NS_SUCCESS;
		}
	}
	nlog (LOG_NOTICE, LOG_CORE, "Couldn't find Bot Nick %s in Bot list", oldnick);
	return NS_FAILURE;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
list_bots (User * u, char **av, int ac)
{
	ModUser *mod_usr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	prefmsg (u->nick, s_Services, "Module Bot List:");
	hash_scan_begin (&bs, bh);
	while ((bn = hash_scan_next (&bs)) != NULL) {
		mod_usr = hnode_get (bn);
		prefmsg (u->nick, s_Services, "Module: %s", mod_usr->modname);
		prefmsg (u->nick, s_Services, "Module Bots: %s", mod_usr->nick);
	}
	prefmsg (u->nick, s_Services, "End of Module Bot List");
	return 0;
}

/** @brief del_bots
 *
 * 
 *
 * @return none
 */
int	del_bots (char* module_name)
{
	ModUser *mod_usr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin (&hscan, bh);
	while ((modnode = hash_scan_next (&hscan)) != NULL) {
		mod_usr = hnode_get (modnode);
		if (!strcasecmp (mod_usr->modname, module_name)) {
			nlog (LOG_DEBUG1, LOG_CORE, "Module %s had bot %s Registered. Deleting..", module_name, mod_usr->nick);
			del_bot (mod_usr->nick, "Module Unloaded");
		}
	}
	return NS_SUCCESS;
}
/** @brief ModuleEvent
 *
 * 
 *
 * @return none
 */
void
ModuleEvent (char *event, char **av, int ac)
{
	Module *module_ptr;
	EventFnList *ev_list;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		ev_list = module_ptr->event_list;
		if (ev_list) {
			while (ev_list->cmd_name != NULL) {
				/* This goes through each Command */
				if (!strcasecmp (ev_list->cmd_name, event)) {
					nlog (LOG_DEBUG1, LOG_CORE, "Running Module %s for Command %s -> %s", module_ptr->info->module_name, event, ev_list->cmd_name);
					SET_SEGV_LOCATION();
					SET_SEGV_INMODULE(module_ptr->info->module_name);
					if (setjmp (sigvbuf) == 0) {
						if (ev_list->function) ev_list->function (av, ac);
					} else {
						nlog (LOG_CRITICAL, LOG_CORE, "setjmp() Failed, Can't call Module %s\n", module_ptr->info->module_name);
					}
					CLEAR_SEGV_INMODULE();
					SET_SEGV_LOCATION();
#ifndef VALGRIND
					break;
#endif
				}
				ev_list++;
			}
		}
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void
ModuleFunction (int cmdptr, char *cmd, char* origin, char **av, int ac)
{
	Module *module_ptr;
	Functions *fn_list;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		fn_list = module_ptr->function_list;
		if(fn_list) {
			while (fn_list->cmd_name != NULL) {
				/* This goes through each Command */
				if (fn_list->srvmsg == cmdptr) {
					if (!strcmp (fn_list->cmd_name, cmd)) {
						nlog (LOG_DEBUG1, LOG_CORE, "Running Module %s for Function %s", module_ptr->info->module_name, fn_list->cmd_name);
						SET_SEGV_LOCATION();
						SET_SEGV_INMODULE(module_ptr->info->module_name);
						if (setjmp (sigvbuf) == 0) {
							fn_list->function (origin, av, ac);
						}
						CLEAR_SEGV_INMODULE();
						SET_SEGV_LOCATION();
						break;
					}
				}
				fn_list++;
			}
		}
	}
}

/** @brief 
 *
 * 
 *
 * @return none
 */
void
ModulesVersion (const char* nick, const char *remoteserver)
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *mn;

	SET_SEGV_LOCATION();
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		module_ptr = hnode_get (mn);
		if(module_ptr->isnewstyle && module_ptr->function_list == NULL) {
			numeric(RPL_VERSION, nick,
				"Module %s version: %s %s %s",
				module_ptr->info->module_name, module_ptr->info->module_version, 
				module_ptr->info->module_build_date, module_ptr->info->module_build_time);
		}
	}
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
load_module (char *modfilename, User * u)
{
#ifndef HAVE_LIBDL
	const char *dl_error;
#else /* HAVE_LIBDL */
	char *dl_error;
#endif /* HAVE_LIBDL */
	void *dl_handle;
	int do_msg;
	char path[255];
	char loadmodname[255];
	char **av;
	int ac = 0;
	int i = 0;
	int isnewstyle = 1;
#ifdef OLD_MODULE_EXPORT_SUPPORT
	ModuleInfo *(*mod_get_info) () = NULL;
	Functions *(*mod_get_funcs) () = NULL;
	EventFnList *(*mod_get_events) () = NULL;
#endif /* OLD_MODULE_EXPORT_SUPPORT */
	ModuleInfo *mod_info_ptr = NULL;
	Functions *mod_funcs_ptr = NULL;
	EventFnList *event_fn_ptr = NULL;
	Module *mod_ptr = NULL;
	hnode_t *mn;
	int (*ModInit) (int modnum, int apiver);

	SET_SEGV_LOCATION();
	if (u == NULL) {
		do_msg = 0;
	} else {
		do_msg = 1;
	}
	strlcpy (loadmodname, modfilename, 255);
	strlwr (loadmodname);
	ircsnprintf (path, 255, "%s/%s.so", MOD_PATH, loadmodname);
	dl_handle = dlopen (path, RTLD_NOW || RTLD_GLOBAL);
	CLEAR_SEGV_INMODULE();
	if (!dl_handle) {
		dl_error = dlerror ();
		if (do_msg) {
			prefmsg (u->nick, s_Services, "Couldn't Load Module: %s %s", dl_error, path);
		}
		nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s %s", dl_error, path);
		return NS_FAILURE;
	}

	/* new system */
	mod_info_ptr = dlsym (dl_handle, "__module_info");
#ifdef OLD_MODULE_EXPORT_SUPPORT
	if(mod_info_ptr == NULL) {
		/* old system */
		isnewstyle = 0;
		mod_get_info = dlsym (dl_handle, "__module_get_info");
#ifndef HAVE_LIBDL
		if (mod_get_info == NULL) {
			dl_error = dlerror ();
#else /* HAVE_LIBDL */
		if ((dl_error = dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
			if (do_msg) {
				prefmsg (u->nick, s_Services, "Couldn't Load Module: %s %s", dl_error, path);
			}
			nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s %s", dl_error, path);
			dlclose (dl_handle);
			return NS_FAILURE;
		}
		mod_info_ptr = (*mod_get_info) ();
		if (mod_info_ptr == NULL) {
			dlclose (dl_handle);
			nlog (LOG_WARNING, LOG_CORE, "Module has no info structure: %s", path);
			return NS_FAILURE;
		}
	}
#else /* OLD_MODULE_EXPORT_SUPPORT */
#ifndef HAVE_LIBDL
	if(mod_info_ptr == NULL) {
		dl_error = dlerror ();
#else /* HAVE_LIBDL */
	if ((dl_error = dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		if (do_msg) {
			prefmsg (u->nick, s_Services, "Couldn't Load Module: %s %s", dl_error, path);
		}
		nlog (LOG_WARNING, LOG_CORE, "Couldn't Load Module: %s %s", dl_error, path);
		dlclose (dl_handle);
		return NS_FAILURE;
	}
#endif /* OLD_MODULE_EXPORT_SUPPORT */

	/* new system */
	mod_funcs_ptr = dlsym (dl_handle, "__module_functions");
#ifdef OLD_MODULE_EXPORT_SUPPORT
	if(mod_funcs_ptr == NULL) {
		/* old system */
		mod_get_funcs = dlsym (dl_handle, "__module_get_functions");
		/* no error check here - this one isn't essential to the functioning of the module */

		if (mod_get_funcs) {
			mod_funcs_ptr = (*mod_get_funcs) ();
		}
	}
#endif /* OLD_MODULE_EXPORT_SUPPORT */

	/* new system */
	event_fn_ptr = dlsym (dl_handle, "__module_events");
#ifdef OLD_MODULE_EXPORT_SUPPORT
	if(event_fn_ptr == NULL)
	{
		/* old system */
		mod_get_events = dlsym (dl_handle, "__module_get_events");
		/* no error check here - this one isn't essential to the functioning of the module */

		if (mod_get_events)
			event_fn_ptr = (*mod_get_events) ();
	}
#endif

	/* Check that the Module hasn't already been loaded */
	if (hash_lookup (mh, mod_info_ptr->module_name)) {
		dlclose (dl_handle);
		if (do_msg)
			prefmsg (u->nick, s_Services, "Module %s already Loaded, Can't Load 2 Copies", mod_info_ptr->module_name);
		return NS_FAILURE;
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
		return NS_FAILURE;
	} else {
		hash_insert (mh, mn, mod_info_ptr->module_name);
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Module Internal name: %s", mod_info_ptr->module_name);
	nlog (LOG_DEBUG1, LOG_CORE, "Module description: %s", mod_info_ptr->module_description);

	mod_ptr->info = mod_info_ptr;
	mod_ptr->function_list = mod_funcs_ptr;
	mod_ptr->dl_handle = dl_handle;
	mod_ptr->event_list = event_fn_ptr;
	mod_ptr->isnewstyle = isnewstyle;

	/* assign a module number to this module */
	i = 0;
	while (ModList[i] != NULL)
		i++;
	/* no need to check for overflow of NUM_MODULES, as its done above */
	ModList[i] = mod_ptr;
	nlog (LOG_DEBUG1, LOG_CORE, "Assigned %d to Module %s for ModuleNum", i, mod_ptr->info->module_name);

	/* call __ModInit (replacement for library __init() call */
	ModInit = dlsym ((int *) dl_handle, "__ModInit");
	if (ModInit) {
		SET_SEGV_LOCATION();
		SET_SEGV_INMODULE(mod_ptr->info->module_name);
		if ((*ModInit) (i, API_VER) < 1) {
			nlog (LOG_NORMAL, LOG_CORE, "Unable to load module %s. See %s.log for further information.", mod_ptr->info->module_name, mod_ptr->info->module_name);
			unload_module(mod_ptr->info->module_name, NULL);
			return NS_FAILURE;
		}
		SET_SEGV_LOCATION();
		CLEAR_SEGV_INMODULE();

	}

	/* Let this module know we are online if we are! */
	if (me.onchan == 1) {
		while (event_fn_ptr->cmd_name != NULL) {
			if (!strcasecmp (event_fn_ptr->cmd_name, EVENT_ONLINE)) {
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
	return NS_SUCCESS;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
get_dl_handle (char *mod_name)
{
	Module *mod_ptr;
	hnode_t *mn;

	mn = hash_lookup (mh, mod_name);
	if (mn) {
		mod_ptr = hnode_get (mn);
		return (int) mod_ptr->dl_handle;
	}
	return 0;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
get_mod_num (char *mod_name)
{
	int i;

	for (i = 0; i < NUM_MODULES; i++) {
		if (ModList[i] != NULL) {
			if (!strcasecmp (ModList[i]->info->module_name, mod_name)) {
				return i;
			}
		}
	}
	/* if we get here, it wasn't found */
	nlog (LOG_DEBUG1, LOG_CORE, "get_mod_num: can't find %s in module number list", mod_name);
	return NS_FAILURE;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
list_modules (User * u, char **av, int ac)
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
	return 0;
}

/** @brief 
 *
 * @param 
 * 
 * @return
 */
int
unload_module (char *module_name, User * u)
{
	Module *mod_ptr;
	hnode_t *modnode;
	int i;
	void (*ModFini) ();

	SET_SEGV_LOCATION();
	/* Check to see if this Module is loaded....  */
	modnode = hash_lookup (mh, module_name);
	if (modnode) {
		chanalert (s_Services, "Unloading Module %s", module_name);
	} else {
		if (u) {
			prefmsg (u->nick, s_Services, "Couldn't Find Module %s Loaded, Try /msg %s modlist", module_name, s_Services);
			chanalert (s_Services, "%s tried to Unload %s but its not loaded", u->nick, module_name);
		}
		return NS_FAILURE;
	}

	/* Check to see if this Module has any timers registered....  */
	del_mod_timers (module_name);

	/* check if the module had a socket registered... */
	del_sockets (module_name);

	/* Remove module....  */
	modnode = hash_lookup (mh, module_name);
	if (modnode) {
		nlog (LOG_DEBUG1, LOG_CORE, "Deleting Module %s from ModHash", module_name);
		globops (me.name, "%s Module Unloaded", module_name);

		i = get_mod_num (module_name);

		mod_ptr = hnode_get (modnode);
		SET_SEGV_INMODULE(module_name);
		
		/* call __ModFini (replacement for library __fini() call */
		ModFini = dlsym ((int *) mod_ptr->dl_handle, "__ModFini");
		if (ModFini) {
			(*ModFini) ();
		}
		CLEAR_SEGV_INMODULE();
		/* now, see if this Module has any bots with it 
		 * we delete the modules *after* we call ModFini, so the bot 
		 * can still send messages generated from ModFini calls */
		del_bots(module_name);

		/* Remove hash */
		hash_delete (mh, modnode);
		hnode_destroy (modnode);
		/* Close module */
		SET_SEGV_INMODULE(module_name);
#ifndef VALGRIND
		dlclose (mod_ptr->dl_handle);
#endif
		CLEAR_SEGV_INMODULE();

		if (i >= 0) {
			nlog (LOG_DEBUG1, LOG_CORE, "Freeing %d from Module Numbers", i);
			/* free the module number */
			ModList[i] = NULL;
		}
		free (mod_ptr);
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief unload all loaded modules
 *
 * For core use only, unloads all loaded modules
 *
 * @param user pointer to user unloading modules or NULL if system unload
 * 
 * @return none
*/
void unload_modules(void)
{
	Module *mod_ptr;
	hscan_t ms;
	hnode_t *mn;
	/* Unload the Modules */
	hash_scan_begin (&ms, mh);
	while ((mn = hash_scan_next (&ms)) != NULL) {
		mod_ptr = hnode_get (mn);
		unload_module (mod_ptr->info->module_name, NULL);
	}
}
