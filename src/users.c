/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
#include "channels.h"
#include "exclude.h"
#include "modules.h"
#include "bots.h"
#include "auth.h"
#include "services.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#endif
#include <arpa/inet.h>

static hash_t *userhash;

static User *
new_user (const char *nick)
{
	User *u;
	hnode_t *un;

	SET_SEGV_LOCATION();
	if (hash_isfull (userhash)) {
		nlog (LOG_CRITICAL, "new_user: user hash is full");
		return NULL;
	}
	u = scalloc (sizeof (User));
	strlcpy (u->nick, nick, MAXNICK);
	un = hnode_create (u);
	hash_insert (userhash, un, u->nick);
	return u;
}

#if !(FEATURES&FEATURE_NICKIP)
static void lookupnickip(char *data, adns_answer *a) 
{
	CmdParams * cmdparams;
	User *u;
	
	u = finduser((char *)data);
	if (a && a->nrrs > 0 && u && a->status == adns_s_ok) {
		u->ipaddr.s_addr = a->rrs.addr->addr.inet.sin_addr.s_addr;
		cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
		cmdparams->source.user = u;	
		SendAllModuleEvent (EVENT_GOTNICKIP, cmdparams);
		sfree (cmdparams);
	}
}
#endif

void
AddUser (const char *nick, const char *user, const char *host, const char *realname, const char *server, const char*ip, const char* TS, const char* numeric)
{
	CmdParams * cmdparams;
	unsigned long ipaddress = 0;
	User *u;
#if !(FEATURES&FEATURE_NICKIP)
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
#if !(FEATURES&FEATURE_NICKIP)
	} else if (me.want_nickip == 1) {
		/* first, if the u->host is a ip address, just convert it */
		ipad = smalloc(sizeof(struct in_addr));
		res = inet_aton(host, ipad);
		if (res > 0) {
			/* its valid */
			ipaddress = htonl(ipad->s_addr);
			sfree(ipad);
		} else {		
			/* kick of a dns reverse lookup for this host */
			dns_lookup((char *)host, adns_r_addr, lookupnickip, (void *)nick);
			ipaddress = 0;
		}		
#endif
	}
	dlog(DEBUG2, "AddUser: %s (%s@%s) %s (%d) -> %s at %s", nick, user, host, realname, (int)htonl (ipaddress), server, TS);
	u = new_user (nick);
	if (!u) {
		return;
	}
	if(TS) {
		u->TS = strtoul (TS, NULL, 10);
	} else {
		u->TS = me.now;
	}
	strlcpy (u->hostname, host, MAXHOST);
	strlcpy (u->vhost, host, MAXHOST);
	strlcpy (u->username, user, MAXUSER);
	strlcpy (u->realname, realname, MAXREALNAME);
	u->server = findserver (server);
	u->tslastmsg = me.now;
	u->chans = list_create (MAXJOINCHANS);
	u->ipaddr.s_addr = htonl (ipaddress);
	if (IsMe(u->server)) {
		u->flags |= NS_FLAGS_ME;
	}
	/* check if the user is excluded */
	ns_do_exclude_user(u);
#ifdef IRCU
	if((ircd_srv.protocol & PROTOCOL_B64SERVER) && numeric) {
		setnickbase64 (u->nick, numeric);
	}
#endif
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;	
	SendAllModuleEvent (EVENT_SIGNON, cmdparams);
	if (me.want_nickip == 1 && ipaddress != 0) {
		/* only fire this event if we have the nickip and some module wants it */
		SendAllModuleEvent (EVENT_GOTNICKIP, cmdparams);
	}
	sfree (cmdparams);
	/* Send CTCP VERSION request if we are configured to do so */
	if(is_synced && config.versionscan && !IsExcluded(u) && !IsMe(u)) {
		privmsg(u->nick, ns_botptr->nick, "\1VERSION\1");
	}
}

