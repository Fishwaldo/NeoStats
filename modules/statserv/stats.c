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

#define IncreaseOpers(x)	(x)->opers++;		stats_network.opers++;
#define DecreaseOpers(x)	(x)->opers--;		stats_network.opers--;

#define IncreaseUsers(x)	(x)->users++;		stats_network.users++;	(x)->totusers++;	stats_network.totusers++; daily.tot_users++;
#define DecreaseUsers(x)	(x)->users--;		stats_network.users--;

#define Increasemems(x)	(x)->members++;	(x)->totmem++;	(x)->t_lastseen = time(NULL); 	(x)->joinstoday++;
#define Decreasemems(x)		(x)->members--;	(x)->t_lastseen = time(NULL);

static char announce_buf[BUFSIZE];

hash_t *Shead;
list_t *Chead;
list_t *Vhead;

static int check_interval()
{
	static int lasttime;
	static int count;

	if (!ss_module->synched) {
		return -1;
	}
	if ((me.now - lasttime) < StatServ.msginterval ) {
		if (++count > StatServ.msglimit)
			return -1;
	} else {
		lasttime = me.now;
		count = 0;
	}
	return 1;
}

static void
announce(int announcetype, const char *msg)
{
	switch(announcetype) {
		case 3:
			irc_wallops (ss_bot, "%s", msg);
			break;
		case 2:
			irc_globops (ss_bot, "%s", msg);
			break;
		case 1:
		default:
			irc_chanalert (ss_bot, "%s", msg);
			break;
	}
}

static void
announce_record(const char *msg, ...)
{
	va_list ap;

	if(StatServ.recordalert <= 0 || check_interval() < 0) {
		return;
	}
	va_start (ap, msg);
	ircvsnprintf (announce_buf, BUFSIZE, msg, ap);
	va_end (ap);
	announce(StatServ.recordalert, announce_buf);
}

static void
announce_lag(const char *msg, ...)
{
	va_list ap;

	if(StatServ.lagalert <= 0 || check_interval() < 0) {
		return;
	}
	va_start (ap, msg);
	ircvsnprintf (announce_buf, BUFSIZE, msg, ap);
	va_end (ap);
	announce(StatServ.lagalert, announce_buf);
}

static CVersions *findctcpversion(char *name)
{
	CVersions *cv;

	cv = lnode_find (Vhead, name, comparef);
	if (!cv) {
		dlog(DEBUG2, "findctcpversion(%s) -> NOT FOUND", name);	
	}
	return cv;
}

int save_client_versions(void)
{
	CVersions *cv;
	lnode_t *cn;
	FILE* output;
	
	output = fopen("data/ssversions.dat", "wb");
	if(output) {
		cn = list_first(Vhead);
		while (cn != NULL) {
			cv = lnode_get(cn);
			fwrite(cv, sizeof(CVersions), 1, output);	
			dlog(DEBUG2, "Save version %s", cv->name);
			cn = list_next(Vhead, cn);
		}
		fclose(output);
	}
	return 1;
}

int load_client_versions(void)
{
	CVersions *clientv;
	FILE* input;
	
	input = fopen("data/ssversions.dat", "rb");
	if(input) {
		clientv = ns_malloc(sizeof(CVersions));
		fread(clientv, sizeof(CVersions), 1, input);	
		while(!feof(input)) {
			lnode_create_append (Vhead, clientv);
			dlog(DEBUG2, "Loaded version %s", clientv->name);
			clientv = ns_malloc(sizeof(CVersions));
			fread(clientv, sizeof(CVersions), 1, input);	
		}
		fclose(input);
	}
	return 1;
}

void list_client_versions(Client * u, int num)
{
	CVersions *cv;
	lnode_t *cn;
	int i;

	if (list_count (Vhead) == 0) {
		irc_prefmsg(ss_bot, u, "No Stats Available.");
		return;
	}
	if (!list_is_sorted (Vhead, topversions)) {
		list_sort (Vhead, topversions);
	}
	irc_prefmsg (ss_bot, u, "Top %d Client Versions:", num);
	irc_prefmsg (ss_bot, u, "======================");
	cn = list_first (Vhead);
	for (i = 0; i <= num; i++) {
		cv = lnode_get (cn);
		irc_prefmsg (ss_bot, u, "%d) %d ->  %s", i, cv->count, cv->name);
		cn = list_next (Vhead, cn);
		if (!cn) {
			break;
		}
	}
	irc_prefmsg (ss_bot, u, "End of list.");
}

