/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "statserv.h"
#include "log.h"
#include "conf.h"

static int ss_chans(User * u, char **av, int ac);
static int ss_daily(User * u, char **av, int ac);
static int ss_stats(User * u, char **av, int ac);
static int ss_tld(User * u, char **av, int ac);
static int ss_tld_map(User * u, char **av, int ac);
static int ss_operlist(User * u, char **av, int ac);
#ifdef GOTBOTMODE
static int ss_botlist(User * u, char **av, int ac);
#endif
static int ss_version(User * u, char **av, int ac);
static int ss_about(User * u, char **av, int ac);
static int ss_server(User * u, char **av, int ac);
static int ss_map(User *u, char **av, int ac);
static int ss_netstats(User *u, char **av, int ac);
static int ss_clientversions(User * u, char **av, int ac);
static int ss_forcehtml(User * u, char **av, int ac);

void ss_html(void);

static void ss_Config();
static int new_m_version(char *origin, char **av, int ac);

char s_StatServ[MAXNICK];

ModuleInfo __module_info = {
	 "statserv",
	 "Statistical Bot For NeoStats",
	 "$Rev$",
	__DATE__,
	__TIME__
};

Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1},
#ifdef GOTTOKENSUPPORT
	{TOK_VERSION, new_m_version, 1},
#endif
	{NULL, NULL, 0}
};

EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{EVENT_PONG, pong},
	{EVENT_NEWSERVER, s_new_server},
	{EVENT_SQUIT, s_del_server},
	{EVENT_SIGNON, s_new_user},
	{EVENT_UMODE, s_user_modes},
	{EVENT_SIGNOFF, s_del_user},
	{EVENT_AWAY, s_user_away},
	{EVENT_KILL, s_user_kill},
	{EVENT_NEWCHAN, s_chan_new},
	{EVENT_DELCHAN, s_chan_del},
	{EVENT_JOINCHAN, s_chan_join},
	{EVENT_PARTCHAN, s_chan_part},
	{EVENT_KICK, s_chan_kick},
	{EVENT_TOPICCHANGE, s_topic_change},
	{EVENT_CLIENTVERSION, s_client_version},
	{NULL, NULL}
};

void ss_Config()
{
	char *tmp;

	SET_SEGV_LOCATION();

	if (GetConf((void *) &tmp, CFGSTR, "Nick") < 0) {
		strlcpy(s_StatServ, "StatServ", MAXNICK);
	} else {
		strlcpy(s_StatServ, tmp, MAXUSER);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "User") < 0) {
		strlcpy(StatServ.user, "SS", MAXUSER);
	} else {
		strlcpy(StatServ.user, tmp, MAXUSER);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Host") < 0) {
		strlcpy(StatServ.host, me.name, MAXHOST);
	} else {
		strlcpy(StatServ.host, tmp, MAXHOST);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Rname") < 0) {
		ircsnprintf(StatServ.rname, MAXREALNAME, "/msg %s help",
			 s_StatServ);
	} else {
		strlcpy(StatServ.rname, tmp, MAXREALNAME);
		free(tmp);
	}
	if (GetConf((void *) &StatServ.lag, CFGINT, "Lag") < 0) {
		StatServ.lag = 0;
	}
	if (GetConf((void *) &tmp, CFGSTR, "HTML_Path") < 0) {
		StatServ.html = 0;
	} else {
		/* assume that html is enabled if we don't have a setting for it */
		StatServ.html = 1;
		if (GetConf((void *) &StatServ.html, CFGINT, "HTML_Enabled") < 0) {
			StatServ.html = 1;
		}
		strlcpy(StatServ.htmlpath, tmp, 255);
		free(tmp);
	}
	if (GetConf((void *) &StatServ.interval, CFGINT, "Wallop_Throttle")
	    < 0) {
		StatServ.interval = 0;
	}
}




