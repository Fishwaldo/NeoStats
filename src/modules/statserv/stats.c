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

static char announce_buf[BUFSIZE];

static int StatsMidnight(void);
static int ss_event_ctcpversion(CmdParams* cmdparams);
static int ss_event_online(CmdParams* cmdparams);
static int ss_event_pong(CmdParams* cmdparams);
static int ss_event_away(CmdParams* cmdparams);
static int ss_event_server(CmdParams* cmdparams);
static int ss_event_squit(CmdParams* cmdparams);
static int ss_event_nickip(CmdParams* cmdparams);
static int ss_event_signon(CmdParams* cmdparams);
static int ss_event_quit(CmdParams* cmdparams);
static int ss_event_mode(CmdParams* cmdparams);
static int ss_event_kill(CmdParams* cmdparams);
static int ss_event_newchan(CmdParams* cmdparams);
static int ss_event_delchan(CmdParams* cmdparams);
static int ss_event_join(CmdParams* cmdparams);
static int ss_event_part(CmdParams* cmdparams);
static int ss_event_topic(CmdParams* cmdparams);
static int ss_event_kick(CmdParams* cmdparams);

ModuleEvent module_events[] = {
	{EVENT_ONLINE,		ss_event_online},
	{EVENT_PONG,		ss_event_pong},
	{EVENT_SERVER,		ss_event_server},
	{EVENT_SQUIT,		ss_event_squit},
	{EVENT_SIGNON,		ss_event_signon},
	{EVENT_GOTNICKIP,	ss_event_nickip},
	{EVENT_UMODE,		ss_event_mode},
	{EVENT_QUIT,		ss_event_quit},
	{EVENT_AWAY,		ss_event_away},
	{EVENT_KILL,		ss_event_kill},
	{EVENT_NEWCHAN,		ss_event_newchan},
	{EVENT_DELCHAN,		ss_event_delchan},
	{EVENT_JOIN,		ss_event_join},
	{EVENT_PART,		ss_event_part},
	{EVENT_KICK,		ss_event_kick},
	{EVENT_TOPIC,		ss_event_topic},
	{EVENT_CTCPVERSION,	ss_event_ctcpversion},
	{EVENT_NULL,		NULL}
};

static int check_interval()
{
	static int lasttime;
	static int count;

	if (StatServ.newdb || !StatServ.onchan || !me.synced) {
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

static int
announce_record(const char *msg, ...)
{
	va_list ap;

	if(StatServ.recordalert < 0) {
		return 1;
	}
	if (check_interval() > 0) {
		va_start (ap, msg);
		ircvsnprintf (announce_buf, BUFSIZE, msg, ap);
		va_end (ap);
		switch(StatServ.recordalert) {
			case 3:
				wallops (ss_bot->nick, "%s", announce_buf);
				break;
			case 2:
				globops (ss_bot->nick, "%s", announce_buf);
				break;
			case 1:
			default:
				chanalert (ss_bot->nick, "%s", announce_buf);
				break;
		}
	}
	return 1;
}

static int
announce_lag(const char *msg, ...)
{
	va_list ap;

	if(StatServ.lagalert < 0) {
		return 1;
	}

	if (check_interval() > 0) {
		va_start (ap, msg);
		ircvsnprintf (announce_buf, BUFSIZE, msg, ap);
		va_end (ap);
		switch(StatServ.lagalert) {
			case 3:
				wallops (ss_bot->nick, "%s", announce_buf);
				break;
			case 2:
				globops (ss_bot->nick, "%s", announce_buf);
				break;
			case 1:
			default:
				chanalert (ss_bot->nick, "%s", announce_buf);
				break;
		}
	}
	return 1;
}

static CVersions *findversions(char *name)
{
	CVersions *cv;
	lnode_t *cn;
	cn = list_find(Vhead, name, comparef);
	if (cn) {
		cv = lnode_get(cn);
	} else {
		nlog(LOG_DEBUG2, "findversions(%s) -> NOT FOUND", name);
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

int ss_event_ctcpversion(CmdParams* cmdparams)
{
	lnode_t *node;
	CVersions *clientv;
	char *nocols = cmdparams->cmdparams->av[1];

	strip_mirc_codes(nocols);
	
	clientv = findversions(nocols);
	if (clientv) {
		nlog(LOG_DEBUG2, "Found Client Version Node %s", nocols);
		clientv->count++;
		return 1;
	}
	clientv = malloc(sizeof(CVersions));
	strlcpy(clientv->name, nocols, MAX_CLIENT_VERSION_NAME);
	clientv->count = 1;
	node = lnode_create(clientv);
	list_append(Vhead, node);
	nlog(LOG_DEBUG2, "Added Version to List %s",
	     clientv->name);
	return 1;
}

int ss_event_newchan(CmdParams* cmdparams)
{
	long count;

	IncreaseChans();
	/* only check exclusions after increasing channel count */
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return 1;
	}
	count = hash_count(ch);
	if (count > stats_network.maxchans) {
		stats_network.maxchans = count;
		stats_network.t_chans = me.now;
		announce_record("\2NEW CHANNEL RECORD\2 Wow, there is now %ld Channels on the Network",
		    stats_network.maxchans);
	}
	if (count > daily.chans) {
		daily.chans = count;
		daily.t_chans = me.now;
	}
	return 1;
}

int ss_event_delchan(CmdParams* cmdparams)
{
	CStats *cs;
	lnode_t *ln;
	
	DecreaseChans();
	/* only check exclusions after increasing channel count */
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return 1;
	}
	ln = list_find(Chead, cmdparams->channel->name, comparef);
	if (ln) {
		cs = lnode_get(ln);
		save_chan(cs);
		list_delete(Chead, ln);
		lnode_destroy(ln);
		free(cs);
	} else {
		nlog(LOG_WARNING, "Ehhh, Couldn't find channel %s when deleting out of stats", cmdparams->av[0]);
	}
	return 1;
}

int ss_event_join(CmdParams* cmdparams)
{
	CStats *cs;

	/* only check exclusions after increasing channel count */
	if (StatServ.exclusions && (IsExcluded(cmdparams->channel)|| IsExcluded(cmdparams->source.user))) {
		return 1;
	}

	cs = findchanstats(cmdparams->channel->name);
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
		cs = load_chan(cmdparams->channel->name);
		Increasemems(cs);
		cs->maxmemtoday++;
		cs->t_maxmemtoday = me.now;
		cs->maxmems++;
		cs->t_maxmems = me.now;
	}
	return 1;
}

int ss_event_part(CmdParams* cmdparams)
{
	CStats *cs;
	/* only check exclusions after increasing channel count */
	if (StatServ.exclusions && (IsExcluded(cmdparams->channel)|| IsExcluded(cmdparams->source.user)))) {
		return 1;
	}
	cs = findchanstats(cmdparams->channel->name);
	if (cs) {
		Decreasemems(cs);
	}
	return 1;
}

