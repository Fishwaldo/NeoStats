/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "statserv.h"
#include "log.h"
#include "GeoIP.h"
#include "GeoIPCity.h"

GeoIP *gi;

void _setup_dbfilename();

void ResetTLD() {
	lnode_t *tn, *tn2;
	TLD *t;
	
	tn = list_first(Thead);
	while (tn != NULL) {
		t = lnode_get(tn);
		t->daily_users = 0;
		if (t->users == 0) {
			/* don't delete the tld entry ??? as its our "unknown" entry */
			if (ircstrcasecmp(t->tld, "???")) {
				tn2 = list_next(Thead, tn);
				free(t->country);
				free(t);
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

void DisplayTLDmap(User *u) {
	TLD *t;
	lnode_t *tn;
	
	list_sort(Thead, sortusers);
	tn = list_first(Thead);
	while (tn) {
		t = lnode_get(tn);
		prefmsg(u->nick, s_StatServ,
			"%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d",
			t->tld, t->users,
			(float) t->users /
			(float) stats_network.users * 100,
			t->country, t->daily_users);
		tn = list_next(Thead, tn);
	}
}

void DelTLD(User * u)
{
	const char *country_code;
	const char *ipaddr;
	lnode_t *tn;
	TLD *t = NULL;
	SET_SEGV_LOCATION();
	
	if (!gi) {
		return;
	}
	ipaddr = inet_ntoa(u->ipaddr);
	country_code = GeoIP_country_code_by_addr(gi, ipaddr);
	if (country_code) {
		tn = list_find(Thead, country_code, findcc);
	} else {
		tn = list_find(Thead, "???", findcc);
	}
	if (tn) {
		t = lnode_get(tn);
		t->users--;
	} 
}


void AddTLD(User * u)
{
	const char *country_name;
	const char *country_code;
	const char *ipaddr;
	lnode_t *tn;
	TLD *t = NULL;
	SET_SEGV_LOCATION();
	
	if (!gi) {
		return;
	}
	
	ipaddr = inet_ntoa(u->ipaddr);
	country_code = GeoIP_country_code_by_addr(gi, ipaddr);
	if (country_code) {
		tn = list_find(Thead, country_code, findcc);
	} else {
		tn = list_find(Thead, "???", findcc);
	}
	if (tn) {
		t = lnode_get(tn);
		t->users++;
		t->daily_users++;
	} else {
		country_name = GeoIP_country_name_by_addr(gi, ipaddr);
		t = malloc(sizeof(TLD));
		strlcpy(t->tld, country_code, 5);
		strlcpy(t->country, country_name, 32);
		t->users = 1;
		t->daily_users = 1;
		tn = lnode_create(t);
		list_append(Thead, tn);
	}
	return;
}

void init_tld()
{
	TLD *t;
	lnode_t *tn;
	SET_SEGV_LOCATION();

	gi = NULL;
	/* setup the GeoIP db filenames */
	_setup_dbfilename();
	
	StatServ.GeoDBtypes = -1;
	/* now open the various DB's */
	if (GeoIP_db_avail(GEOIP_COUNTRY_EDITION)) {
		gi = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
		if (gi != NULL) {
			nlog(LOG_NOTICE, LOG_MOD, "Loaded %s GeoIP Database", GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
		} else {
			nlog(LOG_WARNING, LOG_MOD, "%s Database may be corrupt", GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
		}
	} else {
		nlog(LOG_WARNING, LOG_MOD, "GeoIP Database is not available. TLD stats will not be available");
	}
	Thead = list_create(-1);
	t = malloc(sizeof(TLD));
	bzero(t, sizeof(TLD));
	ircsnprintf(t->tld, 5, "???");
	strlcpy(t->country, "Unknown", 8);
	tn = lnode_create(t);
	list_append(Thead, tn);

}

void fini_tld() {
	TLD *t;
	lnode_t *tn;

	if (gi) {
		GeoIP_delete(gi);
	}
	tn = list_first(Thead);
	while (tn != NULL) {
		t = lnode_get(tn);
		free(t->country);
		free(t);
		tn = list_next(Thead, tn);
	}
	list_destroy_nodes(Thead);
	gi = NULL;
}
