/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      statserv.c, 
** Version: 3.1
** Date:    08/03/2002
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "statserv.h"


extern const char version_date[], version_time[];
static void ss_daily(User *u);
static void ss_reset(User *u);
static void ss_stats(User *u, char *cmd, char *arg, char *arg2);
static void ss_JOIN(User *u, char *chan);
static void ss_tld(User *u, char *tld);
static void ss_tld_map(User *u) ;
static void ss_operlist(User *origuser, char *flags, char *server);
static void ss_botlist(User *origuser);
static void ss_version(User *u);
static void ss_server(User *u, char *server);
static void ss_map(User *);
static void ss_netstats(User *);
/* int s_bot_kill(char *); */
static void ss_cb_Config(char *, int);
static int new_m_version(char *origin, char **av, int ac);
void ss_html();

char s_StatServ[MAXNICK] = "StatServ";

Module_Info Statserv_Info[] = { {
	SSMNAME,
	"Statistical Bot For NeoStats",
	"3.0"
} };


Functions StatServ_fn_list[] = { 
	{ MSG_VERSION,	new_m_version,	1 },
	{ TOK_VERSION,	new_m_version,	1 },
	{ NULL,		NULL,	 0}
};


EventFnList StatServ_Event_List[] = {
	{ "ONLINE",	 Online},
	{ "PONG",	 pong},
	{ "NEWSERVER",	s_new_server},
	{ "SQUIT",	s_del_server},
	{ "SIGNON",	 s_new_user},
	{ "UMODE",	 s_user_modes},
	{ "SIGNOFF",	 s_del_user},
	{ "AWAY",	s_user_away},
	{ "KILL",	s_user_kill},
	{ "NEWCHAN", 	s_chan_new},
	{ "DELCHAN",	s_chan_del},
	{ NULL,	 NULL}
};



Module_Info *__module_get_info() {
	return Statserv_Info;
};

Functions *__module_get_functions() {
	return StatServ_fn_list;
};

EventFnList *__module_get_events() {
	return StatServ_Event_List;
};

static config_option options[] = {
{ "STATSERV_NICK", ARG_STR, ss_cb_Config, 0},
{ "STATSERV_USER", ARG_STR, ss_cb_Config, 1},
{ "STATSERV_HOST", ARG_STR, ss_cb_Config, 2},
{ "STATSERV_LAG", ARG_STR, ss_cb_Config, 3},
{ "HTML_STATS", ARG_STR, ss_cb_Config, 4},
{ "HTML_PATH", ARG_STR, ss_cb_Config, 5}
};


void ss_cb_Config(char *arg, int configtype) {

	strcpy(segv_location, "StatServ-ss_cb_Config");


	if (configtype == 0) {
		/* Nick */
		memcpy(StatServ.nick, arg, MAXNICK);
		memcpy(s_StatServ, StatServ.nick, MAXNICK);
	} else if (configtype == 1) {
		/* User */
		memcpy(StatServ.user, arg, 8);
	} else if (configtype == 2) {
		/* host */
		memcpy(StatServ.host, arg, MAXHOST);
	} else if (configtype == 3) {
		/* lag */
		StatServ.lag = atoi(arg);
	} else if (configtype == 4) {
		/* htmlstats? */
		StatServ.html = 1;
	} else if (configtype == 5) {
		/* htmlpath */
		strcpy(StatServ.htmlpath, arg);
	}
}




int new_m_version(char *origin, char **av, int ac) {
	strcpy(segv_location, "StatServ-new_m_version");
	snumeric_cmd(351, origin, "Module StatServ Loaded, Version: %s %s %s",Statserv_Info[0].module_version,version_date,version_time);
	return 0;
}

