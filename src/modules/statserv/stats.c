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

#include "statserv.h"

#define IncreaseOpers(x)	x->opers++;		stats_network.opers++;
#define DecreaseOpers(x)	x->opers--;		stats_network.opers--;

#define IncreaseUsers(x)	x->users++;		stats_network.users++;	x->totusers++;	stats_network.totusers++; daily.tot_users++;
#define DecreaseUsers(x)	x->users--;		stats_network.users--;

#define Increasemems(x)		x->members++;	x->totmem++;	x->lastseen = time(NULL); 	x->joinstoday++;
#define Decreasemems(x)		x->members--;	x->lastseen = time(NULL);

static char announce_buf[BUFSIZE];

hash_t *Shead;
list_t *Chead;
list_t *Vhead;

static int check_interval()
{
	static int lasttime;
	static int count;

	if (!StatServ.onchan || !me.synced) {
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
			wallops (ss_bot->nick, "%s", msg);
			break;
		case 2:
			globops (ss_bot->nick, "%s", msg);
			break;
		case 1:
		default:
			chanalert (ss_bot->nick, "%s", msg);
			break;
	}
}

static void
announce_record(const char *msg, ...)
{
	va_list ap;

	if(StatServ.recordalert < 0 || check_interval() < 0) {
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

	if(StatServ.lagalert < 0 || check_interval() < 0) {
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
	lnode_t *cn;
	cn = list_find(Vhead, name, comparef);
	if (cn) {
		cv = lnode_get(cn);
	} else {
		nlog(LOG_DEBUG2, "findctcpversion(%s) -> NOT FOUND", name);
		return NULL;
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
			nlog(LOG_DEBUG2, "Save version %s", cv->name);
			cn = list_next(Vhead, cn);
		}
		fclose(output);
	}
	return 1;
}

int load_client_versions(void)
{
	lnode_t *node;
	CVersions *clientv;
	FILE* input;
	
	input = fopen("data/ssversions.dat", "rb");
	if(input) {
		while(!feof(input)) {
			clientv = malloc(sizeof(CVersions));
			fread(clientv, sizeof(CVersions), 1, input);	
			node = lnode_create(clientv);
			list_append(Vhead, node);
			nlog(LOG_DEBUG2, "Loaded version %s", clientv->name);
		}
		fclose(input);
	}
	return 1;
}

void list_client_versions(User* u, int num)
{
	CVersions *cv;
	lnode_t *cn;
	int i;

	if (list_count(Vhead) == 0) {
		prefmsg(u->nick, ss_bot->nick, "No Stats Available.");
		return;
	}
	if (!list_is_sorted(Vhead, topversions)) {
		list_sort(Vhead, topversions);
	}
	cn = list_first(Vhead);
	cv = lnode_get(cn);
	prefmsg(u->nick, ss_bot->nick, "Top%d Client Versions:", num);
	prefmsg(u->nick, ss_bot->nick, "======================");
	for (i = 0; i <= num; i++) {
		prefmsg(u->nick, ss_bot->nick, "%d) %d ->  %s", i, cv->count, cv->name);
		cn = list_next(Vhead, cn);
		if (cn) {
			cv = lnode_get(cn);
		} else {
			break;
		}
	}
	prefmsg(u->nick, ss_bot->nick, "End of List.");
}

void StatsAddChan(Channel* c)
{
	stats_network.chans++;
	if (stats_network.chans > stats_network.maxchans) {
		stats_network.maxchans = stats_network.chans;
		stats_network.t_chans = me.now;
		announce_record("\2NEW CHANNEL RECORD\2 %ld channels on the network",
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
	lnode_t *node;
	CVersions *clientv;

    strlcpy(nocols, version, BUFSIZE);
	strip_mirc_codes(nocols);
	clientv = findctcpversion(nocols);
	if (clientv) {
		nlog(LOG_DEBUG2, "Found version: %s", nocols);
		clientv->count++;
		return;
	}
	clientv = malloc(sizeof(CVersions));
	strlcpy(clientv->name, nocols, BUFSIZE);
	clientv->count = 1;
	node = lnode_create(clientv);
	list_append(Vhead, node);
	nlog(LOG_DEBUG2, "Added version: %s", clientv->name);
}

void StatsDelChan(Channel* c)
{
	CStats *cs;
	lnode_t *ln;
	
	stats_network.chans --;
	cs = findchanstats(c->name);
	if (cs) {
		save_chan(cs);
		list_delete(Chead, ln);
		lnode_destroy(ln);
		free(cs);
	} else {
		nlog(LOG_WARNING, "Couldn't find channel %s when deleting from stats", c->name);
	}
}

void StatsJoinChan(User* u, Channel* c)
{
	CStats *cs;

	cs = findchanstats(c->name);
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
		cs = load_chan(c->name);
		Increasemems(cs);
		cs->maxmemtoday++;
		cs->t_maxmemtoday = me.now;
		cs->maxmems++;
		cs->t_maxmems = me.now;
	}
}

void StatsPartChan(User* u, Channel* c)
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
	lnode_t *cn;

	cn = list_find(Chead, name, comparef);
	if (cn) {
		cs = lnode_get(cn);
		return cs;
	}
	nlog(LOG_DEBUG2, "findchanstats: %s not found", name);
	return NULL;
}

SStats *newserverstats(const char *name)
{
	hnode_t *sn;
	SStats *s;

	SET_SEGV_LOCATION();
	nlog(LOG_DEBUG2, "newserverstats(%s)", name);
	s = scalloc(sizeof(SStats));
	if (!s) {
		FATAL_ERROR("Out of memory.")
	}
	memcpy(s->name, name, MAXHOST);
	s->t_maxusers = me.now;
	s->t_maxopers = me.now;
	s->lastseen = me.now;
	s->starttime = me.now;
	s->t_highest_ping = me.now;
	s->t_lowest_ping = me.now;
	sn = hnode_create(s);
	if (hash_isfull(Shead)) {
		nlog(LOG_CRITICAL, "StatServ Server hash is full!");
		free(s);
		return NULL;
	}
	hash_insert(Shead, sn, s->name);
	return s;
}

SStats *findserverstats(char *name)
{
	hnode_t *sn;

	SET_SEGV_LOCATION();
	sn = hash_lookup(Shead, name);
	if (sn) {
		nlog(LOG_DEBUG2, "findserverstats(%s) - found", name);
		return hnode_get(sn);
	} 
	nlog(LOG_DEBUG2, "findserverstats(%s) - not found", name);
	return NULL;
}

void StatsAddServer(Server* s)
{
	SStats *st;

	SET_SEGV_LOCATION();
	nlog(LOG_DEBUG2, "StatsAddServer(%s)", s->name);
	st = findserverstats(s->name);
	if (!st) {
		st = newserverstats(s->name);
	} else {
		st->lastseen = me.now;
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

void StatsDelServer(Server* s)
{
	SStats *ss;

	SET_SEGV_LOCATION();
	stats_network.servers--;
	ss = findserverstats(s->name);
	if (s->name != me.uplink)
		ss->numsplits ++;
}

void StatsServerPong(Server* s)
{
	SStats *ss;

	SET_SEGV_LOCATION();
	/* we don't want negative pings! */
	if (s->ping < 0)
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
	if (s->ping > ss->highest_ping) {
		ss->highest_ping = s->ping;
		ss->t_highest_ping = me.now;
	}
	if (s->ping < ss->lowest_ping) {
		ss->lowest_ping = s->ping;
		ss->t_lowest_ping = me.now;
	}
	/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if (s->ping > StatServ.lagtime) {
		announce_lag("\2%s\2 is lagged out with a ping of %d",
			s->name, s->ping);
	}
}

void StatsKillUser(User* u)
{
	SStats *s;
	SStats *ss;
	char *rbuf, *cmd, *who;

	SET_SEGV_LOCATION();
	s = findserverstats(u->server->name);
	if (is_oper(u)) {
		nlog(LOG_DEBUG2, "Decreasing OperCount on %s due to kill", u->server->name);
		DecreaseOpers(s);
	}
	if (u->is_away == 1) {
		stats_network.away --;
	}
	DecreaseUsers(s);
	DelTLD(u);
	rbuf = sstrdup(recbuf);
	cmd = rbuf;
	who = strtok(cmd, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, " ");
	cmd = strtok(NULL, "");
	cmd++;
	who++;
	if (finduser(who)) {
		/* it was a User that killed the target */
		ss = findserverstats(u->server->name);
		ss->operkills ++;
	} else if (findserver(who)) {
		ss = findserverstats(who);
		ss->serverkills ++;
	}
	free(rbuf);
}

void StatsUserMode(User* u, char *modes)
{
	int add = 1;
	SStats *s;

	SET_SEGV_LOCATION();
	s = findserverstats(u->server->name);
	if (!s) {
		nlog(LOG_WARNING, "Unable to find stats for %s", u->server->name);
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
				nlog(LOG_DEBUG1, "Increasing OperCount for %s (%d)", u->server->name, s->opers);
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
					nlog(LOG_DEBUG1, "Decreasing OperCount for %s", u->server->name);
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

void StatsUserAway(User* u)
{
	SET_SEGV_LOCATION();
	if (u->is_away == 1) {
		stats_network.away ++;
	} else {
		stats_network.away --;
	}
}

void StatsAddUser(User* u)
{
	SStats *s;

	SET_SEGV_LOCATION();
	s = findserverstats(u->server->name);
	IncreaseUsers(s);
	nlog(LOG_DEBUG2, "added %s to stats, now at %d", u->nick, s->users);
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

void StatsDelUser(User* u)
{
	SStats *s;

	s = findserverstats(u->server->name);
	if (is_oper(u)) {
		nlog(LOG_DEBUG2, "Decreasing OperCount on %s due to signoff", u->server->name);
		DecreaseOpers(s);
	}
	if (u->is_away == 1) {
		stats_network.away = stats_network.away - 1;
	}
	DecreaseUsers(s);
	DelTLD(u);
}

int StatsMidnight(void)
{
	struct tm *ltm = localtime(&me.now);
	lnode_t *cn;
	CStats *c;

	SET_SEGV_LOCATION();
	if (ltm->tm_hour == 0 && ltm->tm_min == 0) {
		/* its Midnight! */
		chanalert(ss_bot->nick, "Reset Daily Statistics - Its Midnight here!");
		nlog(LOG_DEBUG1, "Reset Daily Statistics");
		daily.servers = stats_network.servers;
		daily.t_servers = me.now;
		daily.users = stats_network.users;
		daily.t_users = me.now;
		daily.opers = stats_network.opers;
		daily.t_opers = me.now;
		daily.chans = stats_network.chans;
		daily.t_chans = me.now;
		ResetTLD();
		cn = list_first(Chead);
		while (cn) {
			c = lnode_get(cn);
			c->maxmemtoday = c->members;;
			c->joinstoday = 0;
			c->maxkickstoday = 0;
			c->topicstoday = 0;
			c->t_maxmemtoday = me.now;
			cn = list_next(Chead, cn);
		}
	}
	return 1;
}

void InitStats(void)
{
	Chead = list_create(-1);
	Shead = hash_create(-1, 0, 0);
	Vhead = list_create(-1);
	GetServerList(StatsAddServer);
	GetUserList(StatsAddUser);
	/* TODO get user modes */
	GetChannelList(StatsAddChan);
	/* TODO get member counts */
}

void FiniStats(void)
{
	CStats *c;
	lnode_t *cn;
	SStats *s;
	hnode_t *sn;
	hscan_t ss;

	hash_destroy(Shead);
	hash_scan_begin(&ss, Shead);
	while ((sn = hash_scan_next(&ss))) {
		s = hnode_get(sn);
		free(s);
		hash_scan_delete(Shead, sn);
		hnode_destroy(sn);
	}
	cn = list_first(Chead);
	while (cn) {
		c = lnode_get(cn);
		free(c);
		cn = list_next(Chead, cn);
	}
	list_destroy_nodes(Chead);
}
