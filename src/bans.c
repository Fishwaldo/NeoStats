/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
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
#include "modules.h"
#include "services.h"
#include "lang.h"

static hash_t *banhash;

static Ban *
new_ban (const char *mask)
{
	Ban *ban;

	if (hash_isfull (banhash)) {
		nlog (LOG_CRITICAL, "new_ban: bans hash is full");
		return NULL;
	}
	dlog(DEBUG2, "new_ban: %s", mask);
	ban = ns_calloc (sizeof (Ban));
	strlcpy (ban->mask, mask, MAXHOST);
	hnode_create_insert (banhash, ban, ban->mask);
	return ban;
}

void AddBan(const char* type, const char* user, const char* host, const char* mask,
	const char* reason, const char* setby, const char* tsset, const char* tsexpires)
{
	CmdParams * cmdparams;
	Ban* ban;

	SET_SEGV_LOCATION();
	ban = new_ban (mask);
	if(!ban) {
		return;
	}
	ban->type[0] = type[0];
	ban->type[1] = 0;
	strlcpy(ban->user, user, MAXUSER);
	strlcpy(ban->host, host, MAXHOST);
	strlcpy(ban->mask, mask, MAXHOST);
	strlcpy(ban->reason, reason,BUFSIZE);
	strlcpy(ban->setby ,setby, MAXHOST);
	ban->tsset = atol(tsset);
	ban->tsexpires = atol(tsexpires);

	/* run the module event */
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	AddStringToList (&cmdparams->av, (char*)type, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)user, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)host, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)mask, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)reason, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)setby, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)tsset, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)tsexpires, &cmdparams->ac);
	SendAllModuleEvent (EVENT_ADDBAN, cmdparams);
	ns_free (cmdparams->av);
	ns_free (cmdparams);
}

void 
DelBan(const char* type, const char* user, const char* host, const char* mask,
	const char* reason, const char* setby, const char* tsset, const char* tsexpires)
{
	CmdParams * cmdparams;
	Ban *ban;
	hnode_t *bansnode;

	SET_SEGV_LOCATION();
	bansnode = hash_lookup (banhash, mask);
	if (!bansnode) {
		nlog (LOG_WARNING, "DelBan: unknown ban %s", mask);
		return;
	}
	ban = hnode_get (bansnode);

	/* run the module event */
	cmdparams = (CmdParams*) ns_calloc (sizeof(CmdParams));
	AddStringToList (&cmdparams->av, (char*)type, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)user, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)host, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)mask, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)reason, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)setby, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)tsset, &cmdparams->ac);
	AddStringToList (&cmdparams->av, (char*)tsexpires, &cmdparams->ac);
	SendAllModuleEvent (EVENT_DELBAN, cmdparams);
	ns_free (cmdparams->av);
	ns_free (cmdparams);

	hash_delete (banhash, bansnode);
	hnode_destroy (bansnode);
	ns_free (ban);
}

void
BanDump (void)
{
	Ban *ban;
	hscan_t ss;
	hnode_t *bansnode;

	irc_chanalert (ns_botptr, _("Ban Listing:"));
	hash_scan_begin (&ss, banhash);
	while ((bansnode = hash_scan_next (&ss)) != NULL) {
		ban = hnode_get (bansnode);
		irc_chanalert (ns_botptr, _("Ban: %s "), ban->mask);
	}
	irc_chanalert (ns_botptr, _("End of list."));
}

void FiniBans (void)
{
	Ban *ban;
	hnode_t *bansnode;
	hscan_t hs;

	hash_scan_begin(&hs, banhash);
	while ((bansnode = hash_scan_next(&hs)) != NULL ) {
		ban = hnode_get (bansnode);
		hash_delete (banhash, bansnode);
		hnode_destroy (bansnode);
		ns_free (ban);
	}
	hash_destroy(banhash);
}


int 
InitBans (void)
{
	banhash = hash_create (-1, 0, 0);
	if (!banhash) {
		nlog (LOG_CRITICAL, "Unable to create bans hash");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

hash_t *GetBanHash (void)
{
	return banhash;
}
