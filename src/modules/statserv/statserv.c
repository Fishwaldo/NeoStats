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

static int ss_chans(CmdParams* cmdparams);
static int ss_daily(CmdParams* cmdparams);
static int ss_stats(CmdParams* cmdparams);
static int ss_tld_map(CmdParams* cmdparams);
static int ss_operlist(CmdParams* cmdparams);
#ifdef GOTBOTMODE
static int ss_botlist(CmdParams* cmdparams);
#endif
static int ss_server(CmdParams* cmdparams);
static int ss_map(CmdParams* cmdparams);
static int ss_netstats(CmdParams* cmdparams);
static int ss_clientversions(CmdParams* cmdparams);
static int ss_forcehtml(CmdParams* cmdparams);

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

static User* listu;
static int listindex = 0;
Bot *ss_bot;
static Module* ss_module;

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

ModuleInfo module_info = {
	"StatServ",
	"Network statistical service",
	ns_copyright,
	ss_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
};

static BotInfo ss_botinfo = 
{
	"StatServ", 
	"StatServ", 
	"SS", 
	"", 
	"Statistics service", 
};

static bot_cmd ss_commands[]=
{
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

static bot_setting ss_settings[]=
{
	{"NICK",		&ss_botinfo.nick,		SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",		NULL,		ns_help_set_nick, NULL, (void*)"StatServ" },
	{"ALTNICK",		&ss_botinfo.altnick,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "AltNick",		NULL,		ns_help_set_altnick, NULL, (void*)"StatServ" },
	{"USER",		&ss_botinfo.user,		SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",		NULL,		ns_help_set_user, NULL, (void*)"SS" },
	{"HOST",		&ss_botinfo.host,		SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",		NULL,		ns_help_set_host, NULL, (void*)"" },
	{"REALNAME",	&ss_botinfo.realname,	SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",	NULL,		ns_help_set_realname, NULL, (void*)"Statistics service" },
	{"HTML",		&StatServ.html,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "HTML_Enabled",NULL,		ss_help_set_html},
	{"HTMLPATH",	&StatServ.htmlpath,		SET_TYPE_STRING,	0, MAXPATH,		NS_ULEVEL_ADMIN, "HTML_Path",	NULL,		ss_help_set_htmlpath },
	{"MSGINTERVAL",	&StatServ.msginterval,	SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN, "MsgInterval",	"seconds",	ss_help_set_msginterval },
	{"MSGLIMIT",	&StatServ.msglimit,		SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN, "MsgLimit",	NULL,		ss_help_set_msglimit },
	{"LAGTIME",		&StatServ.lagtime,		SET_TYPE_INT,		1, 256,			NS_ULEVEL_ADMIN, "LagTime",		"seconds",	ss_help_set_lagtime },
	{"LAGALERT",	&StatServ.lagalert,		SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN, "LagAlert",	NULL,		ss_help_set_lagalert },
	{"RECORDALERT", &StatServ.recordalert,	SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN, "RecordAlert",	NULL,		ss_help_set_recordalert },
	{"USEEXCLUSIONS",&StatServ.exclusions,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, "Exclusions",	NULL,		ss_help_set_exclusions },
	{NULL,			NULL,					0,					0, 0,			0,				 NULL,			NULL,		NULL },
};

static int ss_event_online(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	ss_bot = init_bot (&ss_botinfo, services_bot_modes, 
		BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, ss_commands, ss_settings);
	StatServ.onchan = 1;
	/* now that we are online, setup the timer to save the Stats database every so often */
	add_timer (SaveStats, "SaveStats", DBSAVETIME);
	add_timer (ss_html, "ss_html", 3600);
	/* also add a timer to check if its midnight (to reset the daily stats */
	add_timer (StatsMidnight, "StatsMidnight", 60);
	add_timer (DelOldChan, "DelOldChan", 3600);
	return 1;
}

int ModInit(Module* mod_ptr)
{
#ifdef SQLSRV
	lnode_t *lnode;
#endif

	SET_SEGV_LOCATION();
	ss_module = mod_ptr;
	StatServ.onchan = 0;
	StatServ.shutdown = 0;
	/* we want nickip messages */
	me.want_nickip = 1; 
	ModuleConfig(ss_settings);
	if (StatServ.html && StatServ.htmlpath[0] == 0) {
		nlog(LOG_NOTICE, "HTML stats disabled as HTML_PATH is not set");
		StatServ.html = 0;
	}
	InitTLD();
	InitStats();
	LoadStats();
	GetServerList(StatsAddServer);
	GetUserList(StatsAddUser);
	/* TODO get user modes */
	GetChannelList(StatsAddChan);
	/* TODO get member counts */

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
	chanalert(ss_bot->nick, "Saving StatServ Database. this *could* take a while");
	SaveStats();
	chanalert(ss_bot->nick, "Done");
	FiniStats();
	FiniTLD();
	save_client_versions();
#if SQLSRV
	list_destroy_nodes(fakedaily);
	list_destroy_nodes(fakenetwork);
#endif
#ifdef USE_BERKELEY
	DBCloseDatabase();
#endif      
}

static int ss_event_server(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	StatsAddServer(cmdparams->source.server);
	return 1;
}

static int ss_event_squit(CmdParams* cmdparams)
{
	StatsDelServer(cmdparams->source.server);
	return 1;
}

static int ss_event_newchan(CmdParams* cmdparams)
{
	StatsAddChan(cmdparams->channel);
	return 1;
}

static int ss_event_delchan(CmdParams* cmdparams)
{
	StatsDelChan(cmdparams->channel);
	return 1;
}

static int ss_event_join(CmdParams* cmdparams)
{										   
	if (StatServ.exclusions && (IsExcluded(cmdparams->channel) || IsExcluded(cmdparams->source.user))) {
		return 0;
	}
	StatsJoinChan(cmdparams->source.user, cmdparams->channel);
	return 1;
}

static int ss_event_part(CmdParams* cmdparams)
{
	if (StatServ.exclusions && (IsExcluded(cmdparams->channel) || IsExcluded(cmdparams->source.user))) {
		return 0;
	}
	StatsPartChan(cmdparams->source.user, cmdparams->channel);
	return 1;
}

static int ss_event_topic(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return 1;
	}
	StatsChanTopic(cmdparams->channel);
	return 1;
}

static int ss_event_kick(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return 1;
	}
	StatsChanKick(cmdparams->channel);
	return 1;
}

int ss_event_ctcpversion(CmdParams* cmdparams)
{
	StatsAddCTCPVersion(cmdparams->av[1]);
	return 1;
}

static int ss_event_kill(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	StatsKillUser(cmdparams->source.user);
	return 1;
}

static int ss_event_mode(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	StatsUserMode(cmdparams->source.user, cmdparams->av[1]);
	return 1;
}

static int ss_event_quit(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	StatsDelUser(cmdparams->source.user);
	return 1;
}

static int ss_event_away(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	StatsUserAway(cmdparams->source.user);
	return 1;
}

static int ss_event_nickip(CmdParams* cmdparams)
{
	AddTLD(cmdparams->source.user);
	return 1;
}

static int ss_event_signon(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source.user)) {
		return 0;
	}
	StatsAddUser(cmdparams->source.user);
	return 1;
}

static int ss_event_pong(CmdParams* cmdparams)
{
	StatsServerPong(cmdparams->source.server);
	return 1;
}

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

static int ss_clientversions(CmdParams* cmdparams)
{
	int num;

	num = cmdparams->ac > 2 ? atoi(cmdparams->av[2]) : 10;
	if (num < 10) {
		num = 10;
	}
	list_client_versions(cmdparams->source.user, num);
	return 1;
}

static int ss_chans(CmdParams* cmdparams)
{
	CStats *cs;
	lnode_t *cn;
	int i;

	if (!cmdparams->av[2]) {
		/* they want the top10 Channels online atm */
		if (!list_is_sorted(Chead, topchan)) {
			list_sort(Chead, topchan);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Top10 Online Channels:");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "======================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(cmdparams->source.user) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(cmdparams->source.user->nick, ss_bot->nick,
				"Channel %s -> %ld Members", cs->name,
				cs->members);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List.");
	} else if (!ircstrcasecmp(cmdparams->av[2], "POP")) {
		/* they want the top10 Popular Channels (based on joins) */
		if (!list_is_sorted(Chead, topjoin)) {
			list_sort(Chead, topjoin);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Top10 Channels (Ever):");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "======================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(cmdparams->source.user) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Channel %s -> %ld Joins", 
				cs->name, cs->totmem);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List.");
	} else if (!ircstrcasecmp(cmdparams->av[2], "KICKS")) {
		/* they want the top10 most unwelcome channels (based on kicks) */
		if (!list_is_sorted(Chead, topkick)) {
			list_sort(Chead, topkick);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"Top10 Most un-welcome Channels (Ever):");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"======================================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(cmdparams->source.user) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Channel %s -> %ld Kicks", 
				cs->name, cs->kicks);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List.");
	} else if (!ircstrcasecmp(cmdparams->av[2], "TOPICS")) {
		/* they want the top10 most undecisive channels (based on topics) */
		if (!list_is_sorted(Chead, toptopics)) {
			list_sort(Chead, toptopics);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Top10 Most undecisive Channels (Ever):");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "======================================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(findchan(cs->name))
			    && (UserLevel(cmdparams->source.user) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Channel %s -> %ld Topics", 
				cs->name, cs->topics);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List.");
	} else {
		cs = findchanstats(cmdparams->av[2]);
		if (!cs) {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick,
				"Error, Can't find any information about Channel %s", cmdparams->av[2]);
			return 0;
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "\2Channel Information for %s (%s)\2", 
			cmdparams->av[2], (findchan(cmdparams->av[2]) ? "Online" : "Offline"));
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Members: %ld (Max %ld on %s)",
			cs->members, cs->maxmems, sftime(cs->t_maxmems));
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"Max Members today: %ld at %s", cs->maxmemtoday,
			sftime(cs->t_maxmemtoday));
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"Total Number of Channel Joins: %ld", cs->totmem);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, 
			"Total Member Joins today: %ld (Max %ld on %s)",
			cs->joinstoday, cs->maxjoins, sftime(cs->t_maxjoins));
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"Total Topic Changes %ld (Today %ld)", cs->topics, cs->topicstoday);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Total Kicks: %ld", cs->kicks);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Total Kicks today %ld (Max %ld on %s)",
			cs->maxkickstoday, cs->maxkicks, sftime(cs->t_maxkicks));
		if (!findchan(cmdparams->av[2]))
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Channel was last seen at %s",
				sftime(cs->lastseen));
	}
	return 1;
}

