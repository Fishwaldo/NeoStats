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
#include "statserv.h"
#ifdef SQLSRV
#include "sqlsrv/rta.h"
#include "sqlstats.h"
#endif

static int ss_chans(User * u, char **av, int ac);
static int ss_daily(User * u, char **av, int ac);
static int ss_stats(User * u, char **av, int ac);
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
static void ss_Config(void);

ModuleInfo module_info = {
	"statserv",
	"Network statistical service",
	NULL,
	NULL,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

ModuleEvent module_events[] = {
	{EVENT_ONLINE, Online},
	{EVENT_PONG, pong},
	{EVENT_SERVER, s_new_server},
	{EVENT_SQUIT, s_del_server},
	{EVENT_SIGNON, s_new_user},
	{EVENT_GOTNICKIP, s_got_nickip},
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

static void ss_Config(void)
{
	char *tmp;

	SET_SEGV_LOCATION();

	if (GetConf((void *) &tmp, CFGSTR, "Nick") < 0) {
		strlcpy(ss_botinfo.nick, "StatServ", MAXNICK);
	} else {
		strlcpy(ss_botinfo.nick, tmp, MAXNICK);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "User") < 0) {
		strlcpy(ss_botinfo.user, "SS", MAXUSER);
	} else {
		strlcpy(ss_botinfo.user, tmp, MAXUSER);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Host") < 0) {
		strlcpy(ss_botinfo.host, me.name, MAXHOST);
	} else {
		strlcpy(ss_botinfo.host, tmp, MAXHOST);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "RealName") < 0) {
		ircsnprintf(ss_botinfo.realname, MAXREALNAME, "/msg %s help",
			 ss_bot->nick);
	} else {
		strlcpy(ss_botinfo.realname, tmp, MAXREALNAME);
		free(tmp);
	}
	if (GetConf((void *) &StatServ.lagtime, CFGINT, "LagTime") < 0) {
		StatServ.lagtime = 30;
	}
	if (GetConf((void *) &StatServ.exclusions, CFGBOOL, "Exclusions") < 0) {
		StatServ.exclusions = 0;
	}
	if (GetConf((void *) &StatServ.lagalert, CFGINT, "LagAlert") < 0) {
		StatServ.lagalert = 1;
	}
	if (GetConf((void *) &StatServ.recordalert, CFGINT, "RecordAlert") < 0) {
		StatServ.recordalert = 1;
	}
	if (GetConf((void *) &StatServ.msginterval , CFGINT, "MsgInterval") < 0) {
		StatServ.msginterval  = 60;
	}
	if (GetConf((void *) &StatServ.msglimit , CFGINT, "MsgLimit") < 0) {
		StatServ.msglimit  = 5;
	}
	if (GetConf((void *) &tmp, CFGSTR, "HTML_Path") < 0) {
		StatServ.html = 0;
		StatServ.htmlpath[0] = 0;
	} else {
		/* assume that html is enabled if we don't have a setting for it */
		if (GetConf((void *) &StatServ.html, CFGINT, "HTML_Enabled") < 0) {
			StatServ.html = 1;
		}
		strlcpy(StatServ.htmlpath, tmp, MAXPATH);
		free(tmp);
	}
}

int ModInit(Module* mod_ptr)
{
	Server *ss;
	User *u;
	hnode_t *node;
	hscan_t scan;
#ifdef SQLSRV
	lnode_t *lnode;
#endif
	int count, i;
	Channel *c;
	char **av;
	int ac = 0;
	char *chan;
	lnode_t *chanmem;

	SET_SEGV_LOCATION();

	StatServ.onchan = 0;
	StatServ.shutdown = 0;
	/* we want nickip messages */
	me.want_nickip = 1;

	ss_Config();
	if (StatServ.html && StatServ.htmlpath[0] == 0) {
		nlog(LOG_NOTICE,
		     "StatServ HTML stats disabled as HTML_PATH is not set");
		StatServ.html = 0;
	}
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
		nlog(LOG_DEBUG2, "Added Server %s to StatServ List", ss->name);
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
		nlog(LOG_DEBUG2, "Add user %s to StatServ List", u->nick);
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
			nlog(LOG_DEBUG2, "Chanjoin %s", c->name);
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

#ifdef SQLSRV
	/* ok, now export the server and chan data into the sql emulation layers */

	/* for the network and daily stats, we use a fake list, so we can easily import into rta */

	fakedaily = list_create(-1);
	lnode = lnode_create(&daily);
	list_append(fakedaily, lnode);
	fakenetwork = list_create(-1);
	lnode = lnode_create(&stats_network);
	list_append(fakenetwork, lnode);
	
	/* find the address of each list/hash, and export to rta */
	 
	statserv_chans.address = Chead;
	rta_add_table(&statserv_chans);
	statserv_tld.address = Thead;
	rta_add_table(&statserv_tld);
	statserv_servers.address = Shead;
	rta_add_table(&statserv_servers);
	statserv_versions.address = Vhead;
	rta_add_table(&statserv_versions);
	statserv_network.address = fakenetwork;
	rta_add_table(&statserv_network);
	statserv_daily.address = fakedaily;
	rta_add_table(&statserv_daily);

#endif
#ifdef USE_BERKELEY
	DBOpenDatabase();
#endif
	load_client_versions();
	return 1;
}