static void deluser(User* u)
{
	hnode_t *un;

	un = hash_lookup (userhash, u->nick);
	if (!un) {
		nlog (LOG_WARNING, "deluser: %s failed!", u->nick);
		return;
	}
	/* if its one of our bots, remove it from the modlist */
	if ( IsMe(u) ) {
		del_ns_bot (u->nick);
	}
	hash_delete (userhash, un);
	hnode_destroy (un);
	list_destroy (u->chans);
	sfree (u);
}

void 
KillUser (const char *nick, const char *reason)
{
	CmdParams * cmdparams;
	User *u;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "KillUser: %s", nick);
	u = finduser(nick);
	if(!u) {
		nlog (LOG_WARNING, "KillUser: %s failed!", nick);
		return;
	}
	PartAllChannels (u, reason);
	/* run the event to delete a user */
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;
	if(reason) {
		cmdparams->param = (char*)reason;
	}
	SendAllModuleEvent (EVENT_KILL, cmdparams);
	/* if its one of our bots inform the module */
	if ( IsMe(u) ) {
		nlog (LOG_NOTICE, "KillUser: deleting bot %s as it was killed", u->nick);
		SendModuleEvent (EVENT_BOTKILL, cmdparams, findbot(u->nick)->moduleptr);
	}
	deluser(u);
	sfree (cmdparams);
}

void 
QuitUser (const char *nick, const char *reason)
{
	CmdParams * cmdparams;
	User *u;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "QuitUser: %s", nick);
	u = finduser(nick);
	if(!u) {
		nlog (LOG_WARNING, "QuitUser: %s failed!", nick);
		return;
	}
	PartAllChannels (u, reason);
	/* run the event to delete a user */
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;
	if(reason) {
		cmdparams->param = (char*)reason;
	}
	SendAllModuleEvent (EVENT_QUIT, cmdparams);
	deluser(u);
	sfree (cmdparams);
}

void
UserAway (const char *nick, const char *awaymsg)
{
	CmdParams * cmdparams;
	User *u;

	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserAway: unable to find user %s for away", nick);
		return;
	}
	if (awaymsg) {
		strlcpy(u->awaymsg, awaymsg, MAXHOST);
	} else {
		u->awaymsg[0] = 0;
	}
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;
	if ((u->is_away == 1) && (!awaymsg)) {
		u->is_away = 0;
	} else if ((u->is_away == 0) && (awaymsg)) {
		u->is_away = 1;
	}
	SendAllModuleEvent (EVENT_AWAY, cmdparams);
	sfree (cmdparams);
}

int 
UserNick (const char * oldnick, const char *newnick, const char * ts)
{
	CmdParams * cmdparams;
	hnode_t *un;
	lnode_t *cm;
	User * u;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "UserNick: %s -> %s", oldnick, newnick);
	un = hash_lookup (userhash, oldnick);
	if (!un) {
		nlog (LOG_WARNING, "UserNick: can't find user %s", oldnick);
		return NS_FAILURE;
	}
	u = (User *) hnode_get (un);
	cm = list_first (u->chans);
	while (cm) {
		ChanNickChange (findchan (lnode_get (cm)), (char *) newnick, u->nick);
		cm = list_next (u->chans, cm);
	}
	SET_SEGV_LOCATION();
	hash_delete (userhash, un);
	strlcpy (u->nick, newnick, MAXNICK);
	if(ts) {
		u->TS = atoi (ts);
	} else {
		u->TS = me.now;
	}
	hash_insert (userhash, un, u->nick);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;
	cmdparams->param = (char *)oldnick;
	SendAllModuleEvent (EVENT_NICK, cmdparams);
	sfree (cmdparams);
	return NS_SUCCESS;
}
#ifdef IRCU
User *
finduserbase64 (const char *num)
{
	User *u;
	hnode_t *un;
	hscan_t us;

	hash_scan_begin (&us, userhash);
	while ((un = hash_scan_next (&us)) != NULL) {
		u = hnode_get (un);
		if(strncmp(u->name64, num, BASE64NICKSIZE) == 0) {
			dlog(DEBUG1, "finduserbase64: %s -> %s", num, u->nick);
			return u;
		}
	}
	dlog(DEBUG3, "finduserbase64: %s not found", num);
	return NULL;
}
#endif
User *
finduser (const char *nick)
{
	hnode_t *un;

	un = hash_lookup (userhash, nick);
	if (un != NULL) {
		return (User *) hnode_get (un);
	}
	dlog(DEBUG3, "finduser: %s not found", nick);
	return NULL;
}