static int ss_tld_map(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	DisplayTLDmap(cmdparams->source.user);
	return 1;
}

static int ss_netstats(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Network Statistics:-----");
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Users: %ld", stats_network.users);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Users: %ld [%s]",
		stats_network.maxusers, sftime(stats_network.t_maxusers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Total Users Connected: %ld",
		stats_network.totusers);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Channels %ld", stats_network.chans);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Channels %ld [%s]",
		stats_network.maxchans, sftime(stats_network.t_chans));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Opers: %ld", stats_network.opers);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Opers: %ld [%s]",
		stats_network.maxopers, sftime(stats_network.t_maxopers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Users Set Away: %ld", stats_network.away);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Servers: %ld", stats_network.servers);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Servers: %ld [%s]",
		stats_network.maxservers, sftime(stats_network.t_maxservers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "--- End of List ---");
	return 1;
}

static int ss_daily(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Daily Network Statistics:");
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Servers: %-2d %s",
		daily.servers, sftime(daily.t_servers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Users: %-2d %s", daily.users,
		sftime(daily.t_users));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Channel: %-2d %s", daily.chans,
		sftime(daily.t_chans));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum Opers: %-2d %s", daily.opers,
		sftime(daily.t_opers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Total Users Connected: %-2d",
		daily.tot_users);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Daily statistics are reset at midnight");
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of Information.");
	return 1;
}