void _init() {
	Server *ss;
	User *u;
	hnode_t *node;
	hscan_t scan;
	strcpy(segv_location, "StatServ-_init");
	StatServ.onchan = 0;
	globops(me.name, "StatServ Module Loaded");	
	memcpy(StatServ.user, Servbot.user, 8);
	memcpy(StatServ.host, Servbot.host, MAXHOST);
	StatServ.lag = 0;
	StatServ.html = 0;
	if (!config_read("stats.cfg", options) == 0) {
		log("Error, Statserv could not be configured");
		notice(s_Services, "Error, Statserv could not be configured");
	return;
	}
	LoadTLD();
	init_tld();
	LoadStats();



	hash_scan_begin(&scan, sh);
	while ((node = hash_scan_next(&scan)) != NULL ) {
   	ss = hnode_get(node);
	s_new_server(ss);
#ifdef DEBUG
			log("Added Server %s to StatServ List", ss->name);
#endif
	}
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL ) {
		u = hnode_get(node);
	s_new_user(u);
	s_user_modes(u);
#ifdef DEBUG
			log("Adduser user %s to StatServ List", u->nick);
#endif
	}
}

void _fini() {
	SaveStats();
	if (StatServ.onchan) globops(me.name, "StatServ Module Unloaded");
	
}
	
