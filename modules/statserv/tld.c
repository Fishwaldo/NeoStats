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
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "tld.h"
#include "GeoIP.h"
#include "GeoIPCity.h"

#define UNKNOWN_COUNTRY_CODE	"???"

list_t *tldstatlist;
GeoIP *gi;

void ResetTLDStatistics (void) 
{
	lnode_t *tn, *tn2;
	TLD *t;
	
	tn = list_first(tldstatlist);
	while (tn != NULL) {
		t = lnode_get(tn);
		ResetStatistic (&t->users);
		if (t->users.current == 0) {
			/* don't delete the tld entry ??? as its our "unknown" entry */
			if (ircstrcasecmp(t->tld, UNKNOWN_COUNTRY_CODE)) {
				tn2 = list_next(tldstatlist, tn);
				ns_free(t);
				list_delete(tldstatlist, tn);
				lnode_destroy(tn);
				tn = tn2;
				continue;
			}
		}
		tn = list_next(tldstatlist, tn);
	}
}

void AverageTLDStatistics (void)
{
}

static int findcc(const void *v, const void *cc) {
	const TLD *t = (void *)v;
	return (ircstrcasecmp(t->tld, (char *)cc));
}
int sortusers(const void *v, const void *v2) {
	const TLD *t = (void *)v;
	const TLD *t2 = (void *)v2;
	return (t2->users.daily.max - t->users.daily.max);
}

void GetTLDStats (TLDStatHandler handler, void *v)
{
	TLD *t;
	lnode_t *tn;
	
	list_sort (tldstatlist, sortusers);
	tn = list_first (tldstatlist);
	while (tn) {
		t = lnode_get (tn);
		handler (t, v);
		tn = list_next (tldstatlist, tn);
	}
}

void TLDReport (TLD *tld, void *v)
{
	Client *targetuser;

	targetuser = (Client *) v;
	irc_prefmsg(ss_bot, targetuser, 
		"%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d",
		tld->tld, tld->users.alltime.max, ((float) tld->users.current / (float) networkstats.users.current) * 100,
		tld->country, tld->users.current);
}

int ss_cmd_tldmap(CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg(ss_bot, cmdparams->source, "Top Level Domain Statistics:");
	GetTLDStats (TLDReport, (void *)cmdparams->source);
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

/** @brief DelTLDUser
 *
 *  Delete a TLD from the current stats
 *
 *  @param client to delete from TLD list
 *
 *  @return none
 */
void DelTLDUser (Client * u)
{
	const char *country_code;
	TLD *t = NULL;
	
	SET_SEGV_LOCATION();
	if (!gi) {
		return;
	}
	country_code = GeoIP_country_code_by_addr(gi, u->hostip);
	if (country_code) {
		t = lnode_find (tldstatlist, country_code, findcc);
	} else {
		t = lnode_find (tldstatlist, UNKNOWN_COUNTRY_CODE, findcc);
	}
	DecStatistic (&t->users);		
}

void AddTLDUser (Client *u)
{
	const char *country_name;
	const char *country_code;
	TLD *t = NULL;

	if (!gi) {
		return;
	}	
	country_code = GeoIP_country_code_by_addr(gi, u->hostip);
	if (country_code) {
		t = lnode_find (tldstatlist, country_code, findcc);
		if (!t) {
			country_name = GeoIP_country_name_by_addr(gi, u->hostip);
			t = ns_malloc(sizeof(TLD));
			strlcpy(t->tld, country_code, 5);
			strlcpy(t->country, country_name, 32);
			lnode_create_append (tldstatlist, t);
		}
	} else {
		t = lnode_find (tldstatlist, UNKNOWN_COUNTRY_CODE, findcc);
	}
	IncStatistic (&t->users);
}
	
/** @brief AddTLD
 *
 *  Add a TLD to the current stats
 *
 *  @param client to add to TLD list
 *
 *  @return none
 */
int ss_event_nickip (CmdParams *cmdparams)
{
	SET_SEGV_LOCATION();
	AddTLDUser (cmdparams->source);
	return NS_SUCCESS;
}

void SaveTLDStats (void)
{
	lnode_t *tn;
	TLD *t;
	
	tn = list_first(tldstatlist);
	while (tn != NULL) {
		t = lnode_get(tn);
		SetData((void *)t->country, CFGSTR, "TLD", t->tld, "country");
		SaveStatistic (&t->users, "TLD", t->tld, "users");
		tn = list_next(tldstatlist, tn);
	}
}

void LoadTLDStats (void)
{
	TLD *t;
	int i;
	char **data;
	char *tmp;

	if (GetTableData ("TLD", &data) > 0) {
		for (i = 0; data[i] != NULL; i++) {
			if (strncmp (data[i], "???", 5) != 0)
			{
				t = ns_calloc (sizeof (TLD));
				strlcpy (t->tld, data[i], 5);
				if (GetData((void *)&tmp, CFGSTR, "TLD", t->tld, "country") > 0) {
					strlcpy(t->country, tmp, MAXHOST);
					ns_free (tmp);
				} else {
					strlcpy(t->country, "???", MAXHOST);
					continue;
				}
				LoadStatistic (&t->users, "TLD", t->tld, "users");
				lnode_create_append (tldstatlist, t);
			}
		}
	}
	ns_free (data);
}

/** @brief InitTLDStatistics
 *
 *  Init TLD lists
 *
 *  @param none
 *
 *  @return none
 */
void InitTLDStatistics (void)
{
	TLD *t;

	SET_SEGV_LOCATION();
	tldstatlist = list_create(-1);
	gi = NULL;
	/* setup the GeoIP db filenames */
	_setup_dbfilename();	
	/* now open the various DB's */
	if (GeoIP_db_avail(GEOIP_COUNTRY_EDITION)) {
		gi = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
		if (gi != NULL) {
			nlog(LOG_NOTICE, "Loaded %s GeoIP Database", GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
		} else {
			nlog(LOG_WARNING, "%s Database may be corrupt", GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
		}
	} else {
		nlog(LOG_WARNING, "GeoIP Database is not available. TLD stats will not be available");
	}
	t = ns_calloc(sizeof(TLD));
	ircsnprintf(t->tld, 5, UNKNOWN_COUNTRY_CODE);
	strlcpy(t->country, "Unknown", 8);
	lnode_create_append (tldstatlist, t);
	LoadStatistic (&t->users, "TLD", "???", "users");
	LoadTLDStats ();
}

/** @brief FiniTLDStatistics
 *
 *  Clean up TLD lists
 *
 *  @param none
 *
 *  @return none
 */
void FiniTLDStatistics(void) 
{
	SaveTLDStats ();
	if (gi) {
		GeoIP_delete (gi);
		gi = NULL;
	}
	list_destroy_auto (tldstatlist);
}
