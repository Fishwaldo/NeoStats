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
#include "modes.h"
#include "hash.h"
#include "users.h"
#include "channels.h"
#include "exclude.h"
#include "modules.h"
#include "bots.h"
#include "auth.h"
#include "services.h"
#include "ctcp.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

static hash_t *userhash;

static Client *
new_user (const char *nick)
{
	Client *u;

	SET_SEGV_LOCATION();
	if (hash_isfull (userhash)) {
		nlog (LOG_CRITICAL, "new_user: user hash is full");
		return NULL;
	}
	dlog(DEBUG2, "new_user: %s", nick);
	u = scalloc (sizeof (Client));
	strlcpy (u->name, nick, MAXNICK);
	u->user = scalloc (sizeof (User));
	hnode_create_insert (userhash, u, u->name);
	return u;
}

static void lookupnickip(char *data, adns_answer *a) 
{
	CmdParams * cmdparams;
	Client *u;
	
	u = find_user((char *)data);
	if (a && a->nrrs > 0 && u && a->status == adns_s_ok) {
		u->ip.s_addr = a->rrs.addr->addr.inet.sin_addr.s_addr;
		cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
		cmdparams->source = u;	
		SendAllModuleEvent (EVENT_GOTNICKIP, cmdparams);
		sfree (cmdparams);
	}
}

Client *
AddUser (const char *nick, const char *user, const char *host, const char *realname, const char *server, const char*ip, const char* TS, const char* numeric)
{
	CmdParams * cmdparams;
	unsigned long ipaddress = 0;
	Client *u;

	SET_SEGV_LOCATION();
	u = find_user (nick);
	if (u) {
		nlog (LOG_WARNING, "AddUser: trying to add a user that already exists %s", nick);
		return NULL;
	}
	if(ip) {
		ipaddress = strtoul (ip, NULL, 10);
	} else if (!(ircd_srv.protocol&PROTOCOL_NICKIP) && me.want_nickip == 1) {
		struct in_addr *ipad;
		int res;

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
	}
	dlog(DEBUG2, "AddUser: %s (%s@%s) %s (%d) -> %s at %s", nick, user, host, realname, (int)htonl (ipaddress), server, TS);
	u = new_user (nick);
	if (!u) {
		return NULL;
	}
	if(TS) {
		u->tsconnect = strtoul (TS, NULL, 10);
	} else {
		u->tsconnect = me.now;
	}
	strlcpy (u->user->hostname, host, MAXHOST);
	strlcpy (u->user->vhost, host, MAXHOST);
	strlcpy (u->user->username, user, MAXUSER);
	strlcpy (u->info, realname, MAXREALNAME);
	u->user->ulevel = -1;
	u->uplink = find_server (server);
	u->user->tslastmsg = me.now;
	u->user->chans = list_create (MAXJOINCHANS);
	u->ip.s_addr = htonl (ipaddress);
	strlcpy(u->hostip, inet_ntoa (u->ip), HOSTIPLEN);
	if (IsMe(u->uplink)) {
		u->flags |= NS_FLAGS_ME;
	}
	/* check if the user is excluded */
	ns_do_exclude_user(u);
	if((ircd_srv.protocol & PROTOCOL_B64SERVER) && numeric) {
		setnickbase64 (u->name, numeric);
	}
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;	
	SendAllModuleEvent (EVENT_SIGNON, cmdparams);
	if (me.want_nickip == 1 && ipaddress != 0) {
		/* only fire this event if we have the nickip and some module wants it */
		SendAllModuleEvent (EVENT_GOTNICKIP, cmdparams);
	}
	sfree (cmdparams);
	/* Send CTCP VERSION request if we are configured to do so */
	if(is_synched && config.versionscan && !IsExcluded(u) && !IsMe(u)) {
		irc_ctcp_version_req (ns_botptr, u);
	}
	return u;
}

static void deluser(Client * u)
{
	hnode_t *un;

	un = hash_lookup (userhash, u->name);
	if (!un) {
		nlog (LOG_WARNING, "deluser: %s failed!", u->name);
		return;
	}
	/* if its one of our bots, remove it from the modlist */
	if ( IsMe(u) ) {
		del_bot (u->name);
	}
	hash_delete (userhash, un);
	hnode_destroy (un);
	list_destroy (u->user->chans);
	sfree (u->user);
	sfree (u);
}

