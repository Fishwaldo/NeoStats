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
static int ss_version(CmdParams* cmdparams);
static int ss_about(CmdParams* cmdparams);
static int ss_server(CmdParams* cmdparams);
static int ss_map(CmdParams* cmdparams);
static int ss_netstats(CmdParams* cmdparams);
static int ss_clientversions(CmdParams* cmdparams);
static int ss_forcehtml(CmdParams* cmdparams);
static void ss_Config(void);

ModuleInfo module_info = {
	"statserv",
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
			 ss_botinfo.nick);
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
	User *cmdparams->source.user;
	hnode_t *node;
	hscan_t scan;
#ifdef SQLSRV
	lnode_t *lnode;
#endif
	int count, i;
	Channel *c;
	char **cmdparams->av;
	int cmdparams->ac = 0;
	char *chan;
	lnode_t *chanmem;

	SET_SEGV_LOCATION();

	ss_module = mod_ptr;
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
		cmdparams->ac = 0;
		AddStringToList(&cmdparams->av, ss->name, &cmdparams->ac);
		ss_event_server(cmdparams->av, cmdparams->ac);
		free(cmdparams->av);
		cmdparams->ac = 0;
		nlog(LOG_DEBUG2, "Added Server %s to StatServ List", ss->name);
	}
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		cmdparams->source.user = hnode_get(node);
		cmdparams->ac = 0;
		AddStringToList(&cmdparams->av, cmdparams->source.user->nick, &cmdparams->ac);
		ss_event_signon(cmdparams->av, cmdparams->ac);
		AddStringToList(&cmdparams->av, cmdparams->source.user->modes, &cmdparams->ac);
		ss_event_mode(cmdparams->av, cmdparams->ac);
		free(cmdparams->av);
		cmdparams->ac = 0;
		nlog(LOG_DEBUG2, "Add user %s to StatServ List", cmdparams->source.user->nick);
	}
	hash_scan_begin(&scan, ch);
	while ((node = hash_scan_next(&scan)) != NULL) {
		c = hnode_get(node);
		count = list_count(c->chanmembers);
		chanmem = list_first(c->chanmembers);
		chan = lnode_get(chanmem);
		cmdparams->ac = 0;
		AddStringToList(&cmdparams->av, c->name, &cmdparams->ac);
		ss_event_newchan(cmdparams->av, cmdparams->ac);
		free(cmdparams->av);
		cmdparams->ac = 0;
		for (i = 1; i <= count; i++) {
			nlog(LOG_DEBUG2, "Chanjoin %s", c->name);
			cmdparams->ac = 0;
			AddStringToList(&cmdparams->av, c->name, &cmdparams->ac);
			AddStringToList(&cmdparams->av, chan, &cmdparams->ac);
			ss_event_join(cmdparams->av, cmdparams->ac);
			free(cmdparams->av);
			cmdparams->ac = 0;
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
	{"NICK",		&ss_botinfo.nick,		SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, "Nick",		NULL,		ns_help_set_nick },
	{"USER",		&ss_botinfo.user,		SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, "User",		NULL,		ns_help_set_user },
	{"HOST",		&ss_botinfo.host,		SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, "Host",		NULL,		ns_help_set_host },
	{"REALNAME",	&ss_botinfo.realname,	SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, "RealName",	NULL,		ns_help_set_realname },
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
	CVersions *cv;
	lnode_t *cn;
	int i;
	int num;

	chanalert(ss_bot->nick, "%s Wanted to see the Client Version List", cmdparams->source.user->nick);
	num = cmdparams->ac > 2 ? atoi(cmdparams->av[2]) : 10;
	if (num < 10) {
		num = 10;
	}
	if (list_count(Vhead) == 0) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "No Stats Available.");
		return 0;
	}
	if (!list_is_sorted(Vhead, topversions)) {
		list_sort(Vhead, topversions);
	}
	cn = list_first(Vhead);
	cv = lnode_get(cn);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Top%d Client Versions:", num);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "======================");
	for (i = 0; i <= num; i++) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "%d) %d ->  %s", i, cv->count, cv->name);
		cn = list_next(Vhead, cn);
		if (cn) {
			cv = lnode_get(cn);
		} else {
			break;
		}
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List.");
	return 1;
}

static int ss_chans(CmdParams* cmdparams)
{
	CStats *cs;
	lnode_t *cn;
	int i;

	chanalert(ss_bot->nick, "%s Wanted to see Channel Statistics", cmdparams->source.user->nick);
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
	chanalert(ss_bot->nick, "%s Wanted to see a Country Breakdown", cmdparams->source.user->nick);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Top Level Domain Statistics:");
	DisplayTLDmap(cmdparams->source.user);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of List");
	return 1;
}