void StatsAddChan(Channel* c)
{
	stats_network.chans++;
	if (stats_network.chans > stats_network.maxchans) {
		stats_network.maxchans = stats_network.chans;
		stats_network.t_chans = me.now;
		announce_record ("\2NEW CHANNEL RECORD\2 %ld channels on the network",
		    stats_network.maxchans);
	}
	if (stats_network.chans > daily.chans) {
		daily.chans = stats_network.chans;
		daily.t_chans = me.now;
	}
}

void StatsAddCTCPVersion(char* version)
{
	static char nocols[BUFSIZE];
	CVersions *clientv;

    strlcpy (nocols, version, BUFSIZE);
	strip_mirc_codes (nocols);
	clientv = findctcpversion (nocols);
	if (clientv) {
		dlog (DEBUG2, "Found version: %s", nocols);
		clientv->count++;
		return;
	}
	clientv = ns_malloc (sizeof (CVersions));
	strlcpy (clientv->name, nocols, BUFSIZE);
	clientv->count = 1;
	lnode_create_append  (Vhead, clientv);
	dlog (DEBUG2, "Added version: %s", clientv->name);
}

void StatsDelChan(Channel* c)
{
	CStats *cs;
	lnode_t *ln;
	
	stats_network.chans --;
	ln = list_find (Chead, c->name, comparef);
	if (!ln) {
		nlog (LOG_WARNING, "Couldn't find channel %s when deleting from stats", c->name);
		return;
	}
	cs = lnode_get (ln);
	save_chan (cs);
	list_delete (Chead, ln);
	lnode_destroy (ln);
	ns_free (cs);
}

void StatsJoinChan(Client * u, Channel* c)
{
	CStats *cs;

	cs = findchanstats (c->name);
	if (cs) {
		Increasemems(cs);
		if (cs->maxmemtoday < cs->members) {
			cs->maxmemtoday = cs->members;
			cs->t_maxmemtoday = me.now;
		}
		if (cs->maxmems < cs->maxmemtoday) {
			cs->maxmems = cs->members;
			cs->t_maxmems = me.now;
		}
		if (cs->maxjoins < cs->joinstoday) {
			cs->maxjoins = cs->joinstoday;
			cs->t_maxjoins = me.now;
		}
	} else {
		cs = load_chan (c->name);
		Increasemems (cs);
		cs->maxmemtoday++;
		cs->t_maxmemtoday = me.now;
		cs->maxmems++;
		cs->t_maxmems = me.now;
	}
}

void StatsPartChan(Client * u, Channel* c)
{
	CStats *cs;

	cs = findchanstats(c->name);
	if (cs) {
		Decreasemems(cs);
	}
}

void StatsChanTopic(Channel* c)
{
	CStats *cs;

	cs = findchanstats(c->name);
	if (cs) {
		cs->topics++;
		cs->topicstoday++;
	}
}

void StatsChanKick(Channel* c)
{
	CStats *cs;

	cs = findchanstats(c->name);
	if (cs) {
		cs->kicks++;	
		cs->maxkickstoday++;	
		cs->members--;
		if (cs->maxkicks < cs->maxkickstoday) {
			cs->maxkicks = cs->maxkickstoday;
			cs->t_maxkicks = me.now;
		}
	}
}

CStats *findchanstats(char *name)
{
	CStats *cs;

	cs = lnode_find (Chead, name, comparef);
	if (!cs) {
		dlog(DEBUG2, "findchanstats: %s not found", name);
	}	
	return cs;
}

