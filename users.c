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
** $Id: users.c,v 1.57 2003/07/17 15:00:13 fishwaldo Exp $
*/

#include <fnmatch.h>

#include "stats.h"
#include "hash.h"
#include "dl.h"
#include "log.h"



int fnmatch(const char *, const char *, int flags);
void doDelUser(const char *, int);






MyUser *myuhead;

static User *new_user(const char *);




User *new_user(const char *nick)
{
	User *u;
	hnode_t *un;

	strcpy(segv_location, "new_user");
	u = smalloc(sizeof(User));
	if (!nick)
		nick = "";
	memcpy(u->nick, nick, MAXNICK);
	un = hnode_create(u);
	if (hash_isfull(uh)) {
		nlog(LOG_CRITICAL, LOG_CORE, "Eeeek, Hash is full");
	} else {
		hash_insert(uh, un, u->nick);
	}
	return (u);
}

void AddUser(const char *nick, const char *user, const char *host,
	     const char *server, const unsigned long ipaddr,
	     const unsigned long TS)
{
	User *u;

	nlog(LOG_DEBUG2, LOG_CORE,
	     "AddUser(): %s (%s@%s)(%lu) -> %s at %lu", nick, user, host,
	     htonl(ipaddr), server, TS);
	strcpy(segv_location, "AddUser");
	u = finduser(nick);
	if (u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "trying to add a user that already exists? (%s)",
		     nick);
		return;
	}

	u = new_user(nick);
	strncpy(u->hostname, host, MAXHOST);
	strncpy(u->username, user, MAXUSER);
	/* its empty for the moment */
	strncpy(u->realname, "", MAXREALNAME);
	u->server = findserver(server);
	u->t_flood = time(NULL);
	u->flood = 0;
	u->is_away = 0;
	u->myuser = NULL;
	u->Umode = 0;
#ifdef ULTIMATE3
	u->Smode = 0;
#endif
	u->chans = list_create(MAXJOINCHANS);
	strcpy(u->modes, "");
	u->ipaddr.s_addr = htonl(ipaddr);
	u->TS = TS;
}
void AddRealName(const char *nick, const char *realname)
{
	char **av;
	int ac = 0;

	User *u = finduser(nick);

	if (!u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Warning, Can not find User %s for Realname", nick);
		return;
	}
	nlog(LOG_DEBUG2, LOG_CORE, "RealName(%s): %s", nick, realname);
	strncpy(u->realname, realname, MAXREALNAME);
	AddStringToList(&av, u->nick, &ac);
	Module_Event("SIGNON", av, ac);
	free(av);
}

void part_u_chan(list_t * list, lnode_t * node, void *v)
{
	User *u = v;
	part_chan(u, lnode_get(node));
}
void KillUser(const char *nick)
{
	doDelUser(nick, 1);
}
void DelUser(const char *nick)
{
	doDelUser(nick, 0);
}
void doDelUser(const char *nick, int i)
{
	User *u;
	hnode_t *un;
	char **av;
	int ac = 0;

	strcpy(segv_location, "DelUser");
	nlog(LOG_DEBUG2, LOG_CORE, "DelUser(%s)", nick);

	un = hash_lookup(uh, nick);
	if (!un) {
		nlog(LOG_WARNING, LOG_CORE, "DelUser(%s) failed!", nick);
		return;
	}
	u = hnode_get(un);

	list_process(u->chans, u, part_u_chan);



	/* run the event to delete a user */
	AddStringToList(&av, u->nick, &ac);
	if (i == 0) {
		Module_Event("SIGNOFF", av, ac);
	} else if (i == 1) {
		Module_Event("KILL", av, ac);
	}
	free(av);
	
	/* if its one of our bots, remove it from the modlist */
	if (findbot(u->nick)) {
		if (i == 1) 
			nlog(LOG_NOTICE, LOG_CORE, "Deleting Bot %s as it was killed", u->nick);
		del_mod_user(u->nick);
	}

	hash_delete(uh, un);
	hnode_destroy(un);
	list_destroy(u->chans);
	free(u);
}