int new_m_version(char *origin, char **av, int ac)
{
	SET_SEGV_LOCATION();
	snumeric_cmd(RPL_VERSION, origin,
		     "Module StatServ Loaded, Version: %s %s %s",
			 __module_info.module_version, __module_info.module_build_date,
			 __module_info.module_build_time);
	return 0;
}

int __ModInit(int modnum, int apiver)
{
	Server *ss;
	User *u;
	hnode_t *node;
	hscan_t scan;
	int count, i;
	Chans *c;
	char **av;
	int ac = 0;
	char *chan;
	lnode_t *chanmem;

	SET_SEGV_LOCATION();

	StatServ.onchan = 0;
	StatServ.shutdown = 0;
	ss_Config();
	if (StatServ.html) {
		if (StatServ.htmlpath[0] == 0) {
			nlog(LOG_NOTICE, LOG_MOD,
			     "StatServ HTML stats is disabled, as HTML_PATH is not set in the config file");
			StatServ.html = 0;
		}
	}
	LoadTLD();
	init_tld();
	LoadStats();
	Vhead = list_create(-1);


	hash_scan_begin(&scan, sh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		ss = hnode_get(node);
		ac = 0;
		AddStringToList(&av, ss->name, &ac);
		s_new_server(av, ac);
		free(av);
		ac = 0;
		nlog(LOG_DEBUG2, LOG_CORE,
		     "Added Server %s to StatServ List", ss->name);
	}
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
		ac = 0;
		AddStringToList(&av, u->nick, &ac);
		s_new_user(av, ac);
		AddStringToList(&av, u->modes, &ac);
		s_user_modes(av, ac);
		free(av);
		ac = 0;
		nlog(LOG_DEBUG2, LOG_CORE,
		     "Adduser user %s to StatServ List", u->nick);
	}
	hash_scan_begin(&scan, ch);
	while ((node = hash_scan_next(&scan)) != NULL) {
		c = hnode_get(node);
		count = list_count(c->chanmembers);
		chanmem = list_first(c->chanmembers);
		chan = lnode_get(chanmem);
		ac = 0;
		AddStringToList(&av, c->name, &ac);
		s_chan_new(av, ac);
		free(av);
		ac = 0;
		for (i = 1; i <= count; i++) {
			nlog(LOG_DEBUG2, LOG_CORE, "Chanjoin %s", c->name);
			ac = 0;
			AddStringToList(&av, c->name, &ac);
			AddStringToList(&av, chan, &ac);
			s_chan_join(av, ac);
			free(av);
			ac = 0;
			if (i < count) {
				chanmem =
				    list_next(c->chanmembers, chanmem);
				chan = lnode_get(chanmem);
			}
		}
	}
	return 1;
}

void __ModFini()
{
	StatServ.shutdown = 1;
	SaveStats();

}

bot_cmd ss_commands[]=
{
	{"ABOUT",			ss_about,		0, 	NS_ULEVEL_OPER,		ss_help_about, 		 	ss_help_about_oneline},
	{"VERSION",			ss_version,		0, 	NS_ULEVEL_OPER,		ss_help_version, 	 	ss_help_version_oneline},
	{"SERVER",			ss_server,		0, 	NS_ULEVEL_OPER,		ss_help_server,		 	ss_help_server_oneline},
	{"MAP",				ss_map,			0, 	NS_ULEVEL_OPER,		ss_help_map, 		 	ss_help_map_oneline},
	{"CHAN",			ss_chans,		0, 	NS_ULEVEL_OPER,		ss_help_chan, 		 	ss_help_chan_oneline},
	{"NETSTATS",		ss_netstats,	0, 	NS_ULEVEL_OPER,		ss_help_netstats, 	 	ss_help_netstats_oneline},
	{"DAILY",			ss_daily,		0, 	NS_ULEVEL_OPER,		ss_help_daily, 		 	ss_help_daily_oneline},
	{"TLD",				ss_tld,			1, 	NS_ULEVEL_OPER,		ss_help_tld, 		 	ss_help_tld_oneline},
	{"TLDMAP",			ss_tld_map,		0, 	NS_ULEVEL_OPER,		ss_help_tldmap, 	 	ss_help_tldmap_oneline},
	{"OPERLIST",		ss_operlist,	0, 	NS_ULEVEL_OPER,		ss_help_operlist, 	 	ss_help_operlist_oneline},
#ifdef GOTBOTMODE																	
	{"BOTLIST",			ss_botlist,		0, 	NS_ULEVEL_OPER,		ss_help_botlist, 	 	ss_help_botlist_oneline},
#endif																						
	{"CLIENTVERSIONS",	ss_clientversions,0,NS_ULEVEL_OPER,		ss_help_clientversions, ss_help_clientversions_oneline},
	{"FORCEHTML",		ss_forcehtml,	0, 	NS_ULEVEL_ADMIN,	ss_help_forcehtml, 		ss_help_forcehtml_oneline},
	{"STATS",			ss_stats,		1, 	NS_ULEVEL_ADMIN,	ss_help_stats, 			ss_help_stats_oneline},
	{NULL,				NULL,			0, 	0,					NULL, 					NULL}
};