static void makemap(char *uplink, User * u, int level)
{
#if 0
	hscan_t hs;
	hnode_t *sn;
	Server *s;
	SStats *ss;
	char buf[256];
	int i;

	hash_scan_begin(&hs, sh);
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = findserverstats(s->name);
		if ((level == 0) && (s->uplink[0] == 0)) {
			/* its the root server */
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			prefmsg(u->nick, ss_bot->nick,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping, ss->highest_ping);
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
#endif
}

static int ss_map(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2", "\2[LAG/MAX]\2");
	makemap("", cmdparams->source.user, 0);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "--- End of Listing ---");
	return 1;
}

static int ss_server(CmdParams* cmdparams)
{
	SStats *ss;
	Server *s;
	hscan_t hs;
	hnode_t *sn;
	char *server;

	SET_SEGV_LOCATION();
	server = cmdparams->av[2];
	if (!server) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Server listing:");
		hash_scan_begin(&hs, Shead);
		while ((sn = hash_scan_next(&hs))) {
			ss = hnode_get(sn);
			if (findserver(ss->name)) {
				prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Server: %s (*)", ss->name);
			} else {
				prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Server: %s", ss->name);
			}
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"***** End of list (* indicates server is online) *****");
		return 0;
	}

	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findserverstats(server);
	s = findserver(server);
	if (!ss) {
		nlog(LOG_CRITICAL, "Unable to find server statistics for %s", server);
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,
			"Internal Error! Please Consult the Log file");
		return 0;
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Statistics for \2%s\2 since %s",
		ss->name, sftime(ss->starttime));
	if (!s) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Server Last Seen: %s", 
			sftime(ss->lastseen));
	} else {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Users: %-3ld (%2.0f%%)", 
			(long)ss->users, 
			(float) ss->users / (float) stats_network.users * 100);
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum users: %-3ld at %s",
		ss->maxusers, sftime(ss->t_maxusers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Total users connected: %-3ld", ss->totusers);
	if (s) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current opers: %-3ld", (long)ss->opers);
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Maximum opers: %-3ld at %s",
		(long)ss->maxopers, sftime(ss->t_maxopers));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "IRCop kills: %d", ss->operkills);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Server kills: %d", ss->serverkills);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Lowest ping: %-3d at %s",
		(int)ss->lowest_ping, sftime(ss->t_lowest_ping));
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Higest ping: %-3d at %s",
		(int)ss->highest_ping, sftime(ss->t_highest_ping));
	if (s) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Current Ping: %-3d", s->ping);
	}
	if (ss->numsplits >= 1) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, 
			"%s has split from the network %d time %s",
			ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	} else {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,"%s has never split from the network.", 
			ss->name);
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "***** End of Statistics *****");
	return 1;
}