void 
KillUser (const char *nick, const char *reason)
{
	CmdParams * cmdparams;
	Client *u;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "KillUser: %s", nick);
	u = find_user (nick);
	if(!u) {
		nlog (LOG_WARNING, "KillUser: %s failed!", nick);
		return;
	}
	PartAllChannels (u, reason);
	/* run the event to delete a user */
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;
	if(reason) {
		cmdparams->param = (char*)reason;
	}
	SendAllModuleEvent (EVENT_KILL, cmdparams);
	/* if its one of our bots inform the module */
	if ( IsMe(u) ) {
		nlog (LOG_NOTICE, "KillUser: deleting bot %s as it was killed", u->name);
		SendModuleEvent (EVENT_BOTKILL, cmdparams, u->user->bot->moduleptr);
	}
	deluser(u);
	sfree (cmdparams);
}

void 
QuitUser (const char *nick, const char *reason)
{
	CmdParams * cmdparams;
	Client *u;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "QuitUser: %s", nick);
	u = find_user (nick);
	if(!u) {
		nlog (LOG_WARNING, "QuitUser: %s failed!", nick);
		return;
	}
	PartAllChannels (u, reason);
	/* run the event to delete a user */
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;
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
	Client *u;

	u = find_user (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserAway: unable to find user %s for away", nick);
		return;
	}
	if (awaymsg) {
		strlcpy(u->user->awaymsg, awaymsg, MAXHOST);
	} else {
		u->user->awaymsg[0] = 0;
	}
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;
	if ((u->user->is_away == 1) && (!awaymsg)) {
		u->user->is_away = 0;
	} else if ((u->user->is_away == 0) && (awaymsg)) {
		u->user->is_away = 1;
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
	Client * u;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "UserNick: %s -> %s", oldnick, newnick);
	un = hash_lookup (userhash, oldnick);
	if (!un) {
		nlog (LOG_WARNING, "UserNick: can't find user %s", oldnick);
		return NS_FAILURE;
	}
	u = (Client *) hnode_get (un);
	cm = list_first (u->user->chans);
	while (cm) {
		ChanNickChange (find_chan (lnode_get (cm)), (char *) newnick, u->name);
		cm = list_next (u->user->chans, cm);
	}
	SET_SEGV_LOCATION();
	hash_delete (userhash, un);
	strlcpy (u->name, newnick, MAXNICK);
	if(ts) {
		u->tsconnect = atoi (ts);
	} else {
		u->tsconnect = me.now;
	}
	hash_insert (userhash, un, u->name);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;
	cmdparams->param = (char *)oldnick;
	SendAllModuleEvent (EVENT_NICK, cmdparams);
	sfree (cmdparams);
	return NS_SUCCESS;
}

Client *
finduserbase64 (const char *num)
{
	Client *u;
	hnode_t *un;
	hscan_t us;

	hash_scan_begin (&us, userhash);
	while ((un = hash_scan_next (&us)) != NULL) {
		u = hnode_get (un);
		if(strncmp(u->name64, num, BASE64NICKSIZE) == 0) {
			dlog(DEBUG1, "finduserbase64: %s -> %s", num, u->name);
			return u;
		}
	}
	dlog(DEBUG3, "finduserbase64: %s not found", num);
	return NULL;
}

Client *
find_user (const char *nick)
{
	Client *u;

	u = (Client *)hnode_find (userhash, nick);
	if (!u) {
		dlog(DEBUG3, "find_user: %s not found", nick);
	}
	return u;
}

int
InitUsers (void)
{
	userhash = hash_create (U_TABLE_SIZE, 0, 0);
	if(!userhash)	{
		nlog (LOG_CRITICAL, "Unable to create user hash");
		return NS_FAILURE;
	}

	return NS_SUCCESS;
}

