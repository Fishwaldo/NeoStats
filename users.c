/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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

#include "stats.h"
#include "ircd.h"
#include "hash.h"
#include "dl.h"
#include "log.h"
#include "users.h"
#include "chans.h"

hash_t *uh;

static void doDelUser (const char *nick, int killflag);
static User *new_user (const char *nick);

static User *
new_user (const char *nick)
{
	User *u;
	hnode_t *un;

	SET_SEGV_LOCATION();
	u = smalloc (sizeof (User));
	if (!nick)
		strsetnull (u->nick);
	else
		strlcpy (u->nick, nick, MAXNICK);
	un = hnode_create (u);
	if (hash_isfull (uh)) {
		nlog (LOG_CRITICAL, LOG_CORE, "Eeeek, Hash is full");
	} else {
		hash_insert (uh, un, u->nick);
	}
	return (u);
}

void
AddUser (const char *nick, const char *user, const char *host, const char *realname, const char *server, const unsigned long ipaddr, const unsigned long TS)
{
	char **av;
	int ac = 0;
	User *u;
	int i;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "AddUser(): %s (%s@%s) %s (%d) -> %s at %lu", nick, user, host, realname, (int)htonl (ipaddr), server, (unsigned long)TS);
	u = finduser (nick);
	if (u) {
		nlog (LOG_WARNING, LOG_CORE, "trying to add a user that already exists? (%s)", nick);
		return;
	}

	u = new_user (nick);
	strlcpy (u->hostname, host, MAXHOST);
	strlcpy (u->username, user, MAXUSER);
	strlcpy (u->realname, realname, MAXREALNAME);
	u->server = findserver (server);
	u->t_flood = me.now;
	u->flood = 0;
	u->is_away = 0;
	u->Umode = 0;
#ifdef GOTUSERSMODES
	u->Smode = 0;
#endif
	u->chans = list_create (MAXJOINCHANS);
	u->modes[0]= '\0';
	u->ipaddr.s_addr = htonl (ipaddr);
	u->TS = TS;
	
	/* make sure the module pointers are all null */
	for (i = 0; i < NUM_MODULES; i++) {
		u->moddata[i] = NULL;
	}

	AddStringToList (&av, u->nick, &ac);
	ModuleEvent (EVENT_SIGNON, av, ac);
	free (av);
}

void
UserPart (list_t * list, lnode_t * node, void *v)
{
	part_chan ((User *)v, lnode_get (node));
}

void
KillUser (const char *nick)
{
	doDelUser (nick, 1);
}

void
UserQuit (const char *nick, const char *quitmsg)
{
	doDelUser (nick, 0);
}

static void
doDelUser (const char *nick, int killflag)
{
	User *u;
	hnode_t *un;
	char **av;
	int ac = 0;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, LOG_CORE, "UserQuit(%s)", nick);

	un = hash_lookup (uh, nick);
	if (!un) {
		nlog (LOG_WARNING, LOG_CORE, "UserQuit(%s) failed!", nick);
		return;
	}
	u = hnode_get (un);

	list_process (u->chans, u, UserPart);

	/* run the event to delete a user */
	AddStringToList (&av, u->nick, &ac);
	if (killflag == 0) {
		ModuleEvent (EVENT_SIGNOFF, av, ac);
	} else if (killflag == 1) {
		ModuleEvent (EVENT_KILL, av, ac);
	}
	free (av);

	/* if its one of our bots, remove it from the modlist */
	if (findbot (u->nick)) {
		if (killflag == 1)
			nlog (LOG_NOTICE, LOG_CORE, "Deleting Bot %s as it was killed", u->nick);
		del_mod_user (u->nick);
	}

	hash_delete (uh, un);
	hnode_destroy (un);
	list_destroy (u->chans);
	free (u);
}