static int operlistaway = 0;
static char* operlistserver;

static void operlist(User* u)
{
	if (!is_oper(u))
		return;
	if (operlistaway && u->is_away)
		return;
	if (!operlistserver) {
		listindex++;
		prefmsg(listu->nick, ss_bot->nick, "[%2d] %-15s %-15s %-10d", listindex, 
			u->nick, u->server->name, UserLevel(u));
	} else {
		if (ircstrcasecmp(operlistserver, u->server->name))
			return;
		listindex++;
		prefmsg(listu->nick, ss_bot->nick, "[%2d] %-15s %-15s %-10d", listindex, 
			u->nick, u->server->name, UserLevel(u));
	}
}

static int ss_operlist(CmdParams* cmdparams)
{
	char *flags;

	SET_SEGV_LOCATION();
	operlistaway = 0;
	listindex = 0;
	if (cmdparams->ac == 2) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Online IRCops:");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "ID  %-15s %-15s %-10s", 
			"Nick", "Server", "Level");
	}
	flags = cmdparams->av[2];
	operlistserver = cmdparams->av[3];
	if (flags && !ircstrcasecmp(flags, "NOAWAY")) {
		operlistaway = 1;
		flags = NULL;
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Online IRCops (not away):");
	}
	if (!operlistaway && flags && strchr(flags, '.')) {
		operlistserver = flags;
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Online IRCops on server %s", operlistserver);
	}
	listu = cmdparams->source.user;
	GetUserList(operlist);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of Listing.");
	return 1;
}