int ss_event_topic(CmdParams* cmdparams)
{
	CStats *cs;

	/* only check exclusions after increasing channel count */
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return 1;
	}
	cs = findchanstats(cmdparams->channel->name);
	if (cs) {
		IncreaseTops(cs);
	}
	return 1;
}
int ss_event_kick(CmdParams* cmdparams)
{
	CStats *cs;

	/* only check exclusions after increasing channel count */
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return 1;
	}

	cs = findchanstats(cmdparams->channel->name);
	if (cs) {
		IncreaseKicks(cs);
		if (cs->maxkicks < cs->maxkickstoday) {
			cs->maxkicks = cs->maxkickstoday;
			cs->t_maxkicks = me.now;
		}
	}
	return 1;

}

CStats *findchanstats(char *name)
{
	CStats *cs;
	lnode_t *cn;
	cn = list_find(Chead, name, comparef);
	if (cn) {
		cs = lnode_get(cn);
	} else {
		nlog(LOG_DEBUG2, "findchanstats(%s) -> NOT FOUND",
		     name);
		return NULL;
	}
	return cs;
}

int ss_event_server(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	AddStats(cmdparams->source.server);
	IncreaseServers();
	if (stats_network.maxservers < stats_network.servers) {
		stats_network.maxservers = stats_network.servers;
		stats_network.t_maxservers = me.now;
		if (!(StatServ.exclusions && IsExcluded(cmdparams->source.server))) {
			announce_record("\2NEW SERVER RECORD\2 Wow, there are now %ld Servers on the Network",
				stats_network.servers);
		}
	}
	if (stats_network.servers > daily.servers) {
		daily.servers = stats_network.servers;
		daily.t_servers = me.now;
	}
	return 1;
}

int ss_event_squit(CmdParams* cmdparams)
{
	SStats *ss;
	SET_SEGV_LOCATION();
	DecreaseServers();
	ss = findstats(cmdparams->source.server->name);
	if (cmdparams->source.server->name != me.uplink)
		ss->numsplits = ss->numsplits + 1;
	return 1;
}

int ss_event_kill(CmdParams* cmdparams)
{
	SStats *s;
	SStats *ss;
	char *rbuf, *cmd, *who;

	SET_SEGV_LOCATION();
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	s = findstats(cmdparams->source.user->server->name);
	if (is_oper(cmdparams->source.user)) {
		nlog(LOG_DEBUG2, "Decreasing OperCount on %s due to kill", cmdparams->source.user->server->name);
		DecreaseOpers(s);
	}
	if (cmdparams->source.user->is_away == 1) {
		stats_network.away = stats_network.away - 1;
	}
	DecreaseUsers(s);
	DelTLD(cmdparams->source.user);
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
		ss = findstats(cmdparams->source.user->server->name);
		ss->operkills = ss->operkills + 1;
	} else if (findserver(who)) {
		ss = findstats(who);
		ss->serverkills = ss->serverkills + 1;
	}
	free(rbuf);
	return 1;
}