bot_setting ss_settings[]=
{
	{"HTML",		&StatServ.html,		SET_TYPE_BOOLEAN,	0, 0,	"HTML_Enabled",		NULL,		ss_help_set },
	{"HTMLPATH",	&StatServ.htmlpath,	SET_TYPE_STRING,	0, 255,	"HTML_Path",		NULL,		NULL },
	{"MSGTHROTTLE",	&StatServ.interval,	SET_TYPE_INT,		0, 99,	"Wallop_Throttle",	"seconds",	NULL },
	{"LAGWALLOP",	&StatServ.lag,		SET_TYPE_INT,		0, 99,	"Lag",				"seconds",	NULL },
	{NULL,			NULL,				0,					0, 0,	NULL,				NULL,		NULL },
};

int topchan(const void *key1, const void *key2)
{
	const CStats *chan1 = key1;
	const CStats *chan2 = key2;
	return (chan2->members - chan1->members);

}
int topjoin(const void *key1, const void *key2)
{
	const CStats *chan1 = key1;
	const CStats *chan2 = key2;
	return (chan2->totmem - chan1->totmem);
}
int topkick(const void *key1, const void *key2)
{
	const CStats *chan1 = key1;
	const CStats *chan2 = key2;
	return (chan2->kicks - chan1->kicks);
}
int toptopics(const void *key1, const void *key2)
{
	const CStats *chan1 = key1;
	const CStats *chan2 = key2;
	return (chan2->topics - chan1->topics);
}
int topversions(const void *key1, const void *key2)
{
	const CVersions *ver1 = key1;
	const CVersions *ver2 = key2;
	return (ver2->count - ver1->count);
}

static int ss_clientversions(User * u, char **av, int ac)
{
	CVersions *cv;
	lnode_t *cn;
	int i;
	int num;

	chanalert(s_StatServ, "%s Wanted to see the Client Version List", u->nick);
	num = ac > 2 ? atoi(av[2]) : 10;
	if (num < 10) {
		num = 10;
	}
	if (list_count(Vhead) == 0) {
		prefmsg(u->nick, s_StatServ, "No Stats Available.");
		return 0;
	}
	if (!list_is_sorted(Vhead, topversions)) {
		list_sort(Vhead, topversions);
	}
	cn = list_first(Vhead);
	cv = lnode_get(cn);
	prefmsg(u->nick, s_StatServ, "Top%d Client Versions:", num);
	prefmsg(u->nick, s_StatServ, "======================");
	for (i = 0; i <= num; i++) {
		prefmsg(u->nick, s_StatServ, "%d) %d ->  %s", i, cv->count,
			cv->name);
		cn = list_next(Vhead, cn);
		if (cn) {
			cv = lnode_get(cn);
		} else {
			break;
		}
	}
	prefmsg(u->nick, s_StatServ, "End of List.");
	return 1;
}

