/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

#include "neostats.h"
#include "ircd.h"
#include "hash.h"
#include "users.h"
#include "chans.h"
#include "exclude.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

hash_t *uh;

static User *new_user (const char *nick);

static char quitreason[BUFSIZE];

static User *
new_user (const char *nick)
{
	User *u;
	hnode_t *un;

	SET_SEGV_LOCATION();
	u = smalloc (sizeof (User));
	bzero(u, sizeof(User));
	if (!nick) {
		nlog (LOG_CRITICAL, "new_user: trying to add user with NULL nickname");
		return NULL;
	} else {
		strlcpy (u->nick, nick, MAXNICK);
	}
	un = hnode_create (u);
	if (hash_isfull (uh)) {
		nlog (LOG_CRITICAL, "new_user: user hash is full");
		return NULL;
	} else {
		hash_insert (uh, un, u->nick);
	}
	return (u);
}
#ifndef GOTNICKIP
static void 
lookupnickip(char *data, adns_answer *a) {
	User *u;
	char **av;
	int ac = 0;
	
	u = finduser((char *)data);
	if (a && a->nrrs > 0 && u && a->status == adns_s_ok) {
		u->ipaddr.s_addr = a->rrs.addr->addr.inet.sin_addr.s_addr;
		AddStringToList (&av, u->nick, &ac);
		SendModuleEvent (EVENT_GOTNICKIP, av, ac);
	}
}
#endif
void
AddUser (const char *nick, const char *user, const char *host, const char *realname, const char *server, const char*ip, const char* TS, const char* numeric)
{
	unsigned long ipaddress = 0;
	unsigned long time;
	char **av;
	int ac = 0;
	User *u;
	int i;
#ifndef GOTNICKIP
	struct in_addr *ipad;
	int res;
#endif

	SET_SEGV_LOCATION();
	u = finduser (nick);
	if (u) {
		nlog (LOG_WARNING, "AddUser: trying to add a user that already exists %s", nick);
		return;
	}

	if(ip) {
		ipaddress = strtoul (ip, NULL, 10);
	} else {
#ifndef GOTNICKIP
		if (me.want_nickip == 1) {
			/* first, if the u->host is a ip address, just convert it */
			ipad = malloc(sizeof(struct in_addr));
			res = inet_aton(host, ipad);
			if (res > 0) {
				/* its valid */
				ipaddress = htonl(ipad->s_addr);
				free(ipad);
			} else {		
				/* kick of a dns reverse lookup for this host */
				dns_lookup((char *)host, adns_r_addr, lookupnickip, (void *)nick);
				ipaddress = 0;
			}		
		} else {
			ipaddress = 0;
		}
#endif
	}
	if(TS) {
		time = strtoul (TS, NULL, 10);
	} else {
		time = me.now;
	}
	nlog (LOG_DEBUG2, "AddUser: %s (%s@%s) %s (%d) -> %s at %lu", nick, user, host, realname, (int)htonl (ipaddress), server, (unsigned long)time);

	u = new_user (nick);
	if (!u) {
		return;
	}
	strlcpy (u->hostname, host, MAXHOST);
	strlcpy (u->vhost, host, MAXHOST);
	strlcpy (u->username, user, MAXUSER);
	strlcpy (u->realname, realname, MAXREALNAME);
	u->server = findserver (server);
	u->tslastmsg = me.now;
	u->flood = 0;
	u->is_away = 0;
	u->Umode = 0;
	u->flags = 0;
#ifdef GOTUSERSMODES
	u->Smode = 0;
#endif
	u->chans = list_create (MAXJOINCHANS);
	u->modes[0]= '\0';
	u->ipaddr.s_addr = htonl (ipaddress);
	u->TS = time;
	
	/* make sure the module pointers are all null */
	for (i = 0; i < NUM_MODULES; i++) {
		u->moddata[i] = NULL;
	}

	if (!ircstrcasecmp(server, me.name)) {
		u->flags |= NS_FLAGS_ME;
	}

	/* check if the user is excluded */
	ns_do_exclude_user(u);

#ifdef BASE64NICKNAME
	if(numeric) {
		setnickbase64 (u->nick, numeric);
	}
#endif

	AddStringToList (&av, u->nick, &ac);
	SendModuleEvent (EVENT_SIGNON, av, ac);
	if (me.want_nickip == 1 && ipaddress != 0) {
		/* only fire this event if we have the nickip and some module wants it */
		SendModuleEvent (EVENT_GOTNICKIP, av, ac);
	}
	free (av);
}