int ss_event_mode(CmdParams* cmdparams)
{
	int add = 1;
	char *modes;
	SStats *s;

	SET_SEGV_LOCATION();
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return -1;
	}
	s = findstats(cmdparams->source.user->server->name);
	if (!s) {
		nlog(LOG_WARNING,
			"Unable to find stats for %s", cmdparams->source.user->server->name);
		return -1;
	}

	modes = cmdparams->av[1];
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
				nlog(LOG_DEBUG1, "Increasing OperCount for %s (%d)", cmdparams->source.user->server->name, s->opers);
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
				if (is_oper(cmdparams->source.user)) {
					nlog(LOG_DEBUG1, "Decreasing OperCount for %s", cmdparams->source.user->server->name);
					DecreaseOpers(s);
				}
			}
			break;
		default:
			break;
		}
		modes++;
	}
	return 1;
}

int ss_event_quit(CmdParams* cmdparams)
{
	SStats *s;

	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	s = findstats(cmdparams->source.user->server->name);
	
	if (!cmdparams->source.user->modes)
		return -1;
	if (is_oper(cmdparams->source.user)) {
		nlog(LOG_DEBUG2, "Decreasing OperCount on %s due to signoff", cmdparams->source.user->server->name);
		DecreaseOpers(s);
	}
	if (cmdparams->source.user->is_away == 1) {
		stats_network.away = stats_network.away - 1;
	}
	DecreaseUsers(s);
	DelTLD(cmdparams->source.user);
	return 1;
}

int ss_event_away(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	if (cmdparams->source.user->is_away == 1) {
		stats_network.away = stats_network.away + 1;
	} else {
		stats_network.away = stats_network.away - 1;
	}
	return 1;
}

int ss_event_nickip(CmdParams* cmdparams)
{
	AddTLD(cmdparams->source.user);
	return 1;
}

int ss_event_signon(CmdParams* cmdparams)
{
	SStats *s;

	SET_SEGV_LOCATION();
	/* ignore them if they are excluded */
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	s = findstats(cmdparams->source.user->server->name);
	IncreaseUsers(s);
	nlog(LOG_DEBUG2, "added %s to stats, now at %d", cmdparams->source.user->nick, s->users);
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


	return 1;
}

int ss_event_pong(CmdParams* cmdparams)
{
	SStats *ss;

	SET_SEGV_LOCATION();
	/* we don't want negative pings! */
	if (cmdparams->source.server->ping < 0)
		return -1;


	ss = findstats(cmdparams->source.server->name);
	if (!ss)
		return -1;

	/* this is a tidy up from old versions of StatServ that used to have negative pings! */
	if (ss->lowest_ping < 0) {
		ss->lowest_ping = 0;
	}
	if (ss->highest_ping < 0) {
		ss->highest_ping = 0;
	}

	if (cmdparams->source.server->ping > ss->highest_ping) {
		ss->highest_ping = cmdparams->source.server->ping;
		ss->t_highest_ping = me.now;
	}
	if (cmdparams->source.server->ping < ss->lowest_ping) {
		ss->lowest_ping = cmdparams->source.server->ping;
		ss->t_lowest_ping = me.now;
	}

	/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if (cmdparams->source.server->ping > StatServ.lagtime) {
		announce_lag("\2%s\2 is lagged out with a ping of %d",
			cmdparams->source.server->name, cmdparams->source.server->ping);
	}

	return 1;
}

extern bot_cmd ss_commands[];
extern bot_setting ss_settings[];
Bot *ss_bot;
BotInfo ss_botinfo = 
{
	"", 
	"", 
	"", 
	"", 
	"", 
};
Module* ss_module;

int ss_event_online(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	ss_bot = init_bot (ss_module, &ss_botinfo, services_bot_modes, BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, ss_commands, ss_settings);
	StatServ.onchan = 1;
	/* now that we are online, setup the timer to save the Stats database every so often */
	add_timer (ss_module, SaveStats, "SaveStats", DBSAVETIME);
	add_timer (ss_module, ss_html, "ss_html", 3600);
	/* also add a timer to check if its midnight (to reset the daily stats */
	add_timer (ss_module, StatsMidnight, "StatsMidnight", 60);
	add_timer (ss_module, DelOldChan, "DelOldChan", 3600);
	return 1;
}

SStats *new_stats(const char *name)
{
	hnode_t *sn;
	SStats *s;

	SET_SEGV_LOCATION();
	nlog(LOG_DEBUG2, "new_stats(%s)", name);
	s = scalloc(sizeof(SStats), 1);
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
		free s;
		return NULL;
	}
	hash_insert(Shead, sn, s->name);
	return s;
}

void AddStats(Server * s)
{
	SStats *st;

	SET_SEGV_LOCATION();
	nlog(LOG_DEBUG2, "AddStats(%s)", s->name);
	st = findstats(s->name);
	if (!st) {
		st = new_stats(s->name);
	} else {
		st->lastseen = me.now;
	}
}

SStats *findstats(char *name)
{
	hnode_t *sn;

	SET_SEGV_LOCATION();
	sn = hash_lookup(Shead, name);
	if (sn) {
		nlog(LOG_DEBUG2, "findstats(%s) - found", name);
		return hnode_get(sn);
	} 
	nlog(LOG_DEBUG2, "findstats(%s) - not found", name);
	return NULL;
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
