/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond, Mark Hetherington
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
#include "statsrta.h"

static int ss_chans(CmdParams* cmdparams);
static int ss_daily(CmdParams* cmdparams);
static int ss_stats(CmdParams* cmdparams);
static int ss_tld_map(CmdParams* cmdparams);
static int ss_operlist(CmdParams* cmdparams);
static int ss_botlist(CmdParams* cmdparams);
static int ss_server(CmdParams* cmdparams);
static int ss_map(CmdParams* cmdparams);
static int ss_netstats(CmdParams* cmdparams);
static int ss_clientversions(CmdParams* cmdparams);
static int ss_forcehtml(CmdParams* cmdparams);

static int ss_event_ctcpversion(CmdParams* cmdparams);
static int ss_event_pong(CmdParams* cmdparams);
static int ss_event_away(CmdParams* cmdparams);
static int ss_event_server(CmdParams* cmdparams);
static int ss_event_squit(CmdParams* cmdparams);
static int ss_event_nickip(CmdParams* cmdparams);
static int ss_event_signon(CmdParams* cmdparams);
static int ss_event_quit(CmdParams* cmdparams);
static int ss_event_mode(CmdParams* cmdparams);
static int ss_event_globalkill(CmdParams* cmdparams);
static int ss_event_serverkill(CmdParams* cmdparams);
static int ss_event_newchan(CmdParams* cmdparams);
static int ss_event_delchan(CmdParams* cmdparams);
static int ss_event_join(CmdParams* cmdparams);
static int ss_event_part(CmdParams* cmdparams);
static int ss_event_topic(CmdParams* cmdparams);
static int ss_event_kick(CmdParams* cmdparams);

static Client * listu;
static int listindex = 0;
Daily daily;

/** Bot pointer */
Bot *ss_bot;

/** Module pointer */
Module* ss_module;

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_PONG,		ss_event_pong,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SERVER,		ss_event_server,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SQUIT,		ss_event_squit,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SIGNON,		ss_event_signon,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_GOTNICKIP,	ss_event_nickip,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_UMODE,		ss_event_mode,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_QUIT,		ss_event_quit,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_AWAY,		ss_event_away,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_GLOBALKILL,	ss_event_globalkill,EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SERVERKILL,	ss_event_serverkill,EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NEWCHAN,		ss_event_newchan,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_DELCHAN,		ss_event_delchan,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_JOIN,		ss_event_join,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_PART,		ss_event_part,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_KICK,		ss_event_kick,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_TOPIC,		ss_event_topic,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_CTCPVERSIONRPL,	ss_event_ctcpversion,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NULL,		NULL}
};

