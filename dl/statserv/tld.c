/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

#include "statserv.h"
#include "log.h"


void DelTLD(User * u)
{
	TLD *t = NULL;
	char *m;

	strcpy(segv_location, "StatServ-DelTLD");


	m = strrchr(u->hostname, '.');

	if (!m)
		t = findtld("num");
	else
		m++;

	if (!t) {
		if (!isdigit(*m))
			t = findtld(m);
		else
			t = findtld("num");
	}

	if (!t) {
		nlog(LOG_NORMAL, LOG_MOD,
		     "Unable to find TLD entry for %s (%s), damn vhosts!",
		     u->nick, m);
		return;
	}
	t->users--;
}

/* "net" -- not DOT"net" */
TLD *findtld(char *tld)
{
	TLD *t;

	strcpy(segv_location, "StatServ-findtld");


	for (t = tldhead; t; t = t->next) {
		if (!strcasecmp(t->tld, tld))
			return t;
	}

	return NULL;
}

TLD *AddTLD(User * u)
{
	TLD *t = NULL;
	char *m;

	strcpy(segv_location, "StatServ-AddTLD");

	m = strrchr(u->hostname, '.');

	if (!m)
		t = findtld("num");
	else
		m++;

	if (!t) {
		if (!isdigit(*m))
			t = findtld(m);
		else
			t = findtld("num");
	}

	if (!t) {
		nlog(LOG_NORMAL, LOG_MOD,
		     "Unable to find TLD entry for %s (%s), damn vhosts!",
		     u->nick, m);
		return NULL;
	}
	t->users++;
	t->daily_users++;

	return t;
}

void LoadTLD()
{
	register int i;
	FILE *fp;
	char buf[BUFSIZE], buf2[BUFSIZE];
	char *tmp = NULL, *tmp2 = NULL;
	TLD *t;

	strcpy(segv_location, "StatServ-LoadTLD");


	if ((fp = fopen("data/tlds.nfo", "r")) == NULL) {
		nlog(LOG_WARNING, LOG_MOD,
		     "Top Level Domain Statistics not loaded: file not found.");
		return;
	}

	while (fgets(buf, BUFSIZE, fp)) {
		memcpy(buf2, buf, sizeof(buf));
		tmp = strrchr(buf, '(');
		tmp++;
		tmp[strlen(tmp) - 1] = '\0';
		tmp[strlen(tmp) - 1] = '\0';
		for (i = 0; buf[i] != '('; i++)
			buf2[strlen(buf2) + 1] = buf[i];

		if ((tmp2 = strchr(buf2, '\n')))
			*tmp2 = '\0';

		t = malloc(sizeof(TLD));
		t->users = 0;
		t->country = sstrdup(buf2);
		memcpy(t->tld, tmp, sizeof(t->tld));

		if (!tldhead) {
			tldhead = t;
			tldhead->next = NULL;
		} else {
			t->next = tldhead;
			tldhead = t;
		}
	}
	fclose(fp);
}

void init_tld()
{
	TLD *t;

	strcpy(segv_location, "StatServ-init_tld");


	for (t = tldhead; t; t = t->next) {
		t->users = 0;
		t->daily_users = 0;
	}
}