void ModFini()
{
	StatServ.shutdown = 1;
	SaveStats();
	fini_tld();
	save_client_versions();
#if SQLSRV
	list_destroy_nodes(fakedaily);
	list_destroy_nodes(fakenetwork);
#endif
#ifdef USE_BERKELEY
	DBCloseDatabase();
#endif      
}

bot_cmd ss_commands[]=
{
	{"ABOUT",			ss_about,		0, 	0,		ss_help_about, 		 	ss_help_about_oneline},
	{"VERSION",			ss_version,		0, 	0,		ss_help_version, 	 	ss_help_version_oneline},
	{"SERVER",			ss_server,		0, 	0,		ss_help_server,		 	ss_help_server_oneline},
	{"MAP",				ss_map,			0, 	0,		ss_help_map, 		 	ss_help_map_oneline},
	{"CHAN",			ss_chans,		0, 	0,		ss_help_chan, 		 	ss_help_chan_oneline},
	{"NETSTATS",		ss_netstats,	0, 	0,		ss_help_netstats, 	 	ss_help_netstats_oneline},
	{"DAILY",			ss_daily,		0, 	0,		ss_help_daily, 		 	ss_help_daily_oneline},
	{"TLDMAP",			ss_tld_map,		0, 	0,		ss_help_tldmap, 	 	ss_help_tldmap_oneline},
	{"OPERLIST",		ss_operlist,	0, 	0,		ss_help_operlist, 	 	ss_help_operlist_oneline},
#ifdef GOTBOTMODE																	
	{"BOTLIST",			ss_botlist,		0, 	0,		ss_help_botlist, 	 	ss_help_botlist_oneline},
#endif																						
	{"CLIENTVERSIONS",	ss_clientversions,	0,	0,		ss_help_clientversions, ss_help_clientversions_oneline},
	{"FORCEHTML",		ss_forcehtml,	0, 	NS_ULEVEL_ADMIN,	ss_help_forcehtml, 		ss_help_forcehtml_oneline},
	{"STATS",			ss_stats,		1, 	NS_ULEVEL_ADMIN,	ss_help_stats, 			ss_help_stats_oneline},
	{NULL,				NULL,			0, 	0,					NULL, 					NULL}
};