static int ss_chans(User * u, char **av, int ac)
{
	CStats *cs;
	lnode_t *cn;
	int i;
	char *chan;

	if (ac < 2) {
		chan = NULL;
	} else {
		chan = av[2];
	}
	chanalert(s_StatServ, "%s Wanted to see Channel Statistics", u->nick);
	if (!chan) {
		/* they want the top10 Channels online atm */
		if (!list_is_sorted(Chead, topchan)) {
			list_sort(Chead, topchan);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, s_StatServ, "Top10 Online Channels:");
		prefmsg(u->nick, s_StatServ, "======================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(u) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(u->nick, s_StatServ,
				"Channel %s -> %ld Members", cs->name,
				cs->members);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, s_StatServ, "End of List.");
	} else if (!strcasecmp(chan, "POP")) {
		/* they want the top10 Popular Channels (based on joins) */
		if (!list_is_sorted(Chead, topjoin)) {
			list_sort(Chead, topjoin);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, s_StatServ, "Top10 Channels (Ever):");
		prefmsg(u->nick, s_StatServ, "======================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(u) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(u->nick, s_StatServ,
				"Channel %s -> %ld Joins", cs->name,
				cs->totmem);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, s_StatServ, "End of List.");
	} else if (!strcasecmp(chan, "KICKS")) {
		/* they want the top10 most unwelcome channels (based on kicks) */
		if (!list_is_sorted(Chead, topkick)) {
			list_sort(Chead, topkick);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, s_StatServ,
			"Top10 Most un-welcome Channels (Ever):");
		prefmsg(u->nick, s_StatServ,
			"======================================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(u) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(u->nick, s_StatServ,
				"Channel %s -> %ld Kicks", cs->name,
				cs->kicks);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, s_StatServ, "End of List.");
	} else if (!strcasecmp(chan, "TOPICS")) {
		/* they want the top10 most undecisive channels (based on topics) */
		if (!list_is_sorted(Chead, toptopics)) {
			list_sort(Chead, toptopics);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, s_StatServ,
			"Top10 Most undecisive Channels (Ever):");
		prefmsg(u->nick, s_StatServ,
			"======================================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(u) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(u->nick, s_StatServ,
				"Channel %s -> %ld Topics", cs->name,
				cs->topics);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, s_StatServ, "End of List.");
	} else {
		cs = findchanstats(chan);
		if (!cs) {
			prefmsg(u->nick, s_StatServ,
				"Error, Can't find any information about Channel %s",
				chan);
			return 0;
		}
		prefmsg(u->nick, s_StatServ,
			"\2Channel Information for %s (%s)\2", chan,
			(findchan(chan) ? "Online" : "Offline"));
		prefmsg(u->nick, s_StatServ,
			"Current Members: %ld (Max %ld on %s)",
			cs->members, cs->maxmems, sftime(cs->t_maxmems));
		prefmsg(u->nick, s_StatServ,
			"Max Members today: %ld at %s", cs->maxmemtoday,
			sftime(cs->t_maxmemtoday));
		prefmsg(u->nick, s_StatServ,
			"Total Number of Channel Joins: %ld", cs->totmem);
		prefmsg(u->nick, s_StatServ,
			"Total Member Joins today: %ld (Max %ld on %s)",
			cs->joinstoday, cs->maxjoins,
			sftime(cs->t_maxjoins));
		prefmsg(u->nick, s_StatServ,
			"Total Topic Changes %ld (Today %ld)", cs->topics,
			cs->topicstoday);
		prefmsg(u->nick, s_StatServ, "Total Kicks: %ld",
			cs->kicks);
		prefmsg(u->nick, s_StatServ,
			"Total Kicks today %ld (Max %ld on %s)",
			cs->maxkickstoday, cs->maxkicks,
			sftime(cs->t_maxkicks));
		if (!findchan(chan))
			prefmsg(u->nick, s_StatServ,
				"Channel was last seen at %s",
				sftime(cs->lastseen));
	}
	return 1;
}