int __Bot_Message(char *origin, char **av, int ac)
{
	User *u;

	strcpy(segv_location, "StatServ-__Bot_Message");


	u = finduser(origin);
	if (!u) {
		log("Unable to finduser %s (statserv)", origin);
		return -1;
	}

	stats_network.requests++;

	if (flood(u))
		return -1;
	log("%s received message from %s: %s", s_StatServ, u->nick, av[1]);

	if (me.onlyopers && UserLevel(u) < 40) {
		privmsg(u->nick, s_StatServ,
			"This service is only available to IRCops.");
		notice(s_StatServ, "%s tried to use me but is not Authorized", u->nick);
		return -1;
	}

	if (!strcasecmp(av[1], "HELP")) {
		if(ac < 3) {
			notice(s_StatServ, "%s requested %s Help", u->nick, s_StatServ);
		} else {
		  notice(s_StatServ,"%s requested more help from %s on %s",u->nick, s_StatServ, av[2]);
		}
		if (ac < 3) {
			privmsg_list(u->nick, s_StatServ, ss_help);
			if (UserLevel(u) >= 150)
				privmsg_list(u->nick, s_StatServ, ss_myuser_help);
		} else if (!strcasecmp(av[2], "SERVER"))
			privmsg_list(u->nick, s_StatServ, ss_server_help);
		else if (!strcasecmp(av[2], "RESET") && UserLevel(u) >= 190)
			privmsg_list(u->nick, s_StatServ, ss_reset_help);
		else if (!strcasecmp(av[2], "MAP"))
			privmsg_list(u->nick, s_StatServ, ss_map_help);
		else if (!strcasecmp(av[2], "JOIN") && UserLevel(u) >= 190)
			privmsg_list(u->nick, s_StatServ, ss_join_help);
		else if (!strcasecmp(av[2], "NETSTATS"))
			privmsg_list(u->nick, s_StatServ, ss_netstats_help);
		else if (!strcasecmp(av[2], "DAILY"))
			privmsg_list(u->nick, s_StatServ, ss_daily_help);
		else if (!strcasecmp(av[2], "HTMLSTATS"))
			privmsg_list(u->nick, s_StatServ, ss_htmlstats_help);
		else if (!strcasecmp(av[2], "FORCEHTML"))
			privmsg_list(u->nick, s_StatServ, ss_forcehtml_help);
		else if (!strcasecmp(av[2], "NOTICES"))
			privmsg_list(u->nick, s_StatServ, ss_notices_help);
		else if (!strcasecmp(av[2], "TLD"))
			privmsg_list(u->nick, s_StatServ, ss_tld_help);
		else if (!strcasecmp(av[2], "TLDMAP"))
			privmsg_list(u->nick, s_StatServ, ss_tld_map_help);
		else if (!strcasecmp(av[2], "OPERLIST"))
			privmsg_list(u->nick, s_StatServ, ss_operlist_help);
		else if (!strcasecmp(av[2], "BOTLIST"))
			privmsg_list(u->nick, s_StatServ, ss_botlist_help);
		else if (!strcasecmp(av[2], "VERSION"))
			privmsg_list(u->nick, s_StatServ, ss_version_help);
		else if (!strcasecmp(av[2], "STATS") && UserLevel(u) >= 190)
			privmsg_list(u->nick, s_StatServ, ss_stats_help);
		else
			privmsg(u->nick, s_StatServ, "Unknown Help Topic: \2%s\2", av[2]);
	} else if (!strcasecmp(av[1], "SERVER")) {
		ss_server(u, av[2]);
		notice(s_StatServ,"%s Wanted Server Information on %s",u->nick, av[2]);
	} else if (!strcasecmp(av[1], "JOIN") && (UserLevel(u) >= 185)) {
		ss_JOIN(u, av[2]);
	} else if (!strcasecmp(av[1], "MAP")) {
		ss_map(u);
		notice(s_StatServ,"%s Wanted to see the Current Network MAP",u->nick);
	} else if (!strcasecmp(av[1], "VERSION")) {
		ss_version(u);
		notice(s_StatServ,"%s Wanted to know our version number ",u->nick);
	} else if (!strcasecmp(av[1], "NETSTATS")) {
		ss_netstats(u);
		notice(s_StatServ,"%s Wanted to see the NetStats ",u->nick);
	} else if (!strcasecmp(av[1], "DAILY")) {
		ss_daily(u);
		notice(s_StatServ,"%s Wanted to see the Daily NetStats ",u->nick);
	} else if (!strcasecmp(av[1], "FORCEHTML") && (UserLevel(u) >= 185)) {
		log("%s!%s@%s Forced an update of the NeoStats Statistics HTML file with the most current statistics", u->nick, u->username, u->hostname);
		notice(s_StatServ,"%s Forced the NeoStats Statistics HTML file to be updated with the most current statistics",u->nick);
		ss_html();
	} else if (!strcasecmp(av[1], "TLD")) {
		ss_tld(u, av[2]);
		notice(s_StatServ,"%s Wanted to find the Country that is Represented by %s ",u->nick,av[2]);
	} else if (!strcasecmp(av[1], "TLDMAP")) {
		ss_tld_map(u);
		notice(s_StatServ,"%s Wanted to see a Country Breakdown",u->nick);
	} else if (!strcasecmp(av[1], "OPERLIST")) {
	if (ac < 4) {
		privmsg(u->nick, s_StatServ, "OperList Syntax Not Valid");
		privmsg(u->nick, s_StatServ, "For Help: /msg %s HELP OPERLIST", s_StatServ);
	}
		ss_operlist(u, av[2], av[3]);
	} else if (!strcasecmp(av[1], "BOTLIST")) {
		ss_botlist(u);
		notice(s_StatServ,"%s Wanted to see the Bot List",u->nick);
	} else if (!strcasecmp(av[1], "STATS") && (UserLevel(u) >= 185)) {
	if (ac < 5) {
		privmsg(u->nick, s_StatServ, "Incorrect Syntax: /msg %s HELP STATS", s_StatServ);
	}
		ss_stats(u, av[2], av[3], av[4]);
		notice(s_StatServ,"%s Wants to Look at my Stats!! 34/24/34",u->nick);
	} else if (!strcasecmp(av[1], "RESET") && (UserLevel(u) >= 185)) {
		notice(s_StatServ,"%s Wants me to RESET the databases.. here goes..",u->nick);
		ss_reset(u);
	} else {
		privmsg(u->nick, s_StatServ, "Unknown Command: \2%s\2", av[1]);
		notice(s_StatServ,"%s Reqested %s, but that is a Unknown Command",u->nick,av[1]);
	}
	return 1;
}
static void ss_tld_map(User *u) {
	TLD *t;
	
	strcpy(segv_location, "StatServ-ss_tld_map");


	privmsg(u->nick, s_StatServ, "Top Level Domain Statistics:");
	for (t = tldhead; t; t = t->next) {
		if (t->users != 0)
			privmsg(u->nick, s_StatServ, "%3s \2%3d\2 (%2.0f%%) -> %s ---> Daily Total: %d", t->tld, t->users, (float)t->users / (float)stats_network.users * 100, t->country, t->daily_users);
	}		
	privmsg(u->nick, s_StatServ, "End of List");
}	