SStats *newserverstats(const char *name)
{
	SStats *s;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "newserverstats(%s)", name);
	if (hash_isfull (Shead)) {
		nlog (LOG_CRITICAL, "StatServ Server hash is full!");
		return NULL;
	}
	s = ns_calloc(sizeof(SStats));
	memcpy(s->name, name, MAXHOST);
	s->t_maxusers = s->t_maxopers = me.now;
	s->t_lastseen = s->t_start = me.now;
	s->t_highest_ping = s->t_lowest_ping = me.now;
	hnode_create_insert (Shead, s, s->name);
	return s;
}

SStats *findserverstats(char *name)
{
	SStats *stats;

	SET_SEGV_LOCATION();
	stats = (SStats *)hnode_find (Shead, name);
	if (!stats) {
		dlog(DEBUG2, "findserverstats(%s) - not found", name);
	}
	return stats;
}

void StatsAddServer(Client *s)
{
	SStats *st;

	SET_SEGV_LOCATION();
	dlog(DEBUG2, "StatsAddServer(%s)", s->name);
	st = findserverstats(s->name);
	if (!st) {
		st = newserverstats(s->name);
	} else {
		st->t_lastseen = me.now;
	}
	stats_network.servers ++;
	if (stats_network.maxservers < stats_network.servers) {
		stats_network.maxservers = stats_network.servers;
		stats_network.t_maxservers = me.now;
		if (!(StatServ.exclusions && IsExcluded(s))) {
			announce_record("\2NEW SERVER RECORD\2 Wow, there are now %ld Servers on the Network",
				stats_network.servers);
		}
	}
	if (stats_network.servers > daily.servers) {
		daily.servers = stats_network.servers;
		daily.t_servers = me.now;
	}
}

void StatsDelServer(Client *s)
{
	SStats *ss;

	SET_SEGV_LOCATION();
	stats_network.servers--;
	ss = findserverstats(s->name);
	if (s->name != me.uplink)
		ss->numsplits ++;
}

void StatsServerPong(Client *s)
{
	SStats *ss;

	SET_SEGV_LOCATION();
	/* we don't want negative pings! */
	if (s->server->ping < 0)
		return;
	ss = findserverstats(s->name);
	if (!ss)
		return;
	/* this is a tidy up from old versions of StatServ that used to have negative pings! */
	if (ss->lowest_ping < 0) {
		ss->lowest_ping = 0;
	}
	if (ss->highest_ping < 0) {
		ss->highest_ping = 0;
	}
	if (s->server->ping > ss->highest_ping) {
		ss->highest_ping = s->server->ping;
		ss->t_highest_ping = me.now;
	}
	if (s->server->ping < ss->lowest_ping) {
		ss->lowest_ping = s->server->ping;
		ss->t_lowest_ping = me.now;
	}
	/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if (s->server->ping > StatServ.lagtime) {
		announce_lag("\2%s\2 is lagged out with a ping of %d",
			s->name, s->server->ping);
	}
}

void StatsQuitUser(Client * u)
{
	SStats *s;

	s = findserverstats(u->uplink->name);
	if (is_oper(u)) {
		dlog(DEBUG2, "Decreasing OperCount on %s due to signoff", u->uplink->name);
		DecreaseOpers(s);
	}
	if (u->user->is_away == 1) {
		stats_network.away = stats_network.away - 1;
	}
	DecreaseUsers(s);
	DelTLD(u);
}

void StatsKillUser(Client * u)
{
	SStats *ss;
	char *rbuf, *cmd, *who;

	SET_SEGV_LOCATION();
	/* Treat as a quit for stats */
	StatsQuitUser (u);
	rbuf = sstrdup(recbuf);
	cmd = rbuf;
	who = strtok(cmd, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, "");
	cmd++;
	who++;
	if (find_user(who)) {
		/* it was a User that killed the target */
		ss = findserverstats(u->uplink->name);
		ss->operkills ++;
	} else if (find_server(who)) {
		ss = findserverstats(who);
		ss->serverkills ++;
	}
	ns_free(rbuf);
}