static void
dumpuser (CmdParams* cmdparams, Client * u)
{
	lnode_t *cm;
	int i = 0;
					          
	if (ircd_srv.protocol & PROTOCOL_B64SERVER) {
		irc_prefmsg (ns_botptr, cmdparams->source, "User:     %s!%s@%s (%s)", u->name, u->user->username, u->user->hostname, u->name64);
	} else {
		irc_prefmsg (ns_botptr, cmdparams->source, "User:     %s!%s@%s", u->name, u->user->username, u->user->hostname);
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "IP:       %s", u->hostip);
	irc_prefmsg (ns_botptr, cmdparams->source, "Vhost:    %s", u->user->vhost);
	irc_prefmsg (ns_botptr, cmdparams->source, "Flags:    0x%x", u->flags);
	irc_prefmsg (ns_botptr, cmdparams->source, "Modes:    %s (0x%x)", UmodeMaskToString(u->user->Umode), u->user->Umode);
	irc_prefmsg (ns_botptr, cmdparams->source, "Smodes:   %s (0x%x)", SmodeMaskToString(u->user->Smode), u->user->Smode);
	if(u->user->is_away) {
		irc_prefmsg (ns_botptr, cmdparams->source, "Away:     %s", u->user->awaymsg);
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "Version:  %s", u->version);

	cm = list_first (u->user->chans);
	while (cm) {
		if(i==0) {
			irc_prefmsg (ns_botptr, cmdparams->source, "Channels: %s", (char *) lnode_get (cm));
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, "          %s", (char *) lnode_get (cm));
		}
		cm = list_next (u->user->chans, cm);
		i++;
	}
	irc_prefmsg (ns_botptr, cmdparams->source, "========================================");
}

void
UserDump (CmdParams* cmdparams, const char *nick)
{
	Client *u;
	hscan_t us;

#ifndef DEBUG
	if (!config.debug)
		return;
#endif
	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, "================USERDUMP================");
	if (!nick) {
		hnode_t* un;

		hash_scan_begin (&us, userhash);
		while ((un = hash_scan_next (&us)) != NULL) {
			u = hnode_get (un);
			dumpuser (cmdparams, u);
		}
	} else {
		u = (Client *)hnode_find (userhash, nick);
		if (u) {
			dumpuser (cmdparams, u);
		} else {
			irc_prefmsg (ns_botptr, cmdparams->source, "UserDump: can't find user %s", nick);
		}
	}
}

int
UserLevel (Client * u)
{
	/* Have we already calculated the user level? */
	if(u->user->ulevel != -1) {
		return u->user->ulevel;
	}
	u->user->ulevel = UserAuth(u);
	/* Set user level so we no longer need to calculate */
	dlog(DEBUG1, "UserLevel for %s is %d", u->name, u->user->ulevel);
	return u->user->ulevel;
}

void
SetUserVhost(const char* nick, const char* vhost) 
{
	Client *u;

	u = find_user (nick);
	dlog(DEBUG1, "Vhost %s", vhost);
	if (u) {
		strlcpy (u->user->vhost, vhost, MAXHOST);
		/* sethost on Unreal doesn't send +xt, but /umode +x sends +x 
		* so, we will never be 100% sure about +t 
		*/
		if (ircd_srv.features&FEATURE_UMODECLOAK) {
			u->user->Umode |= UMODE_HIDE;
		}
	}
}

void
UserMode (const char *nick, const char *modes)
{
	CmdParams * cmdparams;
	Client *u;
	long oldmode;

	SET_SEGV_LOCATION();
	dlog(DEBUG1, "UserMode: user %s modes %s", nick, modes);
	u = find_user (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserMode: mode change for unknown user %s %s", nick, modes);
		return;
	}
	/* Reset user level so it will be recalculated */
	u->user->ulevel = -1;
	strlcpy (u->user->modes, modes, MODESIZE);
	oldmode = u->user->Umode;
	u->user->Umode |= UmodeStringToMask (modes);
	if (ircd_srv.features&FEATURE_UMODECLOAK) {
		/* Do we have a hidden host any more? */
		if((oldmode & UMODE_HIDE) && (!(u->user->Umode & UMODE_HIDE))) {
			strlcpy(u->user->vhost, u->user->hostname, MAXHOST);
		}
	}
	dlog(DEBUG1, "UserMode: modes for %s now %x", u->name, u->user->Umode);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;	
	cmdparams->param = (char*)modes;
	SendAllModuleEvent (EVENT_UMODE, cmdparams);
	sfree (cmdparams);
}