static void ss_version(User *u)
{
	strcpy(segv_location, "StatServ-ss_version");

		privmsg(u->nick, s_StatServ, "\2StatServ Version Information\2");
		privmsg(u->nick, s_StatServ, "-------------------------------------");
		privmsg(u->nick, s_StatServ, "StatServ Version: %s Compiled %s at %s", Statserv_Info[0].module_version, version_date, version_time);
		privmsg(u->nick, s_StatServ, "http://www.neostats.net");
		privmsg(u->nick, s_StatServ, "-------------------------------------");
}
static void ss_netstats(User *u) {

	strcpy(segv_location, "StatServ-ss_netstats");

	privmsg(u->nick, s_StatServ, "Network Statistics:-----");
	privmsg(u->nick, s_StatServ, "Current Users: %ld", stats_network.users);
	privmsg(u->nick, s_StatServ, "Maximum Users: %ld [%s]", stats_network.maxusers, sftime(stats_network.t_maxusers));
	privmsg(u->nick, s_StatServ, "Current Channels %ld", stats_network.chans);
	privmsg(u->nick, s_StatServ, "Maximum Channels %ld [%s]", stats_network.maxchans, sftime(stats_network.t_chans));
	privmsg(u->nick, s_StatServ, "Current Opers: %ld", stats_network.opers);
	privmsg(u->nick, s_StatServ, "Maximum Opers: %ld [%s]", stats_network.maxopers, sftime(stats_network.t_maxopers));
	privmsg(u->nick, s_StatServ, "Users Set Away: %d", stats_network.away);
	privmsg(u->nick, s_StatServ, "Current Servers: %d", stats_network.servers);
	privmsg(u->nick, s_StatServ, "Maximum Servers: %d [%s]", stats_network.maxservers, sftime(stats_network.t_maxservers));
	privmsg(u->nick, s_StatServ, "--- End of List ---");
}
static void ss_daily(User *u) {

	strcpy(segv_location, "StatServ-ss_daily");

	privmsg(u->nick, s_StatServ, "Daily Network Statistics:");
	privmsg(u->nick, s_StatServ, "Maximum Servers: %-2d %s", daily.servers, sftime(daily.t_servers));
	privmsg(u->nick, s_StatServ, "Maximum Users: %-2d %s", daily.users, sftime(daily.t_users));
	privmsg(u->nick, s_StatServ, "Maximum Chans: %-2d %s", daily.chans, sftime(daily.t_chans));
	privmsg(u->nick, s_StatServ, "Maximum Opers: %-2d %s", daily.opers, sftime(daily.t_opers));
	privmsg(u->nick, s_StatServ, "All Daily Statistics are reset at Midnight");
	privmsg(u->nick, s_StatServ, "End of Information.");
}

static void makemap(char *uplink, User *u, int level) {
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
			privmsg(u->nick, s_StatServ, "\2%-45s	[ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]", ss->name, ss->users, ss->maxusers, ss->opers, ss->maxopers, s->ping, ss->highest_ping);
			makemap(s->name, u, level+1);
		} else if ((level > 0) && !strcasecmp(uplink, s->uplink)) {
			/* its not the root server */
			sprintf(buf, " ");
			for (i = 1; i < level; i++) {
				sprintf(buf, "%s     |", buf);
			}	
			privmsg(u->nick, s_StatServ, "%s \\_\2%-40s	[ %d/%d ]   [ %d/%d ]   [ %ld/%ld ]", buf, ss->name, ss->users, ss->maxusers, ss->opers, ss->maxopers, s->ping, ss->highest_ping);
			makemap(s->name, u, level+1);
		}
	}
	return;
}