static int ss_version(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "\2%s Version Information\2", ss_bot->nick);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "%s Version: %s Compiled %s at %s", ss_bot->nick,
		module_info.version, module_info.build_date, module_info.build_time);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "http://www.neostats.net");
	return 1;
}

static int ss_about(CmdParams* cmdparams)
{
	privmsg_list(cmdparams->source.user->nick, ss_bot->nick, ss_help_about);
	return 1;
}

static int ss_netstats(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see the NetStats ", cmdparams->source.user->nick);
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
	chanalert(ss_bot->nick, "%s Wanted to see the Daily NetStats ", cmdparams->source.user->nick);
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

static void makemap(char *uplink, User * cmdparams->source.user, int level)
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
				makemap(s->name, cmdparams->source.user, level);
			}
			prefmsg(cmdparams->source.user->nick, ss_bot->nick,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping, ss->highest_ping);
			makemap(s->name, cmdparams->source.user, level + 1);
		} else if ((level > 0) && !ircstrcasecmp(uplink, s->uplink)) {
			if (StatServ.exclusions && IsExcluded(s)) {
				makemap(s->name, cmdparams->source.user, level);
			}
			/* its not the root server */
			buf[0]='\0';
			for (i = 1; i < level; i++) {
				strlcat (buf, "     |", 256);
			}
			prefmsg(cmdparams->source.user->nick, ss_bot->nick,
				"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				buf, ss->name, ss->users, (int)ss->maxusers,
				ss->opers, ss->maxopers, (long)s->ping, ss->highest_ping);
			makemap(s->name, cmdparams->source.user, level + 1);
		}
	}
}

static int ss_map(CmdParams* cmdparams)
{
	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see the Current Network MAP", cmdparams->source.user->nick);
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
	server =  cmdparams->av[2];
	chanalert(ss_bot->nick, "%s requested server information on %s", 
		cmdparams->source.user->nick, cmdparams->av[2]);
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
	ss = findstats(server);
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
			"%s has split from the network %d time%s",
			ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	} else {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick,"%s has never split from the network.", 
			ss->name);
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "***** End of Statistics *****");
	return 1;
}

static int ss_operlist(CmdParams* cmdparams)
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

	if (cmdparams->ac == 2) {
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Online IRCops:");
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "ID  %-15s %-15s %-10s", 
			"Nick", "Server", "Level");
		chanalert(ss_bot->nick, "%s requested operlist", cmdparams->source.user->nick);
	}

	flags = cmdparams->av[2];
	server = cmdparams->av[3];
	if (flags && !ircstrcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Online IRCops (not away):");
		chanalert(ss_bot->nick, "%s requested operlist (not away)", cmdparams->source.user->nick);
	}
	if (!away && flags && strchr(flags, '.')) {
		server = flags;
		prefmsg(cmdparams->source.user->nick, ss_bot->nick, "Online IRCops on server %s", server);
		chanalert(ss_bot->nick, "%s requested operlist on server %s",
			  cmdparams->source.user->nick, server);
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
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "[%2d] %-15s %-15s %-10d", j, 
				testuser->nick, testuser->server->name, ulevel);
			continue;
		} else {
			if (ircstrcasecmp(server, testuser->server->name))
				continue;
			j++;
			prefmsg(cmdparams->source.user->nick, ss_bot->nick, "[%2d] %-15s %-15s %-10d", j, 
				testuser->nick, testuser->server->name, ulevel);
			continue;
		}
	}
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "End of Listing.");
	return 1;
}

#ifdef GOTBOTMODE
static int ss_botlist(CmdParams* cmdparams)
{
	register int j = 0;
	register User *testuser;
	hscan_t scan;
	hnode_t *node;

	SET_SEGV_LOCATION();
	chanalert(ss_bot->nick, "%s Wanted to see the Bot List", cmdparams->source.user->nick);
	prefmsg(cmdparams->source.user->nick, ss_bot->nick, "On-Line Bots:");
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		testuser = hnode_get(node);
		if is_bot(testuser) { 
			j++;
			prefmsg(cmdparams->source.user->nick, ss_bot->nick,"[%2d] %-15s %s", j, 
				testuser->nick, testuser->server->name);
		}
	}
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
		st = findstats(cmdparams->av[3]);
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
		st = findstats(cmdparams->av[4]);
		if (st)
			free(st);

		st = findstats(cmdparams->av[3]);
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
	chanalert(ss_bot->nick, "%s forced an update of the HTML file.",
			cmdparams->source.user->nick);
	ss_html();
	return 1;
}