#ifdef GOTBOTMODE
static void botlist(User* u)
{
	if is_bot(u) { 
		listindex++;
		prefmsg(listu->nick, ss_bot->nick,"[%2d] %-15s %s", listindex, 
			u->nick, u->server->name);
	}
}

static int ss_botlist(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	listindex = 0;
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "On-Line Bots:");
	listu = cmdparams->source.user;
	GetUserList(botlist);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of Listing.");
	return 1;
}
#endif

static int ss_stats(CmdParams* cmdparams)
{
	SStats *st;
	hnode_t *node;
	hscan_t scan;

	SET_SEGV_LOCATION();
	if (!ircstrcasecmp(cmdparams->av[2], "LIST")) {
		int i = 1;
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Statistics Database:");
		hash_scan_begin(&scan, Shead);
		while ((node = hash_scan_next(&scan))) {
			st = hnode_get(node);
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "[%-2d] %s", i, st->name);
			i++;
		}
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List.");
		nlog(LOG_NOTICE, "%s requested STATS LIST.", cmdparams->source.user->nick);
	} else if (!ircstrcasecmp(cmdparams->av[2], "DEL")) {
		if (!cmdparams->av[3]) {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Syntax: /msg %s STATS DEL <name>",
				ss_bot->nick);
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "For additonal help, /msg %s HELP", 
				ss_bot->nick);
			return 0;
		}
		st = findserverstats(cmdparams->av[3]);
		if (!st) {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "%s is not in the database", cmdparams->av[3]);
			return 0;
		}
		if (!findserver(cmdparams->av[3])) {
			node = hash_lookup(Shead, cmdparams->av[3]);
			if (node) {
				hash_delete(Shead, node);
				st = hnode_get(node);
				hnode_destroy(node);
				free(st);
				prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Removed %s from the database.",
					cmdparams->av[3]);
				nlog(LOG_NOTICE, "%s requested STATS DEL %s", cmdparams->source.user->nick, cmdparams->av[3]);
				return 0;
			}
		} else {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, 
				"Cannot remove %s from the database, it is online!!", cmdparams->av[3]);
			nlog(LOG_WARNING,
			     "%s requested STATS DEL %s, but that server is online!!",
			     cmdparams->source.user->nick, cmdparams->av[3]);
				return 0;
		}

	} else if (!ircstrcasecmp(cmdparams->av[2], "COPY")) {
		Server *s;

		if (!cmdparams->av[3] || !cmdparams->av[4]) {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Syntax: /msg %s STATS COPY <name> "
				" <newname>", ss_bot->nick);
			return 0;
		}
		st = findserverstats(cmdparams->av[4]);
		if (st)
			free(st);

		st = findserverstats(cmdparams->av[3]);
		if (!st) {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "%s is not in the database", 
				cmdparams->av[3]);
			return 0;
		}
		s = findserver(cmdparams->av[3]);
		if (s) {
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Server %s is online!", cmdparams->av[3]);
			return 0;
		}
		s = NULL;
		memcpy(st->name, cmdparams->av[4], sizeof(st->name));
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Moved database entry for %s to %s", 
			cmdparams->av[3], cmdparams->av[4]);
		nlog(LOG_NOTICE, "%s requested STATS COPY %s -> %s", cmdparams->source.user->nick, 
			cmdparams->av[3], cmdparams->av[4]);
	} else {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Invalid Argument.");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "For help, /msg %s HELP", ss_bot->nick);
	}
	return 1;
}

static int ss_forcehtml(CmdParams* cmdparams)
{
	nlog(LOG_NOTICE, "%s!%s@%s forced an update of the HTML file.",
		    cmdparams->source.user->nick, cmdparams->source.user->username, cmdparams->source.user->hostname);
	ss_html();
	return 1;
}