static int ss_tld_map(User * u, char **av, int ac)
{
	TLD *t;

	SET_SEGV_LOCATION();
	chanalert(s_StatServ, "%s Wanted to see a Country Breakdown", u->nick);
	prefmsg(u->nick, s_StatServ, "Top Level Domain Statistics:");
	for (t = tldhead; t; t = t->next) {
		if (t->users != 0)
			prefmsg(u->nick, s_StatServ,
				"%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d",
				t->tld, t->users,
				(float) t->users /
				(float) stats_network.users * 100,
				t->country, t->daily_users);
	}
	prefmsg(u->nick, s_StatServ, "End of List");
	return 1;
}

static int ss_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, s_StatServ, "\2%s Version Information\2", s_StatServ);
	prefmsg(u->nick, s_StatServ, "%s Version: %s Compiled %s at %s", s_StatServ,
		__module_info.module_version, __module_info.module_build_date, __module_info.module_build_time);
	prefmsg(u->nick, s_StatServ, "http://www.neostats.net");
	return 1;
}

static int ss_about(User * u, char **av, int ac)
{
	privmsg_list(u->nick, s_StatServ, ss_help_about);
	return 1;
}

static int ss_netstats(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(s_StatServ, "%s Wanted to see the NetStats ", u->nick);
	prefmsg(u->nick, s_StatServ, "Network Statistics:-----");
	prefmsg(u->nick, s_StatServ, "Current Users: %ld",
		stats_network.users);
	prefmsg(u->nick, s_StatServ, "Maximum Users: %ld [%s]",
		stats_network.maxusers, sftime(stats_network.t_maxusers));
	prefmsg(u->nick, s_StatServ, "Total Users Connected: %ld",
		stats_network.totusers);
	prefmsg(u->nick, s_StatServ, "Current Channels %ld",
		stats_network.chans);
	prefmsg(u->nick, s_StatServ, "Maximum Channels %ld [%s]",
		stats_network.maxchans, sftime(stats_network.t_chans));
	prefmsg(u->nick, s_StatServ, "Current Opers: %ld",
		stats_network.opers);
	prefmsg(u->nick, s_StatServ, "Maximum Opers: %ld [%s]",
		stats_network.maxopers, sftime(stats_network.t_maxopers));
	prefmsg(u->nick, s_StatServ, "Users Set Away: %ld",
		stats_network.away);
	prefmsg(u->nick, s_StatServ, "Current Servers: %ld",
		stats_network.servers);
	prefmsg(u->nick, s_StatServ, "Maximum Servers: %ld [%s]",
		stats_network.maxservers,
		sftime(stats_network.t_maxservers));
	prefmsg(u->nick, s_StatServ, "--- End of List ---");
	return 1;
}
static int ss_daily(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(s_StatServ, "%s Wanted to see the Daily NetStats ", u->nick);
	prefmsg(u->nick, s_StatServ, "Daily Network Statistics:");
	prefmsg(u->nick, s_StatServ, "Maximum Servers: %-2d %s",
		daily.servers, sftime(daily.t_servers));
	prefmsg(u->nick, s_StatServ, "Maximum Users: %-2d %s", daily.users,
		sftime(daily.t_users));
	prefmsg(u->nick, s_StatServ, "Maximum Chans: %-2d %s", daily.chans,
		sftime(daily.t_chans));
	prefmsg(u->nick, s_StatServ, "Maximum Opers: %-2d %s", daily.opers,
		sftime(daily.t_opers));
	prefmsg(u->nick, s_StatServ, "Total Users Connected: %-2d",
		daily.tot_users);
	prefmsg(u->nick, s_StatServ,
		"All Daily Statistics are reset at Midnight");
	prefmsg(u->nick, s_StatServ, "End of Information.");
	return 1;
}