static void ss_map(User *u) {
	strcpy(segv_location, "StatServ-ss_map");
	privmsg(u->nick, s_StatServ, "%-40s	%-10s %-10s %-10s", "\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2",  "\2[LAG/MAX]\2");
	makemap("", u, 0);
	privmsg(u->nick, s_StatServ, "--- End of Listing ---");
}

static void ss_server(User *u, char *server) {

	SStats *ss;
	Server *s;
	hscan_t hs;
	hnode_t *sn;

	strcpy(segv_location, "StatServ-ss_server");


	if (!server) {
		privmsg(u->nick, s_StatServ, "Error, the Syntax is Incorrect. Please Specify a Server");
		privmsg(u->nick, s_StatServ, "Server Listing:");
		hash_scan_begin(&hs, Shead);
		while ((sn = hash_scan_next(&hs))) {
			ss = hnode_get(sn);
			if (findserver(ss->name)) {
				privmsg(u->nick, s_StatServ, "Server: %s (*)", ss->name);
			} else {
				privmsg(u->nick, s_StatServ, "Server: %s",ss->name);
			}
		}		
		privmsg(u->nick, s_StatServ, "***** End of List (* indicates Server is online at the moment) *****");		
		return;
	}

	/* ok, found the Server, lets do some Statistics work now ! */
	ss = findstats(server);
	s=findserver(server);
	if (!ss) {
		log("Wups, Problem, Cant find Server Statistics for Server %s", server);
		privmsg(u->nick, s_StatServ, "Internal Error! Please Consult the Log file");
		return;
	}
	privmsg(u->nick, s_StatServ, "Statistics for \2%s\2 since %s", ss->name, sftime(ss->starttime));
	if (!s) privmsg(u->nick, s_StatServ, "Server Last Seen: %s", sftime(ss->lastseen));
	if (s) privmsg(u->nick, s_StatServ, "Current Users: %-3ld (%2.0f%%)", ss->users, (float)ss->users / (float)stats_network.users * 100);
	privmsg(u->nick, s_StatServ, "Maximum Users: %-3ld at %s", ss->maxusers, sftime(ss->t_maxusers));
	if (s) privmsg(u->nick, s_StatServ, "Current Opers: %-3ld", ss->opers);
	privmsg(u->nick, s_StatServ, "Maximum Opers: %-3ld at %s", ss->maxopers, sftime(ss->t_maxopers));
	privmsg(u->nick, s_StatServ, "IRCop Kills: %d", ss->operkills);
	privmsg(u->nick, s_StatServ, "Server Kills: %d", ss->serverkills);
	privmsg(u->nick, s_StatServ, "Lowest Ping: %-3d at %s", ss->lowest_ping, sftime(ss->t_lowest_ping));
	privmsg(u->nick, s_StatServ, "Higest Ping: %-3d at %s", ss->highest_ping, sftime(ss->t_highest_ping));
	if (s) privmsg(u->nick, s_StatServ, "Current Ping: %-3d", s->ping);
	if (ss->numsplits >= 1) 
		privmsg(u->nick, s_StatServ, "%s has Split from the network %d time%s", ss->name, ss->numsplits, (ss->numsplits == 1) ? "" : "s");
	else
		privmsg(u->nick, s_StatServ, "%s has never split from the Network.", ss->name);
	privmsg(u->nick, s_StatServ, "***** End of Statistics *****");	
}