void
UserPart (list_t * list, lnode_t * node, void *v)
{
	part_chan ((User *)v, lnode_get (node), quitreason[0] != 0 ? quitreason : NULL);
}

void
DelUser (const char *nick, int killflag, const char *reason)
{
	char killnick[MAXNICK];
	int botflag = 0;
	User *u;
	hnode_t *un;
	char **av;
	int ac = 0;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG2, "doDelUser: %s", nick);

	u = finduser(nick);
	if(!u) {
		nlog (LOG_WARNING, "doDelUser: %s failed!", nick);
		return;
	}

	un = hash_lookup (uh, u->nick);
	if (!un) {
		nlog (LOG_WARNING, "doDelUser: %s failed!", nick);
		return;
	}
	u = hnode_get (un);

	bzero(quitreason, BUFSIZE);
	if(reason) {
		strlcpy(quitreason, reason, BUFSIZE);
		strip_mirc_codes(quitreason);
	}
	list_process (u->chans, u, UserPart);

	/* run the event to delete a user */
	AddStringToList (&av, u->nick, &ac);
	if(reason) {
		AddStringToList (&av, (char*)quitreason, &ac);
	}
	if (killflag == 0) {
		SendModuleEvent (EVENT_SIGNOFF, av, ac);
	} else if (killflag == 1) {
		SendModuleEvent (EVENT_KILL, av, ac);
	}
	free (av);

	/* if its one of our bots, remove it from the modlist */
	if (findbot (u->nick)) {
		if (killflag == 1) {
			nlog (LOG_NOTICE, "doDelUser: deleting bot %s as it was killed", u->nick);
			strlcpy(killnick,u->nick,MAXNICK);
			botflag = 1;
		}
		del_mod_user (u->nick);
	}

	hash_delete (uh, un);
	hnode_destroy (un);
	list_destroy (u->chans);
	free (u);
	if(botflag) {
		ac = 0;
		AddStringToList (&av, killnick, &ac);
		SendModuleEvent (EVENT_BOTKILL, av, ac);
		free (av);
	}
}

void
UserAway (const char *nick, const char *awaymsg)
{
	char **av;
	int ac = 0;
	User *u;

	u = finduser (nick);
	if (u) {
		if (awaymsg) {
			strlcpy(u->awaymsg, awaymsg, MAXHOST);
		} else {
			u->awaymsg[0] = 0;
		}
		AddStringToList (&av, u->nick, &ac);
		if ((u->is_away == 1) && (!awaymsg)) {
			u->is_away = 0;
			SendModuleEvent (EVENT_AWAY, av, ac);
			free (av);
		} else if ((u->is_away == 0) && (awaymsg)) {
			u->is_away = 1;
			AddStringToList (&av, (char *) awaymsg, &ac);
			SendModuleEvent (EVENT_AWAY, av, ac);
			free (av);
		}
	} else {
		nlog (LOG_WARNING, "UserAway: unable to find user %s for away", nick);
	}
}

int 
UserNick (const char * oldnick, const char *newnick, const char * ts)
{
	char uoldnick[MAXNICK];
	hnode_t *un;
	lnode_t *cm;
	char **av;
	int ac = 0;
	User * u;
	time_t time;

	SET_SEGV_LOCATION();
	u = finduser (oldnick);
	if (!u) {
		nlog (LOG_WARNING, "UserNick: can't find user %s", oldnick);
		return NS_FAILURE;
	}
	strlcpy(uoldnick, u->nick, MAXNICK);

	if(ts) {
		time = atoi (ts);
	} else {
		time = me.now;
	}

	nlog (LOG_DEBUG2, "UserNick: %s -> %s", u->nick, newnick);
	un = hash_lookup (uh, u->nick);
	if (!un) {
		nlog (LOG_WARNING, "UserNick: %s -> %s failed!", u->nick, newnick);
		return NS_FAILURE;
	}
	cm = list_first (u->chans);
	while (cm) {
		ChanNickChange (findchan (lnode_get (cm)), (char *) newnick, u->nick);
		cm = list_next (u->chans, cm);
	}
	SET_SEGV_LOCATION();
	hash_delete (uh, un);
	strlcpy (u->nick, newnick, MAXNICK);
	if(ts) {
		u->TS = time;
	}
	hash_insert (uh, un, u->nick);
	AddStringToList (&av, (char*)uoldnick, &ac);
	AddStringToList (&av, u->nick, &ac);
	if(ts) {
		AddStringToList (&av, (char*)ts, &ac);
	}
	SendModuleEvent (EVENT_NICKCHANGE, av, ac);
	free (av);
	return NS_SUCCESS;
}