static void makemap(char *uplink, User * u, int level)
{
	hscan_t hs;
	hnode_t *sn;
	Server *s;
	SStats *ss;
	char buf[256];
	int i;
	hash_scan_begin(&hs, sh);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = findstats(s->name);
		if ((level == 0) && (s->uplink[0] == 0)) {
			/* its the root server */
			prefmsg(u->nick, s_StatServ,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping,
				ss->highest_ping);
			makemap(s->name, u, level + 1);
		} else if ((level > 0) && !strcasecmp(uplink, s->uplink)) {
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				strlcat (buf, "     |", 256);
			}
			prefmsg(u->nick, s_StatServ,
				"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				buf, ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping,
				ss->highest_ping);
			makemap(s->name, u, level + 1);
		}
	}
}

static int ss_map(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(s_StatServ, "%s Wanted to see the Current Network MAP", u->nick);
	prefmsg(u->nick, s_StatServ, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2",
		"\2[LAG/MAX]\2");
	makemap("", u, 0);
	prefmsg(u->nick, s_StatServ, "--- End of Listing ---");
	return 1;
}

static int ss_server(User * u, char **av, int ac)
{
	SStats *ss;
	Server *s;
	hscan_t hs;
	hnode_t *sn;
	char *server;

	SET_SEGV_LOCATION();
	server =  av[2];
	chanalert(s_StatServ, "%s Wanted Server Information on %s", u->nick, av[2]);
	if (!server) {
		prefmsg(u->nick, s_StatServ, "Server Listing:");
		hash_scan_begin(&hs, Shead);
		while ((sn = hash_scan_next(&hs))) {
			ss = hnode_get(sn);
			if (findserver(ss->name)) {
				prefmsg(u->nick, s_StatServ,
					"Server: %s (*)", ss->name);
			} else {
				prefmsg(u->nick, s_StatServ, "Server: %s",
					ss->name);
			}
		}
		prefmsg(u->nick, s_StatServ,
			"***** End of List (* indicates Server is online at the moment) *****");
		return 0;
	}

	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findstats(server);
	s = findserver(server);
	if (!ss) {
		nlog(LOG_CRITICAL, LOG_CORE,
		     "Wups, Problem, Cant find Server Statistics for Server %s",
		     server);
		prefmsg(u->nick, s_StatServ,
			"Internal Error! Please Consult the Log file");
		return 0;
	}
	prefmsg(u->nick, s_StatServ, "Statistics for \2%s\2 since %s",
		ss->name, sftime(ss->starttime));
	if (!s)
		prefmsg(u->nick, s_StatServ, "Server Last Seen: %s",
			sftime(ss->lastseen));
	if (s)
		prefmsg(u->nick, s_StatServ,
			"Current Users: %-3ld (%2.0f%%)", (long)ss->users,
			(float) ss->users / (float) stats_network.users *
			100);
	prefmsg(u->nick, s_StatServ, "Maximum Users: %-3ld at %s",
		ss->maxusers, sftime(ss->t_maxusers));
	prefmsg(u->nick, s_StatServ, "Total Users Connected: %-3ld",
		ss->totusers);
	if (s)
		prefmsg(u->nick, s_StatServ, "Current Opers: %-3ld",
			(long)ss->opers);
	prefmsg(u->nick, s_StatServ, "Maximum Opers: %-3ld at %s",
		(long)ss->maxopers, sftime(ss->t_maxopers));
	prefmsg(u->nick, s_StatServ, "IRCop Kills: %d", ss->operkills);
	prefmsg(u->nick, s_StatServ, "Server Kills: %d", ss->serverkills);
	prefmsg(u->nick, s_StatServ, "Lowest Ping: %-3d at %s",
		(int)ss->lowest_ping, sftime(ss->t_lowest_ping));
	prefmsg(u->nick, s_StatServ, "Higest Ping: %-3d at %s",
		(int)ss->highest_ping, sftime(ss->t_highest_ping));
	if (s)
		prefmsg(u->nick, s_StatServ, "Current Ping: %-3d",
			s->ping);
	if (ss->numsplits >= 1)
		prefmsg(u->nick, s_StatServ,
			"%s has Split from the network %d time%s",
			ss->name, ss->numsplits,
			(ss->numsplits == 1) ? "" : "s");
	else
		prefmsg(u->nick, s_StatServ,
			"%s has never split from the Network.", ss->name);
	prefmsg(u->nick, s_StatServ, "***** End of Statistics *****");
	return 1;
}