void StatsUserMode(Client * u, char *modes)
{
	int add = 1;
	SStats *s;

	SET_SEGV_LOCATION();
	s = findserverstats(u->uplink->name);
	if (!s) {
		nlog (LOG_WARNING, "Unable to find stats for %s", u->uplink->name);
		return;
	}
	while (*modes) {
		switch (*modes) {
		case '+':
			add = 1;
			break;
		case '-':
			add = 0;
			break;
		case 'O':
		case 'o':
			if (add) {
				dlog(DEBUG1, "Increasing OperCount for %s (%d)", u->uplink->name, s->opers);
				IncreaseOpers(s);
				if (stats_network.maxopers <
				    stats_network.opers) {
					stats_network.maxopers = stats_network.opers;
					stats_network.t_maxopers = me.now;
					announce_record("\2Oper Record\2 The Network has reached a New Record for Opers at %ld",
							     stats_network.opers);
				}
				if (s->maxopers < s->opers) {
					s->maxopers = s->opers;
					s->t_maxopers = me.now;
					announce_record("\2Server Oper Record\2 Wow, the Server %s now has a New record with %d Opers",
							     s->name,s->opers);
				}
				if (s->opers > daily.opers) {
					daily.opers = s->opers;
					daily.t_opers = me.now;
				}
			} else {
				if (is_oper(u)) {
					dlog(DEBUG1, "Decreasing OperCount for %s", u->uplink->name);
					DecreaseOpers(s);
				}
			}
			break;
		default:
			break;
		}
		modes++;
	}
}

void StatsUserAway(Client * u)
{
	SET_SEGV_LOCATION();
	if (u->user->is_away) {
		stats_network.away ++;
	} else {
		stats_network.away --;
	}
}

void StatsAddUser(Client * u)
{
	SStats *s;

	SET_SEGV_LOCATION();
	s = findserverstats(u->uplink->name);
	IncreaseUsers(s);
	dlog(DEBUG2, "added %s to stats, now at %d", u->name, s->users);
	if (s->maxusers < s->users) {
		/* New User Record */
		s->maxusers = s->users;
		s->t_maxusers = me.now;
		announce_record("\2NEW USER RECORD!\2 Wow, %s is cranking at the moment with %d users!",
				     s->name, s->users);
	}
	if (stats_network.maxusers < stats_network.users) {
		stats_network.maxusers = stats_network.users;
		stats_network.t_maxusers = me.now;
		announce_record("\2NEW NETWORK RECORD!\2 Wow, a New Global User record has been reached with %ld users!",
				     stats_network.users);
	}
	if (stats_network.users > daily.users) {
		daily.users = stats_network.users;
		daily.t_users = me.now;
	}
}

int StatsMidnight(void)
{
	lnode_t *cn;
	CStats *c;

	SET_SEGV_LOCATION();
	irc_chanalert (ss_bot, "Reset Daily Statistics - Its Midnight here!");
	dlog (DEBUG1, "Reset Daily Statistics");
	daily.servers = stats_network.servers;
	daily.t_servers = me.now;
	daily.users = stats_network.users;
	daily.t_users = me.now;
	daily.opers = stats_network.opers;
	daily.t_opers = me.now;
	daily.chans = stats_network.chans;
	daily.t_chans = me.now;
	ResetTLD ();
	cn = list_first (Chead);
	while (cn) {
		c = lnode_get (cn);
		c->maxmemtoday = c->members;;
		c->joinstoday = 0;
		c->maxkickstoday = 0;
		c->topicstoday = 0;
		c->t_maxmemtoday = me.now;
		cn = list_next (Chead, cn);
	}
	return 1;
}

void InitStats(void)
{
	Chead = list_create(-1);
	Shead = hash_create(-1, 0, 0);
	Vhead = list_create(-1);
}

void FiniStats(void)
{
	SStats *s;
	hnode_t *sn;
	hscan_t ss;

	hash_scan_begin(&ss, Shead);
	while ((sn = hash_scan_next(&ss))) {
		s = hnode_get(sn);
		hash_scan_delete(Shead, sn);
		hnode_destroy(sn);
		ns_free (s);
	}
	hash_destroy(Shead);
	list_destroy_auto (Chead);
}