#ifdef BASE64NICKNAME
User *
finduserbase64 (const char *num)
{
	User *u;
	hnode_t *un;
	hscan_t us;

	hash_scan_begin (&us, uh);
	while ((un = hash_scan_next (&us)) != NULL) {
		u = hnode_get (un);
		if(strncmp(u->nick64, num, BASE64NICKSIZE) == 0) {
			nlog (LOG_DEBUG1, "finduserbase64: %s -> %s", num, u->nick);
			return u;
		}
	}
	nlog (LOG_DEBUG3, "finduserbase64: %s not found", num);
	return NULL;
}
#endif

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
	nlog (LOG_DEBUG3, "finduser: %s not found", nick);
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

void *display_vhost(void *tbl, char *col, char *sql, void *row) {
	User *u = row;
#ifdef UMODE_HIDE
	/* Do we have a hidden host? */
	if(u->Umode & UMODE_HIDE) {
			return u->vhost;
	}
	return "*";	
#else
	return u->hostname;
#endif
}

#ifdef GOTUSERSMODES
void *display_smode(void *tbl, char *col, char *sql, void *row) {
	User *data = row;
	return SmodeMaskToString(data->Smode);
}
#endif

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
		display_vhost,
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
#ifdef GOTUSERSMODES
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
#endif
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
		"flags",
		RTA_INT,
		sizeof(int),
		offsetof(struct User, flags),
		RTA_READONLY,
		NULL,
		NULL,
		"Flags for this user"
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
		"the users server"
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
		"the users channels."
	},
	{	
		"users",
		"awaymsg",
		RTA_STR,
		MAXHOST,
		offsetof(struct User, awaymsg),
		RTA_READONLY,
		NULL,
		NULL,
		"the users away message."
	},
	{	
		"users",
		"swhois",
		RTA_STR,
		MAXHOST,
		offsetof(struct User, swhois),
		RTA_READONLY,
		NULL,
		NULL,
		"the users swhois."
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

static void
dumpuser (User* u)
{
	lnode_t *cm;
	int i = 0;
					          
#ifdef BASE64NICKNAME
	debugtochannel("User:     %s!%s@%s (%s)", u->nick, u->username, u->hostname, u->nick64);
#else
	debugtochannel("User:     %s!%s@%s", u->nick, u->username, u->hostname);
#endif
	debugtochannel("IP:       %s", inet_ntoa(u->ipaddr));
	debugtochannel("Vhost:    %s", u->vhost);
#ifdef GOTUSERSMODES
	debugtochannel("Flags:    0x%lx Modes: %s (0x%lx) Smodes: %lx", u->flags, UmodeMaskToString(u->Umode), u->Umode, u->Smode);
#else
	debugtochannel("Flags:    0x%lx Modes: %s (0x%lx)", u->flags, UmodeMaskToString(u->Umode), u->Umode);
#endif
	if(u->is_away) {
		debugtochannel("Away:     %s ", u->awaymsg);
	}

	cm = list_first (u->chans);
	while (cm) {
		if(i==0) {
			debugtochannel("Channels: %s", (char *) lnode_get (cm));
		} else {
			debugtochannel("          %s", (char *) lnode_get (cm));
		}
		cm = list_next (u->chans, cm);
		i++;
	}
	debugtochannel("========================================");
}

void
UserDump (const char *nick)
{
	User *u;
	hnode_t *un;
	hscan_t us;
	SET_SEGV_LOCATION();
	debugtochannel("================USERDUMP================");
	if (!nick) {
		hash_scan_begin (&us, uh);
		while ((un = hash_scan_next (&us)) != NULL) {
			u = hnode_get (un);
			dumpuser (u);
		}
	} else {
		un = hash_lookup (uh, nick);
		if (un) {
			u = hnode_get (un);
			dumpuser (u);
		} else {
			debugtochannel("UserDump: can't find user %s", nick);
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
		getauth = ns_dlsym ((int *) i, "__do_auth");
		return NS_SUCCESS;
	} 
	return NS_FAILURE;
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
		if(user_umodes[i].level == 0)
			break;
		if (u->Umode & user_umodes[i].umode) {
			tmplvl = user_umodes[i].level;
			break;
		}
	}
	nlog (LOG_DEBUG1, "UmodeAuth: umode level for %s is %d", u->nick, tmplvl);

/* I hate SMODEs damn it */
#ifdef GOTUSERSMODES
	/* hey, smode can equal 0 as well you know */
	/* see umode comments above */
	for (i = 0; i < ircd_smodecount; i++) {
		if(user_smodes[i].level == 0)
			break;
		if (u->Smode & user_smodes[i].umode) {
			/* only if the smode level is higher than standard, do we alter tmplvl */
			if (user_smodes[i].level > tmplvl) 
				tmplvl = user_smodes[i].level;
			break;
		}
	}
	nlog (LOG_DEBUG1, "UmodeAuth: smode level for %s is %d", u->nick, tmplvl);
#endif
	return tmplvl;
}

int
UserLevel (User * u)
{
	int i = 0;
	int tmplvl = 0;
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
	if (!ircstrcasecmp (u->nick, "FISH"))
		tmplvl = NS_ULEVEL_ROOT;
	if (!ircstrcasecmp (u->nick, "SHMAD"))
		tmplvl = NS_ULEVEL_ROOT;
#endif
#endif

	nlog (LOG_DEBUG1, "UserLevel for %s is %d (%d)", u->nick, tmplvl, i);
	return tmplvl;
}

void
SetUserVhost(const char* nick, const char* vhost) 
{
	User *u;
	u = finduser (nick);
	nlog(LOG_DEBUG1, "Vhost %s", vhost);
	if (u) {
		strlcpy (u->vhost, vhost, MAXHOST);
/* these are precautions */
/* damn Unreal. /sethost on IRC doesn't send +xt, but /umode +x sends +x */
/* so, we will never be 100% sure about +t */
#ifdef UMODE_HIDE
		u->Umode |= UMODE_HIDE;
#endif
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
	long oldmode;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG1, "UserMode: user %s modes %s", nick, modes);
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserMode: mode change for unknown user %s %s", nick, modes);
		return;
	}
	nlog (LOG_DEBUG1, "Modes: %s", modes);
	strlcpy (u->modes, modes, MODESIZE);
	AddStringToList (&av, u->nick, &ac);
	AddStringToList (&av, (char *) modes, &ac);
	SendModuleEvent (EVENT_UMODE, av, ac);
	free (av);
	oldmode = u->Umode;
	u->Umode = UmodeStringToMask(modes, u->Umode);
	/* This needs to track +x and +t really but
	 * should be enough for Trystan to work on the SQL stuff
	 */
#ifdef UMODE_HIDE
	/* Do we have a hidden host any more? */
	if((oldmode & UMODE_HIDE) && (!(u->Umode & UMODE_HIDE))) {
		strlcpy(u->vhost, u->hostname, MAXHOST);
	}
#endif
	nlog (LOG_DEBUG1, "UserMode: modes for %s is now %p", u->nick, (int *)u->Umode);
}