static int ss_tld(User * u, char **av, int ac)
{
	TLD *tmp;
	char *tld;
    
	SET_SEGV_LOCATION();
	tld = av[2];
	chanalert(s_StatServ, "%s Wanted to find the Country that is Represented by %s ", u->nick, av[2]);
	if (!tld) {
		prefmsg(u->nick, s_StatServ, "Syntax: /msg %s TLD <tld>",
			s_StatServ);
		prefmsg(u->nick, s_StatServ,
			"For additional help, /msg %s HELP", s_StatServ);
		return 0;
	}

	if (*tld == '*')
		tld++;
	if (*tld == '.')
		tld++;

	tmp = findtld(tld);

	if (!tmp)
		prefmsg(u->nick, s_StatServ,
			"Top Level Domain \2%s\2 does not exist.", tld);
	else
		prefmsg(u->nick, s_StatServ, tmp->country);
	return 1;
}

static int ss_operlist(User * u, char **av, int ac)
{
	register int j = 0;
	int away = 0;
	register User *testuser;
	int ulevel = 0;
	hscan_t scan;
	hnode_t *node;
	char *flags;
	char *server;

	SET_SEGV_LOCATION();

	if (ac == 2) {
		prefmsg(u->nick, s_StatServ, "On-Line IRCops:");
		prefmsg(u->nick, s_StatServ, "ID  %-15s %-15s %-10s", "Nick", "Server",
			"Level");
		chanalert(s_StatServ, "%s Requested OperList", u->nick);
	}

	flags = av[2];
	server = av[3];
	if (flags && !strcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		prefmsg(u->nick, s_StatServ, "On-Line IRCops (Not Away):");
		chanalert(s_StatServ,
			  "%s Reqested Operlist of Non-Away Opers", u->nick);
	}
	if (!away && flags && strchr(flags, '.')) {
		server = flags;
		prefmsg(u->nick, s_StatServ,
			"On-Line IRCops on Server %s", server);
		chanalert(s_StatServ, "%s Reqested Operlist on Server %s",
			  u->nick, server);
	}
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		testuser = hnode_get(node);
		if (!is_oper(testuser))
			continue;
		ulevel = UserLevel(testuser);
		if (ulevel < NS_ULEVEL_OPER)
			continue;
		if (away && testuser->is_away)
			continue;
		if (!strcasecmp(testuser->server->name, me.services_name))
			continue;
		if (!server) {
			j++;
			prefmsg(u->nick, s_StatServ,
				"[%2d] %-15s %-15s %-10d", j, testuser->nick,
				testuser->server->name, ulevel);
			continue;
		} else {
			if (strcasecmp(server, testuser->server->name))
				continue;
			j++;
			prefmsg(u->nick, s_StatServ,
				"[%2d] %-15s %-15s %-10d", j, testuser->nick,
				testuser->server->name, ulevel);
			continue;
		}
	}
	prefmsg(u->nick, s_StatServ, "End of Listing.");
	return 1;
}


#ifdef GOTBOTMODE
static int ss_botlist(User * u, char **av, int ac)
{
	register int j = 0;
	register User *testuser;
	hscan_t scan;
	hnode_t *node;

	SET_SEGV_LOCATION();
	chanalert(s_StatServ, "%s Wanted to see the Bot List", u->nick);
	prefmsg(u->nick, s_StatServ, "On-Line Bots:");
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		testuser = hnode_get(node);
		if is_bot(testuser) { 
			j++;
			prefmsg(u->nick, s_StatServ,
				"[%2d] %-15s %s", j, testuser->nick,
				testuser->server->name);
		}
	}
	prefmsg(u->nick, s_StatServ, "End of Listing.");
	return 1;
}
#endif

