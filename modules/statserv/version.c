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
#include "statserv.h"
#include "stats.h"
#include "version.h"

list_t *versionstatlist;

int topversions(const void *key1, const void *key2)
{
	const ctcpversionstat *ver1 = key1;
	const ctcpversionstat *ver2 = key2;
	return (ver2->users.current - ver1->users.current);
}

static ctcpversionstat *findctcpversion(char *name)
{
	ctcpversionstat *cv;

	cv = lnode_find (versionstatlist, name, comparef);
	if (!cv) {
		dlog(DEBUG2, "findctcpversion(%s) -> NOT FOUND", name);	
	}
	return cv;
}

void save_client_versions(void)
{
	ctcpversionstat *cv;
	lnode_t *cn;
	FILE* output;
	
	output = fopen("data/ssversions.dat", "wb");
	if(output) {
		cn = list_first(versionstatlist);
		while (cn != NULL) {
			cv = (ctcpversionstat *) lnode_get(cn);
			fwrite(cv, sizeof(ctcpversionstat), 1, output);	
			dlog(DEBUG2, "Save version %s", cv->name);
			cn = list_next(versionstatlist, cn);
		}
		fclose(output);
	}
}

static void LoadVersionStats (void)
{
	ctcpversionstat *clientv;
	FILE* input;
	
	input = fopen("data/ssversions.dat", "rb");
	if(input) {
		clientv = ns_malloc(sizeof(ctcpversionstat));
		fread(clientv, sizeof(ctcpversionstat), 1, input);	
		while(!feof(input)) {
			lnode_create_append (versionstatlist, clientv);
			dlog(DEBUG2, "Loaded version %s", clientv->name);
			clientv = ns_malloc(sizeof(ctcpversionstat));
			fread(clientv, sizeof(ctcpversionstat), 1, input);	
		}
		fclose(input);
		ns_free (clientv);
	}
}

int ss_cmd_userversion(CmdParams *cmdparams)
{
	ctcpversionstat *cv;
	lnode_t *cn;
	int i;
	int num;

	num = cmdparams->ac > 0 ? atoi(cmdparams->av[0]) : 10;
	if (num < 10) {
		num = 10;
	}
	if (list_count (versionstatlist) == 0) {
		irc_prefmsg(ss_bot, cmdparams->source, "No Stats Available.");
		return NS_SUCCESS;
	}
	if (!list_is_sorted (versionstatlist, topversions)) {
		list_sort (versionstatlist, topversions);
	}
	irc_prefmsg (ss_bot, cmdparams->source, "Top %d Client Versions:", num);
	irc_prefmsg (ss_bot, cmdparams->source, "======================");
	cn = list_first (versionstatlist);
	for (i = 0; i <= num, cn; i++) {
		cv = lnode_get (cn);
		irc_prefmsg (ss_bot, cmdparams->source, "%d) %d ->  %s", i, cv->users.current, cv->name);
		cn = list_next (versionstatlist, cn);
	}
	irc_prefmsg (ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

int ss_event_ctcpversion(CmdParams *cmdparams)
{
	static char nocols[BUFSIZE];
	ctcpversionstat *clientv;

    strlcpy (nocols, cmdparams->param, BUFSIZE);
	strip_mirc_codes (nocols);
	clientv = findctcpversion (nocols);
	if (clientv) {
		dlog (DEBUG2, "Found version: %s", nocols);
		IncStatistic (&clientv->users);
		return NS_SUCCESS;
	}
	clientv = ns_calloc (sizeof (ctcpversionstat));
	strlcpy (clientv->name, nocols, BUFSIZE);
	IncStatistic (&clientv->users);
	lnode_create_append  (versionstatlist, clientv);
	dlog (DEBUG2, "Added version: %s", clientv->name);
	return NS_SUCCESS;
}

void InitVersionStats (void)
{
	versionstatlist = list_create(-1);
	LoadVersionStats ();
}

void FiniVersionStats (void)
{
	save_client_versions ();
	list_destroy_auto (versionstatlist);
}