#ifdef GOTUSERSMODES
void
UserSMode (const char *nick, const char *modes)
{
	User *u;
	char **av;
	int ac = 0;

	SET_SEGV_LOCATION();
	nlog (LOG_DEBUG1, "UserSMode: user %s modes %s", nick, modes);
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserSMode: smode change for unknown user %s %s", nick, modes);
		return;
	}
	nlog (LOG_DEBUG1, "Smodes: %s", modes);
	AddStringToList (&av, u->nick, &ac);
	AddStringToList (&av, (char *) modes, &ac);
	SendModuleEvent (EVENT_SMODE, av, ac);
	free (av);
	u->Smode = SmodeStringToMask(modes, u->Smode);
	nlog (LOG_DEBUG1, "UserSMode: smode for %s is now %p", u->nick, (int *)u->Smode);
}
#endif

void SetUserServicesTS(const char* nick, const char* ts) 
{
	User* u;
	u = finduser(nick);
	if(u) {
		u->servicestamp = strtoul(ts, NULL, 10);
	}
}

/* @brief Free up all the user structs and free memory. Called when we close down
 *
 */


void
FreeUsers ()
{
	User *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();


	hash_scan_begin(&hs, uh);
	while ((un = hash_scan_next(&hs)) != NULL) {
		u = hnode_get (un);
		list_process (u->chans, u, UserPart);
		
		/* something is wrong if its our bots */
		if (findbot (u->nick)) {
			nlog (LOG_NOTICE, "Ehhh. FreeUsers called while we still have bots online. Baaad User: %s", u->nick);
		}
		hash_scan_delete (uh, un);
		hnode_destroy (un);
		list_destroy (u->chans);
		free (u);
	}
	hash_destroy(uh);
	hash_destroy(ch);
}