void Do_Away(User * u, const char *awaymsg)
{
	char **av;
	int ac = 0;
	if (u) {
		AddStringToList(&av, u->nick, &ac);
		if ((u->is_away == 1) && (!awaymsg)) {
			u->is_away = 0;
			Module_Event("AWAY", av, ac);
		} else if ((u->is_away == 0) && (awaymsg)) {
			u->is_away = 1;
			AddStringToList(&av, (char *) awaymsg, &ac);
			Module_Event("AWAY", av, ac);
		}
		free(av);
	}
}

void Change_User(User * u, const char *newnick)
{
	hnode_t *un;
	lnode_t *cm;
	char **av;
	int ac = 0;
	char *oldnick;

	strcpy(segv_location, "Change_User");
	nlog(LOG_DEBUG2, LOG_CORE, "Change_User(%s, %s)", u->nick,
	     newnick);
	un = hash_lookup(uh, u->nick);
	if (!un) {
		nlog(LOG_WARNING, LOG_CORE, "ChangeUser(%s) Failed!",
		     u->nick);
		return;
	}
	cm = list_first(u->chans);
	while (cm) {
		change_user_nick(findchan(lnode_get(cm)), (char *) newnick,
				 u->nick);
		cm = list_next(u->chans, cm);
	}
	strcpy(segv_location, "Change_User_Return");
	hash_delete(uh, un);
	oldnick = malloc(MAXNICK);
	strncpy(oldnick, u->nick, MAXNICK);
	AddStringToList(&av, oldnick, &ac);
	strncpy(u->nick, newnick, MAXNICK);
	hash_insert(uh, un, u->nick);

	AddStringToList(&av, u->nick, &ac);
	Module_Event("NICK_CHANGE", av, ac);
	free(av);
	free(oldnick);
}
void sendcoders(char *message, ...)
{
	va_list ap;
	char tmp[512];
	strcpy(segv_location, "sendcoders");
	va_start(ap, message);
	vsnprintf(tmp, 512, message, ap);
#ifndef DEBUG
	if (!me.coder_debug)
		return;
#endif
	chanalert(s_Services, tmp);
	va_end(ap);
}

User *finduser(const char *nick)
{
	User *u;
	hnode_t *un;
	strcpy(segv_location, "finduser");
	un = hash_lookup(uh, nick);
	if (un != NULL) {
		u = hnode_get(un);
		return u;
	} else {
		nlog(LOG_DEBUG2, LOG_CORE, "FindUser(%s) -> NOTFOUND",
		     nick);
		return NULL;
	}

}


void init_user_hash()
{
	uh = hash_create(U_TABLE_SIZE, 0, 0);

}

void UserDump(char *nick)
{
	User *u;
	hnode_t *un;
	lnode_t *cm;
	hscan_t us;
	strcpy(segv_location, "UserDump");
	if (!nick) {
		sendcoders("Users======================");
		hash_scan_begin(&us, uh);
		while ((un = hash_scan_next(&us)) != NULL) {
			u = hnode_get(un);
			sendcoders("User: %s", u->nick);
			cm = list_first(u->chans);
			while (cm) {
				sendcoders("     Chans: %s",
					   (char *) lnode_get(cm));
				cm = list_next(u->chans, cm);
			}
		}
	} else {
		un = hash_lookup(uh, nick);
		if (un) {
			u = hnode_get(un);
			sendcoders("User: %s", u->nick);
			cm = list_first(u->chans);
			while (cm) {
				sendcoders("     Chans: %s",
					   (char *) lnode_get(cm));
				cm = list_next(u->chans, cm);
			}
		} else {
			sendcoders("Can't find user %s", nick);
		}
	}
}