bot_setting ss_settings[]=
{
	{"NICK",		&ss_botinfo.nick,		SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",	NULL,	ns_help_set_nick },
	{"USER",		&ss_botinfo.user,		SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",	NULL,	ns_help_set_user },
	{"HOST",		&ss_botinfo.host,		SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",	NULL,	ns_help_set_host },
	{"REALNAME",	&ss_botinfo.realname,	SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",NULL,	ns_help_set_realname },
	{"HTML",		&StatServ.html,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN,	"HTML_Enabled",		NULL,		ss_help_set_html},
	{"HTMLPATH",	&StatServ.htmlpath,		SET_TYPE_STRING,	0, MAXPATH,		NS_ULEVEL_ADMIN,	"HTML_Path",		NULL,		ss_help_set_htmlpath },
	{"MSGINTERVAL",	&StatServ.msginterval,	SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN,	"MsgInterval",		"seconds",	ss_help_set_msginterval },
	{"MSGLIMIT",	&StatServ.msglimit,		SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN,	"MsgLimit",			NULL,		ss_help_set_msglimit },
	{"LAGTIME",		&StatServ.lagtime,		SET_TYPE_INT,		1, 256,			NS_ULEVEL_ADMIN,	"LagTime",			"seconds",	ss_help_set_lagtime },
	{"LAGALERT",	&StatServ.lagalert,		SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN,	"LagAlert",			NULL,		ss_help_set_lagalert },
	{"RECORDALERT", &StatServ.recordalert,	SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN,	"RecordAlert",		NULL,		ss_help_set_recordalert },
	{"USEEXCLUSIONS", &StatServ.exclusions, SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN,	"Exclusions",		NULL,		ss_help_set_exclusions },
	{NULL,			NULL,					0,					0, 0,			0,					NULL,				NULL,		NULL },
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

	chanalert(ss_bot->nick, "%s Wanted to see the Client Version List", u->nick);
	num = ac > 2 ? atoi(av[2]) : 10;
	if (num < 10) {
		num = 10;
	}
	if (list_count(Vhead) == 0) {
		prefmsg(u->nick, ss_bot->nick, "No Stats Available.");
		return 0;
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
	return 1;
}

static int ss_chans(User * u, char **av, int ac)
{
	CStats *cs;
	lnode_t *cn;
	int i;

	chanalert(ss_bot->nick, "%s Wanted to see Channel Statistics", u->nick);
	if (!av[2]) {
		/* they want the top10 Channels online atm */
		if (!list_is_sorted(Chead, topchan)) {
			list_sort(Chead, topchan);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, ss_bot->nick, "Top10 Online Channels:");
		prefmsg(u->nick, ss_bot->nick, "======================");
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
			prefmsg(u->nick, ss_bot->nick,
				"Channel %s -> %ld Members", cs->name,
				cs->members);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, ss_bot->nick, "End of List.");
	} else if (!ircstrcasecmp(av[2], "POP")) {
		/* they want the top10 Popular Channels (based on joins) */
		if (!list_is_sorted(Chead, topjoin)) {
			list_sort(Chead, topjoin);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, ss_bot->nick, "Top10 Channels (Ever):");
		prefmsg(u->nick, ss_bot->nick, "======================");
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
			prefmsg(u->nick, ss_bot->nick,
				"Channel %s -> %ld Joins", cs->name,
				cs->totmem);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, ss_bot->nick, "End of List.");
	} else if (!ircstrcasecmp(av[2], "KICKS")) {
		/* they want the top10 most unwelcome channels (based on kicks) */
		if (!list_is_sorted(Chead, topkick)) {
			list_sort(Chead, topkick);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, ss_bot->nick,
			"Top10 Most un-welcome Channels (Ever):");
		prefmsg(u->nick, ss_bot->nick,
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
			prefmsg(u->nick, ss_bot->nick,
				"Channel %s -> %ld Kicks", cs->name,
				cs->kicks);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, ss_bot->nick, "End of List.");
	} else if (!ircstrcasecmp(av[2], "TOPICS")) {
		/* they want the top10 most undecisive channels (based on topics) */
		if (!list_is_sorted(Chead, toptopics)) {
			list_sort(Chead, toptopics);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(u->nick, ss_bot->nick, "Top10 Most undecisive Channels (Ever):");
		prefmsg(u->nick, ss_bot->nick, "======================================");
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
			prefmsg(u->nick, ss_bot->nick, "Channel %s -> %ld Topics", cs->name,
				cs->topics);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(u->nick, ss_bot->nick, "End of List.");
	} else {
		cs = findchanstats(av[2]);
		if (!cs) {
			prefmsg(u->nick, ss_bot->nick,
				"Error, Can't find any information about Channel %s", av[2]);
			return 0;
		}
		prefmsg(u->nick, ss_bot->nick, "\2Channel Information for %s (%s)\2", 
			av[2], (findchan(av[2]) ? "Online" : "Offline"));
		prefmsg(u->nick, ss_bot->nick, "Current Members: %ld (Max %ld on %s)",
			cs->members, cs->maxmems, sftime(cs->t_maxmems));
		prefmsg(u->nick, ss_bot->nick,
			"Max Members today: %ld at %s", cs->maxmemtoday,
			sftime(cs->t_maxmemtoday));
		prefmsg(u->nick, ss_bot->nick,
			"Total Number of Channel Joins: %ld", cs->totmem);
		prefmsg(u->nick, ss_bot->nick, 
			"Total Member Joins today: %ld (Max %ld on %s)",
			cs->joinstoday, cs->maxjoins, sftime(cs->t_maxjoins));
		prefmsg(u->nick, ss_bot->nick,
			"Total Topic Changes %ld (Today %ld)", cs->topics, cs->topicstoday);
		prefmsg(u->nick, ss_bot->nick, "Total Kicks: %ld", cs->kicks);
		prefmsg(u->nick, ss_bot->nick, "Total Kicks today %ld (Max %ld on %s)",
			cs->maxkickstoday, cs->maxkicks, sftime(cs->t_maxkicks));
		if (!findchan(av[2]))
			prefmsg(u->nick, ss_bot->nick, "Channel was last seen at %s",
				sftime(cs->lastseen));
	}
	return 1;
}

static int ss_tld_map(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see a Country Breakdown", u->nick);
	prefmsg(u->nick, ss_bot->nick, "Top Level Domain Statistics:");
	DisplayTLDmap(u);
	prefmsg(u->nick, ss_bot->nick, "End of List");
	return 1;
}

static int ss_version(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	prefmsg(u->nick, ss_bot->nick, "\2%s Version Information\2", ss_bot->nick);
	prefmsg(u->nick, ss_bot->nick, "%s Version: %s Compiled %s at %s", ss_bot->nick,
		module_info.version, module_info.build_date, module_info.build_time);
	prefmsg(u->nick, ss_bot->nick, "http://www.neostats.net");
	return 1;
}

static int ss_about(User * u, char **av, int ac)
{
	privmsg_list(u->nick, ss_bot->nick, ss_help_about);
	return 1;
}

static int ss_netstats(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see the NetStats ", u->nick);
	prefmsg(u->nick, ss_bot->nick, "Network Statistics:-----");
	prefmsg(u->nick, ss_bot->nick, "Current Users: %ld", stats_network.users);
	prefmsg(u->nick, ss_bot->nick, "Maximum Users: %ld [%s]",
		stats_network.maxusers, sftime(stats_network.t_maxusers));
	prefmsg(u->nick, ss_bot->nick, "Total Users Connected: %ld",
		stats_network.totusers);
	prefmsg(u->nick, ss_bot->nick, "Current Channels %ld", stats_network.chans);
	prefmsg(u->nick, ss_bot->nick, "Maximum Channels %ld [%s]",
		stats_network.maxchans, sftime(stats_network.t_chans));
	prefmsg(u->nick, ss_bot->nick, "Current Opers: %ld",
		stats_network.opers);
	prefmsg(u->nick, ss_bot->nick, "Maximum Opers: %ld [%s]",
		stats_network.maxopers, sftime(stats_network.t_maxopers));
	prefmsg(u->nick, ss_bot->nick, "Users Set Away: %ld", stats_network.away);
	prefmsg(u->nick, ss_bot->nick, "Current Servers: %ld", stats_network.servers);
	prefmsg(u->nick, ss_bot->nick, "Maximum Servers: %ld [%s]",
		stats_network.maxservers, sftime(stats_network.t_maxservers));
	prefmsg(u->nick, ss_bot->nick, "--- End of List ---");
	return 1;
}

static int ss_daily(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see the Daily NetStats ", u->nick);
	prefmsg(u->nick, ss_bot->nick, "Daily Network Statistics:");
	prefmsg(u->nick, ss_bot->nick, "Maximum Servers: %-2d %s",
		daily.servers, sftime(daily.t_servers));
	prefmsg(u->nick, ss_bot->nick, "Maximum Users: %-2d %s", daily.users,
		sftime(daily.t_users));
	prefmsg(u->nick, ss_bot->nick, "Maximum Channel: %-2d %s", daily.chans,
		sftime(daily.t_chans));
	prefmsg(u->nick, ss_bot->nick, "Maximum Opers: %-2d %s", daily.opers,
		sftime(daily.t_opers));
	prefmsg(u->nick, ss_bot->nick, "Total Users Connected: %-2d",
		daily.tot_users);
	prefmsg(u->nick, ss_bot->nick, "Daily statistics are reset at midnight");
	prefmsg(u->nick, ss_bot->nick, "End of Information.");
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
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			prefmsg(u->nick, ss_bot->nick,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping,
				ss->highest_ping);
			makemap(s->name, u, level + 1);
		} else if ((level > 0) && !ircstrcasecmp(uplink, s->uplink)) {
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				strlcat (buf, "     |", 256);
			}
			prefmsg(u->nick, ss_bot->nick,
				"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				buf, ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping, ss->highest_ping);
			makemap(s->name, u, level + 1);
		}
	}
}

static int ss_map(User * u, char **av, int ac)
{
	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see the Current Network MAP", u->nick);
	prefmsg(u->nick, ss_bot->nick, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2", "\2[LAG/MAX]\2");
	makemap("", u, 0);
	prefmsg(u->nick, ss_bot->nick, "--- End of Listing ---");
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
	chanalert(ss_bot->nick, "%s Wanted Server Information on %s", u->nick, av[2]);
	if (!server) {
		prefmsg(u->nick, ss_bot->nick, "Server Listing:");
		hash_scan_begin(&hs, Shead);
		while ((sn = hash_scan_next(&hs))) {
			ss = hnode_get(sn);
			if (findserver(ss->name)) {
				prefmsg(u->nick, ss_bot->nick, "Server: %s (*)", ss->name);
			} else {
				prefmsg(u->nick, ss_bot->nick, "Server: %s", ss->name);
			}
		}
		prefmsg(u->nick, ss_bot->nick,
			"***** End of List (* indicates Server is online) *****");
		return 0;
	}

	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findstats(server);
	s = findserver(server);
	if (!ss) {
		nlog(LOG_CRITICAL, "Unable to find server statistics for %s", server);
		prefmsg(u->nick, ss_bot->nick,
			"Internal Error! Please Consult the Log file");
		return 0;
	}
	prefmsg(u->nick, ss_bot->nick, "Statistics for \2%s\2 since %s",
		ss->name, sftime(ss->starttime));
	if (!s) {
		prefmsg(u->nick, ss_bot->nick, "Server Last Seen: %s", 
			sftime(ss->lastseen));
	} else {
		prefmsg(u->nick, ss_bot->nick,
			"Current Users: %-3ld (%2.0f%%)", (long)ss->users,
			(float) ss->users / (float) stats_network.users *
			100);
	}
	prefmsg(u->nick, ss_bot->nick, "Maximum Users: %-3ld at %s",
		ss->maxusers, sftime(ss->t_maxusers));
	prefmsg(u->nick, ss_bot->nick, "Total Users Connected: %-3ld", ss->totusers);
	if (s) {
		prefmsg(u->nick, ss_bot->nick, "Current Opers: %-3ld", (long)ss->opers);
	}
	prefmsg(u->nick, ss_bot->nick, "Maximum Opers: %-3ld at %s",
		(long)ss->maxopers, sftime(ss->t_maxopers));
	prefmsg(u->nick, ss_bot->nick, "IRCop Kills: %d", ss->operkills);
	prefmsg(u->nick, ss_bot->nick, "Server Kills: %d", ss->serverkills);
	prefmsg(u->nick, ss_bot->nick, "Lowest Ping: %-3d at %s",
		(int)ss->lowest_ping, sftime(ss->t_lowest_ping));
	prefmsg(u->nick, ss_bot->nick, "Higest Ping: %-3d at %s",
		(int)ss->highest_ping, sftime(ss->t_highest_ping));
	if (s) {
		prefmsg(u->nick, ss_bot->nick, "Current Ping: %-3d", s->ping);
	}
	if (ss->numsplits >= 1) {
		prefmsg(u->nick, ss_bot->nick,
			"%s has Split from the network %d time%s",
			ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	} else {
		prefmsg(u->nick, ss_bot->nick,
			"%s has never split from the Network.", ss->name);
	}
	prefmsg(u->nick, ss_bot->nick, "***** End of Statistics *****");
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
		prefmsg(u->nick, ss_bot->nick, "On-Line IRCops:");
		prefmsg(u->nick, ss_bot->nick, "ID  %-15s %-15s %-10s", "Nick", "Server",
			"Level");
		chanalert(ss_bot->nick, "%s Requested OperList", u->nick);
	}

	flags = av[2];
	server = av[3];
	if (flags && !ircstrcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		prefmsg(u->nick, ss_bot->nick, "On-Line IRCops (Not Away):");
		chanalert(ss_bot->nick,
			  "%s Reqested Operlist of Non-Away Opers", u->nick);
	}
	if (!away && flags && strchr(flags, '.')) {
		server = flags;
		prefmsg(u->nick, ss_bot->nick, "On-Line IRCops on Server %s", server);
		chanalert(ss_bot->nick, "%s Reqested Operlist on Server %s",
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
		if (!ircstrcasecmp(testuser->server->name, me.services_name))
			continue;
		if (!server) {
			j++;
			prefmsg(u->nick, ss_bot->nick,
				"[%2d] %-15s %-15s %-10d", j, testuser->nick,
				testuser->server->name, ulevel);
			continue;
		} else {
			if (ircstrcasecmp(server, testuser->server->name))
				continue;
			j++;
			prefmsg(u->nick, ss_bot->nick,
				"[%2d] %-15s %-15s %-10d", j, testuser->nick,
				testuser->server->name, ulevel);
			continue;
		}
	}
	prefmsg(u->nick, ss_bot->nick, "End of Listing.");
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
	chanalert(ss_bot->nick, "%s Wanted to see the Bot List", u->nick);
	prefmsg(u->nick, ss_bot->nick, "On-Line Bots:");
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		testuser = hnode_get(node);
		if is_bot(testuser) { 
			j++;
			prefmsg(u->nick, ss_bot->nick,
				"[%2d] %-15s %s", j, testuser->nick,
				testuser->server->name);
		}
	}
	prefmsg(u->nick, ss_bot->nick, "End of Listing.");
	return 1;
}
#endif

static int ss_stats(User * u, char **av, int ac)
{
	SStats *st;
	hnode_t *node;
	hscan_t scan;

	SET_SEGV_LOCATION();
	if (!ircstrcasecmp(av[2], "LIST")) {
		int i = 1;
		prefmsg(u->nick, ss_bot->nick, "Statistics Database:");
		hash_scan_begin(&scan, Shead);
		while ((node = hash_scan_next(&scan))) {
			st = hnode_get(node);
			prefmsg(u->nick, ss_bot->nick, "[%-2d] %s", i, st->name);
			i++;
		}
		prefmsg(u->nick, ss_bot->nick, "End of List.");
		nlog(LOG_NOTICE, "%s requested STATS LIST.", u->nick);
	} else if (!ircstrcasecmp(av[2], "DEL")) {
		if (!av[3]) {
			prefmsg(u->nick, ss_bot->nick, "Syntax: /msg %s STATS DEL <name>", ss_bot->nick);
			prefmsg(u->nick, ss_bot->nick, "For additonal help, /msg %s HELP", ss_bot->nick);
			return 0;
		}
		st = findstats(av[3]);
		if (!st) {
			prefmsg(u->nick, ss_bot->nick, "%s is not in the database!", av[3]);
			return 0;
		}
		if (!findserver(av[3])) {
			node = hash_lookup(Shead, av[3]);
			if (node) {
				hash_delete(Shead, node);
				st = hnode_get(node);
				hnode_destroy(node);
				free(st);
				prefmsg(u->nick, ss_bot->nick, "Removed %s from the database.", av[3]);
				nlog(LOG_NOTICE, "%s requested STATS DEL %s", u->nick, av[3]);
				return 0;
			}
		} else {
			prefmsg(u->nick, ss_bot->nick, 
				"Cannot remove %s from the database, it is online!!",
				av[3]);
			nlog(LOG_WARNING,
			     "%s requested STATS DEL %s, but that server is online!!",
			     u->nick, av[3]);
				return 0;
		}

	} else if (!ircstrcasecmp(av[2], "COPY")) {
		Server *s;

		if (!av[3] || !av[4]) {
			prefmsg(u->nick, ss_bot->nick, "Syntax: /msg %s STATS COPY <name> "
				" <newname>", ss_bot->nick);
			return 0;
		}
		st = findstats(av[4]);
		if (st)
			free(st);

		st = findstats(av[3]);
		if (!st) {
			prefmsg(u->nick, ss_bot->nick,
				"No entry in the database for %s", av[3]);
			return 0;
		}
		s = findserver(av[3]);
		if (s) {
			prefmsg(u->nick, ss_bot->nick, "Server %s is online!", av[3]);
			return 0;
		}
		s = NULL;
		memcpy(st->name, av[4], sizeof(st->name));
		prefmsg(u->nick, ss_bot->nick, 
			"Moved database entry for %s to %s", av[3], av[4]);
		nlog(LOG_NOTICE,
		     "%s requested STATS COPY %s -> %s", u->nick, av[3], av[4]);
	} else {
		prefmsg(u->nick, ss_bot->nick, "Invalid Argument.");
		prefmsg(u->nick, ss_bot->nick, "For help, /msg %s HELP", ss_bot->nick);
	}
	return 1;
}

static int ss_forcehtml(User * u, char **av, int ac)
{
	nlog(LOG_NOTICE, "%s!%s@%s forced an update of the StatServ HTML file.",
		    u->nick, u->username, u->hostname);
	chanalert(ss_bot->nick, "%s forced an update of the StatServ HTML file.",
			u->nick);
	ss_html();
	return 1;
}
