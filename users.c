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
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif

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
	AddStringToList (&av, (char*)oldnick, &ac);
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

#ifdef SQLSRV

/* @brief Returns the users server in text form that they are connected too
*/

void *display_server(void *tbl, char *col, char *sql, void *row) {
        User *data = row;
	return data->server->name;                        
}                        

void *display_umode(void *tbl, char *col, char *sql, void *row) {
	User *data = row;
	return UmodeMaskToString(data->Umode);
}

void *display_smode(void *tbl, char *col, char *sql, void *row) {
	User *data = row;
	return SmodeMaskToString(data->Smode);
}

static char userschannellist[MAXCHANLIST];

void *display_chans(void *tbl, char *col, char *sql, void *row) {
	User *data = row;
	lnode_t *cn;
	userschannellist[0] = '\0';
	cn = list_first(data->chans);
	while (cn != NULL) {
		strlcat(userschannellist, lnode_get(cn), MAXCHANLIST);
		strlcat(userschannellist, " ", MAXCHANLIST);
		cn = list_next(data->chans, cn);
	}
	return userschannellist;
}

COLDEF neo_userscols[] = {
	{
		"users",
		"nick",
		RTA_STR,
		MAXNICK,
		offsetof(struct User, nick),
		RTA_READONLY,
		NULL,
		NULL,
		"The nickname of the user"
	},
	{
		"users",
		"hostname",
		RTA_STR,
		MAXHOST,
		offsetof(struct User, hostname),
		RTA_READONLY,
		NULL, 
		NULL,
		"The real Hostname of the user"
	},
	{
		"users",
		"ident",
		RTA_STR,
		MAXUSER,
		offsetof(struct User, username),
		RTA_READONLY,
		NULL,
		NULL,
		"The ident portion of the users connection"
	},
	{
		"users",
		"realname",
		RTA_STR,
		MAXREALNAME,
		offsetof(struct User, realname),
		RTA_READONLY,
		NULL,
		NULL,
		"The users realname/info message"
	},
	{	
		"users",
		"vhost",
		RTA_STR,
		MAXHOST,
		offsetof(struct User, vhost),
		RTA_READONLY,
		NULL,
		NULL,
		"The users Vhost, if the IRCd supports VHOSTS"
	},
	{	
		"users",
		"away",
		RTA_INT,
		sizeof(int),
		offsetof(struct User, is_away),
		RTA_READONLY,
		NULL,
		NULL,
		"Boolean variable indiciating if the user is away"
	},
	{	
		"users",
		"modes",
		RTA_STR,
		64, 				/* as defined in ircd.c */
		offsetof(struct User, Umode),
		RTA_READONLY,
		display_umode,
		NULL,
		"the users umodes. Does not include SMODES."
	},
	{	
		"users",
		"smodes",
		RTA_STR,
		64,
		offsetof(struct User, Smode),
		RTA_READONLY,
		display_smode,
		NULL,
		"the users Smodes, if the IRCd supports it.  Does not include UMODES."
	},
	{	
		"users",
		"connected",
		RTA_INT,
		sizeof(int),
		offsetof(struct User, TS),
		RTA_READONLY,
		NULL,
		NULL,
		"When the User Connected"
	},
	{	
		"users",
		"server",
		RTA_STR,
		MAXHOST,
		offsetof(struct User, server),
		RTA_READONLY,
		display_server,
		NULL,
		"the users Smodes, if the IRCd supports it.  Does not include UMODES."
	},
	{	
		"users",
		"channels",
		RTA_STR,
		MAXCHANLIST,
		offsetof(struct User, chans),
		RTA_READONLY,
		display_chans,
		NULL,
		"the users Smodes, if the IRCd supports it.  Does not include UMODES."
	},

};

TBLDEF neo_users = {
	"users",
	NULL, 	/* for now */
	sizeof(struct User),
	0,
	TBL_HASH,
	neo_userscols,
	sizeof(neo_userscols) / sizeof(COLDEF),
	"",
	"The list of users connected to the IRC network"
};
#endif /* SQLSRV */



int
init_user_hash ()
{
	uh = hash_create (U_TABLE_SIZE, 0, 0);
	if(!uh)	
		return NS_FAILURE;

#ifdef SQLSRV
	/* add the server hash to the sql library */
	neo_users.address = uh;
	rta_add_table(&neo_users);
#endif


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

/* Do dl lookups in advance to speed up UserLevel processing 
 *
 */
#ifdef EXTAUTH
int (*getauth) (User *, int curlvl);
int InitExtAuth(void)
{
	int i;
	i = get_dl_handle ("extauth");
	if (i > 0) {
		getauth = dlsym ((int *) i, "__do_auth");
	}
}
#endif

int UmodeAuth(User * u)
{
	int i, tmplvl = 0;
	/* Note, tables have been reordered highest to lowest so the 
	 * first hit will give the highest level for a given umode
	 * combination so we can just set it without checking against
	 * the current level 
	 * we can also quit on the first occurrence of 0
	 * should be a lot faster!
	 */
	for (i = 0; i < ircd_umodecount; i++) {
		if(user_umodes[i].umode == 0)
			break;
		if (u->Umode & user_umodes[i].umode) {
			tmplvl = user_umodes[i].level;
			break;
		}
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Umode Level for %s is %d", u->nick, tmplvl);

/* I hate SMODEs damn it */
#ifdef GOTUSERSMODES
	/* see umode comments above */
	for (i = 0; i < ircd_smodecount; i++) {
		if(user_smodes[i].umode == 0)
			break;
		if (u->Smode & user_smodes[i].umode) {
			tmplvl = user_smodes[i].level;
			break;
		}
	}
	nlog (LOG_DEBUG1, LOG_CORE, "Smode Level for %s is %d", u->nick, tmplvl);
#endif
	return tmplvl;
}

int
UserLevel (User * u)
{
	int i, tmplvl = 0;
	tmplvl = UmodeAuth(u);

#ifdef EXTAUTH
	if (getauth)
		i = (*getauth) (u, tmplvl);
	/* if tmplvl is greater than 1000, then extauth is authoritive */
	if (i > tmplvl)
		tmplvl = i;
#endif

#ifdef DEBUG
#ifdef CODERHACK
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
	if (!strcasecmp (u->nick, "FISH"))
		tmplvl = NS_ULEVEL_ROOT;
	if (!strcasecmp (u->nick, "SHMAD"))
		tmplvl = NS_ULEVEL_ROOT;
#endif
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

/* I don't know why, but I spent like 3 hours trying to make this function work and 
	I finally got it... what a waste of time... gah, oh well... basically, it sets both the User Flags, and also the User Levels.. 
	if a user is losing modes (ie -o) then its a real pain in the butt, but tough... */
void
UserMode (const char *nick, const char *modes)
{
	User *u;
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
	u->Umode = UmodeStringToMask(modes, u->Umode);
	nlog (LOG_DEBUG1, LOG_CORE, "Modes for %s are now %p", u->nick, (int *)u->Umode);
}

#ifdef GOTUSERSMODES
void
UserSMode (const char *nick, const char *modes)
{
	User *u;
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
	u->Smode = SmodeStringToMask(modes, u->Smode);
	nlog (LOG_DEBUG1, LOG_CORE, "SMODE for %s is are now %p", u->nick, (int *)u->Smode);
}
#endif