int UserLevel(User * u)
{
	int i, tmplvl = 0;
#ifdef EXTAUTH
	int (*getauth) (User *, int curlvl);
#endif

	strcpy(segv_location, "UserLevel");
	for (i = 0; i < ((sizeof(usr_mds) / sizeof(usr_mds[0])) - 1); i++) {
		if (u->Umode & usr_mds[i].umodes) {
			if (usr_mds[i].level > tmplvl)
				tmplvl = usr_mds[i].level;
		}
	}
	nlog(LOG_DEBUG1, LOG_CORE, "Umode Level for %s is %d", u->nick,
	     tmplvl);

/* I hate SMODEs damn it */
#ifdef ULTIMATE3
	for (i = 0; i < ((sizeof(susr_mds) / sizeof(susr_mds[0])) - 1);
	     i++) {
		if (u->Smode & susr_mds[i].umodes) {
			if (susr_mds[i].level > tmplvl)
				tmplvl = susr_mds[i].level;
		}
	}
#endif
	nlog(LOG_DEBUG1, LOG_CORE, "Smode Level for %s is %d", u->nick,
	     tmplvl);
#ifdef DEBUG
#ifdef CODERHACK
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
	if (!strcasecmp(u->nick, "FISH"))
		tmplvl = 200;
	if (!strcasecmp(u->nick, "SHMAD"))
		tmplvl = 200;
#endif
#endif

#ifdef EXTAUTH
	i = get_dl_handle("extauth");
	if (i > 0) {
		getauth = dlsym((int *) i, "__do_auth");
		if (getauth)
			i = (*getauth) (u, tmplvl);
	}
	/* if tmplvl is greater than 1000, then extauth is authoritive */
	if (i > tmplvl)
		tmplvl = i;
#endif



	nlog(LOG_DEBUG1, LOG_CORE, "UserLevel for %s is %d (%d)", u->nick,
	     tmplvl, i);
	return tmplvl;
}


void UserMode(const char *nick, const char *modes, int smode)
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

	strcpy(segv_location, "UserMode");
	u = finduser(nick);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Warning, Changing Modes for a Unknown User %s!",
		     nick);
		nlog(LOG_DEBUG1, LOG_CORE, "Recbuf: %s", recbuf);
		return;
	}
	/* support for Smodes */
	if (smode > 0)
		nlog(LOG_DEBUG1, LOG_CORE, "Smodes: %s", modes);
	else
		nlog(LOG_DEBUG1, LOG_CORE, "Modes: %s", modes);

	if (smode == 0)
		strncpy(u->modes, modes, MODESIZE);

	AddStringToList(&av, u->nick, &ac);
	AddStringToList(&av, (char *) modes, &ac);


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
			if (smode > 0) {
				for (i = 0;
				     i <
				     ((sizeof(susr_mds) /
				       sizeof(susr_mds[0])) - 1); i++) {
					if (susr_mds[i].mode == tmpmode) {
						if (add) {
							u->Smode |=
							    susr_mds[i].
							    umodes;
							break;
						} else {
							u->Smode &=
							    ~susr_mds[i].
							    umodes;
							break;
						}
					}
				}
			} else {
				for (i = 0;
				     i <
				     ((sizeof(usr_mds) /
				       sizeof(usr_mds[0])) - 1); i++) {
					if (usr_mds[i].mode == tmpmode) {
						if (add) {
							u->Umode |=
							    usr_mds[i].
							    umodes;
							break;
						} else {
							u->Umode &=
							    ~usr_mds[i].
							    umodes;
							break;
						}
					}
				}
			}
		}
		tmpmode = *modes++;
	}
	if (smode > 0) {
		nlog(LOG_DEBUG1, LOG_CORE, "SMODE for %s is are now %p",
		     u->nick, u->Smode);
		     Module_Event("SMODE", av, ac);
	} else {
		nlog(LOG_DEBUG1, LOG_CORE, "Modes for %s are now %p",
		     u->nick, u->Umode);
		Module_Event("UMODE", av, ac);
	}
	free(av);



}
