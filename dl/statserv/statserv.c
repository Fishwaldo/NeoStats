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
** $Id: statserv.c,v 1.68 2003/07/15 10:53:19 fishwaldo Exp $
*/

#include <stdio.h>
#include <fnmatch.h>
#include "dl.h"
#include "stats.h"
#include "statserv.h"
#include "log.h"
#include "conf.h"


extern const char version_date[], version_time[];
static void ss_chans(User * u, char *chan);
static void ss_daily(User * u);
static void ss_stats(User * u, char *cmd, char *arg, char *arg2);
static void ss_tld(User * u, char *tld);
static void ss_tld_map(User * u);
static void ss_operlist(User * origuser, char *flags, char *server);
static void ss_botlist(User * origuser);
static void ss_version(User * u);
static void ss_server(User * u, char *server);
static void ss_map(User *);
static void ss_netstats(User *);
static void ss_set(User *, char **, int);
static void ss_versions(User * u, int num);
/* int s_bot_kill(char *); */
static void ss_Config();
static int new_m_version(char *origin, char **av, int ac);
void ss_html();

Module_Info Statserv_Info[] = { {
				 SSMNAME,
				 "Statistical Bot For NeoStats",
				 "3.9"}
};


Functions StatServ_fn_list[] = {
	{MSG_VERSION, new_m_version, 1},
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1},
#endif
	{NULL, NULL, 0}
};


EventFnList StatServ_Event_List[] = {
	{"ONLINE", Online},
	{"PONG", pong},
	{"NEWSERVER", s_new_server},
	{"SQUIT", s_del_server},
	{"SIGNON", s_new_user},
	{"UMODE", s_user_modes},
	{"SIGNOFF", s_del_user},
	{"AWAY", s_user_away},
	{"KILL", s_user_kill},
	{"NEWCHAN", s_chan_new},
	{"DELCHAN", s_chan_del},
	{"JOINCHAN", s_chan_join},
	{"PARTCHAN", s_chan_part},
	{"KICK", s_chan_kick},
	{"TOPICCHANGE", s_topic_change},
	{"CLIENTVERSION", s_client_version},
	{NULL, NULL}
};



Module_Info *__module_get_info()
{
	return Statserv_Info;
};

Functions *__module_get_functions()
{
	return StatServ_fn_list;
};

EventFnList *__module_get_events()
{
	return StatServ_Event_List;
};

void ss_Config()
{
	char *tmp;

	strcpy(segv_location, "StatServ-ss_Config");

	if (GetConf((void *) &s_StatServ, CFGSTR, "Nick") < 0) {
		s_StatServ = malloc(MAXNICK);
		snprintf(s_StatServ, MAXNICK, "StatServ");
	}
	if (GetConf((void *) &tmp, CFGSTR, "User") < 0) {
		snprintf(StatServ.user, MAXUSER, "SS");
	} else {
		snprintf(StatServ.user, MAXUSER, "%s", tmp);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Host") < 0) {
		snprintf(StatServ.host, MAXHOST, me.name);
	} else {
		snprintf(StatServ.host, MAXHOST, "%s", tmp);
		free(tmp);
	}
	if (GetConf((void *) &tmp, CFGSTR, "Rname") < 0) {
		snprintf(StatServ.rname, MAXHOST, "/msg %s help",
			 s_StatServ);
	} else {
		snprintf(StatServ.rname, MAXHOST, "%s", tmp);
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
		GetConf((void *) StatServ.html, CFGINT, "HTML_Enabled");
		snprintf(StatServ.htmlpath, 255, "%s", tmp);
		free(tmp);
	}
	if (GetConf((void *) &StatServ.interval, CFGINT, "Wallop_Throttle")
	    < 0) {
		StatServ.interval = 0;
	}
}




int new_m_version(char *origin, char **av, int ac)
{
	strcpy(segv_location, "StatServ-new_m_version");
	snumeric_cmd(351, origin,
		     "Module StatServ Loaded, Version: %s %s %s",
		     Statserv_Info[0].module_version, version_date,
		     version_time);
	return 0;
}