static void ss_tld(User *u, char *tld)
{
	TLD *tmp;

	strcpy(segv_location, "StatServ-ss_tld");


	if (!tld) {
		privmsg(u->nick, s_StatServ, "Syntax: /msg %s TLD <tld>", s_StatServ);
		privmsg(u->nick, s_StatServ, "For additional help, /msg %s HELP", 
			s_StatServ);
		return;
	}

	if (*tld == '*')
		tld++;
	if (*tld == '.')
		tld++;

	tmp = findtld(tld);

	if (!tmp)
		privmsg(u->nick, s_StatServ, "Top Level Domain \2%s\2 does not exist.",
			tld);
	else
		privmsg(u->nick, s_StatServ, "%s", tmp->country);
}

static void ss_operlist(User *origuser, char *flags, char *server)
{
	register int j = 0;
	int away = 0;
	register User *u;
	int tech = 0;
	hscan_t scan;
	hnode_t *node;

	strcpy(segv_location, "StatServ-ss_operlist");


	if (!flags) {
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops:");
		notice (s_StatServ, "%s Requested OperList",origuser->nick);
	}		

	if (flags && !strcasecmp(flags, "NOAWAY")) {
		away = 1;
		flags = NULL;
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops (Not Away):");
		notice(s_StatServ,"%s Reqested Operlist of Non-Away Opers",origuser->nick);
	}
	if (!away && flags && strchr(flags, '.')) {
		server = flags;
		privmsg(origuser->nick, s_StatServ, "On-Line IRCops on Server %s", server);
		notice(s_StatServ,"%s Reqested Operlist on Server %s",origuser->nick, server);
	}
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
			u = hnode_get(node);
			tech = UserLevel(u);
			if (away && u->is_away)
				continue;
			if (!strcasecmp(u->server->name, me.services_name))
				continue;
			if (tech < 40)
				continue;
			if (!server) {
				if (UserLevel(u) < 40)	continue;
				j++;
				privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %-15s %-15s %-10d",j, u->nick,u->modes,
					u->server->name, tech);
				continue;
			} else {
				if (strcasecmp(server, u->server->name))	continue;
				if (UserLevel(u) < 40)	continue;
				j++;
				privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %-15s %-15s %-10d",j, u->nick,u->modes,
					u->server->name, tech);
				continue;
			}
	}
	privmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_botlist(User *origuser)
{
		register int j = 0;
		register User *u;
	hscan_t scan;
	hnode_t *node;
		strcpy(segv_location, "StatServ-ss_botlist");


		privmsg(origuser->nick, s_StatServ, "On-Line Bots:");
	hash_scan_begin(&scan, uh);
	while ((node = hash_scan_next(&scan)) != NULL) {
		u = hnode_get(node);
#ifdef UNREAL
				if (u->Umode & UMODE_BOT) {
#elif ULTIMATE
		if ((u->Umode & UMODE_RBOT) || (u->Umode & UMODE_SBOT)) {
#endif
					j++;
						privmsg(origuser->nick, s_StatServ, "[%2d] %-15s %s",j, u->nick, u->server->name);
						continue;
	   		}
		}	   
		privmsg(origuser->nick, s_StatServ, "End of Listing.");
}


static void ss_stats(User *u, char *cmd, char *arg, char *arg2)
{
	SStats *st;
	hscan_t hs;
	hnode_t *sn;

	strcpy(segv_location, "StatServ-ss_stats");


	if (UserLevel(u) < 190) {
		log("Access Denied (STATS) to %s", u->nick);
		privmsg(u->nick, s_StatServ, "Access Denied.");
		return;
	}

	if (!cmd) {
		privmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS [DEL|LIST|COPY]",
			s_StatServ);
		privmsg(u->nick, s_StatServ, "For additional help, /msg %s HELP",
			s_StatServ);	
		return;
	}
	if (!strcasecmp(cmd, "LIST")) {
		int i = 1;
		privmsg(u->nick, s_StatServ, "Statistics Database:");
		hash_scan_begin(&hs, Shead);
		while ((sn = hash_scan_next(&hs))) {
			st = hnode_get(sn);
			privmsg(u->nick, s_StatServ, "[%-2d] %s", i, st->name);
			i++;
		}
		privmsg(u->nick, s_StatServ, "End of List.");
		log("%s requested STATS LIST.", u->nick);
	} else if (!strcasecmp(cmd, "DEL")) {
		if (!arg) {
			privmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS DEL <name>",
				s_StatServ);
			privmsg(u->nick, s_StatServ, "For additonal help, /msg %s HELP",
				s_StatServ);
			return;
		}
		st = findstats(arg);
		if (!st) {
			privmsg(u->nick, s_StatServ, "%s is not in the database!",
				arg);
			return;
		}
		sn = hash_lookup(Shead, arg);
		if (sn) {
			hash_delete(Shead, sn);
			st = hnode_get(sn);
			hnode_destroy(sn);
			free(st);
		}
		privmsg(u->nick, s_StatServ, "Removed %s from the database.", arg);
		log("%s requested STATS DEL %s", u->nick, arg);
	} else if (!strcasecmp(cmd, "COPY")) {
		Server *s;

		if (!arg || !arg2) {
			privmsg(u->nick, s_StatServ, "Syntax: /msg %s STATS COPY <name> "
				" <newname>", s_StatServ);
			return;
		}
		st = findstats(arg2);
		if (st)
			free(st);

		st = findstats(arg);
		if (!st) {
			privmsg(u->nick, s_StatServ, "No entry in the database for %s",
				arg);
			return;
		}
		s = findserver(arg);
		if (s) {
			privmsg(u->nick, s_StatServ, "Server %s is online!", arg);
			return;
		}
		s = NULL;
		memcpy(st->name, arg2, sizeof(st->name));
		privmsg(u->nick, s_StatServ, "Moved database entry for %s to %s",
			arg, arg2);
		log("%s requested STATS COPY %s -> %s", u->nick, arg, arg2);
	} else {
		privmsg(u->nick, s_StatServ, "Invalid Argument.");
		privmsg(u->nick, s_StatServ, "For help, /msg %s HELP", s_StatServ);
	}
}