/** Copyright info */
static const char *ns_copyright[] = {
	"Copyright (c) 1999-2004, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
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

/** Bot comand table */
static bot_cmd ss_commands[]=
{
	{"SERVER",			ss_server,		0, 	0,		ss_help_server,		 	ss_help_server_oneline},
	{"MAP",				ss_map,			0, 	0,		ss_help_map, 		 	ss_help_map_oneline},
	{"CHAN",			ss_chans,		0, 	0,		ss_help_chan, 		 	ss_help_chan_oneline},
	{"NETSTATS",		ss_netstats,	0, 	0,		ss_help_netstats, 	 	ss_help_netstats_oneline},
	{"DAILY",			ss_daily,		0, 	0,		ss_help_daily, 		 	ss_help_daily_oneline},
	{"TLDMAP",			ss_tld_map,		0, 	0,		ss_help_tldmap, 	 	ss_help_tldmap_oneline},
	{"OPERLIST",		ss_operlist,	0, 	0,		ss_help_operlist, 	 	ss_help_operlist_oneline},
	{"BOTLIST",			ss_botlist,		0, 	0,		ss_help_botlist, 	 	ss_help_botlist_oneline},
	{"CLIENTVERSIONS",	ss_clientversions,	0,	0,		ss_help_clientversions, ss_help_clientversions_oneline},
	{"FORCEHTML",		ss_forcehtml,	0, 	NS_ULEVEL_ADMIN,	ss_help_forcehtml, 		ss_help_forcehtml_oneline},
	{"STATS",			ss_stats,		1, 	NS_ULEVEL_ADMIN,	ss_help_stats, 			ss_help_stats_oneline},
	{NULL,				NULL,			0, 	0,					NULL, 					NULL}
};

/** Bot setting table */
static bot_setting ss_settings[]=
{
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

/** BotInfo */
static BotInfo ss_botinfo = 
{
	"StatServ", 
	"StatServ1", 
	"SS", 
	BOT_COMMON_HOST, 
	"Statistics service", 
	BOT_FLAG_SERVICEBOT|BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
	ss_commands, 
	ss_settings,
};

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param pointer to my module
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit (Module *mod_ptr)
{
	SET_SEGV_LOCATION();
	ss_module = mod_ptr;
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

#ifdef RTA_SUPPORT
	statserv_rta_init ();
#endif

#ifdef USE_BERKELEY
	DBOpenDatabase();
#endif
	load_client_versions();
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch (void)
{
	SET_SEGV_LOCATION();
	ss_bot = init_bot (&ss_botinfo);
	if (!ss_bot) {
		return NS_FAILURE;
	}
	/* now that we are online, setup the timer to save the Stats database every so often */
	add_timer (TIMER_TYPE_INTERVAL, SaveStats, "SaveStats", DBSAVETIME);
	add_timer (TIMER_TYPE_INTERVAL, ss_html, "ss_html", 3600);
	/* also add a timer to check if its midnight (to reset the daily stats */
	add_timer (TIMER_TYPE_MIDNIGHT, StatsMidnight, "StatsMidnight", 60);
	add_timer (TIMER_TYPE_INTERVAL, DelOldChan, "DelOldChan", 3600);
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return none
 */

void ModFini (void)
{
	StatServ.shutdown = 1;
	irc_chanalert (ss_bot, "Saving StatServ Database. this *could* take a while");
	SaveStats ();
	irc_chanalert (ss_bot, "Done");
	FiniStats ();
	FiniTLD ();
	save_client_versions ();

#ifdef RTA_SUPPORT
	statserv_rta_fini();
#endif      

#ifdef USE_BERKELEY
	DBCloseDatabase();
#endif      
}

static int ss_event_server(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	StatsAddServer(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_squit(CmdParams* cmdparams)
{
	StatsDelServer(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_newchan(CmdParams* cmdparams)
{
	StatsAddChan(cmdparams->channel);
	return NS_SUCCESS;
}

static int ss_event_delchan(CmdParams* cmdparams)
{
	StatsDelChan(cmdparams->channel);
	return NS_SUCCESS;
}

static int ss_event_join(CmdParams* cmdparams)
{										   
	if (StatServ.exclusions && (IsExcluded(cmdparams->channel) || IsExcluded(cmdparams->source))) {
		return NS_SUCCESS;
	}
	StatsJoinChan(cmdparams->source, cmdparams->channel);
	return NS_SUCCESS;
}

static int ss_event_part(CmdParams* cmdparams)
{
	if (StatServ.exclusions && (IsExcluded(cmdparams->channel) || IsExcluded(cmdparams->source))) {
		return NS_SUCCESS;
	}
	StatsPartChan(cmdparams->source, cmdparams->channel);
	return NS_SUCCESS;
}

static int ss_event_topic(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return NS_SUCCESS;
	}
	StatsChanTopic(cmdparams->channel);
	return NS_SUCCESS;
}

static int ss_event_kick(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->channel)) {
		return NS_SUCCESS;
	}
	StatsChanKick(cmdparams->channel);
	return NS_SUCCESS;
}

int ss_event_ctcpversion(CmdParams* cmdparams)
{
	StatsAddCTCPVersion(cmdparams->param);
	return NS_SUCCESS;
}

static int ss_event_globalkill(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source)) {
		return NS_SUCCESS;
	}
	StatsGlobalKill(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_serverkill(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source)) {
		return NS_SUCCESS;
	}
	StatsServerKill(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_mode(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source)) {
		return NS_SUCCESS;
	}
	StatsUserMode(cmdparams->source, cmdparams->param);
	return NS_SUCCESS;
}

static int ss_event_quit(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source)) {
		return NS_SUCCESS;
	}
	StatsQuitUser(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_away(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source)) {
		return NS_SUCCESS;
	}
	StatsUserAway(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_nickip(CmdParams* cmdparams)
{
	AddTLD(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_signon(CmdParams* cmdparams)
{
	if (StatServ.exclusions && IsExcluded(cmdparams->source)) {
		return NS_SUCCESS;
	}
	StatsAddUser(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_event_pong(CmdParams* cmdparams)
{
	StatsServerPong(cmdparams->source);
	return NS_SUCCESS;
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

	num = cmdparams->ac > 0 ? atoi(cmdparams->av[0]) : 10;
	if (num < 10) {
		num = 10;
	}
	list_client_versions(cmdparams->source, num);
	return NS_SUCCESS;
}

static int ss_chans(CmdParams* cmdparams)
{
	CStats *cs;
	lnode_t *cn;
	int i;

	if (cmdparams->ac == 0) {
		/* they want the top10 Channels online atm */
		if (!list_is_sorted(Chead, topchan)) {
			list_sort(Chead, topchan);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		irc_prefmsg(ss_bot, cmdparams->source, "Top10 Online Channels:");
		irc_prefmsg(ss_bot, cmdparams->source, "======================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(find_chan(cs->name))
			    && (UserLevel(cmdparams->source) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			irc_prefmsg(ss_bot,cmdparams->source, 
				"Channel %s -> %ld Members", cs->name,
				cs->members);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else if (!ircstrcasecmp(cmdparams->av[0], "POP")) {
		/* they want the top10 Popular Channels (based on joins) */
		if (!list_is_sorted(Chead, topjoin)) {
			list_sort(Chead, topjoin);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		irc_prefmsg(ss_bot, cmdparams->source, "Top10 Channels (Ever):");
		irc_prefmsg(ss_bot, cmdparams->source, "======================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(find_chan(cs->name))
			    && (UserLevel(cmdparams->source) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			irc_prefmsg(ss_bot, cmdparams->source, "Channel %s -> %ld Joins", 
				cs->name, cs->totmem);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else if (!ircstrcasecmp(cmdparams->av[0], "KICKS")) {
		/* they want the top10 most unwelcome channels (based on kicks) */
		if (!list_is_sorted(Chead, topkick)) {
			list_sort(Chead, topkick);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		irc_prefmsg(ss_bot,cmdparams->source, 
			"Top10 Most un-welcome Channels (Ever):");
		irc_prefmsg(ss_bot,cmdparams->source, 
			"======================================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(find_chan(cs->name))
			    && (UserLevel(cmdparams->source) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			irc_prefmsg(ss_bot, cmdparams->source, "Channel %s -> %ld Kicks", 
				cs->name, cs->kicks);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else if (!ircstrcasecmp(cmdparams->av[0], "TOPICS")) {
		/* they want the top10 most undecisive channels (based on topics) */
		if (!list_is_sorted(Chead, toptopics)) {
			list_sort(Chead, toptopics);
		}
		cn = list_first(Chead);
		cs = lnode_get(cn);
		irc_prefmsg(ss_bot, cmdparams->source, "Top10 Most undecisive Channels (Ever):");
		irc_prefmsg(ss_bot, cmdparams->source, "======================================");
		for (i = 0; i <= 10; i++) {
			/* only show hidden chans to operators */
			if (is_hidden_chan(find_chan(cs->name))
			    && (UserLevel(cmdparams->source) < NS_ULEVEL_OPER)) {
				i--;
				cn = list_next(Chead, cn);
				if (cn) {
					cs = lnode_get(cn);
				} else {
					break;
				}
				continue;
			}
			irc_prefmsg(ss_bot, cmdparams->source, "Channel %s -> %ld Topics", 
				cs->name, cs->topics);
			cn = list_next(Chead, cn);
			if (cn) {
				cs = lnode_get(cn);
			} else {
				break;
			}
		}
		irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	} else {
		cs = findchanstats(cmdparams->av[0]);
		if (!cs) {
			irc_prefmsg(ss_bot,cmdparams->source, 
				"Error, Can't find any information about Channel %s", cmdparams->av[0]);
			return NS_SUCCESS;
		}
		irc_prefmsg(ss_bot, cmdparams->source, "\2Channel Information for %s (%s)\2", 
			cmdparams->av[0], (find_chan(cmdparams->av[0]) ? "Online" : "Offline"));
		irc_prefmsg(ss_bot, cmdparams->source, "Current Members: %ld (Max %ld on %s)",
			cs->members, cs->maxmems, sftime(cs->t_maxmems));
		irc_prefmsg(ss_bot,cmdparams->source, 
			"Max Members today: %ld at %s", cs->maxmemtoday,
			sftime(cs->t_maxmemtoday));
		irc_prefmsg(ss_bot,cmdparams->source, 
			"Total Number of Channel Joins: %ld", cs->totmem);
		irc_prefmsg(ss_bot, cmdparams->source, 
			"Total Member Joins today: %ld (Max %ld on %s)",
			cs->joinstoday, cs->maxjoins, sftime(cs->t_maxjoins));
		irc_prefmsg(ss_bot,cmdparams->source, 
			"Total Topic Changes %ld (Today %ld)", cs->topics, cs->topicstoday);
		irc_prefmsg(ss_bot, cmdparams->source, "Total Kicks: %ld", cs->kicks);
		irc_prefmsg(ss_bot, cmdparams->source, "Total Kicks today %ld (Max %ld on %s)",
			cs->maxkickstoday, cs->maxkicks, sftime(cs->t_maxkicks));
		if (!find_chan(cmdparams->av[0]))
			irc_prefmsg(ss_bot, cmdparams->source, "Channel was last seen at %s",
				sftime(cs->t_lastseen));
	}
	return NS_SUCCESS;
}

static int ss_tld_map(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	DisplayTLDmap(cmdparams->source);
	return NS_SUCCESS;
}

static int ss_netstats(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg(ss_bot, cmdparams->source, "Network Statistics:-----");
	irc_prefmsg(ss_bot, cmdparams->source, "Current Users: %ld", stats_network.users);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Users: %ld [%s]",
		stats_network.maxusers, sftime(stats_network.t_maxusers));
	irc_prefmsg(ss_bot, cmdparams->source, "Total Users Connected: %ld",
		stats_network.totusers);
	irc_prefmsg(ss_bot, cmdparams->source, "Current Channels %ld", stats_network.chans);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Channels %ld [%s]",
		stats_network.maxchans, sftime(stats_network.t_chans));
	irc_prefmsg(ss_bot, cmdparams->source, "Current Opers: %ld", stats_network.opers);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Opers: %ld [%s]",
		stats_network.maxopers, sftime(stats_network.t_maxopers));
	irc_prefmsg(ss_bot, cmdparams->source, "Users Set Away: %ld", stats_network.away);
	irc_prefmsg(ss_bot, cmdparams->source, "Current Servers: %ld", stats_network.servers);
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Servers: %ld [%s]",
		stats_network.maxservers, sftime(stats_network.t_maxservers));
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static int ss_daily(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg(ss_bot, cmdparams->source, "Daily Network Statistics:");
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Servers: %-2d %s",
		daily.servers, sftime(daily.t_servers));
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Users: %-2d %s", daily.users,
		sftime(daily.t_users));
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Channel: %-2d %s", daily.chans,
		sftime(daily.t_chans));
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum Opers: %-2d %s", daily.opers,
		sftime(daily.t_opers));
	irc_prefmsg(ss_bot, cmdparams->source, "Total Users Connected: %-2d",
		daily.tot_users);
	irc_prefmsg(ss_bot, cmdparams->source, "Daily statistics are reset at midnight");
	irc_prefmsg(ss_bot, cmdparams->source, "End of Information.");
	return NS_SUCCESS;
}

static void makemap(char *uplink, Client * u, int level)
{
	hscan_t hs;
	hnode_t *sn;
	Client *s;
	SStats *ss;
	char buf[256];
	int i;

	hash_scan_begin(&hs, GetServerHash ());
	while ((sn = hash_scan_next(&hs))) {
		s = hnode_get(sn);
		ss = findserverstats(s->name);
		if ((level == 0) && (s->uplinkname[0] == 0)) {
			/* its the root server */
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			irc_prefmsg (ss_bot, u,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->server->ping, ss->highest_ping);
			makemap(s->name, u, level + 1);
		} else if ((level > 0) && !ircstrcasecmp(uplink, s->uplinkname)) {
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, u, level);
			}
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				strlcat (buf, "     |", 256);
			}
			irc_prefmsg (ss_bot, u,
				"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				buf, ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->server->ping, ss->highest_ping);
			makemap(s->name, u, level + 1);
		}
	}
}

static int ss_map(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	irc_prefmsg(ss_bot, cmdparams->source, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2", "\2[LAG/MAX]\2");
	makemap("", cmdparams->source, 0);
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static int ss_server(CmdParams* cmdparams)
{
	SStats *ss;
	Client *s;
	hscan_t hs;
	hnode_t *sn;
	char *server;

	SET_SEGV_LOCATION();
	if (cmdparams->ac ==0) {
		irc_prefmsg(ss_bot, cmdparams->source, "Server listing:");
		hash_scan_begin(&hs, Shead);
		while ((sn = hash_scan_next(&hs))) {
			ss = hnode_get(sn);
			if (find_server(ss->name)) {
				irc_prefmsg(ss_bot, cmdparams->source, "%s (*)", ss->name);
			} else {
				irc_prefmsg(ss_bot, cmdparams->source, "%s", ss->name);
			}
		}
		irc_prefmsg(ss_bot,cmdparams->source, "(* indicates server is online)");
		irc_prefmsg(ss_bot,cmdparams->source, "End of list.");

		return NS_SUCCESS;
	}

	server = cmdparams->av[0];
	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findserverstats(server);
	if (!ss) {
		nlog(LOG_CRITICAL, "Unable to find server statistics for %s", server);
		irc_prefmsg(ss_bot,cmdparams->source, 
			"Internal Error! Please Consult the Log file");
		return NS_SUCCESS;
	}
	irc_prefmsg(ss_bot, cmdparams->source, "Statistics for \2%s\2 since %s",
		ss->name, sftime(ss->t_start));
	s = find_server(server);
	if (!s) {
		irc_prefmsg(ss_bot, cmdparams->source, "Server Last Seen: %s", 
			sftime(ss->t_lastseen));
	} else {
		irc_prefmsg(ss_bot, cmdparams->source, "Current Users: %-3ld (%2.0f%%)", 
			(long)ss->users, 
			(float) ss->users / (float) stats_network.users * 100);
	}
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum users: %-3ld at %s",
		ss->maxusers, sftime(ss->t_maxusers));
	irc_prefmsg(ss_bot, cmdparams->source, "Total users connected: %-3ld", ss->totusers);
	if (s) {
		irc_prefmsg(ss_bot, cmdparams->source, "Current opers: %-3ld", (long)ss->opers);
	}
	irc_prefmsg(ss_bot, cmdparams->source, "Maximum opers: %-3ld at %s",
		(long)ss->maxopers, sftime(ss->t_maxopers));
	irc_prefmsg(ss_bot, cmdparams->source, "IRCop kills: %d", ss->operkills);
	irc_prefmsg(ss_bot, cmdparams->source, "Server kills: %d", ss->serverkills);
	irc_prefmsg(ss_bot, cmdparams->source, "Lowest ping: %-3d at %s",
		(int)ss->lowest_ping, sftime(ss->t_lowest_ping));
	irc_prefmsg(ss_bot, cmdparams->source, "Higest ping: %-3d at %s",
		(int)ss->highest_ping, sftime(ss->t_highest_ping));
	if (s) {
		irc_prefmsg(ss_bot, cmdparams->source, "Current Ping: %-3d", s->server->ping);
	}
	if (ss->numsplits >= 1) {
		irc_prefmsg(ss_bot, cmdparams->source, 
			"%s has split from the network %d time %s",
			ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	} else {
		irc_prefmsg(ss_bot, cmdparams->source, "%s has never split from the network.", 
			ss->name);
	}
	irc_prefmsg(ss_bot, cmdparams->source, "***** End of Statistics *****");
	return NS_SUCCESS;
}

static int operlistaway = 0;
static char* operlistserver;

static void operlist(Client * u)
{
	if (!is_oper(u))
		return;
	if (operlistaway && u->user->is_away)
		return;
	if (!operlistserver) {
		listindex++;
		irc_prefmsg(ss_bot, listu, "[%2d] %-15s %-15s %-10d", listindex, 
			u->name, u->uplink->name, UserLevel(u));
	} else {
		if (ircstrcasecmp(operlistserver, u->uplink->name))
			return;
		listindex++;
		irc_prefmsg(ss_bot, listu, "[%2d] %-15s %-15s %-10d", listindex, 
			u->name, u->uplink->name, UserLevel(u));
	}
}

static int ss_operlist(CmdParams* cmdparams)
{
	char *flags = NULL;

	SET_SEGV_LOCATION();
	operlistaway = 0;
	listindex = 0;
	operlistserver = NULL;
	if (cmdparams->ac == 0) {
		irc_prefmsg(ss_bot, cmdparams->source, "Online IRCops:");
		irc_prefmsg(ss_bot, cmdparams->source, "ID  %-15s %-15s %-10s", 
			"Nick", "Server", "Level");
	}
	if (cmdparams->ac != 0) {
		flags = cmdparams->av[0];
		operlistserver = cmdparams->av[1];
	}
	if (flags && !ircstrcasecmp(flags, "NOAWAY")) {
		operlistaway = 1;
		flags = NULL;
		irc_prefmsg(ss_bot, cmdparams->source, "Online IRCops (not away):");
	}
	if (!operlistaway && flags && strchr(flags, '.')) {
		operlistserver = flags;
		irc_prefmsg(ss_bot, cmdparams->source, "Online IRCops on server %s", operlistserver);
	}
	listu = cmdparams->source;
	GetUserList(operlist);
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static void botlist(Client * u)
{
	if is_bot(u) { 
		listindex++;
		irc_prefmsg(ss_bot, listu, "[%2d] %-15s %s", listindex, 
			u->name, u->uplink->name);
	}
}

static int ss_botlist(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	listindex = 0;
	irc_prefmsg(ss_bot, cmdparams->source, "Online bots:");
	listu = cmdparams->source;
	GetUserList(botlist);
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	return NS_SUCCESS;
}

static int ss_stats_list (CmdParams* cmdparams)
{
	SStats *st;
	hnode_t *node;
	hscan_t scan;

	irc_prefmsg(ss_bot, cmdparams->source, "Statistics Database:");
	hash_scan_begin(&scan, Shead);
	while ((node = hash_scan_next(&scan))) {
		st = hnode_get(node);
		irc_prefmsg(ss_bot, cmdparams->source, "  %s", st->name);
	}
	irc_prefmsg(ss_bot, cmdparams->source, "End of list.");
	nlog(LOG_NOTICE, "%s requested STATS LIST.", cmdparams->source->name);
	return NS_SUCCESS;
}

static int ss_stats_del (CmdParams* cmdparams)
{
	SStats *st;
	hnode_t *node;

	st = findserverstats(cmdparams->av[1]);
	if (!st) {
		irc_prefmsg(ss_bot, cmdparams->source, "%s is not in the database", cmdparams->av[1]);
		return NS_SUCCESS;
	}
	if (!find_server(cmdparams->av[1])) {
		node = hash_lookup(Shead, cmdparams->av[1]);
		if (node) {
			ns_free (hnode_get(node));
			hash_delete(Shead, node);
			hnode_destroy(node);
			irc_prefmsg(ss_bot, cmdparams->source, "Removed %s from the database.",
				cmdparams->av[1]);
			nlog(LOG_NOTICE, "%s requested STATS DEL %s", cmdparams->source->name, cmdparams->av[1]);
			return NS_SUCCESS;
		}
	} else {
		irc_prefmsg(ss_bot, cmdparams->source, 
			"Cannot remove %s from the database, it is online!!", cmdparams->av[1]);
		nlog(LOG_WARNING,
			    "%s requested STATS DEL %s, but that server is online!!",
			    cmdparams->source->name, cmdparams->av[1]);
			return NS_SUCCESS;
	}
	return NS_SUCCESS;
}

static int ss_stats_copy (CmdParams* cmdparams)
{
	SStats *st;
	Client *s;

	if (!cmdparams->av[1] || !cmdparams->av[2]) {
		irc_prefmsg(ss_bot, cmdparams->source, "Syntax: /msg %s STATS COPY <name> "
			" <newname>", ss_bot->name);
		return NS_SUCCESS;
	}
	st = findserverstats(cmdparams->av[2]);
	if (st)
		ns_free(st);

	st = findserverstats(cmdparams->av[1]);
	if (!st) {
		irc_prefmsg(ss_bot, cmdparams->source, "%s is not in the database", 
			cmdparams->av[1]);
		return NS_SUCCESS;
	}
	s = find_server(cmdparams->av[1]);
	if (s) {
		irc_prefmsg(ss_bot, cmdparams->source, "Server %s is online!", cmdparams->av[1]);
		return NS_SUCCESS;
	}
	s = NULL;
	memcpy(st->name, cmdparams->av[2], sizeof(st->name));
	irc_prefmsg(ss_bot, cmdparams->source, "Moved database entry for %s to %s", 
		cmdparams->av[1], cmdparams->av[1]);
	nlog(LOG_NOTICE, "%s requested STATS COPY %s -> %s", cmdparams->source->name, 
		cmdparams->av[1], cmdparams->av[2]);
	return NS_SUCCESS;
}

static int ss_stats (CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	if (!ircstrcasecmp(cmdparams->av[0], "LIST")) {
		return ss_stats_list (cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "DEL")) {
		if (cmdparams->ac < 2) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		return ss_stats_del (cmdparams);
	} else if (!ircstrcasecmp(cmdparams->av[0], "COPY")) {
		if (cmdparams->ac < 3) {
			return NS_ERR_NEED_MORE_PARAMS;
		}
		return ss_stats_copy (cmdparams);	
	}
	return NS_ERR_SYNTAX_ERROR;
}

static int ss_forcehtml(CmdParams* cmdparams)
{
	nlog(LOG_NOTICE, "%s!%s@%s forced an update of the HTML file.",
		    cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname);
	ss_html();
	return NS_SUCCESS;
}