void
UserSMode (const char *nick, const char *modes)
{
	CmdParams * cmdparams;
	Client *u;

	SET_SEGV_LOCATION();
	dlog(DEBUG1, "UserSMode: user %s smodes %s", nick, modes);
	u = find_user (nick);
	if (!u) {
		nlog (LOG_WARNING, "UserSMode: smode change for unknown user %s %s", nick, modes);
		return;
	}
	/* Reset user level so it will be recalculated */
	u->user->ulevel = -1;
	u->user->Smode |= SmodeStringToMask(modes);
	dlog(DEBUG1, "UserSMode: smode for %s is now %x", u->name, u->user->Smode);
	cmdparams = (CmdParams*) scalloc (sizeof(CmdParams));
	cmdparams->source = u;	
	cmdparams->param = (char*)modes;
	SendAllModuleEvent (EVENT_SMODE, cmdparams);
	sfree (cmdparams);
}

void SetUserServicesTS(const char* nick, const char* ts) 
{
	Client * u;

	u = find_user (nick);
	if(u) {
		u->user->servicestamp = strtoul(ts, NULL, 10);
	}
}

/* @brief Free up all the user structs and free memory. Called when we close down
 *
 */
void FiniUsers (void)
{
	Client *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin(&hs, userhash);
	while ((un = hash_scan_next(&hs)) != NULL) {
		u = hnode_get (un);
		PartAllChannels (u, NULL);
		/* something is wrong if its our bots */
		if ( IsMe(u) ) {
			nlog (LOG_NOTICE, "FiniUsers called with a neostats bot online: %s", u->name);
		}
		hash_scan_delete (userhash, un);
		hnode_destroy (un);
		list_destroy (u->user->chans);
		sfree (u->user);
		sfree (u);
	}
	hash_destroy(userhash);
}

void QuitServerUsers (Client *s)
{
	Client *u;
	hnode_t *un;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin(&hs, userhash);
	while ((un = hash_scan_next(&hs)) != NULL) {
		u = hnode_get (un);
		if(u->uplink == s) 
		{
			dlog (DEBUG1, "QuitServerUsers: deleting %s from %s", u->name, s->name);
			QuitUser(u->name, s->name);
		}
	}
}

void GetUserList(UserListHandler handler)
{
	Client *u;
	hscan_t scan;
	hnode_t *node;

	SET_SEGV_LOCATION();
	hash_scan_begin(&scan, userhash);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
		handler(u);
	}
}

void
AddFakeUser(const char *mask)
{
	char maskcopy[MAXHOST];
	char *nick;
	char *user;
	char *host;
	Client *u;

	SET_SEGV_LOCATION();
	strcpy(maskcopy, mask);
	nick = strtok(maskcopy, "!");
	user = strtok(NULL, "@");
	host = strtok(NULL, "");
	u = find_user (nick);
	if (u) {
		nlog (LOG_WARNING, "AddUser: trying to add a user that already exists %s", nick);
		return;
	}
	u = new_user (nick);
	if (!u) {
		return;
	}
	u->tsconnect = me.now;
	strlcpy (u->user->hostname, host, MAXHOST);
	strlcpy (u->user->vhost, host, MAXHOST);
	strlcpy (u->user->username, user, MAXUSER);
	strlcpy (u->info, "fake user", MAXREALNAME);
	u->user->tslastmsg = me.now;
	u->user->chans = list_create (MAXJOINCHANS);
}

void
DelFakeUser(const char *mask)
{
	char maskcopy[MAXHOST];
	char *nick;
	char *user;
	char *host;
	Client *u;

	SET_SEGV_LOCATION();
	strcpy(maskcopy, mask);
	nick = strtok(maskcopy, "!");
	user = strtok(NULL, "@");
	host = strtok(NULL, "");
	u = find_user (nick);
	deluser(u);
}

hash_t *GetUserHash (void)
{
	return userhash;
}