static void ss_reset(User *u)
{

	strcpy(segv_location, "StatServ-ss_reset");


		if (UserLevel(u) < 190) {
				log("Access Denied (RELOAD) to %s", u->nick);
				privmsg(u->nick, s_StatServ, "Access Denied.");
				return;
		}
		remove("data/nstats.db");
		remove("data/stats.db");
		globops(s_StatServ, "%s requested \2RESET\2 databases.", u->nick);
		log("%s requested RESET.", u->nick);
		globops(s_StatServ, "Rebuilding Statistics DataBase after RESET...");
	privmsg(u->nick, s_StatServ, "Databases Reset! I hope you wanted to really do that!");
}

static void ss_JOIN(User *u, char *chan)
{

	strcpy(segv_location, "StatServ-ss_JOIN");


	if (UserLevel(u) < 190) {
		log("Access Denied (JOIN) to %s", u->nick);
		privmsg(u->nick, s_StatServ, "Access Denied.");
		notice(s_StatServ,"%s Requested JOIN, but is not a god!",u->nick);
		return;
	}
	if (!chan) {
		privmsg(u->nick, s_StatServ, "Syntax: /msg %s JOIN <chan>",s_StatServ);
		return;
	}
	globops(s_StatServ, "JOINING CHANNEL -\2(%s)\2- Thanks to %s!%s@%s)", chan, u->nick, u->username, u->hostname);
	privmsg(me.chan, s_StatServ, "%s Asked me to Join %s, So, I'm Leaving %s", u->nick, chan, me.chan);
	spart_cmd(s_StatServ, me.chan);
	log("%s!%s@%s Asked me to Join %s, I was on %s", u->nick, u->username, u->hostname, chan, me.chan);
	sjoin_cmd(s_StatServ, chan);
	schmode_cmd(me.name, chan, "+o", s_StatServ);
}