void
UserAway (User * u, const char *awaymsg)
{
	char **av;
	int ac = 0;
	if (u) {
		AddStringToList (&av, u->nick, &ac);
		if ((u->is_away == 1) && (!awaymsg)) {
			u->is_away = 0;
			ModuleEvent (EVENT_AWAY, av, ac);
			free (av);
		} else if ((u->is_away == 0) && (awaymsg)) {
			u->is_away = 1;
			AddStringToList (&av, (char *) awaymsg, &ac);
			ModuleEvent (EVENT_AWAY, av, ac);
			free (av);
		}
	}
}

int 
UserNick (const char * oldnick, const char *newnick)
{
	hnode_t *un;
	lnode_t *cm;
	char **av;
	int ac = 0;
	User * u;

	SET_SEGV_LOCATION();
	u = finduser (oldnick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "UserNick: can't find user %s", oldnick);
		return NS_FAILURE;
	}
	nlog (LOG_DEBUG2, LOG_CORE, "UserNick(%s, %s)", u->nick, newnick);
	un = hash_lookup (uh, u->nick);
	if (!un) {
		nlog (LOG_WARNING, LOG_CORE, "UserNick(%s) Failed!", u->nick);
		return NS_FAILURE;
	}
	cm = list_first (u->chans);
	while (cm) {
		change_user_nick (findchan (lnode_get (cm)), (char *) newnick, u->nick);
		cm = list_next (u->chans, cm);
	}
	SET_SEGV_LOCATION();
	hash_delete (uh, un);
	strlcpy (u->nick, newnick, MAXNICK);
	hash_insert (uh, un, u->nick);
	AddStringToList (&av, oldnick, &ac);
	AddStringToList (&av, u->nick, &ac);
	ModuleEvent (EVENT_NICKCHANGE, av, ac);
	free (av);
	return NS_SUCCESS;
}

User *
finduser (const char *nick)
{
	User *u;
	hnode_t *un;
	un = hash_lookup (uh, nick);
	if (un != NULL) {
		u = hnode_get (un);
		return u;
	}
	nlog (LOG_DEBUG2, LOG_CORE, "FindUser(%s) -> NOTFOUND", nick);
	return NULL;
}


int
init_user_hash ()
{
	uh = hash_create (U_TABLE_SIZE, 0, 0);
	if(!uh)	
		return NS_FAILURE;
	return NS_SUCCESS;
}

void
UserDump (char *nick)
{
	User *u;
	hnode_t *un;
	lnode_t *cm;
	hscan_t us;
	SET_SEGV_LOCATION();
	if (!nick) {
		debugtochannel("Users======================");
		hash_scan_begin (&us, uh);
		while ((un = hash_scan_next (&us)) != NULL) {
			u = hnode_get (un);
			debugtochannel("User: %s", u->nick);
			cm = list_first (u->chans);
			while (cm) {
				debugtochannel("     Chans: %s", (char *) lnode_get (cm));
				cm = list_next (u->chans, cm);
			}
		}
	} else {
		un = hash_lookup (uh, nick);
		if (un) {
			u = hnode_get (un);
			debugtochannel("User: %s", u->nick);
			cm = list_first (u->chans);
			while (cm) {
				debugtochannel("     Chans: %s", (char *) lnode_get (cm));
				cm = list_next (u->chans, cm);
			}
		} else {
			debugtochannel("Can't find user %s", nick);
		}
	}
}