static int ss_stats(User * u, char **av, int ac)
{
	SStats *st;
	hnode_t *node;
	hscan_t scan;
	char *cmd;
	char *arg; 
	char *arg2;

	SET_SEGV_LOCATION();
	cmd = av[2];
	arg = av[3]; 
	arg2 = av[4];
	if (!strcasecmp(cmd, "LIST")) {
		int i = 1;
		prefmsg(u->nick, s_StatServ, "Statistics Database:");
		hash_scan_begin(&scan, Shead);
		while ((node = hash_scan_next(&scan))) {
			st = hnode_get(node);
			prefmsg(u->nick, s_StatServ, "[%-2d] %s", i, st->name);
			i++;
		}
		prefmsg(u->nick, s_StatServ, "End of List.");
		nlog(LOG_NOTICE, LOG_MOD, "%s requested STATS LIST.", u->nick);
	} else if (!strcasecmp(cmd, "DEL")) {
		if (!arg) {
			prefmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS DEL <name>", s_StatServ);
			prefmsg(u->nick, s_StatServ, "For additonal help, /msg %s HELP", s_StatServ);
			return 0;
		}
		st = findstats(arg);
		if (!st) {
			prefmsg(u->nick, s_StatServ, "%s is not in the database!", arg);
			return 0;
		}
		if (!findserver(arg)) {
			node = hash_lookup(Shead, arg);
			if (node) {
				hash_delete(Shead, node);
				st = hnode_get(node);
				hnode_destroy(node);
				free(st);
				prefmsg(u->nick, s_StatServ, "Removed %s from the database.", arg);
				nlog(LOG_NOTICE, LOG_MOD, "%s requested STATS DEL %s", u->nick, arg);
				return 0;
			}
		} else {
			prefmsg(u->nick, s_StatServ, 
				"Cannot remove %s from the database, it is online!!",
				arg);
			nlog(LOG_WARNING, LOG_MOD,
			     "%s requested STATS DEL %s, but that server is online!!",
			     u->nick, arg);
				return 0;
		}

	} else if (!strcasecmp(cmd, "COPY")) {
		Server *s;

		if (!arg || !arg2) {
			prefmsg(u->nick, s_StatServ,
				"Syntax: /msg %s STATS COPY <name> "
				" <newname>", s_StatServ);
			return 0;
		}
		st = findstats(arg2);
		if (st)
			free(st);

		st = findstats(arg);
		if (!st) {
			prefmsg(u->nick, s_StatServ,
				"No entry in the database for %s", arg);
			return 0;
		}
		s = findserver(arg);
		if (s) {
			prefmsg(u->nick, s_StatServ,
				"Server %s is online!", arg);
			return 0;
		}
		s = NULL;
		memcpy(st->name, arg2, sizeof(st->name));
		prefmsg(u->nick, s_StatServ,
			"Moved database entry for %s to %s", arg, arg2);
		nlog(LOG_NOTICE, LOG_MOD,
		     "%s requested STATS COPY %s -> %s", u->nick, arg,
		     arg2);
	} else {
		prefmsg(u->nick, s_StatServ, "Invalid Argument.");
		prefmsg(u->nick, s_StatServ, "For help, /msg %s HELP",
			s_StatServ);
	}
	return 1;
}

static int ss_forcehtml(User * u, char **av, int ac)
{
	nlog(LOG_NOTICE, LOG_MOD,
		    "%s!%s@%s Forced an update of the NeoStats Statistics HTML file with the most current statistics",
		    u->nick, u->username, u->hostname);
	chanalert(s_StatServ,
			"%s Forced the NeoStats Statistics HTML file to be updated with the most current statistics",
			u->nick);
	ss_html();
	return 1;
}

