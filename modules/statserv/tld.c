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
#include "GeoIP.h"
#include "GeoIPCity.h"

#define UNKNOWN_COUNTRY_CODE	"???"

list_t *Thead;
GeoIP *gi;

void _setup_dbfilename();

void ResetTLD() 
{
	lnode_t *tn, *tn2;
	TLD *t;
	
	tn = list_first(Thead);
	while (tn != NULL) {
		t = lnode_get(tn);
		t->daily_users = 0;
		if (t->users == 0) {
			/* don't delete the tld entry ??? as its our "unknown" entry */
			if (ircstrcasecmp(t->tld, UNKNOWN_COUNTRY_CODE)) {
				tn2 = list_next(Thead, tn);
				ns_free(t);
				list_delete(Thead, tn);
				lnode_destroy(tn);
				tn = tn2;
				continue;
			}
		}
		tn = list_next(Thead, tn);
	}
}


static int findcc(const void *v, const void *cc) {
	const TLD *t = (void *)v;
	return (ircstrcasecmp(t->tld, (char *)cc));
}
int sortusers(const void *v, const void *v2) {
	const TLD *t = (void *)v;
	const TLD *t2 = (void *)v2;
	return (t2->users - t->users);
}

void DisplayTLDmap(Client *u) 
{
	TLD *t;
	lnode_t *tn;
	
	irc_prefmsg(ss_bot, u, "Top Level Domain Statistics:");
	list_sort(Thead, sortusers);
	tn = list_first(Thead);
	while (tn) {
		t = lnode_get(tn);
		irc_prefmsg(ss_bot, u, 
			"%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d",
			t->tld, t->users,
			((float) t->users / (float) stats_network.users) * 100,
			t->country, t->daily_users);
		tn = list_next(Thead, tn);
	}
	irc_prefmsg(ss_bot, u, "End of List");
}

/** @brief DelTLD
 *
 *  Delete a TLD from the current stats
 *
 *  @param client to delete from TLD list
 *
 *  @return none
 */
void DelTLD(Client * u)
{
	const char *country_code;
	TLD *t = NULL;
	
	SET_SEGV_LOCATION();
	if (!gi) {
		return;
	}
	country_code = GeoIP_country_code_by_addr(gi, u->hostip);
	if (country_code) {
		t = lnode_find (Thead, country_code, findcc);
	} else {
		t = lnode_find (Thead, UNKNOWN_COUNTRY_CODE, findcc);
	}
	if (t) {
		t->users--;
	} 
}

/** @brief AddTLD
 *
 *  Add a TLD to the current stats
 *
 *  @param client to add to TLD list
 *
 *  @return none
 */
void AddTLD(Client * u)
{
	const char *country_name;
	const char *country_code;
	TLD *t = NULL;
	
	SET_SEGV_LOCATION();
	if (!gi) {
		return;
	}	
	country_code = GeoIP_country_code_by_addr(gi, u->hostip);
	if (country_code) {
		t = lnode_find (Thead, country_code, findcc);
	} else {
		t = lnode_find (Thead, UNKNOWN_COUNTRY_CODE, findcc);
	}
	if (t) {
		t->users++;
		t->daily_users++;
	} else {
		country_name = GeoIP_country_name_by_addr(gi, u->hostip);
		t = ns_malloc(sizeof(TLD));
		strlcpy(t->tld, country_code, 5);
		strlcpy(t->country, country_name, 32);
		t->users = 1;
		t->daily_users = 1;
		lnode_create_append (Thead, t);
	}
	return;
}

/** @brief InitTLD
 *
 *  Init TLD lists
 *
 *  @param none
 *
 *  @return none
 */
void InitTLD(void)
{
	TLD *t;

	SET_SEGV_LOCATION();
	Thead = list_create(-1);
	gi = NULL;
	/* setup the GeoIP db filenames */
	_setup_dbfilename();	
	StatServ.GeoDBtypes = -1;
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
	lnode_create_append (Thead, t);
}

/** @brief FiniTLD
 *
 *  Clean up TLD lists
 *
 *  @param none
 *
 *  @return none
 */
void FiniTLD(void) 
{
	if (gi) {
		GeoIP_delete (gi);
		gi = NULL;
	}
	list_destroy_auto (Thead);
}