#ifdef SQLSRV

/* @brief Returns the users server in text form that they are connected too
*/

void *display_server(void *tbl, char *col, char *sql, void *row) 
{
	User *data = row;
	return data->server->name;                        
}                        

void *display_umode(void *tbl, char *col, char *sql, void *row) 
{
	User *data = row;
	return UmodeMaskToString(data->Umode);
}

void *display_vhost(void *tbl, char *col, char *sql, void *row) 
{
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

void *display_smode(void *tbl, char *col, char *sql, void *row) 
{
	User *data = row;
	return SmodeMaskToString(data->Smode);
}

static char userschannellist[MAXCHANLIST];

void *display_chans(void *tbl, char *col, char *sql, void *row) 
{
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
InitUsers (void)
{
	userhash = hash_create (U_TABLE_SIZE, 0, 0);
	if(!userhash)	{
		nlog (LOG_CRITICAL, "Unable to create user hash");
		return NS_FAILURE;
	}

#ifdef SQLSRV
	/* add the server hash to the sql library */
	neo_users.address = userhash;
	rta_add_table(&neo_users);
#endif
	return NS_SUCCESS;
}

static void
dumpuser (User* u)
{
	lnode_t *cm;
	int i = 0;
					          
	if (ircd_srv.protocol & PROTOCOL_B64SERVER) {
		chanalert (ns_botptr->nick, "User:     %s!%s@%s (%s)", u->nick, u->username, u->hostname, u->name64);
	} else {
		chanalert (ns_botptr->nick, "User:     %s!%s@%s", u->nick, u->username, u->hostname);
	}
	chanalert (ns_botptr->nick, "IP:       %s", inet_ntoa(u->ipaddr));
	chanalert (ns_botptr->nick, "Vhost:    %s", u->vhost);
	chanalert (ns_botptr->nick, "Flags:    0x%lx", u->flags);
	chanalert (ns_botptr->nick, "Modes:    %s (0x%lx)", UmodeMaskToString(u->Umode), u->Umode);
	chanalert (ns_botptr->nick, "Smodes:   %s (0x%lx)", SmodeMaskToString(u->Smode), u->Smode);
	if(u->is_away) {
		chanalert (ns_botptr->nick, "Away:     %s", u->awaymsg);
	}

	cm = list_first (u->chans);
	while (cm) {
		if(i==0) {
			chanalert (ns_botptr->nick, "Channels: %s", (char *) lnode_get (cm));
		} else {
			chanalert (ns_botptr->nick, "          %s", (char *) lnode_get (cm));
		}
		cm = list_next (u->chans, cm);
		i++;
	}
	chanalert (ns_botptr->nick, "========================================");
}

void
UserDump (const char *nick)
{
	User *u;
	hnode_t *un;
	hscan_t us;

#ifndef DEBUG
	if (!config.debug)
		return;
#endif
	SET_SEGV_LOCATION();
	chanalert (ns_botptr->nick, "================USERDUMP================");
	if (!nick) {
		hash_scan_begin (&us, userhash);
		while ((un = hash_scan_next (&us)) != NULL) {
			u = hnode_get (un);
			dumpuser (u);
		}
	} else {
		un = hash_lookup (userhash, nick);
		if (un) {
			u = hnode_get (un);
			dumpuser (u);
		} else {
			chanalert (ns_botptr->nick, "UserDump: can't find user %s", nick);
		}
	}
}

int
UserLevel (User * u)
{
	int ulevel = 0;

	ulevel = UserAuth(u);
#ifdef DEBUG
#ifdef CODERHACK
	/* this is only cause I dun have the right O lines on some of my "Beta" Networks, so I need to hack this in :) */
	if (!ircstrcasecmp (u->nick, "FISH"))
		ulevel = NS_ULEVEL_ROOT;
	if (!ircstrcasecmp (u->nick, "SHMAD"))
		ulevel = NS_ULEVEL_ROOT;
	if (!ircstrcasecmp (u->nick, "MARK"))
		ulevel = NS_ULEVEL_ROOT;
#endif
#endif
	dlog(DEBUG1, "UserLevel for %s is %d", u->nick, ulevel);
	return ulevel;
}

void
SetUserVhost(const char* nick, const char* vhost) 
{
	User *u;

	u = finduser (nick);
	dlog(DEBUG1, "Vhost %s", vhost);
	if (u) {
		strlcpy (u->vhost, vhost, MAXHOST);
	/* sethost on Unreal doesn't send +xt, but /umode +x sends +x 
	 * so, we will never be 100% sure about +t 
	 */
#ifdef UMODE_HIDE
		u->Umode |= UMODE_HIDE;
#endif
	}
}

void
UserMode (const char *nick, const char *modes)
{
	CmdParams * cmdparams;
	User *u;
	long oldmode;

	SET_SEGV_LOCATION();
	dlog(DEBUG1, "UserMode: user %s modes %s", nick, modes);
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserMode: mode change for unknown user %s %s", nick, modes);
		return;
	}
	strlcpy (u->modes, modes, MODESIZE);
	oldmode = u->Umode;
	u->Umode = UmodeStringToMask(modes, u->Umode);
#ifdef UMODE_HIDE
	/* Do we have a hidden host any more? */
	if((oldmode & UMODE_HIDE) && (!(u->Umode & UMODE_HIDE))) {
		strlcpy(u->vhost, u->hostname, MAXHOST);
	}
#endif
	dlog(DEBUG1, "UserMode: modes for %s is now %p", u->nick, (int *)u->Umode);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;	
	cmdparams->param = (char*)modes;
	SendAllModuleEvent (EVENT_UMODE, cmdparams);
	sfree (cmdparams);
}

void
UserSMode (const char *nick, const char *modes)
{
	CmdParams * cmdparams;
	User *u;

	SET_SEGV_LOCATION();
	dlog(DEBUG1, "UserSMode: user %s smodes %s", nick, modes);
	u = finduser (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserSMode: smode change for unknown user %s %s", nick, modes);
		return;
	}
	u->Smode = SmodeStringToMask(modes, u->Smode);
	dlog(DEBUG1, "UserSMode: smode for %s is now %p", u->nick, (int *)u->Smode);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source.user = u;	
	cmdparams->param = (char*)modes;
	SendAllModuleEvent (EVENT_SMODE, cmdparams);
	sfree (cmdparams);
}

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
void FiniUsers (void)
{
	User *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin(&hs, userhash);
	while ((un = hash_scan_next(&hs)) != NULL) {
		u = hnode_get (un);
		PartAllChannels (u, NULL);
		/* something is wrong if its our bots */
		if ( IsMe(u) ) {
			nlog (LOG_NOTICE, "FiniUsers called with a neostats bot online: %s", u->nick);
		}
		hash_scan_delete (userhash, un);
		hnode_destroy (un);
		list_destroy (u->chans);
		sfree (u);
	}
	hash_destroy(userhash);
}

void QuitServerUsers (Server* s)
{
	User *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin(&hs, userhash);
	while ((un = hash_scan_next(&hs)) != NULL) {
		u = hnode_get (un);
		if(u->server == s) 
		{
			dlog (DEBUG1, "QuitServerUsers: deleting %s for %s", u->nick, s->name);
			QuitUser(u->nick, s->name);
		}
	}
}

void GetUserList(UserListHandler handler)
{
	User *u;
	hscan_t scan;
	hnode_t *node;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, userhash);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
		handler(u);
	}
}

int IsServiceRoot(User* u)
{
	if ((match(config.rootuser.nick, u->nick))
	&& (match(config.rootuser.user, u->username))
	&& (match(config.rootuser.host, u->hostname))) {
		return (1);
	}
	return (0);
}