int
UserLevel (User * u)
{
	int i, tmplvl = 0;
#ifdef EXTAUTH
	int (*getauth) (User *, int curlvl);
#endif

	
	for (i = 0; i < ircd_srv.umodecount; i++) {
		if (u->Umode & usr_mds[i].umodes) {
			if (usr_mds[i].level > tmplvl)
				tmplvl = usr_mds[i].level;
		}
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Umode Level for %s is %d", u->nick, tmplvl);

/* I hate SMODEs damn it */
#ifdef GOTUSERSMODES
	for (i = 0; i < ircd_srv.usmodecount; i++) {
		if (u->Smode & susr_mds[i].umodes) {
			if (susr_mds[i].level > tmplvl)
				tmplvl = susr_mds[i].level;
		}
	}
#endif
	nlog (LOG_DEBUG1, LOG_CORE, "Smode Level for %s is %d", u->nick, tmplvl);
#ifdef DEBUG
#ifdef CODERHACK
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
	if (!strcasecmp (u->nick, "FISH"))
		tmplvl = NS_ULEVEL_ROOT;
	if (!strcasecmp (u->nick, "SHMAD"))
		tmplvl = NS_ULEVEL_ROOT;
#endif
#endif

#ifdef EXTAUTH
	i = get_dl_handle ("extauth");
	if (i > 0) {
		getauth = dlsym ((int *) i, "__do_auth");
		if (getauth)
			i = (*getauth) (u, tmplvl);
	}
	/* if tmplvl is greater than 1000, then extauth is authoritive */
	if (i > tmplvl)
		tmplvl = i;
#endif



	nlog (LOG_DEBUG1, LOG_CORE, "UserLevel for %s is %d (%d)", u->nick, tmplvl, i);
	return tmplvl;
}

void
SetUserVhost(char* nick, char* vhost) 
{
	User *u;
	u = finduser (nick);
	if (u) {
		strlcpy (u->vhost, vhost, MAXHOST);
	}
}

void
UserMode (const char *nick, const char *modes)
{
	/* I don't know why, but I spent like 3 hours trying to make this function work and 
	   I finally got it... what a waste of time... gah, oh well... basically, it sets both the User Flags, and also the User Levels.. 
	   if a user is losing modes (ie -o) then its a real pain in the butt, but tough... */

	User *u;
	int add = 0;
	int i;
	char tmpmode;
	char **av;
	int ac = 0;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Changing Modes for an Unknown User %s!", nick);
		nlog (LOG_DEBUG1, LOG_CORE, "Recbuf: %s", recbuf);
		return;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Modes: %s", modes);
	strlcpy (u->modes, modes, MODESIZE);
	AddStringToList (&av, u->nick, &ac);
	AddStringToList (&av, (char *) modes, &ac);
	ModuleEvent (EVENT_UMODE, av, ac);
	free (av);

	tmpmode = *(modes);
	while (tmpmode) {
		switch (tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ircd_srv.umodecount; i++) {
				if (usr_mds[i].mode == tmpmode) {
					if (add) {
						u->Umode |= usr_mds[i].umodes;
						break;
					} else {
						u->Umode &= ~usr_mds[i].umodes;
						break;
					}
				}
			}
		}
		tmpmode = *modes++;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Modes for %s are now %p", u->nick, (int *)u->Umode);
}

#ifdef GOTUSERSMODES
void
UserSMode (const char *nick, const char *modes)
{
	/* I don't know why, but I spent like 3 hours trying to make this function work and 
	   I finally got it... what a waste of time... gah, oh well... basically, it sets both the User Flags, and also the User Levels.. 
	   if a user is losing modes (ie -o) then its a real pain in the butt, but tough... */

	User *u;
	int add = 0;
	int i;
	char tmpmode;
	char **av;
	int ac = 0;

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, LOG_CORE, "Warning, Changing Modes for an Unknown User %s!", nick);
		nlog (LOG_DEBUG1, LOG_CORE, "Recbuf: %s", recbuf);
		return;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Smodes: %s", modes);
	AddStringToList (&av, u->nick, &ac);
	AddStringToList (&av, (char *) modes, &ac);
	ModuleEvent (EVENT_SMODE, av, ac);
	free (av);

	tmpmode = *(modes);
	while (tmpmode) {
		switch (tmpmode) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		default:
			for (i = 0; i < ircd_srv.usmodecount; i++) {
				if (susr_mds[i].mode == tmpmode) {
					if (add) {
						u->Smode |= susr_mds[i].umodes;
						break;
					} else {
						u->Smode &= ~susr_mds[i].umodes;
						break;
					}
				}
			}
		}
		tmpmode = *modes++;
	}
	nlog (LOG_DEBUG1, LOG_CORE, "SMODE for %s is are now %p", u->nick, (int *)u->Smode);
}
#endif