void _init()
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

	strcpy(segv_location, "StatServ-_init");
	strcpy(segvinmodule, SSMNAME);

	StatServ.onchan = 0;
	ss_Config();
	if (StatServ.html) {
		if (strlen(StatServ.htmlpath) < 1) {
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
}

void _fini()
{
	SaveStats();

}

int __Bot_Message(char *origin, char **av, int ac)
{
	User *u;

	strcpy(segv_location, "StatServ-__Bot_Message");


	u = finduser(origin);
	if (!u) {
		nlog(LOG_WARNING, LOG_CORE,
		     "Unable to finduser %s (statserv)", origin);
		return -1;
	}

	stats_network.requests++;

	nlog(LOG_NORMAL, LOG_MOD, "%s received message from %s: %s",
	     s_StatServ, u->nick, av[1]);

	if (me.onlyopers && UserLevel(u) < 40) {
		prefmsg(u->nick, s_StatServ,
			"This service is only available to IRCops.");
		chanalert(s_StatServ,
			  "%s tried to use me but is not Authorized",
			  u->nick);
		return -1;
	}

	if (!strcasecmp(av[1], "HELP")) {
		if (ac < 3) {
			chanalert(s_StatServ, "%s requested %s Help",
				  u->nick, s_StatServ);
		} else {
			chanalert(s_StatServ,
				  "%s requested more help from %s on %s",
				  u->nick, s_StatServ, av[2]);
		}
		if (ac < 3) {
			privmsg_list(u->nick, s_StatServ, ss_help);
			if (UserLevel(u) >= 150)
				privmsg_list(u->nick, s_StatServ,
					     ss_myuser_help);
		} else if (!strcasecmp(av[2], "SERVER"))
			privmsg_list(u->nick, s_StatServ, ss_server_help);
		else if (!strcasecmp(av[2], "CHAN"))
			privmsg_list(u->nick, s_StatServ, ss_chan_help);
		else if (!strcasecmp(av[2], "CLIENTVERSIONS"))
			privmsg_list(u->nick, s_StatServ,
				     ss_clientversions_help);
		else if (!strcasecmp(av[2], "MAP"))
			privmsg_list(u->nick, s_StatServ, ss_map_help);
		else if (!strcasecmp(av[2], "NETSTATS"))
			privmsg_list(u->nick, s_StatServ,
				     ss_netstats_help);
		else if (!strcasecmp(av[2], "DAILY"))
			privmsg_list(u->nick, s_StatServ, ss_daily_help);
		else if (!strcasecmp(av[2], "FORCEHTML"))
			privmsg_list(u->nick, s_StatServ,
				     ss_forcehtml_help);
		else if (!strcasecmp(av[2], "TLD"))
			privmsg_list(u->nick, s_StatServ, ss_tld_help);
		else if (!strcasecmp(av[2], "TLDMAP"))
			privmsg_list(u->nick, s_StatServ, ss_tld_map_help);
		else if (!strcasecmp(av[2], "OPERLIST"))
			privmsg_list(u->nick, s_StatServ,
				     ss_operlist_help);
		else if (!strcasecmp(av[2], "BOTLIST"))
			privmsg_list(u->nick, s_StatServ, ss_botlist_help);
		else if (!strcasecmp(av[2], "VERSION"))
			privmsg_list(u->nick, s_StatServ, ss_version_help);
		else if (!strcasecmp(av[2], "STATS")
			 && UserLevel(u) >= 185)
			privmsg_list(u->nick, s_StatServ, ss_stats_help);
		else if (!strcasecmp(av[2], "SET") && UserLevel(u) >= 185)
			privmsg_list(u->nick, s_StatServ, ss_set_help);
		else
			prefmsg(u->nick, s_StatServ,
				"Unknown Help Topic: \2%s\2", av[2]);
	} else if (!strcasecmp(av[1], "CHAN")) {
		if (ac < 2) {
			ss_chans(u, NULL);
		} else {
			ss_chans(u, av[2]);
		}
		chanalert(s_StatServ,
			  "%s Wanted to see Channel Statistics", u->nick);
	} else if (!strcasecmp(av[1], "SET")) {
		if (UserLevel(u) >= 185) {
			ss_set(u, av, ac);
		} else {
			prefmsg(u->nick, s_StatServ, "Permission Denied");
			chanalert(s_StatServ,
				  "%s tried to set, but don't have access",
				  u->nick);
		}
		return 1;
	} else if (!strcasecmp(av[1], "SERVER")) {
		ss_server(u, av[2]);
		chanalert(s_StatServ, "%s Wanted Server Information on %s",
			  u->nick, av[2]);
	} else if (!strcasecmp(av[1], "CLIENTVERSIONS")) {
		ss_versions(u, ac > 2 ? atoi(av[2]) : 10);
		chanalert(s_StatServ,
			  "%s Wanted to see the Client Version List",
			  u->nick);
	} else if (!strcasecmp(av[1], "MAP")) {
		ss_map(u);
		chanalert(s_StatServ,
			  "%s Wanted to see the Current Network MAP",
			  u->nick);
	} else if (!strcasecmp(av[1], "VERSION")) {
		ss_version(u);
		chanalert(s_StatServ,
			  "%s Wanted to know our version number ",
			  u->nick);
	} else if (!strcasecmp(av[1], "NETSTATS")) {
		ss_netstats(u);
		chanalert(s_StatServ, "%s Wanted to see the NetStats ",
			  u->nick);
	} else if (!strcasecmp(av[1], "DAILY")) {
		ss_daily(u);
		chanalert(s_StatServ,
			  "%s Wanted to see the Daily NetStats ", u->nick);
	} else if (!strcasecmp(av[1], "FORCEHTML")
		   && (UserLevel(u) >= 185)) {
		nlog(LOG_NOTICE, LOG_MOD,
		     "%s!%s@%s Forced an update of the NeoStats Statistics HTML file with the most current statistics",
		     u->nick, u->username, u->hostname);
		chanalert(s_StatServ,
			  "%s Forced the NeoStats Statistics HTML file to be updated with the most current statistics",
			  u->nick);
		ss_html();
	} else if (!strcasecmp(av[1], "TLD")) {
		ss_tld(u, av[2]);
		chanalert(s_StatServ,
			  "%s Wanted to find the Country that is Represented by %s ",
			  u->nick, av[2]);
	} else if (!strcasecmp(av[1], "TLDMAP")) {
		ss_tld_map(u);
		chanalert(s_StatServ,
			  "%s Wanted to see a Country Breakdown", u->nick);
	} else if (!strcasecmp(av[1], "OPERLIST")) {
		if (ac < 2) {
			prefmsg(u->nick, s_StatServ,
				"OperList Syntax Not Valid");
			prefmsg(u->nick, s_StatServ,
				"For Help: /msg %s HELP OPERLIST",
				s_StatServ);
		}
		ss_operlist(u, av[2], av[3]);
	} else if (!strcasecmp(av[1], "BOTLIST")) {
		ss_botlist(u);
		chanalert(s_StatServ, "%s Wanted to see the Bot List",
			  u->nick);
	} else if (!strcasecmp(av[1], "STATS") && (UserLevel(u) >= 185)) {
		ss_stats(u, av[2], av[3], av[4]);
		if (ac < 3) {
			chanalert(s_StatServ,
				  "%s Wants to Look at my Stats!! 34/24/34",
				  u->nick);
		}
	} else {
		prefmsg(u->nick, s_StatServ, "Unknown Command: \2%s\2",
			av[1]);
		chanalert(s_StatServ,
			  "%s Reqested %s, but that is a Unknown Command",
			  u->nick, av[1]);
	}
	return 1;
}

static void ss_set(User * u, char **av, int ac)
{

	if (ac == 2) {
		prefmsg(u->nick, s_StatServ, "Current Settings are:");
		prefmsg(u->nick, s_StatServ, "HTML Statistics: %s",
			StatServ.html > 0 ? "Enabled" : "Disabled");
		if (StatServ.html > 0)
			prefmsg(u->nick, s_StatServ, "HTML Path: %s",
				StatServ.htmlpath);
		prefmsg(u->nick, s_StatServ,
			"Wallop Throttling: %d (0 = Disabled)",
			StatServ.interval);
		prefmsg(u->nick, s_StatServ,
			"Lag Warnings: %d (0 = Disabled)", StatServ.lag);
		return;
	}
	if (!strcasecmp(av[2], "HTML")) {
		if (StatServ.html == 0) {
			/* its disabled, enable it */
			if (strlen(StatServ.htmlpath) > 0) {
				StatServ.html = 1;
				chanalert(s_StatServ,
					  "%s Enabled HTML output",
					  u->nick);
				prefmsg(u->nick, s_StatServ,
					"HTML output is now enabled to: %s",
					StatServ.htmlpath);
				SetConf((void *) 1, CFGINT,
					"HTML_Enabled");
			} else {
				prefmsg(u->nick, s_StatServ,
					"Error, No Path Defined for HTML output, can not enable");
				prefmsg(u->nick, s_StatServ,
					"Please see /msg %s help set",
					s_StatServ);
			}
		} else {
			/* its enabled, disable it */
			StatServ.html = 0;
			chanalert(s_StatServ, "%s Disabled HTML output",
				  u->nick);
			prefmsg(u->nick, s_StatServ,
				"HTML output is now disabled");
			SetConf((void *) 0, CFGINT, "HTML_Enabled");
		}
		return;
	} else if (!strcasecmp(av[2], "MSGTHROTTLE")) {
		if (ac != 4) {
			/* wrong number of arguments */
			prefmsg(u->nick, s_StatServ,
				"Invalid Syntax. /msg %s help set for more info",
				s_StatServ);
			return;
		}
		if (!strcasecmp(av[3], "off")) {
			prefmsg(u->nick, s_StatServ,
				"Record Yelling is Now Disabled. ");
			chanalert(s_StatServ,
				  "%s disabled Record Broadcasts",
				  u->nick);
			StatServ.interval = -1;
		} else {
			StatServ.interval = atoi(av[3]);
			/* atoi will = 0 if av[3] != isnumeric */
			if (StatServ.interval <= 0) {
				prefmsg(u->nick, s_StatServ,
					"Wallop Throttles are disabled");
				chanalert(s_StatServ,
					  "%s disabled Wallop Throttling",
					  u->nick);
				StatServ.interval = 0;
			} else {
				prefmsg(u->nick, s_StatServ,
					"Wallop Throttle is now set to 5 messages per %d Seconds",
					StatServ.interval);
				chanalert(s_StatServ,
					  "%s set Wallop Throttle to 5 messages per %d Seconds",
					  u->nick, StatServ.interval);
			}
		}
		SetConf((void *) StatServ.interval, CFGINT,
			"Wallop_Throttle");
		return;
	} else if (!strcasecmp(av[2], "HTMLPATH")) {
		if (ac != 4) {
			/* wrong number of params */
			prefmsg(u->nick, s_StatServ,
				"Invalid Syntax. /msg %s help set for more info",
				s_StatServ);
			return;
		}
		snprintf(StatServ.htmlpath, 255, "%s", av[3]);
		prefmsg(u->nick, s_StatServ, "HTML path now set to %s",
			StatServ.htmlpath);
		chanalert(s_StatServ, "%s changed the HTML path to %s",
			  u->nick, StatServ.htmlpath);
		SetConf((void *) StatServ.htmlpath, CFGSTR, "HTML_Path");
		return;
	} else if (!strcasecmp(av[2], "LAGWALLOP")) {
		if (ac != 4) {
			/* wrong number of params */
			prefmsg(u->nick, s_StatServ,
				"Invalid Syntax. /msg %s help set for more info",
				s_StatServ);
			return;
		}
		StatServ.lag = atoi(av[3]);
		if (StatServ.lag <= 0) {
			prefmsg(u->nick, s_StatServ,
				"Lag Warnings are now disabled");
			chanalert(s_StatServ, "%s Disabled Lag Warnings",
				  u->nick);
		} else {
			prefmsg(u->nick, s_StatServ,
				"Server Lag Warnings now set to %d seconds",
				StatServ.lag);
			chanalert(u->nick, s_StatServ,
				  "%s changed Server Lag Warning threshold to %d",
				  u->nick, StatServ.lag);
		}
		SetConf((void *) StatServ.lag, CFGINT, "Lag");
		return;
	} else {
		prefmsg(u->nick, s_StatServ,
			"Invalid Syntax. /msg %s help set for more info",
			s_StatServ);
		return;
	}
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

static void ss_versions(User * u, int num)
{
	CVersions *cv;
	lnode_t *cn;
	int i;
	if (num < 10) {
		num = 10;
	}
	if (list_count(Vhead) == 0) {
		prefmsg(u->nick, s_StatServ, "No Stats Available.");
		return;
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
}

static void ss_chans(User * u, char *chan)
{
	CStats *cs;
	lnode_t *cn;
	int i;
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
			    && (UserLevel(u) < 40)) {
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
			    && (UserLevel(u) < 40)) {
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
			    && (UserLevel(u) < 40)) {
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
			    && (UserLevel(u) < 40)) {
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
			return;
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
}

static void ss_tld_map(User * u)
{
	TLD *t;

	strcpy(segv_location, "StatServ-ss_tld_map");


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
}

static void ss_version(User * u)
{
	strcpy(segv_location, "StatServ-ss_version");

	prefmsg(u->nick, s_StatServ, "\2StatServ Version Information\2");
	prefmsg(u->nick, s_StatServ,
		"-------------------------------------");
	prefmsg(u->nick, s_StatServ,
		"StatServ Version: %s Compiled %s at %s",
		Statserv_Info[0].module_version, version_date,
		version_time);
	prefmsg(u->nick, s_StatServ, "http://www.neostats.net");
	prefmsg(u->nick, s_StatServ,
		"-------------------------------------");
	prefmsg(u->nick, s_StatServ, "HTML Stats is: %s",
		(StatServ.html ? StatServ.htmlpath : "Disabled"));
}
static void ss_netstats(User * u)
{

	strcpy(segv_location, "StatServ-ss_netstats");

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
	prefmsg(u->nick, s_StatServ, "Users Set Away: %d",
		stats_network.away);
	prefmsg(u->nick, s_StatServ, "Current Servers: %d",
		stats_network.servers);
	prefmsg(u->nick, s_StatServ, "Maximum Servers: %d [%s]",
		stats_network.maxservers,
		sftime(stats_network.t_maxservers));
	prefmsg(u->nick, s_StatServ, "--- End of List ---");
}
static void ss_daily(User * u)
{

	strcpy(segv_location, "StatServ-ss_daily");

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

		if ((level == 0) && (strlen(s->uplink) <= 0)) {
			/* its the root server */
			prefmsg(u->nick, s_StatServ,
				"\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				ss->name, ss->users, ss->maxusers,
				ss->opers, ss->maxopers, s->ping,
				ss->highest_ping);
			makemap(s->name, u, level + 1);
		} else if ((level > 0) && !strcasecmp(uplink, s->uplink)) {
			/* its not the root server */
			sprintf(buf, " ");
			for (i = 1; i < level; i++) {
				sprintf(buf, "%s     |", buf);
			}
			prefmsg(u->nick, s_StatServ,
				"%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]",
				buf, ss->name, ss->users, ss->maxusers,
				ss->opers, ss->maxopers, s->ping,
				ss->highest_ping);
			makemap(s->name, u, level + 1);
		}
	}
	return;
}

static void ss_map(User * u)
{
	strcpy(segv_location, "StatServ-ss_map");
	prefmsg(u->nick, s_StatServ, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2",
		"\2[LAG/MAX]\2");
	makemap("", u, 0);
	prefmsg(u->nick, s_StatServ, "--- End of Listing ---");
}

static void ss_server(User * u, char *server)
{

	SStats *ss;
	Server *s;
	hscan_t hs;
	hnode_t *sn;

	strcpy(segv_location, "StatServ-ss_server");


	if (!server) {
		prefmsg(u->nick, s_StatServ,
			"Error, the Syntax is Incorrect. Please Specify a Server");
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
		return;
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
		return;
	}
	prefmsg(u->nick, s_StatServ, "Statistics for \2%s\2 since %s",
		ss->name, sftime(ss->starttime));
	if (!s)
		prefmsg(u->nick, s_StatServ, "Server Last Seen: %s",
			sftime(ss->lastseen));
	if (s)
		prefmsg(u->nick, s_StatServ,
			"Current Users: %-3ld (%2.0f%%)", ss->users,
			(float) ss->users / (float) stats_network.users *
			100);
	prefmsg(u->nick, s_StatServ, "Maximum Users: %-3ld at %s",
		ss->maxusers, sftime(ss->t_maxusers));
	prefmsg(u->nick, s_StatServ, "Total Users Connected: %-3ld",
		ss->totusers);
	if (s)
		prefmsg(u->nick, s_StatServ, "Current Opers: %-3ld",
			ss->opers);
	prefmsg(u->nick, s_StatServ, "Maximum Opers: %-3ld at %s",
		ss->maxopers, sftime(ss->t_maxopers));
	prefmsg(u->nick, s_StatServ, "IRCop Kills: %d", ss->operkills);
	prefmsg(u->nick, s_StatServ, "Server Kills: %d", ss->serverkills);
	prefmsg(u->nick, s_StatServ, "Lowest Ping: %-3d at %s",
		ss->lowest_ping, sftime(ss->t_lowest_ping));
	prefmsg(u->nick, s_StatServ, "Higest Ping: %-3d at %s",
		ss->highest_ping, sftime(ss->t_highest_ping));
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
}

static void ss_tld(User * u, char *tld)
{
	TLD *tmp;

	strcpy(segv_location, "StatServ-ss_tld");


	if (!tld) {
		prefmsg(u->nick, s_StatServ, "Syntax: /msg %s TLD <tld>",
			s_StatServ);
		prefmsg(u->nick, s_StatServ,
			"For additional help, /msg %s HELP", s_StatServ);
		return;
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
		prefmsg(u->nick, s_StatServ, "%s", tmp->country);
}

static void ss_operlist(User * origuser, char *flags, char *server)
{
	register int j = 0;
	int away = 0;
	register User *u;
	int tech = 0;
	hscan_t scan;
	hnode_t *node;

	strcpy(segv_location, "StatServ-ss_operlist");


	if (!flags) {
		prefmsg(origuser->nick, s_StatServ, "On-Line IRCops:");
		prefmsg(origuser->nick, s_StatServ,
			"ID  %-15s %-15s %-10s", "Nick", "Server",
			"Level");
		chanalert(s_StatServ, "%s Requested OperList",
			  origuser->nick);
	}

	if (flags && !strcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		prefmsg(origuser->nick, s_StatServ,
			"On-Line IRCops (Not Away):");
		chanalert(s_StatServ,
			  "%s Reqested Operlist of Non-Away Opers",
			  origuser->nick);
	}
	if (!away && flags && strchr(flags, '.')) {
		server = flags;
		prefmsg(origuser->nick, s_StatServ,
			"On-Line IRCops on Server %s", server);
		chanalert(s_StatServ, "%s Reqested Operlist on Server %s",
			  origuser->nick, server);
	}
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
		if (!is_oper(u))
			continue;
		tech = UserLevel(u);
		if (away && u->is_away)
			continue;
		if (!strcasecmp(u->server->name, me.services_name))
			continue;
		if (tech < 40)
			continue;
		if (!server) {
			j++;
			prefmsg(origuser->nick, s_StatServ,
				"[%2d] %-15s %-15s %-10d", j, u->nick,
				u->server->name, tech);
			continue;
		} else {
			if (strcasecmp(server, u->server->name))
				continue;
			j++;
			prefmsg(origuser->nick, s_StatServ,
				"[%2d] %-15s %-15s %-10d", j, u->nick,
				u->server->name, tech);
			continue;
		}
	}
	prefmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_botlist(User * origuser)
{
	register int j = 0;
	register User *u;
	hscan_t scan;
	hnode_t *node;
	strcpy(segv_location, "StatServ-ss_botlist");


	prefmsg(origuser->nick, s_StatServ, "On-Line Bots:");
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
#ifdef UNREAL
		if (u->Umode & UMODE_BOT) {
#elif ULTIMATE3
		if (0) {
#elif ULTIMATE
		if ((u->Umode & UMODE_RBOT) || (u->Umode & UMODE_SBOT)) {
#else
		if (0) {
#endif
			j++;
			prefmsg(origuser->nick, s_StatServ,
				"[%2d] %-15s %s", j, u->nick,
				u->server->name);
			continue;
		}
	}
	prefmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_stats(User * u, char *cmd, char *arg, char *arg2)
{
	SStats *st;
	hnode_t *node;
	hscan_t scan;


	strcpy(segv_location, "StatServ-ss_stats");


	if (UserLevel(u) < 185) {
		nlog(LOG_NORMAL, LOG_MOD, "Access Denied (STATS) to %s",
		     u->nick);
		prefmsg(u->nick, s_StatServ, "Access Denied.");
		return;
	}

	if (!cmd) {
		prefmsg(u->nick, s_StatServ,
			"Syntax: /msg %s STATS [DEL|LIST|COPY]",
			s_StatServ);
		prefmsg(u->nick, s_StatServ,
			"For additional help, /msg %s HELP", s_StatServ);
		return;
	}
	if (!strcasecmp(cmd, "LIST")) {
		int i = 1;
		prefmsg(u->nick, s_StatServ, "Statistics Database:");
		hash_scan_begin(&scan, Shead);
		while ((node = hash_scan_next(&scan))) {
			st = hnode_get(node);
			prefmsg(u->nick, s_StatServ, "[%-2d] %s", i,
				st->name);
			i++;
		}
		prefmsg(u->nick, s_StatServ, "End of List.");
		nlog(LOG_NOTICE, LOG_MOD, "%s requested STATS LIST.",
		     u->nick);
	} else if (!strcasecmp(cmd, "DEL")) {
		if (!arg) {
			prefmsg(u->nick, s_StatServ,
				"Syntax: /msg %s STATS DEL <name>",
				s_StatServ);
			prefmsg(u->nick, s_StatServ,
				"For additonal help, /msg %s HELP",
				s_StatServ);
			return;
		}
		st = findstats(arg);
		if (!st) {
			prefmsg(u->nick, s_StatServ,
				"%s is not in the database!", arg);
			return;
		}
		if (!findserver(arg)) {
			node = hash_lookup(Shead, arg);
			if (node) {
				hash_delete(Shead, node);
				st = hnode_get(node);
				hnode_destroy(node);
				free(st);
				prefmsg(u->nick, s_StatServ,
					"Removed %s from the database.",
					arg);
				nlog(LOG_NOTICE, LOG_MOD,
				     "%s requested STATS DEL %s", u->nick,
				     arg);
				return;
			}
		} else {
			prefmsg(u->nick, s_StatServ,
				"Cannot remove %s from the database, it is online!!",
				arg);
			nlog(LOG_WARNING, LOG_MOD,
			     "%s requested STATS DEL %s, but that server is online!!",
			     u->nick, arg);
			return;
		}

	} else if (!strcasecmp(cmd, "COPY")) {
		Server *s;

		if (!arg || !arg2) {
			prefmsg(u->nick, s_StatServ,
				"Syntax: /msg %s STATS COPY <name> "
				" <newname>", s_StatServ);
			return;
		}
		st = findstats(arg2);
		if (st)
			free(st);

		st = findstats(arg);
		if (!st) {
			prefmsg(u->nick, s_StatServ,
				"No entry in the database for %s", arg);
			return;
		}
		s = findserver(arg);
		if (s) {
			prefmsg(u->nick, s_StatServ,
				"Server %s is online!", arg);
			return;
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
}
